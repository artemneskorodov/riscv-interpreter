# Instruction behavior DSL

## Run instructions definition generator
```sh
python3 insns/generate.py insns/rv32i.insn output.hh
```

## Instruction behavior is declared with pattern
```text
[name]:
    [statement_1]
    [statement_2]
    ...
    [statement_n]
```

## Statements must be assignment operations
```text
[target] = [expression]
```
Expressions are default python ast module supported operations. Supported generator operations are listed in `generate.py`

## Values and targets
- `x[index]` - access to programmer register. Index can be `rd`, `rs1`, `rs2`, integer or expression.
- `pc` - access to program counter
- `(mem8, mem16, mem32)[address]` - access to memory. The `mem8`, `mem16` or `mem32` defines the access width.
- `immI`, `immS`, `immB`, `immU` and `immJ` select an instruction immediate.
- `sx(immX)` - sign extent of underlying value. The immediate width is defined by `X-Type`
