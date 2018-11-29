#include <cstring>

#include "cartesi-constants.h"
#include "riscv-constants.h"
#include "machine-config.h"

machine_config::machine_config(void):
    processor{},
    ram{},
    rom{},
    flash{},
    clint{},
    htif{},
    interactive{false} {
    // Starting address is 4k
    processor.pc = 0x1000;
    // M-mode
    processor.iflags = PRV_M << IFLAGS_PRV_SHIFT;
    // No reservation
    processor.ilrsc = -1;
    processor.mstatus = ((uint64_t)MXL << MSTATUS_UXL_SHIFT) |
        ((uint64_t)MXL << MSTATUS_SXL_SHIFT);
    // Set our extensions in misa
    processor.misa = MXL;
    processor.misa <<= (XLEN-2); /* set xlen to 64 */
    processor.misa |= MISAEXT_S | MISAEXT_U | MISAEXT_I |
        MISAEXT_M | MISAEXT_A;
    // Set our ids
    processor.mvendorid = VENDORID;
    processor.marchid = ARCHID;
    processor.mimpid = IMPID;
}