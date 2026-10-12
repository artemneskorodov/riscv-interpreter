# Instruction behavior DSL

Instruction definitions DSL use python syntax.
Every statement in definition body must be assignment or if statements.

## Values and assignment targets

- `x[index]` reads or writes a programmer register. The index can use `rd`,
  `rs1`, `rs2`, an integer, or an expression.
- `pc` reads or writes the program counter.
- `mem8[address]`, `mem16[address]`, and `mem32[address]` read or write memory.
  Generated code checks the memory operation and stops the instruction on an
  error.
- `immI`, `immS`, `immB`, `immU`, and `immJ` are unsigned instruction
  immediates. `sx(immX)` reads the same immediate with sign extension.
- Assigning to any other name creates a 32-bit local.

## Expressions

The generator supports integer arithmetic, shifts, bitwise operators,
comparisons, `and`, `or` and `not`.

For clarity and correct short-circuit behavior, memory reads inside `and`,
`or`, or conditional expressions are rejected. Use an `if` statement instead.

Generator errors include the source filename, line, and column so malformed or
unsupported definitions can be fixed at the declaration site.
