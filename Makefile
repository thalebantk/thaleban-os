# thaleban-os — auto-detecting build
#
#   make          build build/kernel
#   make iso      build build/thaleban-os.iso
#   make run      boot the ISO in QEMU
#   make clean    remove build/
#   make print    show what was auto-detected

NAME    := thaleban-os
SRCDIR  := src
BUILD   := build
OBJDIR  := $(BUILD)/obj
KERNEL  := $(BUILD)/kernel
ISO     := $(BUILD)/$(NAME).iso

CC   := gcc
AS   := gcc
LD   := ld
NASM := nasm

# Every directory under src/ becomes an include path, so headers are found
# wherever they live.
INCDIRS := $(shell find $(SRCDIR) -type d)

CFLAGS := -g -O2 -pipe -Wall -Wextra -std=gnu11
CFLAGS += -ffreestanding -fno-stack-protector -fno-stack-check -fno-PIC
CFLAGS += -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel
CFLAGS += $(addprefix -I,$(INCDIRS))

ASFLAGS  := -g -ffreestanding $(addprefix -I,$(INCDIRS))
NASMFLAGS := -f elf64 -g -F dwarf

# Auto-detected sources: C, GAS assembly (.S), NASM assembly (.asm).
CSRCS   := $(shell find $(SRCDIR) -name '*.c'   | sort)
SSRCS   := $(shell find $(SRCDIR) -name '*.S'   | sort)
ASMSRCS := $(shell find $(SRCDIR) -name '*.asm' | sort)

OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.c.o,$(CSRCS)) \
        $(patsubst $(SRCDIR)/%.S,$(OBJDIR)/%.S.o,$(SSRCS)) \
        $(patsubst $(SRCDIR)/%.asm,$(OBJDIR)/%.asm.o,$(ASMSRCS))

DEPS := $(OBJS:.o=.d)

# Auto-detected linker script (first *.ld found under src/).
LDSCRIPT := $(firstword $(shell find $(SRCDIR) -name '*.ld' | sort))
LDFLAGS  := -nostdlib -static -z max-page-size=0x1000 -T $(LDSCRIPT)

.PHONY: all iso run run-uefi clean distclean print
.SUFFIXES:

all: $(KERNEL)

$(KERNEL): $(OBJS) $(LDSCRIPT)
	@mkdir -p $(dir $@)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR)/%.S.o: $(SRCDIR)/%.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR)/%.asm.o: $(SRCDIR)/%.asm
	@mkdir -p $(dir $@)
	$(NASM) $(NASMFLAGS) -MD $(@:.o=.d) $< -o $@

-include $(DEPS)

# ---------------------------------------------------------------- bootloader

limine/limine:
	rm -rf limine
	git clone https://github.com/limine-bootloader/limine.git \
		--branch=v9.x-binary --depth=1 limine
	$(MAKE) -C limine

# ---------------------------------------------------------------------- iso

iso: $(ISO)

$(ISO): $(KERNEL) limine/limine limine.conf
	rm -rf $(BUILD)/iso_root
	mkdir -p $(BUILD)/iso_root/boot/limine $(BUILD)/iso_root/EFI/BOOT
	cp $(KERNEL) $(BUILD)/iso_root/boot/kernel
	cp limine.conf $(BUILD)/iso_root/boot/limine/
	cp limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin \
		$(BUILD)/iso_root/boot/limine/
	cp limine/BOOTX64.EFI limine/BOOTIA32.EFI $(BUILD)/iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J \
		-b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(BUILD)/iso_root -o $@
	./limine/limine bios-install $@
	rm -rf $(BUILD)/iso_root

# ---------------------------------------------------------------------- run

run: $(ISO)
	qemu-system-x86_64 -M q35 -m 512M -cdrom $(ISO) -boot d

# ------------------------------------------------------------------- extras

print:
	@echo "linker script : $(LDSCRIPT)"
	@echo "include dirs  : $(INCDIRS)"
	@echo "sources       :"
	@for s in $(CSRCS) $(SSRCS) $(ASMSRCS); do echo "    $$s"; done
	@echo "objects       :"
	@for o in $(OBJS); do echo "    $$o"; done

clean:
	rm -rf $(BUILD)

distclean: clean
	rm -rf limine

# ------------------------------------------------------- compilation database
# compile_commands.json for clangd / ccls, built from the same auto-detected
# sources and flags used above. Regenerated whenever a source or this Makefile
# changes; `arguments` form is used so no shell quoting is involved.

COMPDB := compile_commands.json

.PHONY: compdb
compdb: $(COMPDB)

$(COMPDB): Makefile $(CSRCS) $(SSRCS)
	@printf '[\n' > $@
	@sep=''; \
	emit() { \
	  src=$$1; obj=$$2; shift 2; \
	  printf '%b  {\n' "$$sep" >> $@; \
	  printf '    "directory": "%s",\n' '$(CURDIR)' >> $@; \
	  printf '    "file": "%s",\n' "$$src" >> $@; \
	  printf '    "output": "%s",\n' "$$obj" >> $@; \
	  printf '    "arguments": [' >> $@; \
	  asep=''; \
	  for a in "$$@"; do printf '%s"%s"' "$$asep" "$$a" >> $@; asep=', '; done; \
	  printf ']\n  }' >> $@; \
	  sep=',\n'; \
	}; \
	for src in $(CSRCS); do \
	  rel=$${src#$(SRCDIR)/}; \
	  emit "$$src" "$(OBJDIR)/$$rel.o" $(CC) $(CFLAGS) -c "$$src" -o "$(OBJDIR)/$$rel.o"; \
	done; \
	for src in $(SSRCS); do \
	  rel=$${src#$(SRCDIR)/}; \
	  emit "$$src" "$(OBJDIR)/$$rel.o" $(AS) $(ASFLAGS) -c "$$src" -o "$(OBJDIR)/$$rel.o"; \
	done; \
	printf '\n]\n' >> $@
	@echo "wrote $@"
