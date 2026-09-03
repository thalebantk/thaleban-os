#include <stddef.h>

#include <font/font.h>

#define PSF2_MAGIC 0x864ab572u

struct psf2_header {
	uint32_t magic;
	uint32_t version;
	uint32_t headersize;
	uint32_t flags;
	uint32_t numglyph;
	uint32_t bytesperglyph;
	uint32_t height;
	uint32_t width;
};

extern const uint8_t ter128b_psf[];
extern const uint8_t ter128b_psf_end[];

static const struct psf2_header *hdr = NULL;
static const uint8_t *glyphs;
static uint32_t stride;

void font_init(void)
{
	const struct psf2_header *h = (const struct psf2_header *)ter128b_psf;

	if ((uint64_t)(ter128b_psf_end - ter128b_psf) < sizeof(*h) ||
	    h->magic != PSF2_MAGIC ||
	    h->height == 0 || h->width == 0 ||
	    h->bytesperglyph < h->height) {
		return;
	}

	hdr = h;
	glyphs = ter128b_psf + h->headersize;
	stride = h->bytesperglyph / h->height;
	return;
}

uint32_t font_width(void)  { return hdr == NULL ? 0 : hdr->width; }
uint32_t font_height(void) { return hdr == NULL ? 0 : hdr->height; }
uint32_t font_stride(void) { return hdr == NULL ? 0 : stride; }

const uint8_t *font_glyph(char c)
{
	if (hdr == NULL) {
		return NULL;
	}

	/* Glyphs are indexed by byte value, and the font carries 256 of them.
	 * Anything past the end falls back to '?' so unmapped bytes stay
	 * visible instead of silently vanishing. */
	uint32_t index = (uint8_t)c;
	if (index >= hdr->numglyph) {
		index = '?';
		if (index >= hdr->numglyph) {
			return NULL;
		}
	}

	return glyphs + (uint64_t)index * hdr->bytesperglyph;
}
