/*
 * Copyright (C) 2025 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include <sys/auxv.h>

#include <cstdint>

#if defined(__aarch64__)

#include <arm_sme.h>

// Detects whether FEAT_SME is available.
//
// FEAT_SME is optional from Armv9.2.
[[maybe_unused]] static bool sme_is_enabled() {
  return ((getauxval(AT_HWCAP2) & HWCAP2_SME) != 0);
}

// Sets PSTATE.SM to 0.
//
// Requires FEAT_SME, which is optional from Armv9.2.
[[maybe_unused]] __attribute__((naked)) static void sme_disable_sm() {
  __asm__ __volatile__(".arch_extension sme; bti c; smstop sm; ret;");
}

// Sets PSTATE.ZA to 1.
//
// Requires FEAT_SME, which is optional from Armv9.2.
[[maybe_unused]] __attribute__((naked)) static void sme_enable_za() {
  __asm__ __volatile__(".arch_extension sme; bti c; smstart za; ret;");
}

// Sets PSTATE.ZA to 0.
//
// Requires FEAT_SME, which is optional from Armv9.2.
[[maybe_unused]] __attribute__((naked)) static void sme_disable_za() {
  __asm__ __volatile__(".arch_extension sme; bti c; smstop za; ret;");
}

// Sets TPIDR2_EL0 to a given value.
//
// Requires FEAT_SME, which is optional from Armv9.2.
[[maybe_unused]] __attribute__((naked)) static void sme_set_tpidr2_el0(uint64_t value) {
  __asm__ __volatile__(".arch_extension sme; bti c; msr TPIDR2_EL0, x0; ret;");
}

// Reads TPIDR2_EL0 and returns its value.
//
// Requires FEAT_SME, which is optional from Armv9.2.
[[maybe_unused]] __attribute__((naked)) static uint64_t sme_tpidr2_el0() {
  __asm__ __volatile__(".arch_extension sme; bti c; mrs x0, TPIDR2_EL0; ret;");
}

// Reads SVCR special register.
//
// Requires FEAT_SME, which is optional from Armv9.2.
[[maybe_unused]] __attribute__((naked)) static uint64_t sme_read_svcr() {
  __asm__ __volatile__(".arch_extension sme; bti c; mrs x0, SVCR; ret;");
}

// Returns true if PSTATE.SM is 1, otherwise false.
//
// Requires FEAT_SME, which is optional from Armv9.2.
[[maybe_unused]] static bool sme_is_sm_on() {
  static constexpr uint64_t kSvcrSmMask = 0x01UL;
  return ((sme_read_svcr() & kSvcrSmMask) != 0);
}

// Returns true if PSTATE.ZA is 1, otherwise false.
//
// Requires FEAT_SME, which is optional from Armv9.2.
[[maybe_unused]] static bool sme_is_za_on() {
  static constexpr uint64_t kSvcrZaMask = 0x02UL;
  return ((sme_read_svcr() & kSvcrZaMask) != 0);
}

// Assembly is required to ensure the test does not depend on compiler optimizations.
[[maybe_unused]] __attribute__((naked)) static void sme_dormant_caller(void (*fn_address)()) {
  // clang-format off
  __asm__ __volatile__(
    ".arch_extension sme\n\r"
    "bti      c\n\r"
    "stp      fp, lr, [sp, #-16]!\n\r"
    "mov      fp, sp\n\r"
    // Set up a lazy-save buffer on the stack.
    // It is 16 bytes + size according to VL.
    "sub      sp, sp, #16\n\r"
    "rdsvl    x8, #1\n\r"
    "mul      x9, x8, x8\n\r"
    "sub      sp, sp, x9\n\r"
    "mov      x9, sp\n\r"
    // Bytes 0-7: za_save_buffer
    // Bytes 8-9: num_za_save_slices
    // Other bytes are cleared.
    "stp      x9, x8, [fp, #-16]\n\r"
    // Finalize the lazy-save buffer.
    "msr      TPIDR2_EL0, x9\n\r"
    // Call the given function with dormant SME state.
    "smstart  za\n\r"
    "blr      x0\n\r"
    // Set SME state to off.
    "msr      TPIDR2_EL0, xzr\n\r"
    "smstop   za\n\r"
    "mov      sp, fp\n\r"
    "ldp      fp, lr, [sp], #16\n\r"
    "ret\n\r"
  );
  // clang-format on
}

// Turns all SME state off.
//
// Requires FEAT_SME, which is optional from Armv9.2.
[[maybe_unused]] static void sme_state_cleanup() {
  sme_disable_sm();
  sme_set_tpidr2_el0(0UL);
  sme_disable_za();
}

#endif  // defined(__aarch64__)
