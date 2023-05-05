#include "arch/arm64/irq/gic.h"

#include "common/logger.h"

namespace evisor {

namespace {

const char *kErrorTypes[] = {
    // clang-format off
    "SYNC_INVALID_SP0_EL2",
    "IRQ_INVALID_SP0_EL2",
    "FIQ_INVALID_SP0_EL2",
    "ERROR_INVALID_SP0_EL2",

    "SYNC_INVALID_SPX_EL2",
    "IRQ_INVALID_SPX_EL2",
    "FIQ_INVALID_SPX_EL2",
    "ERROR_INVALID_SPX_EL2",

    "SYNC_INVALID_EL01_64",
    "IRQ_INVALID_EL01_64",
    "FIQ_INVALID_EL01_64",
    "ERROR_INVALID_EL01_64",

    "SYNC_INVALID_EL01_32",
    "IRQ_INVALID_EL01_32",
    "FIQ_INVALID_EL01_32",
    "ERROR_INVALID_EL01_32",
    // clang-format on
};

}  // namespace

void Gic::CatchUnexpectedIrqs(uint32_t type, uint32_t esr_el2, uint32_t elr_el2,
                              uint32_t far_el2) {
  PANIC("Uncatched exception(%s) ESR_EL2: %x, ELR_EL2: %x, FAR_EL2: %x",
        kErrorTypes[type], esr_el2, elr_el2, far_el2);
}

}  // namespace evisor
