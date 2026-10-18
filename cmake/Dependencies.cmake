include_guard(GLOBAL)

add_subdirectory(
    "${PROJECT_SOURCE_DIR}/3rd/spdlog"
    "${PROJECT_BINARY_DIR}/3rd/spdlog"
)

add_subdirectory(
    "${PROJECT_SOURCE_DIR}/3rd/ELFIO"
    "${PROJECT_BINARY_DIR}/3rd/ELFIO"
    EXCLUDE_FROM_ALL
)

add_subdirectory(
    "${PROJECT_SOURCE_DIR}/3rd/riscv-disassembler"
    "${PROJECT_BINARY_DIR}/3rd/riscv-disassembler"
    EXCLUDE_FROM_ALL
)
