//===----------------------------------------------------------------------===//
//
// This source file is part of the Codira open source project
//
// Copyright (c) 2024 Apple Inc. and the Codira project authors.
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://language.org/LICENSE.txt for license information
// See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
//
//===----------------------------------------------------------------------===//

#include <stddef.h>
#include <stdint.h>

int main(int argc, char *argv[]);
void qemu_exit(void);
int puts(const char *);

__attribute__((noreturn))
void reset(void) {
  main(0, NULL);
  qemu_exit();
  __builtin_trap();
}

void interrupt(void) {
  puts("INTERRUPT\n");
  qemu_exit();
  while (1) {
  }
}

__attribute__((aligned(4))) char stack[2048];

__attribute((used))
__attribute((section(".vectors"))) void *vector_table[114] = {
    (void *)&stack[2048 - 4],  // initial SP
    reset,                 // Reset

    interrupt,  // NMI
    interrupt,  // HardFault
    interrupt,  // MemManage
    interrupt,  // BusFault
    interrupt,  // UsageFault

    0  // NULL for all the other handlers
};

void qemu_exit() {
  __asm__ volatile("mov r0, #0x18");
  __asm__ volatile("movw r1, #0x0026");
  __asm__ volatile("movt r1, #0x2"); // construct 0x20026 in r1
  __asm__ volatile("bkpt #0xab");
}

int putchar(int c) {
  // This is only valid in an emulator (QEMU), and it's skipping a proper configuration of the UART device
  // and waiting for a "ready to transit" state.

  // STM32F1 specific location of USART1 and its DR register
  *(volatile uint32_t *)(0x40013800 + 0x04) = c;
  return c;
}
