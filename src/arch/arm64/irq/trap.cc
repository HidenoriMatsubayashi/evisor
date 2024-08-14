#include "arch/arm64/irq/trap.h"

#include "common/logger.h"
#include "common/macro.h"
#include "kernel/sched/sched.h"
#include "mm/kmm_trap.h"

namespace {

/// Exception Class. Indicates the reason for the exception that this register
/// holds information about. See:
/// https://developer.arm.com/documentation/ddi0601/2022-03/AArch64-Registers/ESR-EL2--Exception-Syndrome-Register--EL2-
const char* kExceptionClass[] = {
    "Unknown reason.",
    "Trapped WF* instruction execution.",
    "Unknown reason.",
    "Trapped MCR or MRC access with (coproc==0b1111) ...",
    "Trapped MCRR or MRRC access with (coproc==0b1111) ...",
    "Trapped MCR or MRC access with (coproc==0b1110).",
    "Trapped LDC or STC access.",
    "Access to SME, SVE, Advanced SIMD or floating-point functionality ...",
    "Trapped VMRS access, from ID group trap, that is not reported ...",
    "Trapped use of a Pointer authentication instruction because ...",
    "An exception from an LD64B or ST64B* instruction.",
    "Unknown reason.",
    "Trapped MRRC access with (coproc==0b1110).",
    "Branch Target Exception.",
    "Illegal Execution state.",
    "Unknown reason.",
    "Unknown reason.",
    "SVC instruction execution in AArch32 state.",
    "HVC instruction execution in AArch32 state.",
    "SMC instruction execution in AArch32 state.",
    "Unknown reason.",
    "SVC instruction execution in AArch64 state.",
    "HVC instruction execution in AArch64 state.",
    "SMC instruction execution in AArch64 state.",
    "Trapped MSR, MRS or System instruction execution in AArch64 state, ...",
    "Access to SVE functionality trapped as a result of ...",
    "Trapped ERET, ERETAA, or ERETAB instruction execution.",
    "Exception from an access to a TSTART instruction at EL0 ...",
    "Exception from a Pointer Authentication instruction authentication ...",
    "Access to SME functionality trapped as a result of ...",
    "Exception from a Granule Protection Check",
    "Unknown reason.",
    "Instruction Abort from a lower Exception level.",
    "Instruction Abort taken without a change in Exception level.",
    "PC alignment fault exception.",
    "Unknown reason.",
    "Data Abort from a lower Exception level, ...",
    "Data Abort exception without a change in Exception level, or Data ...",
    "SP alignment fault exception.",
    "Memory Operation Exception.",
    "Trapped floating-point exception taken from AArch32 state.",
    "Unknown reason.",
    "Unknown reason.",
    "Unknown reason.",
    "Trapped floating-point exception taken from AArch64 state.",
    "Unknown reason.",
    "Unknown reason.",
    "SError interrupt.",
    "Breakpoint exception from a lower Exception level.",
    "Breakpoint exception taken without a change in Exception level.",
    "Software Step exception from a lower Exception level.",
    "Software Step exception taken without a change in Exception level.",
    "Watchpoint from a lower Exception level, ...",
    "Watchpoint exceptions without a change in Exception level, ...",
    "Unknown reason.",
    "Unknown reason.",
    "BKPT instruction execution in AArch32 state.",
    "Unknown reason.",
    "Vector Catch exception from AArch32 state.",
    "Unknown reason.",
    "BRK instruction execution in AArch64 state.",
};

constexpr uint8_t kEsrEl2EcShift = 26;
constexpr uint8_t kEsrEl2EcTrapWfx = 0b000001;
constexpr uint8_t kEsrEl2EcTrapFpReg = 0b000111;
constexpr uint8_t kEsrEl2EcHvc64 = 0b010110;
constexpr uint8_t kEsrEl2EcTrapSystem = 0b011000;
constexpr uint8_t kEsrEl2EcTrapSve = 0b011001;
constexpr uint8_t kEsrEl2EcDataAboartFromLow = 0b100100;

constexpr uint8_t kIssTrappedMcrOrMrcAccessCp15 = 0b0010;
constexpr uint8_t kIssTrappedMcrrOrMrrcAccessCp15 = 0b0011;
constexpr uint8_t kOp1SCR = 0b000;
constexpr uint8_t kOp1PCR = 0b001;
constexpr uint8_t kOp1DBG = 0b010;
constexpr uint8_t kOp1VMSA_SCR = 0b011;
[[maybe_unused]] constexpr uint8_t kOp1VMSA_HCR = 0b100;
[[maybe_unused]] constexpr uint8_t kOp1VMSA_VTCR = 0b101;
constexpr uint8_t kCrnSCTLR_EL1 = 0;
constexpr uint8_t kCrnACTLR_EL1 = 1;
[[maybe_unused]] constexpr uint8_t kCrnCPACR_EL1 = 2;
[[maybe_unused]] constexpr uint8_t kCrnSCR_EL1 = 3;
[[maybe_unused]] constexpr uint8_t kCrnSDER32_EL3 = 4;
[[maybe_unused]] constexpr uint8_t kCrnAFSR0_EL1 = 5;
[[maybe_unused]] constexpr uint8_t kCrnAFSR1_EL1 = 6;
[[maybe_unused]] constexpr uint8_t kCrnESR_EL1 = 7;
[[maybe_unused]] constexpr uint8_t kCrnFAR_EL1 = 8;
[[maybe_unused]] constexpr uint8_t kCrnHPFAR_EL2 = 9;
[[maybe_unused]] constexpr uint8_t kCrnPAR_EL1 = 10;
[[maybe_unused]] constexpr uint8_t kCrnPMCR_EL0 = 11;
[[maybe_unused]] constexpr uint8_t kCrnPMCCFILTR_EL0 = 12;

constexpr uint8_t kIssDirectionWrite = 0b0;
[[maybe_unused]] constexpr uint8_t kIssDirectionRead = 0b1;

inline uint8_t EsrEl2Ec(uint64_t esr) {
  return ((esr >> kEsrEl2EcShift) & 0x3f);
}

inline bool TrapMcrMrcInstructions(Tcb* tsk,
                                   const uint8_t opc2,
                                   const uint8_t opc1,
                                   const uint8_t crn,
                                   const uint8_t rt,
                                   const uint8_t crm,
                                   const uint8_t dir) {
#define MSR(name, _op1, _crn, _crm, _op2)                                     \
  do {                                                                        \
    if (opc1 == (_op1) && crn == (_crn) && crm == (_crm) && opc2 == (_op2)) { \
      tsk->vcpu_sysregs.name = regs->regs[rt];                                \
      sched.IncrementCurrentTaskPc(4);                                        \
      return true;                                                            \
    }                                                                         \
  } while (false)

#define MRS(name, _op1, _crn, _crm, _op2)                                     \
  do {                                                                        \
    if (opc1 == (_op1) && crn == (_crn) && crm == (_crm) && opc2 == (_op2)) { \
      regs->regs[rt] = tsk->vcpu_sysregs.name;                                \
      sched.IncrementCurrentTaskPc(4);                                        \
      return true;                                                            \
    }                                                                         \
  } while (false)

  auto& sched = evisor::Sched::Get();
  auto* regs = sched.GetVCpuRegs(tsk);
  if (dir == kIssDirectionWrite) {
    MSR(actlr_el1, kOp1SCR, kCrnACTLR_EL1, 0, 1);
    MSR(csselr_el1, kOp1PCR, kCrnSCTLR_EL1, 0, 0);
  } else {
    MRS(actlr_el1, kOp1SCR, kCrnACTLR_EL1, 0, 1);

    MRS(id_pfr0_el1, kOp1SCR, kCrnSCTLR_EL1, 1, 0);
    MRS(id_pfr1_el1, kOp1SCR, kCrnSCTLR_EL1, 1, 1);
    MRS(id_mmfr0_el1, kOp1SCR, kCrnSCTLR_EL1, 1, 4);
    MRS(id_mmfr1_el1, kOp1SCR, kCrnSCTLR_EL1, 1, 5);
    MRS(id_mmfr2_el1, kOp1SCR, kCrnSCTLR_EL1, 1, 6);
    MRS(id_mmfr3_el1, kOp1SCR, kCrnSCTLR_EL1, 1, 7);

    MRS(id_isar0_el1, kOp1SCR, kCrnSCTLR_EL1, 2, 0);
    MRS(id_isar1_el1, kOp1SCR, kCrnSCTLR_EL1, 2, 1);
    MRS(id_isar2_el1, kOp1SCR, kCrnSCTLR_EL1, 2, 2);
    MRS(id_isar3_el1, kOp1SCR, kCrnSCTLR_EL1, 2, 3);
    MRS(id_isar4_el1, kOp1SCR, kCrnSCTLR_EL1, 2, 4);
    MRS(id_isar5_el1, kOp1SCR, kCrnSCTLR_EL1, 2, 5);

    MRS(mvfr0_el1, kOp1SCR, kCrnSCTLR_EL1, 3, 0);
    MRS(mvfr1_el1, kOp1SCR, kCrnSCTLR_EL1, 3, 1);
    MRS(mvfr2_el1, kOp1SCR, kCrnSCTLR_EL1, 3, 2);

    MRS(id_aa64pfr0_el1, kOp1SCR, kCrnSCTLR_EL1, 4, 0);
    MRS(id_aa64pfr1_el1, kOp1SCR, kCrnSCTLR_EL1, 4, 1);

    MRS(id_aa64dfr0_el1, kOp1SCR, kCrnSCTLR_EL1, 5, 0);
    MRS(id_aa64dfr1_el1, kOp1SCR, kCrnSCTLR_EL1, 5, 1);

    MRS(id_aa64isar0_el1, kOp1SCR, kCrnSCTLR_EL1, 6, 0);
    MRS(id_aa64isar1_el1, kOp1SCR, kCrnSCTLR_EL1, 6, 1);

    MRS(id_aa64mmfr0_el1, kOp1SCR, kCrnSCTLR_EL1, 7, 0);
    MRS(id_aa64mmfr1_el1, kOp1SCR, kCrnSCTLR_EL1, 7, 1);
    //MRS(id_aa64mmfr2_el1, kOp1SCR, kCrnSCTLR_EL1, 7, 2);
    MRS(mpidr_el1, kOp1SCR, kCrnSCTLR_EL1, 7, 3);

    MRS(id_aa64afr0_el1, kOp1SCR, kCrnSCTLR_EL1, 5, 4);
    MRS(id_aa64afr1_el1, kOp1SCR, kCrnSCTLR_EL1, 5, 5);

    MRS(revidr_el1, kOp1SCR, kCrnSCTLR_EL1, 0, 6);

    MRS(ccsidr_el1, kOp1PCR, kCrnSCTLR_EL1, 0, 0);
    MRS(clidr_el1, kOp1PCR, kCrnSCTLR_EL1, 0, 1);

    MRS(aidr_el1, kOp1PCR, kCrnSCTLR_EL1, 0, 7);
    MRS(csselr_el1, kOp1DBG, kCrnSCTLR_EL1, 0, 0);
    MRS(ctr_el0, kOp1VMSA_SCR, kCrnSCTLR_EL1, 0, 1);
  }
  return false;
}

inline void HandleTrapWfx() {
  auto& sched = evisor::Sched::Get();
  sched.Schedule();
  sched.IncrementCurrentTaskPc(4);
  sched.GetCurrentTask()->stat.wfx_traps++;
}

inline void HandleTrapSystem(uint32_t iss) {
  /// COND, bits [23:20]
  /// 0b0000:	Unknown reason
  /// 0b0001:	Trapped WFI or WFE instruction
  /// 0b0010:	Trapped MCR or MRC access to a CP15 register
  /// 0b0011:	Trapped MCRR or MRRC access to a CP15 register
  /// 0b0100:	Trapped MCR or MRC access to a CP14 register
  /// 0b0101:	Trapped LDC or STC access to a CP14 register
  /// 0b0110:	Trapped VMRS or VMSR access to a CP10 register
  /// 0b0111:	Trapped MRRC access to a 64-bit NEON or VFP register
  /// 0b1000:	Trapped VMRS or VMSR access to a CP11 register
  /// 0b1001:	Trapped LDC or STC access to a CP10 register
  /// 0b1010:	Trapped access to CP14 register using an MCR or MRC instruction
  /// 0b1011:	Trapped access to CP14 register using an LDC or STC instruction
  /// 0b1100:	Trapped access to CP14 register using an MRC or MCR instruction
  ///         from a lower exception level
  /// 0b1101:	Trapped access to CP14 register
  ///         using an LDC or STC instruction from a lower exception level
  /// 0b1110: Trapped
  /// MSR, MRS or System instruction 0b1111	Implementation defined
  const uint8_t cond = (iss >> 20) & 0xf;
  /// Opc2, bits [19:17]
  /// The Opc2 value from the issued instruction.
  const uint8_t opc2 = (iss >> 17) & 0x7;
  /// Opc1, bits [16:14]
  /// The Opc1 value from the issued instruction.
  const uint8_t opc1 = (iss >> 14) & 0x7;
  /// CRn, bits [13:10]
  /// The CRn value from the issued instruction.
  const uint8_t crn = (iss >> 10) & 0xf;
  /// Rt, bits [9:5]
  /// The Rt value from the issued instruction, the general-purpose register
  /// used for the transfer.
  const uint8_t rt = (iss >> 5) & 0x1f;
  /// CRm, bits [4:1]
  /// The CRm value from the issued instruction.
  const uint8_t crm = (iss >> 1) & 0xf;
  /// Direction, bit [0]
  /// Write to System register space. MCR instruction.
  /// 0b0:  Write to System register space. MCR instruction.
  /// 0b1:  Read from System register space. MRC or VMRS instruction.
  const uint8_t dir = iss & 0x1;

  auto* tsk = evisor::Sched::Get().GetCurrentTask();
  tsk->stat.sysreg_traps++;
  switch (cond) {
    // MCR, MCRR, MRC, MRRC instructions
    case kIssTrappedMcrOrMrcAccessCp15:
    case kIssTrappedMcrrOrMrrcAccessCp15:
      if (TrapMcrMrcInstructions(tsk, opc2, opc1, crn, rt, crm, dir)) {
        return;
      }
      break;
    default:
      break;
  }
  PANIC("The event (%x) was not handled in HandleTrapSystem", cond);
}

/* ISS encoding for an exception from a Data Abort */

/// DFSC, bits [5:0] / Data Fault Status Code
inline uint8_t ESR_EL2_ISS_EXCEPTION_FROM_DATA_ABORT_DFSC(uint64_t esr) {
  return esr & 0x3f;
}
[[maybe_unused]] constexpr uint8_t kEsrEl2DfscAddressSizeFault = 0b0000;
constexpr uint8_t kEsrEl2DfscTranslationFault = 0b0001;
[[maybe_unused]] constexpr uint8_t kEsrEl2DfscAccessFlagFault = 0b0010;
constexpr uint8_t kEsrEl2DfscPermissionFault = 0b0011;

/// SRT, bits [20:16]:
/// Syndrome Register Transfer. The register number of the Wt/Xt/Rt operand of
/// the faulting instruction.
inline uint8_t ESR_EL2_ISS_EXCEPTION_FROM_DATA_ABORT_SRT(uint64_t esr) {
  return (esr >> 16) & 0x1f;
}

/// WnR, bit [6]:
/// Write not Read. Indicates whether a synchronous abort was caused by an
/// instruction writing to a memory location, or by an instruction reading from
/// a memory location.
inline uint8_t ESR_EL2_ISS_EXCEPTION_FROM_DATA_ABORT_WNR(uint64_t esr) {
  return (esr >> 6) & 0x1;
}
constexpr uint8_t kEsrEl2IssExceptionFromDataAboartCausedByRead = 0;
[[maybe_unused]] constexpr uint8_t kEsrEl2IssExceptionFromDataAboartCausedByWrite = 1;

inline bool HandleTrapMemAbort(va_t addr, uint64_t esr) {
  const auto dfsc = ESR_EL2_ISS_EXCEPTION_FROM_DATA_ABORT_DFSC(esr);
  const uint8_t dfsc_without_level = dfsc >> 2;

  switch (dfsc_without_level) {
    case kEsrEl2DfscTranslationFault: {
      return evisor::HandleMmTrapMemoryAccessFault(addr);
    }
    case kEsrEl2DfscPermissionFault: {
      const uint8_t srt = ESR_EL2_ISS_EXCEPTION_FROM_DATA_ABORT_SRT(esr);
      const auto wnr = ESR_EL2_ISS_EXCEPTION_FROM_DATA_ABORT_WNR(esr);
      return evisor::HandleMmTrapRegisterAccess(
          addr, srt, wnr == kEsrEl2IssExceptionFromDataAboartCausedByRead);
    }
    default:
      LOG_WARN("Uncaught exception: %d", dfsc);
      break;
  }

  return false;
}

}  // namespace

void TrapHandleLowerElAarch64Sync(uint64_t esr,
                                  uint64_t elr,
                                  uint64_t far,
                                  uint64_t hvc_nr) {
  const auto ec = EsrEl2Ec(esr);
  switch (ec) {
    case kEsrEl2EcTrapWfx:
      HandleTrapWfx();
      break;
    case kEsrEl2EcTrapFpReg:
      PANIC("ESR_EL2_EC_TRAP_FP_REG has not yet been implemented.");
      break;
    case kEsrEl2EcHvc64:
      PANIC("ESR_EL2_EC_HVC64 has not yet been implemented.");
      UNUSED(hvc_nr);
      break;
    case kEsrEl2EcTrapSystem:
      HandleTrapSystem(esr & 0xfff'ffff);
      break;
    case kEsrEl2EcTrapSve:
      PANIC("ESR_EL2_EC_TRAP_SVE has not yet been implemented.");
      break;
    case kEsrEl2EcDataAboartFromLow:
      if (!HandleTrapMemAbort(far, esr)) {
        PANIC("Failed to handle memory abort trap");
      }
      break;
    default:
      PANIC(
          "Uncaught exception: cause: %s, address: %x, far_el2: "
          "%x",
          kExceptionClass[ec], esr, elr, far);
      break;
  }
}
