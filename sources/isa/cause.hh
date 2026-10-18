#ifndef RVI_ISA_CAUSE_HH__
#define RVI_ISA_CAUSE_HH__

#include <cstdint>
#include <string_view>

#include "helpers/diagnostics.hh"

#include "riscv-opcodes/encoding.out.h"

namespace rvi
{
namespace isa
{

enum class ExceptionCause : uint32_t
{
    MisalignedFetch        = CAUSE_MISALIGNED_FETCH,
    FetchAccess            = CAUSE_FETCH_ACCESS,
    IllegalInstruction     = CAUSE_ILLEGAL_INSTRUCTION,
    Breakpoint             = CAUSE_BREAKPOINT,
    MisalignedLoad         = CAUSE_MISALIGNED_LOAD,
    LoadAccess             = CAUSE_LOAD_ACCESS,
    MisalignedStore        = CAUSE_MISALIGNED_STORE,
    StoreAccess            = CAUSE_STORE_ACCESS,
    UserEcall              = CAUSE_USER_ECALL,
    SupervisorEcall        = CAUSE_SUPERVISOR_ECALL,
    VirtualSupervisorEcall = CAUSE_VIRTUAL_SUPERVISOR_ECALL,
    MachineEcall           = CAUSE_MACHINE_ECALL,
    FetchPageFault         = CAUSE_FETCH_PAGE_FAULT,
    LoadPageFault          = CAUSE_LOAD_PAGE_FAULT,
    StorePageFault         = CAUSE_STORE_PAGE_FAULT,
    DoubleTrap             = CAUSE_DOUBLE_TRAP,
    SoftwareCheckFault     = CAUSE_SOFTWARE_CHECK_FAULT,
    HardwareErrorFault     = CAUSE_HARDWARE_ERROR_FAULT,
    FetchGuestPageFault    = CAUSE_FETCH_GUEST_PAGE_FAULT,
    LoadGuestPageFault     = CAUSE_LOAD_GUEST_PAGE_FAULT,
    VirtualInstruction     = CAUSE_VIRTUAL_INSTRUCTION,
    StoreGuestPageFault    = CAUSE_STORE_GUEST_PAGE_FAULT,
};

struct Trap
{
    ExceptionCause cause;
    uint32_t value = 0;
};

inline std::string_view
causeToStr( ExceptionCause cause)
{
    switch ( static_cast<int>( cause) )
    {
        #define DECLARE_CAUSE( message_, value_) \
            case (value_): return (message_);

        #include "riscv-opcodes/encoding.out.h"

        #undef DECLARE_CAUSE

        default:
        {
            RVI_UNREACHABLE();
        }
    }
}


} // ! namespace isa
} // ! namespace rvi

#endif // ! RVI_ISA_CAUSE_HH__
