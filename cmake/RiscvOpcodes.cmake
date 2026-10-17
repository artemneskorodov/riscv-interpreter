include_guard(GLOBAL)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_program(UV_EXECUTABLE uv REQUIRED)

set(RISCV_OPCODES_DIR "${PROJECT_SOURCE_DIR}/3rd/riscv-opcodes")
set(GENERATED_DIR "${PROJECT_BINARY_DIR}/generated")
set(RISCV_GENERATED_DIR "${GENERATED_DIR}/riscv-opcodes")
set(RISCV_ENCODING_HEADER "${RISCV_GENERATED_DIR}/encoding.out.h")

add_custom_command(
    OUTPUT
        "${RISCV_ENCODING_HEADER}"

    COMMAND
        "${CMAKE_COMMAND}"
        -E make_directory
        "${RISCV_GENERATED_DIR}"

    COMMAND
        "${CMAKE_COMMAND}"
        -E chdir
        "${RISCV_GENERATED_DIR}"
        "${UV_EXECUTABLE}"
        run
        --project "${RISCV_OPCODES_DIR}"
        --locked
        riscv_opcodes
        -c
        rv_i
        rv32_i
        rv_m
        rv_f
        rv_zbb
        rv32_zbb

    DEPENDS
        "${RISCV_OPCODES_DIR}/extensions/rv_i"
        "${RISCV_OPCODES_DIR}/extensions/rv32_i"
        "${RISCV_OPCODES_DIR}/extensions/rv_m"
        "${RISCV_OPCODES_DIR}/extensions/rv_f"
        "${RISCV_OPCODES_DIR}/extensions/rv_zbb"
        "${RISCV_OPCODES_DIR}/extensions/rv32_zbb"
        "${RISCV_OPCODES_DIR}/pyproject.toml"
        "${RISCV_OPCODES_DIR}/uv.lock"

    COMMENT "Generating files from riscv-opcodes"
    VERBATIM
)
