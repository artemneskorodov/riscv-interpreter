#ifndef RISCV_INTERPRETER_SOFTFLOAT_PLATFORM_H__
#define RISCV_INTERPRETER_SOFTFLOAT_PLATFORM_H__

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define LITTLEENDIAN 1
#elif defined(_WIN32)
#define LITTLEENDIAN 1
#endif

#define INLINE static inline

// CPUModel owns the architectural state. SoftFloat state is reset before
// each operation, so process-local globals are sufficient here.
#define THREAD_LOCAL
#define SOFTFLOAT_BUILTIN_CLZ 1
#define SOFTFLOAT_INTRINSIC_INT128 1

#include "opts-GCC.h"

#endif // ! RISCV_INTERPRETER_SOFTFLOAT_PLATFORM_H__
