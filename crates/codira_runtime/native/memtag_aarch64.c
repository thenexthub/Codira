/*
 * Copyright (c) 2026 Omnira CJSC
 * Author: Tunjay Akbarli
 * Date: October 14, 2026
 *
 * Functionality:
 * - ARM MTE (memory tagging) primitives for the Codira runtime on aarch64
 *   Linux, used by `--sanitize=memtag` builds (experimental): turning on
 *   synchronous tag-check faults for a thread, mapping tagged memory, giving a
 *   range a random tag (IRG + ST2G/STG) and clearing it again.
 *
 * Freestanding on purpose: system calls are made with `svc #0` directly and
 * nothing from a C library is used, so the same file links into the runtime
 * and into a static test program with no C library at all
 * (`codira_runtime/tests/memtag.rs`), and compiles anywhere Clang or GCC can
 * target aarch64 -- which is how it is checked on hosts that have no MTE.
 *
 * Tagging conventions (see `codira_runtime::memtag`):
 * - tag 0 is never handed out (IRG always excludes it), so memory with tag 0
 *   -- fresh, freed or reset -- faults for every pointer the runtime gave out;
 * - an allocation can also exclude its neighbour's tag, so a linear overflow
 *   into the next allocation always faults instead of 14 times in 15.
 */

#include <stddef.h>
#include <stdint.h>

#if defined(__aarch64__)

#define CODIRA_SYS_OPENAT 56
#define CODIRA_SYS_CLOSE 57
#define CODIRA_SYS_READ 63
#define CODIRA_SYS_PRCTL 167
#define CODIRA_SYS_MMAP 222
#define CODIRA_SYS_MPROTECT 226

#define CODIRA_PROT_READ 0x1
#define CODIRA_PROT_WRITE 0x2
#define CODIRA_PROT_EXEC 0x4
#define CODIRA_PROT_MTE 0x20
#define CODIRA_MAP_PRIVATE 0x02
#define CODIRA_MAP_ANONYMOUS 0x20
#define CODIRA_AT_FDCWD (-100)

#define CODIRA_PR_SET_TAGGED_ADDR_CTRL 55
#define CODIRA_PR_TAGGED_ADDR_ENABLE (1UL << 0)
#define CODIRA_PR_MTE_TCF_SYNC (1UL << 1)
#define CODIRA_PR_MTE_TAG_SHIFT 3

/* Bits 56..63 of a pointer: the top byte TBI ignores, tag in bits 56..59. */
#define CODIRA_ADDRESS_MASK 0x00ffffffffffffffUL

static long codira_syscall(long number, long a0, long a1, long a2, long a3, long a4,
                           long a5) {
    register long x8 __asm__("x8") = number;
    register long x0 __asm__("x0") = a0;
    register long x1 __asm__("x1") = a1;
    register long x2 __asm__("x2") = a2;
    register long x3 __asm__("x3") = a3;
    register long x4 __asm__("x4") = a4;
    register long x5 __asm__("x5") = a5;
    __asm__ volatile("svc #0"
                     : "+r"(x0)
                     : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5)
                     : "memory", "cc");
    return x0;
}

/* Turns on the tagged-address ABI and synchronous tag-check faults for the
 * calling thread (threads it creates afterwards inherit both), with IRG
 * allowed every tag but 0. Returns 0, or a negative errno: -EINVAL when the
 * kernel or CPU has no MTE. */
long codira_mte_enable(void) {
    unsigned long control = CODIRA_PR_TAGGED_ADDR_ENABLE | CODIRA_PR_MTE_TCF_SYNC |
                            (0xfffeUL << CODIRA_PR_MTE_TAG_SHIFT);
    return codira_syscall(CODIRA_SYS_PRCTL, CODIRA_PR_SET_TAGGED_ADDR_CTRL, (long)control, 0,
                          0, 0, 0);
}

/* Maps `size` bytes of zeroed memory with `prot` plus PROT_MTE. Every granule
 * starts with tag 0. Returns the (untagged) address, or 0. */
uintptr_t codira_mte_map(size_t size, int prot) {
    long result = codira_syscall(CODIRA_SYS_MMAP, 0, (long)size, prot | CODIRA_PROT_MTE,
                                 CODIRA_MAP_PRIVATE | CODIRA_MAP_ANONYMOUS, -1, 0);
    if ((unsigned long)result > (unsigned long)-4096L) {
        return 0;
    }
    return (uintptr_t)result;
}

/* Changes the protection of tagged pages, keeping PROT_MTE. `addr` may carry
 * a tag. Returns 0 or a negative errno. */
long codira_mte_protect(uintptr_t addr, size_t size, int prot) {
    return codira_syscall(CODIRA_SYS_MPROTECT, (long)(addr & CODIRA_ADDRESS_MASK), (long)size,
                          prot | CODIRA_PROT_MTE, 0, 0, 0);
}

/* Sets the tag of every granule of [addr, addr + size) to the tag `addr`
 * carries. `addr` is 16-byte aligned and `size` a multiple of 16. The memory
 * must be writable. */
void codira_mte_set_tags(uintptr_t addr, size_t size) {
    uintptr_t p = addr;
    uintptr_t end = addr + size;
    while (end - p >= 32) {
        __asm__ volatile(".arch_extension memtag\n\tst2g %0, [%0]" : : "r"(p) : "memory");
        p += 32;
    }
    if (p != end) {
        __asm__ volatile(".arch_extension memtag\n\tstg %0, [%0]" : : "r"(p) : "memory");
    }
}

/* Gives [addr, addr + size) a random tag that is neither 0 nor one of the
 * tags set in `exclude` (bit n excludes tag n), and returns `addr` carrying
 * it. `addr` is untagged and 16-byte aligned, `size` a multiple of 16. */
uintptr_t codira_mte_tag(uintptr_t addr, size_t size, uint64_t exclude) {
    uintptr_t tagged;
    __asm__ volatile(".arch_extension memtag\n\tirg %0, %1, %2"
                     : "=r"(tagged)
                     : "r"(addr & CODIRA_ADDRESS_MASK), "r"(exclude | 1));
    codira_mte_set_tags(tagged, size);
    return tagged;
}

/* `addr` with the tag the memory at it has (LDG), for tests and reports. */
uintptr_t codira_mte_memory_tag(uintptr_t addr) {
    uintptr_t loaded = addr;
    __asm__ volatile(".arch_extension memtag\n\tldg %0, [%1]"
                     : "+r"(loaded)
                     : "r"(addr & CODIRA_ADDRESS_MASK)
                     : "memory");
    return loaded;
}

static int codira_hex_digit(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

/* Adds PROT_MTE to the writable mapping that contains `sp` (the calling
 * thread's stack), found in /proc/self/maps, so tags written to stack slots
 * by `--sanitize=memtag` code are kept and checked. Returns 0, or a negative
 * errno (-ENOENT when no such mapping was found). */
long codira_mte_tag_stack(uintptr_t sp) {
    static const char path[] = "/proc/self/maps";
    long fd = codira_syscall(CODIRA_SYS_OPENAT, CODIRA_AT_FDCWD, (long)path, 0, 0, 0, 0);
    if (fd < 0) {
        return fd;
    }
    /* One line at a time: "start-end perms ...". `field` 0 is the start, 1
     * the end, 2 the permissions, 3 the rest of the line. */
    char buffer[512];
    uintptr_t start = 0, end = 0, found_start = 0, found_end = 0;
    int field = 0, perm_index = 0, writable = 0, found = 0;
    for (;;) {
        long n = codira_syscall(CODIRA_SYS_READ, fd, (long)buffer, sizeof buffer, 0, 0, 0);
        if (n <= 0) {
            break;
        }
        for (long i = 0; i < n && !found; i++) {
            char c = buffer[i];
            if (c == '\n') {
                if (writable && start <= sp && sp < end) {
                    found_start = start;
                    found_end = end;
                    found = 1;
                }
                start = end = 0;
                field = perm_index = writable = 0;
            } else if (field == 0) {
                if (c == '-') {
                    field = 1;
                } else {
                    start = (start << 4) | (uintptr_t)codira_hex_digit(c);
                }
            } else if (field == 1) {
                if (c == ' ') {
                    field = 2;
                } else {
                    end = (end << 4) | (uintptr_t)codira_hex_digit(c);
                }
            } else if (field == 2) {
                if (c == ' ') {
                    field = 3;
                } else if (perm_index++ == 1) {
                    writable = c == 'w';
                }
            }
        }
        if (found) {
            break;
        }
    }
    codira_syscall(CODIRA_SYS_CLOSE, fd, 0, 0, 0, 0, 0);
    if (!found) {
        return -2; /* -ENOENT */
    }
    return codira_syscall(CODIRA_SYS_MPROTECT, (long)found_start, (long)(found_end - found_start),
                          CODIRA_PROT_READ | CODIRA_PROT_WRITE | CODIRA_PROT_MTE, 0, 0, 0);
}

#endif /* __aarch64__ */
