#include <cassert>

#include "shadow.h"
#include "machine.h"
#include "pma.h"

static void write_shadow(uint8_t *base, uint64_t offset, uint64_t value) {
    assert((offset & (sizeof(uint64_t)-1)) == 0);
    *reinterpret_cast<uint64_t *>(base + offset) = value;
}

uint64_t shadow_get_csr_rel_addr(shadow_csr reg) {
    return static_cast<uint64_t>(reg);
}

uint64_t shadow_get_register_rel_addr(int reg) {
    assert(reg >= 0 && reg < 32);
    return reg*sizeof(uint64_t);
}

/// \name Base of board shadow, where PMAs start
#define SHADOW_PMA_BASE     UINT64_C(0x800)

uint64_t shadow_get_pma_rel_addr(int p) {
    assert(p >= 0 && p < 32);
    return SHADOW_PMA_BASE + 2*p*sizeof(uint64_t);
}

/// \brief Shadow device peek callback. See ::pma_peek.
static bool shadow_peek(const pma_entry *pma, uint64_t page_offset, const uint8_t **page_data, uint8_t *shadow) {
    const machine_state *s = reinterpret_cast<const machine_state *>(pma_get_context(pma));
    // There is only one page: 0
    if (page_offset != 0) {
        *page_data = nullptr;
        return false;
    }
    // Clear page
    memset(shadow, 0, PMA_PAGE_SIZE);
    // Copy general-purpose registers
    for (int i = 0; i < 32; ++i) {
        write_shadow(shadow, shadow_get_register_rel_addr(i),
            machine_read_register(s, i));
    }
    // Copy named registers
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::pc),
        machine_read_pc(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mvendorid),
        machine_read_mvendorid(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::marchid),
        machine_read_marchid(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mimpid),
        machine_read_mimpid(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mcycle),
        machine_read_mcycle(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::minstret),
        machine_read_minstret(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mstatus),
        machine_read_mstatus(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mtvec),
        machine_read_mtvec(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mscratch),
        machine_read_mscratch(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mepc),
        machine_read_mepc(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mcause),
        machine_read_mcause(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mtval),
        machine_read_mtval(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::misa),
        machine_read_misa(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mie),
        machine_read_mie(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mip),
        machine_read_mip(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::medeleg),
        machine_read_medeleg(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mideleg),
        machine_read_mideleg(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::mcounteren),
        machine_read_mcounteren(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::stvec),
        machine_read_stvec(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::sscratch),
        machine_read_sscratch(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::sepc),
        machine_read_sepc(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::scause),
        machine_read_scause(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::stval),
        machine_read_stval(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::satp),
        machine_read_satp(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::scounteren),
        machine_read_scounteren(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::ilrsc),
        machine_read_ilrsc(s));
    write_shadow(shadow, shadow_get_csr_rel_addr(shadow_csr::iflags),
        machine_read_iflags(s));
    // Copy PMAs
    for (int i = 0; i < machine_get_pma_count(s); ++i) {
        auto pi = machine_get_pma(s, i);
        auto ai = shadow_get_pma_rel_addr(i);
        write_shadow(shadow, ai, pma_get_istart(pi));
        write_shadow(shadow, ai + sizeof(uint64_t), pma_get_ilength(pi));
    }
    *page_data = shadow;
    return true;
}

static const pma_driver shadow_driver = {
    "SHADOW",
    pma_read_error,
    pma_write_error,
    shadow_peek
};

bool shadow_register_mmio(machine_state *s, uint64_t start, uint64_t length) {
    auto pma = machine_register_shadow(s, start, length, s, &shadow_driver);
    return pma && machine_set_shadow_pma(s, pma);
}
