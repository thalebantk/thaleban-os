/* The PSF2 font is linked straight into the kernel image. The Makefile puts
 * every directory under src/ on the assembler's include path, so .incbin
 * finds the file next to this one. */

	.section .rodata
	.balign 16

	.globl ter128b_psf
ter128b_psf:
	.incbin "ter-128b.psf"
	.globl ter128b_psf_end
ter128b_psf_end:

	.section .note.GNU-stack,"",@progbits
