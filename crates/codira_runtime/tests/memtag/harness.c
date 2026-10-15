/*
 * Copyright (c) 2026 Omnira CJSC
 * Author: Tunjay Akbarli
 * Date: October 14, 2026
 *
 * Functionality:
 * - A freestanding aarch64 Linux program (no C library) that exercises the
 *   runtime's MTE primitives (`native/memtag_aarch64.c`) one scenario at a
 *   time, for `tests/memtag.rs`. A scenario that must fault ends in SIGSEGV;
 *   any other ending is an exit status saying what went wrong:
 *   2 MTE could not be enabled, 3 mapping failed, 4 a tag was wrong,
 *   5 a value read back was wrong, 6 a faulting access did not fault,
 *   7 unknown scenario.
 */

#include <stddef.h>
#include <stdint.h>

long codira_mte_enable(void);
uintptr_t codira_mte_map(size_t size, int prot);
void codira_mte_set_tags(uintptr_t addr, size_t size);
uintptr_t codira_mte_tag(uintptr_t addr, size_t size, uint64_t exclude);
uintptr_t codira_mte_memory_tag(uintptr_t addr);

__asm__(".globl _start\n"
        "_start:\n"
        "\tmov x0, sp\n"
        "\tbl codira_harness_main\n"
        "\tmov x8, #93\n" /* exit */
        "\tsvc #0\n");

static unsigned tag_of(uintptr_t addr) { return (unsigned)((addr >> 56) & 0xf); }

static int same(const char *a, const char *b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

int codira_harness_main(uintptr_t *sp) {
    long argc = (long)sp[0];
    const char *scenario = argc > 1 ? (const char *)sp[2] : "ok";

    if (codira_mte_enable() != 0) {
        return 2;
    }
    uintptr_t base = codira_mte_map(4096, 3);
    if (base == 0) {
        return 3;
    }
    /* Two neighbouring 32-byte allocations, as a tagged region hands out. */
    uintptr_t a = codira_mte_tag(base, 32, 0);
    uintptr_t b = codira_mte_tag(base + 32, 32, 1ULL << tag_of(a));
    if (tag_of(a) == 0 || tag_of(b) == 0 || tag_of(a) == tag_of(b)) {
        return 4;
    }
    if (tag_of(codira_mte_memory_tag(a)) != tag_of(a)) {
        return 4;
    }

    volatile uint64_t *pa = (volatile uint64_t *)a;
    if (same(scenario, "ok")) {
        pa[0] = 41;
        pa[3] = 1;
        return pa[0] + pa[3] == 42 ? 0 : 5;
    }
    if (same(scenario, "overflow")) {
        pa[4] = 1; /* the first granule of `b`, through `a`'s tag */
        return 6;
    }
    if (same(scenario, "untagged")) {
        *(volatile uint64_t *)base = 1; /* tag 0 against `a`'s tag */
        return 6;
    }
    if (same(scenario, "stale")) {
        codira_mte_set_tags(base, 32); /* freed: back to tag 0 */
        pa[0] = 1;
        return 6;
    }
    return 7;
}
