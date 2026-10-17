include_guard(GLOBAL)

set(SOFTFLOAT_DIR "${PROJECT_SOURCE_DIR}/3rd/berkeley-softfloat-3")
set(SOFTFLOAT_SOURCE_DIR "${SOFTFLOAT_DIR}/source")

if(NOT EXISTS "${SOFTFLOAT_SOURCE_DIR}/include/softfloat.h")
    message(FATAL_ERROR
        "Berkeley SoftFloat was not found at ${SOFTFLOAT_DIR}. "
        "Initialize the berkeley-softfloat-3 submodule before configuring."
    )
endif()

# Build only the binary32 operations required by RV32F.  The upstream build
# compiles every supported floating-point format, most of which the interpreter
# does not use.
add_library(softfloat STATIC
    "${SOFTFLOAT_SOURCE_DIR}/softfloat_state.c"
    "${SOFTFLOAT_SOURCE_DIR}/i32_to_f32.c"
    "${SOFTFLOAT_SOURCE_DIR}/ui32_to_f32.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_to_i32.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_to_ui32.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_add.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_sub.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_mul.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_mulAdd.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_div.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_sqrt.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_eq.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_le.c"
    "${SOFTFLOAT_SOURCE_DIR}/f32_lt.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_roundToUI32.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_roundToI32.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_normSubnormalF32Sig.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_roundPackToF32.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_normRoundPackToF32.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_addMagsF32.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_subMagsF32.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_mulAddF32.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_shiftRightJam32.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_shiftRightJam64.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_shortShiftRightJam64.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_countLeadingZeros8.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_countLeadingZeros32.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_countLeadingZeros64.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_approxRecip_1Ks.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_approxRecip32_1.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_approxRecipSqrt_1Ks.c"
    "${SOFTFLOAT_SOURCE_DIR}/s_approxRecipSqrt32_1.c"
    "${SOFTFLOAT_SOURCE_DIR}/RISCV/softfloat_raiseFlags.c"
    "${SOFTFLOAT_SOURCE_DIR}/RISCV/s_propagateNaNF32UI.c"
)

target_include_directories(softfloat
    PUBLIC
        "${SOFTFLOAT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/softfloat"
        "${SOFTFLOAT_SOURCE_DIR}/RISCV"
)

target_compile_definitions(softfloat PRIVATE
    SOFTFLOAT_FAST_INT64
    SOFTFLOAT_ROUND_ODD
    INLINE_LEVEL=5
)
