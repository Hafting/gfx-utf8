# gfx-utf8
utf8 printing extension for the adafruit arduino gfx library

Goals:
 * print utf8 strings from source, file or received from the net
 * store glyphs beyond ascii in a compact way
 * create necessary fonts using the unmodified fontconvert utility

Secondary goal
 * Also implement getTextBounds_utf8()

The problem:
drawChar(), print() and println() work with 8-bit bytes only. So there is room for no more than 128 bytes beyond ascii. Not enough if you want to support all latin characters, or even greek, cyrillic, emojis, alchemy symbols, etc. And even if you only need a few extras; if they come from external sources, chances are they are in utf8 which uses several bytes per character.

The fontconvert utility uses 16-bit internally, and can therefore convert fonts with lots of interesting glyphs. But it will only do continous series of glyphs. So if you need ascii + character number 7000, you get a font with 7000 glyphs wasting precious memory.

Solutions:
The fontconvert problem is handled by making separate fonts for each range of glyphs needed. This avoids wasting space for unused glyphs. print_utf8() handle switching between sub-fonts internally.

print_utf8() decodes utf8 sequences to find the glyph number. Then, the correct sub-font is selected and the glyph is printed using the original print()

Non-solution:
extending print() to handle utf8 directly is possible, and backward compatible with ascii printing. It is not backwards compatible with existing 8-bit printing using iso8859 though.
