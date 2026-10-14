cmake_minimum_required(VERSION 3.29)
project(riscv-interpreter)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

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
        rv_m

    DEPENDS
        "${RISCV_OPCODES_DIR}/extensions/rv_i"
        "${RISCV_OPCODES_DIR}/extensions/rv32_i"
        "${RISCV_OPCODES_DIR}/extensions/rv_m"
        "${RISCV_OPCODES_DIR}/pyproject.toml"
        "${RISCV_OPCODES_DIR}/uv.lock"

    COMMENT
        "Generating files from riscv-opcodes"

    VERBATIM
)

#
# Debug and diagnostics
#
add_subdirectory(3rd/spdlog)

set(RISCV_DIAGNOSTICS_LEVEL "DEBUG" CACHE STRING "Compile-time diagnostics level")
set_property(
    CACHE RISCV_DIAGNOSTICS_LEVEL
    PROPERTY STRINGS
        TRACE
        DEBUG
        INFO
        WARNING
        ERROR
        CRITICAL
        OFF
)
string(TOUPPER "${RISCV_DIAGNOSTICS_LEVEL}" RISCV_DIAGNOSTICS_LEVEL)

if(RISCV_DIAGNOSTICS_LEVEL STREQUAL "TRACE")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_TRACE)
    set(RVI_DEBUG_ENABLED 1)
elseif(RISCV_DIAGNOSTICS_LEVEL STREQUAL "DEBUG")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_DEBUG)
    set(RVI_DEBUG_ENABLED 1)
elseif(RISCV_DIAGNOSTICS_LEVEL STREQUAL "INFO")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_INFO)
    set(RVI_DEBUG_ENABLED 0)
elseif(RISCV_DIAGNOSTICS_LEVEL STREQUAL "WARNING")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_WARN)
    set(RVI_DEBUG_ENABLED 0)
elseif(RISCV_DIAGNOSTICS_LEVEL STREQUAL "ERROR")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_ERROR)
    set(RVI_DEBUG_ENABLED 0)
elseif(RISCV_DIAGNOSTICS_LEVEL STREQUAL "CRITICAL")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_CRITICAL)
    set(RVI_DEBUG_ENABLED 0)
elseif(RISCV_DIAGNOSTICS_LEVEL STREQUAL "OFF")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_OFF)
    set(RVI_DEBUG_ENABLED 0)
else()
    message(FATAL_ERROR "Unknown RISCV_DIAGNOSTICS_LEVEL: " "${RISCV_DIAGNOSTICS_LEVEL}")
endif()

message(STATUS "Building with diagnostics level: " "${RISCV_DIAGNOSTICS_LEVEL}")

set(RISCV_DEBUGGER_MODE "OFF" CACHE STRING "Enable debugging mode in compile time")
set_property(
    CACHE RISCV_DEBUGGER_MODE
    PROPERTY STRINGS
        ON
)
string(TOUPPER "${RISCV_DEBUGGER_MODE}" RISCV_DEBUGGER_MODE)
if(RISCV_DEBUGGER_MODE STREQUAL "ON")
    set(RVI_DEBUGGER_MODE_ENABLED 1)
    message(STATUS "Building with debugger mode enabled")
elseif(RISCV_DEBUGGER_MODE STREQUAL "OFF")
    set(RVI_DEBUGGER_MODE_ENABLED 0)
    message(STATUS "Building with debugger mode disabled")
else()
    message(FATAL_ERROR "Unknown RISCV_DEBUGGER_MODE: " "${RISCV_DEBUGGER_MODE}")
endif()

#
# Including ELFIO
#
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/3rd/ELFIO"
    "${PROJECT_BINARY_DIR}/3rd/ELFIO"
    EXCLUDE_FROM_ALL
)

#
# Including larkmjc/riscv-disassembler
#
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/3rd/riscv-disassembler"
    "${PROJECT_SOURCE_DIR}/3rd/riscv-disassembler"
    EXCLUDE_FROM_ALL
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
        "${SOURCE_DIR}/memory.cc"
        "${SOURCE_DIR}/elf_loader.cc"
        "${SOURCE_DIR}/abi.cc"
        "${SOURCE_DIR}/disasm.cc"
        "${SOURCE_DIR}/debugger.cc"
        "${SOURCE_DIR}/insn/rv32i.cc"
        "${SOURCE_DIR}/insn/rv32m.cc"

    PRIVATE
        FILE_SET HEADERS
        BASE_DIRS
            "${SOURCE_DIR}"
            "${GENERATED_DIR}"
        FILES
            "${RISCV_ENCODING_HEADER}"
            "${SOURCE_DIR}/instructions.hh"
            "${SOURCE_DIR}/bits.hh"
            "${SOURCE_DIR}/cpu_model.hh"
            "${SOURCE_DIR}/decoder.hh"
            "${SOURCE_DIR}/memory.hh"
            "${SOURCE_DIR}/debug.hh"
            "${SOURCE_DIR}/errors.hh"
            "${SOURCE_DIR}/abi.hh"
            "${SOURCE_DIR}/disasm.hh"
            "${SOURCE_DIR}/debugger.hh"
            "${SOURCE_DIR}/insn/execute.hh"
            "${SOURCE_DIR}/insn/helpers.hh"
)
target_compile_features(riscv-interpreter PRIVATE cxx_std_23)
if(RVI_DEBUG_ENABLED)
    target_compile_options(riscv-interpreter PRIVATE -Werror -Wall -Wextra -Wpedantic)
endif()
target_link_libraries(
    riscv-interpreter
    PRIVATE
    spdlog::spdlog
    elfio::elfio
    rvasm)
target_include_directories(
    riscv-interpreter
    PRIVATE
    "${PROJECT_SOURCE_DIR}/3rd/riscv-disassembler/src"
)
target_compile_definitions(riscv-interpreter PRIVATE
    SPDLOG_ACTIVE_LEVEL=${RVI_SPDLOG_LEVEL}
    RVI_DEBUG_ENABLED=${RVI_DEBUG_ENABLED}
    RVI_DEBUGGER_MODE_ENABLED=${RVI_DEBUGGER_MODE_ENABLED})
