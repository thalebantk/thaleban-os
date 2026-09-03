#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/* Segment selectors: the byte offset of the descriptor in the table. The
 * order is not arbitrary. SYSCALL derives its selectors from a single MSR --
 * CS from STAR[47:32] and SS from that plus 8, while SYSRET takes CS from
 * STAR[63:48] plus 16 and SS from plus 8 -- so kernel code must sit directly
 * before kernel data, and user data directly before user code. Reordering
 * these breaks fast system calls later. */
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_DATA   0x18
#define GDT_USER_CODE   0x20
#define GDT_TSS         0x28

/* Replaces the bootloader's GDT with our own, reloads every segment register
 * and loads the task register. Limine leaves a usable GDT behind, but it is
 * free to reclaim that memory once the kernel is running, so the kernel needs
 * a table it owns. */
void gdt_init(void);

/* Sets the stack the CPU switches to on a privilege change into ring 0
 * (TSS.RSP0). Needed once user mode exists; harmless before then. */
void gdt_set_kernel_stack(uint64_t rsp0);

#endif
