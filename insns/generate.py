import argparse
import ast
from pathlib import Path
import re
import sys
import textwrap

#
# Class to store instruction definitions
#
class InstructionDefinition:
    def __init__(self, name: str, statements: list[ast.stmt], source_name: str):
        self.name = name
        self.statements = statements
        self.source_name = source_name

#
# Compare string and instruction definition header
#
_INSTRUCTION_HEADER = re.compile(
    r"^(?P<name>[a-z][a-z0-9_]*):(?:\s*#.*)?(?:\r?\n)?$"
)

#
# Initial source parser.
# Returns list of instruction instruction definitions.
#
def parse_definitions(text: str, source_name: str) -> list[InstructionDefinition]:
    lines = text.splitlines(keepends=True)
    headers: list[tuple[str, int]] = []
    used_names: set[str] = set()

    for i in range(len(lines)):
        line = lines[i].split("#", 1)[0]
        if not line:
            continue
        match = _INSTRUCTION_HEADER.fullmatch(line)
        if not match:
            continue
        name = match.group("name")
        if name in used_names:
            raise GeneratorError(f"{source_name}:{i}: {name} is already used")
        headers.append((name, i))
        used_names.add(name)

    if not headers:
        raise RuntimeError("No instruction definitions found")

    definitions: list[InstructionDefinition] = []
    for i, (name, line) in enumerate(headers):
        if i + 1 < len(headers):
            _, next_line = headers[i + 1]
        else:
            next_line = len(lines)

        definition_lines = lines[line + 1 : next_line]
        definition_lines = "".join(definition_lines)
        definition_lines = textwrap.dedent(definition_lines)

        try:
            module = ast.parse(definition_lines, filename=source_name, mode="exec")
        except SyntaxError as error:
            error.lineno += line + 1
            raise error

        statements = module.body
        for stmt in statements:
            ast.increment_lineno(stmt, line + 1)
            
        definitions.append(InstructionDefinition(name, statements, source_name))

    return definitions

#
# Exception, raised by CppEmitter
#
class GeneratorError(Exception):
    pass

#
# Emitter of C++ code. Generates one instruction definition function.
#
class CppEmitter:
    _BINARY_OPERATORS = {
        ast.Add: "+",
        ast.Sub: "-",
        ast.Mult: "*",
        ast.FloorDiv: "/",
        ast.Mod: "%",
        ast.BitAnd: "&",
        ast.BitOr: "|",
        ast.BitXor: "^",
        ast.LShift: "<<",
        ast.RShift: ">>",
    }
    _UNARY_OPERATORS = {
        ast.Invert: "~",
        ast.UAdd: "+",
        ast.USub: "-",
        ast.Not: "!",
    }
    _COMPARE_OPERATORS = {
        ast.Eq: "==",
        ast.NotEq: "!=",
        ast.Lt: "<",
        ast.LtE: "<=",
        ast.Gt: ">",
        ast.GtE: ">=",
    }
    _FIELDS = {
        "rd": "instruction.rd()",
        "rs1": "instruction.rs1()",
        "rs2": "instruction.rs2()",
    }
    _IMMEDIATES = {
        "immI": "I",
        "immS": "S",
        "immB": "B",
        "immU": "U",
        "immJ": "J",
    }
    _MEMORIES = {
        "mem8": "uint8_t",
        "mem16": "uint16_t",
        "mem32": "uint32_t",
    }
    _HELPERS = {
        "mulh",
        "mulhsu",
        "mulhu",
        "mul",
    }

    def __init__(self):
        self.source_name = "<unknown>"
        self.temporary_index = 0
        self.local_names: set[str] = set()

    def _fail(self, node: ast.AST, message: str) -> RuntimeError:
        location = f"{self.source_name}:{node.lineno}"
        return GeneratorError(f"{location}: {message}")

    def _create_tmp(self) -> str:
        name = f"tmp_{self.temporary_index}"
        self.temporary_index += 1
        self.local_names.add(name)
        return name

    def _emit_expression_constant(self, node: ast.Constant) -> str:
        if node.value < 0:
            return f"uint32_t( {node.value})"
        else:
            return f"uint32_t( {node.value}u)"

    def _emit_expression_name(self, node: ast.Name) -> str:
        if node.id in self._FIELDS:
            return self._FIELDS[node.id]
        elif node.id in self._IMMEDIATES:
            kind = self._IMMEDIATES[node.id]
            return f"instruction.imm<isa::ImmType::{kind}, false>()"
        elif node.id == "pc":
            return "cpu_model.getPC()"
        elif node.id in self.local_names:
            return node.id
        raise self._fail(node, f"unknown value '{node.id}'")

    def _emit_expression_subscript(self, node: ast.Subscript, prelude: list[str]) -> str:
        if not isinstance(node.value, ast.Name):
            raise self._fail(node, "subscript base must be x, mem8, mem16 or mem32")
        index = self._emit_expression(node.slice, prelude)
        if node.value.id == "x":
            return f"cpu_model.getX( {index})"
        else: # Memory access
            value_type = self._MEMORIES.get(node.value.id)
            if value_type is None:
                raise self._fail(node, f"unknown subscript base '{node.value.id}'")
            result = self._create_tmp()
            prelude.extend([
                f"Expected<{value_type}> {result} = cpu_model.memory().load<{value_type}>( {index});",
                f"if ( !{result}.ok() ) return {result}.error();"
            ])
            return f"{result}.value()"

    def _emit_expression_bin_op(self, node: ast.BinOp, prelude: list[str]) -> str:
        symbol = self._BINARY_OPERATORS.get(type(node.op))
        if symbol is None:
            raise self._fail(node, f"unsupported binary operation: {type(node.op).__name__}")
        left = self._emit_expression(node.left, prelude)
        right = self._emit_expression(node.right, prelude)
        result = self._create_tmp()
        prelude.append(f"uint32_t {result} = {left} {symbol} {right};")
        return result

    def _emit_expression_bool_op(self, node: ast.BoolOp, prelude: list[str]) -> str:
        if isinstance(node.op, ast.And):
            symbol = "&&"
        elif isinstance(node.op, ast.Or):
            symbol = "||"
        else:
            raise self._fail(node, f"unexpected boolean operation: {type(node.op).__name__}")
        operands: list[str] = []
        for value in node.values:
            tmp_name = self._create_tmp()
            prelude.append(f"bool {tmp_name} = ({self._emit_expression(value, prelude)});")
            operands.append(f"{tmp_name}")
        return f"({f' {symbol} '.join(operands)})"

    def _emit_expression_unary_op(self, node: ast.UnaryOp, prelude: list[str]) -> str:
        symbol = self._UNARY_OPERATORS.get(type(node.op))
        if symbol is None:
            raise self._fail(node, f"unsupported unary operator '{type(node.op).__name__}'")
        operand = self._emit_expression(node.operand, prelude)
        return f"({symbol}{operand})"

    def _emit_expression_compare(self, node: ast.Compare, prelude: list[str]) -> str:
        if len(node.ops) != 1 or len(node.comparators) != 1:
            raise self._fail(node, "chained comparisons are not supported")
        symbol = self._COMPARE_OPERATORS.get(type(node.ops[0]))
        if symbol is None:
            raise self._fail(node, f"unsupported comparison operator: '{type(node.ops[0]).__name__}'")
        left = self._emit_expression(node.left, prelude)
        right = self._emit_expression(node.comparators[0], prelude)
        return f"({left} {symbol} {right})"

    def _emit_expression_call(self, node: ast.Call, prelude: list[str]) -> str:
        if isinstance(node.func, ast.Name) and node.func.id == "sx":
            if len(node.args) >= 1 and isinstance(node.args[0], ast.Name) and node.args[0].id in self._IMMEDIATES:
                if len(node.args) != 1:
                    raise self._fail(node, "Expected sx() called for immediate argument of "
                                           "instruction to be called with single argument")
                kind = self._IMMEDIATES[node.args[0].id]
                return f"instruction.imm<isa::ImmType::{kind}, true>()"
            else:
                if len(node.args) != 2 or not isinstance(node.args[1], ast.Constant):
                    raise self._fail(node, "Expected sx() called for any expression to have 2 arguments: "
                                           "expression to be sign extended and width of initial value")
                expression = self._emit_expression(node.args[0], prelude)
                width = node.args[1].value
                if width > 32:
                    raise self._fail(node, f"Width for sign extent must be less than 32: have {width}")
                return f"bits::signExtend<uint32_t>( {expression}, {width})"
        elif isinstance(node.func, ast.Name) and node.func.id == "signed":
            if len(node.args) != 1:
                raise self._fail(node, "signed() function expects only one argument")
            argument = self._emit_expression(node.args[0], prelude)
            return f"static_cast<int32_t>( {argument})"
        elif isinstance(node.func, ast.Name) and node.func.id in self._HELPERS:
            if len(node.args) != 2:
                raise self._fail(node, "helpers functions expect 2 arguments")
            lhs = self._emit_expression(node.args[0], prelude)
            rhs = self._emit_expression(node.args[1], prelude)
            return f"{node.func.id}({lhs}, {rhs})"
        else:
            raise self._fail(node, "expected only sx() or signed() functions to be called")

    def _emit_expression(self, node: ast.expr, prelude: list[str]) -> str:
        if isinstance(node, ast.Constant) and type(node.value) is int:
            return self._emit_expression_constant(node)
        elif isinstance(node, ast.Name):
            return self._emit_expression_name(node)
        elif isinstance(node, ast.Subscript):
            return self._emit_expression_subscript(node, prelude)
        elif isinstance(node, ast.BinOp):
            return self._emit_expression_bin_op(node, prelude)
        elif isinstance(node, ast.BoolOp):
            return self._emit_expression_bool_op(node, prelude)
        elif isinstance(node, ast.UnaryOp):
            return self._emit_expression_unary_op(node, prelude)
        elif isinstance(node, ast.Compare):
            return self._emit_expression_compare(node, prelude)
        elif isinstance(node, ast.Call):
            return self._emit_expression_call(node, prelude)
        else:
            raise self._fail(node, f"unsupported expression '{type(node).__name__}'")

    def _emit_assignment(self, statement: ast.Assign) -> list[str]:
        if len(statement.targets) != 1:
            raise self._fail(statement, "chained assignments are not supported")
        prelude: list[str] = []
        value = self._emit_expression(statement.value, prelude)
        target = statement.targets[0]

        if isinstance(target, ast.Name):
            if target.id == "pc":
                prelude.append(f"cpu_model.setPC( {value});")
            elif target.id in self.local_names:
                prelude.append(f"{target.id} = {value};")
            else:
                raise self._fail(target, f"cannot assign to name '{target.id}'")
            return prelude
        elif isinstance(target, ast.Subscript):
            if not isinstance(target.value, ast.Name):
                raise self._fail(target, "Only x, pc, mem8, mem16 or mem32 must be a targets of assignment")
            index = self._emit_expression(target.slice, prelude)
            if target.value.id == "x":
                return prelude + [f"cpu_model.setX( {index}, {value});"]
            # Only memory operations
            value_type = self._MEMORIES.get(target.value.id)
            if value_type is None:
                raise self._fail(target, f"unknown assignment target: '{target.value.id}'")
            result = self._create_tmp()
            prelude.extend([
                f"Expected<void> {result} = cpu_model.memory().store<{value_type}>( {index}, static_cast<{value_type}>({value}));",
                f"if ( !{result}.ok() ) return {result}.error();",
            ])
            return prelude
        else:
            raise self._fail(statement, "target must be pc, x, memN or local")

    def _add_tabs(self, lines: list[str]) -> list[str]:
        return [f"    {line}" if line else line for line in lines]

    def _emit_single_statement(self, stmt: ast.stmt) -> list[str]:
        if isinstance(stmt, ast.Assign):
            return self._emit_assignment(stmt)
        elif isinstance(stmt, ast.If):
            prelude: list[str] = []
            condition = self._emit_expression(stmt.test, prelude)
            body = self._emit_statements(stmt.body)
            lines = prelude + [f"if ( {condition} )", "{"]
            lines.extend(self._add_tabs(body))
            if not stmt.orelse:
                lines.append("}")
            else:
                lines.extend(["} else", "{"])
                lines.extend(self._add_tabs(self._emit_statements(stmt.orelse)))
                lines.append("}")
            return lines
        elif isinstance(stmt, ast.Raise):
            if not all([
                    isinstance(stmt.exc, ast.Name),
                    (stmt.exc.id == "Breakpoint" or stmt.exc.id == "EnvironmentCall")
                ]):
                raise self._fail(stmt, "raise() function expects only one argument: "
                                       "type of exception: Breakpoint or EnvironmentCall")
            return [f"return StepResult::{stmt.exc.id};"]
        else:
            raise self._fail(stmt, f"unsupported statement '{type(stmt).__name__}', expected assign or if")


    def _emit_statements(self, statements: list[ast.stmt]) -> list[str]:
        lines: list[str] = []
        for stmt in statements:
            lines.extend(self._emit_single_statement(stmt))
        return lines

    def _collect_locals(self, stmts: list[ast.stmt]) -> set[str]:
        _RESERVED = (set(self._FIELDS) | set(self._IMMEDIATES) | set(self._MEMORIES) | {"pc", "x"})
        tree = ast.Module(body=stmts, type_ignores=[])
        local_names: set[str] = set()
        for node in ast.walk(tree):
            if not isinstance(node, ast.Assign):
                continue
            for target in node.targets:
                if not isinstance(target, ast.Name):
                    continue
                name = target.id
                if name == "pc":
                    continue
                if name in _RESERVED:
                    raise self._fail(target, f"cannot use reserved name '{name}' as a variable")
                if re.fullmatch(r"tmp_\d+", name):
                    raise self._fail(target, f"local name {name} is reserved for generator tmp's")
                local_names.add(name)
        return local_names

    def emit_definition(self, definition: InstructionDefinition) -> list[str]:
        self.source_name = definition.source_name
        self.local_names = self._collect_locals(definition.statements)
        self.temporary_index = 0

        lines = [
                "inline Expected<StepResult>",
                f"runInstruction_{definition.name}_generated( CPUModel& cpu_model,",
                " " * (27 + len(definition.name)) + "isa::InstructionBits bits)",
                "{",
                "    [[maybe_unused]] isa::Instruction instruction( bits);",
            ]

        for local in self.local_names:
            lines.append(f"    uint32_t {local} = 0;")
        if self.local_names:
            lines.append("")

        lines.extend(self._add_tabs(self._emit_statements(definition.statements)))
        lines.extend([
            "    return StepResult::Continue;",
            "}"
        ])
        return lines

#
# Generates text for C++ definition of arguments
#
def generate(
    definitions: list[InstructionDefinition],
    source_names: list[str],
    generator_name: str = "insns/generate.py",
) -> str:
    implemented = [definition for definition in definitions if definition.statements]
    emitter = CppEmitter()
    sources = ", ".join(source_names)
    lines = [
        "//",
        f"// This file is generated by {generator_name} from {sources}.",
        "//",
        "#ifndef RISCV_INTERPRETER_BEHAVIOR_GENERATED_HH__",
        "#define RISCV_INTERPRETER_BEHAVIOR_GENERATED_HH__",
        "",
        "#include <cstdint>",
        "",
        '#include "cpu_model.hh"',
        '#include "decoder.hh"',
        '#include "bits.hh"',
        '#include "insns_helpers.hh"',
        "",
        "namespace riscv",
        "{",
        "namespace insn",
        "{",
        "",
        "enum class StepResult",
        "{",
        "    Continue,",
        "    Breakpoint,",
        "    EnvironmentCall,",
        "};"
        ""
    ]
    for definition in implemented:
        lines.extend(emitter.emit_definition(definition))
        lines.append("")

    lines.extend(
        [
            "inline Expected<StepResult>",
            "runGeneratedInstruction( CPUModel& cpu_model,",
            "                         isa::Operation operation,",
            "                         isa::InstructionBits bits)",
            "{",
            "    switch ( operation )",
            "    {",
        ]
    )
    for definition in implemented:
        lines.extend(
            [
                f"        case isa::Operation::instr_{definition.name}:",
                "        {",
                f"            return runInstruction_{definition.name}_generated( cpu_model, bits);",
                "        }",
            ]
        )
    lines.extend(
        [
            "        default:",
            "        {",
            "            return Error( ErrorCode::UnsupportedInstruction,",
            "                         std::format( \"Unsupported instruction: 0x{:x}\",",
            "                                      static_cast<int>( operation)));",
            "        }",
            "    }",
            "}",
            "",
            "} // ! namespace insn",
            "} // ! namespace riscv",
            "",
            "#endif // ! RISCV_INTERPRETER_BEHAVIOR_GENERATED_HH__",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", type=Path, nargs="+", help="instruction definition files")
    parser.add_argument("-o", "--output", type=Path, required=True,
                        help="generated C++ header")
    arguments = parser.parse_args()

    try:
        definitions: list[InstructionDefinition] = []
        definition_sources: dict[str, str] = {}
        source_names: list[str] = []

        for input_path in arguments.inputs:
            source_name = str(input_path)
            source_names.append(source_name)
            parsed = parse_definitions(input_path.read_text(encoding="utf-8"), source_name)

            for definition in parsed:
                previous_source = definition_sources.get(definition.name)
                if previous_source is not None:
                    raise GeneratorError(
                        f"{source_name}: instruction '{definition.name}' "
                        f"is already defined in {previous_source}"
                    )
                definition_sources[definition.name] = source_name
                definitions.append(definition)

        generated = generate(definitions, source_names)
        arguments.output.parent.mkdir(parents=True, exist_ok=True)
        arguments.output.write_text(generated, encoding="utf-8")
    except (OSError, GeneratorError, SyntaxError) as error:
        print(f"Error while generating instructions definitions: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
