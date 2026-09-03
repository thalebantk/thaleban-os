#include <cpu/gdt.h>

/* Access byte: P DPL DPL S E DC RW A */
#define ACCESS_PRESENT   0x80
#define ACCESS_RING3     0x60
#define ACCESS_SEGMENT   0x10  /* code or data, as opposed to a system entry */
#define ACCESS_EXEC      0x08
#define ACCESS_RW        0x02
#define ACCESS_TSS       0x89  /* present, DPL 0, available 64-bit TSS */

#define KCODE_ACCESS (ACCESS_PRESENT | ACCESS_SEGMENT | ACCESS_EXEC | ACCESS_RW)
#define KDATA_ACCESS (ACCESS_PRESENT | ACCESS_SEGMENT | ACCESS_RW)
#define UCODE_ACCESS (KCODE_ACCESS | ACCESS_RING3)
#define UDATA_ACCESS (KDATA_ACCESS | ACCESS_RING3)

/* Flags nibble: G D/B L AVL. A 64-bit code segment sets L and clears D/B;
 * setting both is an invalid combination the CPU rejects. */
#define FLAGS_CODE64 0xa0
#define FLAGS_DATA   0xc0

/* Base and limit are ignored for code and data segments in long mode -- the
 * CPU treats them as base 0 with no limit -- but the fields still have to be
 * encoded, so they carry the conventional flat values. */
#define LIMIT_FLAT 0xfffff

#define GDT_ENTRIES 7 /* null, 4 segments, and 2 slots for the 16-byte TSS */

struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t limit_high_flags;
	uint8_t base_high;
} __attribute__((packed));

/* A system descriptor is twice as wide as a segment descriptor, because it
 * carries a full 64-bit base. */
struct gdt_system_entry {
	struct gdt_entry low;
	uint32_t base_upper;
	uint32_t reserved;
} __attribute__((packed));

struct gdtr {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

/* The 64-bit TSS holds no task state: long mode uses it only for the stack
 * pointers the CPU loads on a privilege or interrupt-stack switch. */
struct tss {
	uint32_t reserved0;
	uint64_t rsp[3];
	uint64_t reserved1;
	uint64_t ist[7];
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iomap_base;
} __attribute__((packed));

#define KERNEL_STACK_SIZE 16384

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdtr gdtr;
static struct tss tss;

/* The stack the CPU lands on when an interrupt or trap enters ring 0. The
 * ABI wants 16-byte alignment at the entry point. */
static uint8_t kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(16)));

static void set_entry(int index, uint32_t base, uint32_t limit, uint8_t access,
		      uint8_t flags)
{
	gdt[index].limit_low = (uint16_t)(limit & 0xffff);
	gdt[index].base_low = (uint16_t)(base & 0xffff);
	gdt[index].base_mid = (uint8_t)((base >> 16) & 0xff);
	gdt[index].access = access;
	gdt[index].limit_high_flags =
		(uint8_t)(((limit >> 16) & 0x0f) | (flags & 0xf0));
	gdt[index].base_high = (uint8_t)((base >> 24) & 0xff);
	return;
}

static void set_tss_entry(int index, uint64_t base, uint32_t limit)
{
	struct gdt_system_entry *entry =
		(struct gdt_system_entry *)&gdt[index];

	set_entry(index, (uint32_t)(base & 0xffffffff), limit, ACCESS_TSS, 0);
	entry->base_upper = (uint32_t)(base >> 32);
	entry->reserved = 0;
	return;
}

/* Installs the table, then reloads every segment register. CS cannot be
 * assigned directly, so the new selector and a return address are pushed and
 * a far return pops them -- the architecturally sanctioned way to change CS
 * without a task switch. */
static void gdt_flush(void)
{
	asm volatile (
		"lgdt %[gdtr]\n\t"
		"pushq %[kcode]\n\t"
		"leaq 1f(%%rip), %%rax\n\t"
		"pushq %%rax\n\t"
		"lretq\n"
		"1:\n\t"
		"movw %[kdata], %%ax\n\t"
		"movw %%ax, %%ds\n\t"
		"movw %%ax, %%es\n\t"
		"movw %%ax, %%ss\n\t"
		"movw %%ax, %%fs\n\t"
		"movw %%ax, %%gs\n\t"
		"ltr %[tss]"
		:
		: [gdtr] "m" (gdtr),
		  [kcode] "i" (GDT_KERNEL_CODE),
		  [kdata] "i" (GDT_KERNEL_DATA),
		  [tss] "r" ((uint16_t)GDT_TSS)
		: "rax", "memory");
	return;
}

void gdt_init(void)
{
	set_entry(0, 0, 0, 0, 0);
	set_entry(1, 0, LIMIT_FLAT, KCODE_ACCESS, FLAGS_CODE64);
	set_entry(2, 0, LIMIT_FLAT, KDATA_ACCESS, FLAGS_DATA);
	set_entry(3, 0, LIMIT_FLAT, UDATA_ACCESS, FLAGS_DATA);
	set_entry(4, 0, LIMIT_FLAT, UCODE_ACCESS, FLAGS_CODE64);

	/* No I/O permission bitmap: pointing the base past the end of the
	 * segment tells the CPU there is none. */
	tss.iomap_base = sizeof(tss);
	tss.rsp[0] = (uint64_t)(kernel_stack + sizeof(kernel_stack));
	set_tss_entry(5, (uint64_t)&tss, sizeof(tss) - 1);

	gdtr.limit = (uint16_t)(sizeof(gdt) - 1);
	gdtr.base = (uint64_t)&gdt;

	gdt_flush();
	return;
}

void gdt_set_kernel_stack(uint64_t rsp0)
{
	tss.rsp[0] = rsp0;
	return;
}
