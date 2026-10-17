cmake_minimum_required(VERSION 3.29)
project(riscv-interpreter)

include(cmake/RiscvOpcodes.cmake)
include(cmake/Dependencies.cmake)
include(cmake/SoftFloat.cmake)

#
# Debug and diagnostics
#
set(RVI_DIAGNOSTICS_LEVEL "DEBUG" CACHE STRING "Compile-time diagnostics level")
set_property(
    CACHE RVI_DIAGNOSTICS_LEVEL
    PROPERTY STRINGS
        TRACE
        DEBUG
        INFO
        WARNING
        ERROR
        CRITICAL
        OFF
)
string(TOUPPER "${RVI_DIAGNOSTICS_LEVEL}" RVI_DIAGNOSTICS_LEVEL)

if(RVI_DIAGNOSTICS_LEVEL STREQUAL "TRACE")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_TRACE)
    set(RVI_DEBUG_ENABLED 1)
elseif(RVI_DIAGNOSTICS_LEVEL STREQUAL "DEBUG")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_DEBUG)
    set(RVI_DEBUG_ENABLED 1)
elseif(RVI_DIAGNOSTICS_LEVEL STREQUAL "INFO")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_INFO)
    set(RVI_DEBUG_ENABLED 0)
elseif(RVI_DIAGNOSTICS_LEVEL STREQUAL "WARNING")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_WARN)
    set(RVI_DEBUG_ENABLED 0)
elseif(RVI_DIAGNOSTICS_LEVEL STREQUAL "ERROR")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_ERROR)
    set(RVI_DEBUG_ENABLED 0)
elseif(RVI_DIAGNOSTICS_LEVEL STREQUAL "CRITICAL")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_CRITICAL)
    set(RVI_DEBUG_ENABLED 0)
elseif(RVI_DIAGNOSTICS_LEVEL STREQUAL "OFF")
    set(RVI_SPDLOG_LEVEL SPDLOG_LEVEL_OFF)
    set(RVI_DEBUG_ENABLED 0)
else()
    message(FATAL_ERROR "Unknown RVI_DIAGNOSTICS_LEVEL: " "${RVI_DIAGNOSTICS_LEVEL}")
endif()

message(STATUS "Building with diagnostics level: " "${RVI_DIAGNOSTICS_LEVEL}")

set(RVI_DEBUGGER_MODE "OFF" CACHE STRING "Enable debugging mode in compile time")
set_property(
    CACHE RVI_DEBUGGER_MODE
    PROPERTY STRINGS
        ON
)
string(TOUPPER "${RVI_DEBUGGER_MODE}" RVI_DEBUGGER_MODE)
if(RVI_DEBUGGER_MODE STREQUAL "ON")
    set(RVI_DEBUGGER_MODE_ENABLED 1)
    message(STATUS "Building with debugger mode enabled")
elseif(RVI_DEBUGGER_MODE STREQUAL "OFF")
    set(RVI_DEBUGGER_MODE_ENABLED 0)
    message(STATUS "Building with debugger mode disabled")
else()
    message(FATAL_ERROR "Unknown RVI_DEBUGGER_MODE: " "${RVI_DEBUGGER_MODE}")
endif()

#
# Building riscv-interpreter target
#
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/sources")
add_executable(riscv-interpreter)
target_sources(riscv-interpreter
    PRIVATE
        "${SOURCE_DIR}/insn/rv32i.cc"
        "${SOURCE_DIR}/insn/rv32m.cc"
        "${SOURCE_DIR}/insn/rv32f.cc"
        "${SOURCE_DIR}/insn/rv32zbb.cc"
        "${SOURCE_DIR}/interpreter/interpreter.cc"
        "${SOURCE_DIR}/main.cc"
        "${SOURCE_DIR}/machine/memory.cc"
        "${SOURCE_DIR}/runtime/elf_loader.cc"
        "${SOURCE_DIR}/runtime/process.cc"
        "${SOURCE_DIR}/runtime/syscalls.cc"
        "${SOURCE_DIR}/tools/debugger.cc"
        "${SOURCE_DIR}/tools/disassembler.cc"

    PRIVATE
        FILE_SET HEADERS
        BASE_DIRS
            "${SOURCE_DIR}"
            "${GENERATED_DIR}"
        FILES
            "${RISCV_ENCODING_HEADER}"
            "${SOURCE_DIR}/insn/dispatch.hh"
            "${SOURCE_DIR}/insn/helpers.hh"
            "${SOURCE_DIR}/isa/decoder.hh"
            "${SOURCE_DIR}/isa/instruction.hh"
            "${SOURCE_DIR}/isa/registers.hh"
            "${SOURCE_DIR}/machine/cpu.hh"
            "${SOURCE_DIR}/machine/memory.hh"
            "${SOURCE_DIR}/interpreter/interpreter.hh"
            "${SOURCE_DIR}/runtime/elf_loader.hh"
            "${SOURCE_DIR}/runtime/process.hh"
            "${SOURCE_DIR}/runtime/syscalls.hh"
            "${SOURCE_DIR}/tools/debugger.hh"
            "${SOURCE_DIR}/tools/disassembler.hh"
            "${SOURCE_DIR}/helpers/bits.hh"
            "${SOURCE_DIR}/helpers/diagnostics.hh"
            "${SOURCE_DIR}/helpers/error.hh"
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
    rvasm
    softfloat)
target_include_directories(
    riscv-interpreter
    PRIVATE
    "${PROJECT_SOURCE_DIR}/3rd/riscv-disassembler/src"
)
target_compile_definitions(riscv-interpreter PRIVATE
    SPDLOG_ACTIVE_LEVEL=${RVI_SPDLOG_LEVEL}
    RVI_DEBUG_ENABLED=${RVI_DEBUG_ENABLED}
    RVI_DEBUGGER_MODE_ENABLED=${RVI_DEBUGGER_MODE_ENABLED})
