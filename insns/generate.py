from __future__ import annotations

import argparse
import ast
from dataclasses import dataclass
from pathlib import Path
import re
import sys

class InstructionDefinition:
    def __init__(self, name: str, statements: list[ast.Assign]):
        self.name = name
        self.statements = statements

INSTRUCTION_NAME_MATCH = re.compile(r"([a-z][a-z0-9_]*):")

def parse_definitions(text: str) -> list[InstructionDefinition]:
    definitions: list[InstructionDefinition] = []
    names: set[str] = set()
    current_statements: list[ast.Assign] = []

    original_lines = text.splitlines(keepends=True)
    line_number = 0

    while line_number < len(original_lines):
        # Removing command and spaces
        line = original_lines[line_number].split("#", 1)[0].strip()

        # Skipping empty lines
        if not line:
            line_number += 1
            continue

        # Checking that name matches instruction name line pattern
        match = INSTRUCTION_NAME_MATCH.fullmatch(line)
        if not match:
            raise RuntimeError(f"Unexpected instruction declaration: {line}")
        current_name = match.groups(0)[0]
        if current_name in names:
            raise RuntimeError(f"Double definition for {current_name}")
        names.add(current_name)

        line_number += 1

        current_statements: list[ast.Assign] = []
        while line_number < len(original_lines):
            # Removing command and spaces
            statement_line = original_lines[line_number].split("#", 1)[0].strip()

            # Skipping empty lines
            if not statement_line:
                line_number += 1
                continue

            if INSTRUCTION_NAME_MATCH.fullmatch(statement_line):
                break

            statement = ast.parse(statement_line, mode="exec")
            if len(statement.body) != 1:
                raise RuntimeError(f"More than one statement on line is unexpected: \"{statement_line}\"")
            if not isinstance(statement.body[0], ast.Assign):
                raise RuntimeError(f"Expected assignment to be top level node: \"{statement_line}\", "
                                   f"got: {type(statement.body[0])}")
            current_statements.append(statement.body[0])
            line_number += 1
        definitions.append(InstructionDefinition(current_name, current_statements))

    return definitions

class CppEmitter:
    _BINARY_OPERATORS = {
        ast.Add: "+",
        ast.Sub: "-",
        ast.Mult: "*",
        ast.BitAnd: "&",
        ast.BitOr: "|",
        ast.BitXor: "^",
        ast.LShift: "<<",
        ast.RShift: ">>",
    }
    _UNARY_OPERATORS = {ast.Invert: "~", ast.UAdd: "+", ast.USub: "-"}
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
    _MEMORIES = {"mem8": "uint8_t", "mem16": "uint16_t", "mem32": "uint32_t"}

    def __init__(self):
        self.temporary_index = 0

    def emit_expression(self, node: ast.expr, prelude: list[str]) -> str:
        if isinstance(node, ast.Constant) and type(node.value) is int:
            if node.value < 0:
                return f"uint32_t( {node.value})"
            else:
                return f"uint32_t( {node.value}u)"

        if isinstance(node, ast.Name):
            if node.id in self._FIELDS:
                return self._FIELDS[node.id]
            elif node.id in self._IMMEDIATES:
                kind = self._IMMEDIATES[node.id]
                return f"instruction.imm<isa::ImmType::{kind}, false>()"
            elif node.id == "pc":
                return "cpu_model.getPC()"
            else:
                raise RuntimeError(f"Unknown value {node.id}")

        if isinstance(node, ast.Subscript):
            if not isinstance(node.value, ast.Name):
                raise RuntimeError("Subscript base must be x, mem8, mem16 or mem32")
            index = self.emit_expression(node.slice, prelude)
            if node.value.id == "x":
                return f"cpu_model.getX({index})"
            value_type = self._MEMORIES.get(node.value.id)
            if value_type is None:
                raise RuntimeError(f"Unknown subscript base {node.value.id}")
            tmp_name = f"tmp_{self.temporary_index}"
            self.temporary_index += 1
            prelude.append(f"{value_type} {tmp_name} = 0;")
            prelude.append(f"cpu_model.load( {index}, sizeof( {tmp_name}), &{tmp_name})")
            return temporary

        if isinstance(node, ast.BinOp):
            sym = self._BINARY_OPERATORS.get(type(node.op))
            if sym is None:
                raise RuntimeError("Unsupported binary operation")
            left = self.emit_expression(node.left, prelude)
            right = self.emit_expression(node.right, prelude)
            return f"({left} {sym} {right})"

        if isinstance(node, ast.UnaryOp):
            sym = self._UNARY_OPERATORS.get(type(node.op))
            if sym is None:
                raise RuntimeError("Unsupported unary operation")
            operand = self.emit_expression(node.operand, prelude)
            return f"({sym}{operand})"

        if isinstance(node, ast.Compare):
            if len(node.ops) != 1 or len(node.comparators) != 1:
                raise self.fail(line, "chained comparisons are not supported")
            symbol = self._COMPARE_OPERATORS.get(type(node.ops[0]))
            if symbol is None:
                raise self.fail(line, "unsupported comparison operator")
            left = self.emit_expression(node.left, line, prelude)
            right = self.emit_expression(node.comparators[0], line, prelude)
            return f"({left} {symbol} {right})"

        if isinstance(node, ast.Call):
            if not isinstance(node.func, ast.Name) or len(node.args) != 1 or node.keywords:
                raise RuntimeError("Functions take only one argument")
            function = node.func.id
            argument = node.args[0]

            if function == "sx":
                if not isinstance(argument, ast.Name) or argument.id not in self._IMMEDIATES:
                    raise RuntimeError("sx() accept an immediate imm[TYPE]")
                kind = self._IMMEDIATES[argument.id]
                return f"instruction.imm<isa::ImmType::{kind}, true>()"

            else:
                raise RuntimeError("Only sx (sign extend) function is supported")

        if isinstance(node, ast.IfExp):
            condition = self.emit_expression(node.text, prelude)
            true_path = self.emit_expression(node.body, prelude)
            false_path = self.emit_expression(node.orelse, prelude)
            return f"{condition} ? {true_path} : {false_path}"

        raise RuntimeError("Unsupported expression")

    def emit_statement(self, statement: ast.Assign) -> list[str]:
        prelude: list[str] = []
        value = self.emit_expression(statement.value, prelude)
        target = statement.targets[0]

        if isinstance(target, ast.Name) and target.id == "pc":
            return prelude + [f"cpu_model.setPC( {value});"]
        elif isinstance(target, ast.Subscript) and isinstance(target.value, ast.Name):
            index = self.emit_expression(target.slice, prelude)
            if target.value.id == "x":
                return prelude + [f"cpu_model.setX( {index}, {value});"]
            value_type = self._MEMORIES.get(target.value.id)
            if not value_type:
                raise RuntimeError(f"TODO error")
            tmp_name = f"tmp_{self.temporary_index}"
            self.temporary_index += 1
            prelude.append(f"{value_type} {tmp_name} = static_cast<{value_type}>( {value});")
            prelude.append(f"cpu_model.store( {index}, sizeof( {tmp_name}), &{tmp_name})")
            return prelude
        else:
            raise RuntimeError(f"TODO error")

    def emit_definition(self, definition: InstructionDefinition) -> list[str]:
        second_argument_line = " " * (10 + 15 + len(definition.name) + 2) + "isa::InstructionBits bits)"
        lines = [
            "inline void",
           f"runInstruction_{definition.name}_generated( CPUModel& cpu_model,",
            second_argument_line,
            "{",
            "    isa::Instruction instruction( bits);",
        ]

        for statement in definition.statements:
            emitted_lines = self.emit_statement(statement)
            for line in emitted_lines:
                lines.append(f"    {line}")
        lines.append("}")

        return lines

def generate(definitions: list[InstructionDefinition], source_name: str) -> str:
    implemented = [definition for definition in definitions if definition.statements]
    emitter = CppEmitter()
    lines = [
        "//",
        f"// This file is generated using {__file__} from {source_name}.",
        "//",
        "#ifndef RISCV_INTERPRETER_RV32I_BEHAVIOR_GENERATED_HH__",
        "#define RISCV_INTERPRETER_RV32I_BEHAVIOR_GENERATED_HH__",
        "",
        "#include <bit>",
        "#include <cstdint>",
        "",
        "#include \"cpu_model.hh\"",
        "#include \"decoder.hh\"",
        "",
        "namespace riscv",
        "{",
        "namespace insn",
        "{",
        "",
    ]
    for definition in implemented:
        lines.extend(emitter.emit_definition(definition))
        lines.append("")

    lines.extend(
        [
            "inline bool",
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
               f"            runInstruction_{definition.name}_generated( cpu_model, bits);",
                "            return true;",
                "        }",
            ]
        )
    lines.extend(
        [
            "        default:",
            "        {",
            "            return false;",
            "        }",
            "    }",
            "}",
            "",
            "} // ! namespace insn",
            "} // ! namespace riscv",
            "",
            "#endif // ! RISCV_INTERPRETER_RV32I_BEHAVIOR_GENERATED_HH__",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    arguments = parser.parse_args()
    try:
        definitions = parse_definitions(arguments.input.read_text(encoding="utf-8"))
        generated = generate(definitions, arguments.input.name)
        arguments.output.parent.mkdir(parents=True, exist_ok=True)
        arguments.output.write_text(generated, encoding="utf-8")
    except (OSError, GeneratorError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
