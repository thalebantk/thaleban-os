# Fonts

## ter-128b.psf

Terminus 14x28 bold, PSF2 format, 256 glyphs with a Unicode table.
Taken from Fedora's `terminus-fonts-console` package
(`/usr/lib/kbd/consolefonts/ter-128b.psf.gz`), decompressed.

Terminus Font is copyright (c) 2020 Dimitar Toshkov Zhekov and is licensed
under the SIL Open Font License, Version 1.1.
Upstream: https://terminus-font.sourceforge.net/

### Layout

| field           | value  |
| --------------- | ------ |
| header size     | 32     |
| glyph count     | 256    |
| bytes per glyph | 56     |
| glyph size      | 14x28  |
| row stride      | 2      |

Glyph *n* starts at `headersize + n * bytesperglyph`. Each row is 2 bytes,
big-endian, MSB first: the leftmost pixel is bit 15, and the low 2 bits of
each row are padding.
