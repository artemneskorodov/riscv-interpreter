cmake_minimum_required(VERSION 3.29)
project(riscv-interpreter)

#
# Generating riscv-opcodes encoding.out.h file
#
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

    DEPENDS
        "${RISCV_OPCODES_DIR}/extensions/rv_i"
        "${RISCV_OPCODES_DIR}/extensions/rv32_i"
        "${RISCV_OPCODES_DIR}/pyproject.toml"
        "${RISCV_OPCODES_DIR}/uv.lock"

    COMMENT
        "Generating files from riscv-opcodes"

    VERBATIM
)

#
# Building riscv-interpreter target
#
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/sources")
add_executable(riscv-interpreter)
target_sources(riscv-interpreter
    PRIVATE
        "${SOURCE_DIR}/main.cc"
        "${SOURCE_DIR}/instructions.cc"

    PRIVATE
        FILE_SET HEADERS
        BASE_DIRS
            "${SOURCE_DIR}"
            "${GENERATED_DIR}"
        FILES
            "${RISCV_ENCODING_HEADER}"
            "${SOURCE_DIR}/instructions.hh"
            "${SOURCE_DIR}/cpu.hh"
)
target_compile_features(riscv-interpreter PRIVATE cxx_std_20)
target_compile_options(riscv-interpreter PRIVATE -Werror -Wall -Wextra -Wpedantic)
