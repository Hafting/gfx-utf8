# gfx-utf8
utf8 printing extension for the Adafruit Arduino GFX library

Goals:
 * print utf8 strings from source, file or received from the net    DONE
 * store glyphs beyond ascii in a compact way                       OK
 * create necessary fonts using the unmodified fontconvert utility  OK

Secondary goals
 * Also implement getTextBounds for utf8 strings                    DONE
 * support println(), print(int), print(float)                      DONE

The problem:
drawChar(), print() and println() work with 8-bit bytes only. So there is room for no more than 128 bytes beyond ascii. Not enough if you want to support all latin characters, or even greek, cyrillic, arrows, dingbats, alchemy symbols, etc. And even if you only need a few extras; if they come from external sources, chances are they are in utf8 which uses several bytes per character.

The fontconvert utility uses 16-bit internally, and can therefore convert fonts with lots of interesting glyphs. But it will only do continous series of glyphs. So if you need ascii + character number 7000, you get a font with 7000 glyphs wasting too much memory.

Solutions:
The fontconvert problem is handled by making separate fonts for each range of glyphs needed. These fonts are then collected in an array of fonts. This avoids wasting space for unused glyphs. utf8_GFX handles switching between sub-fonts in a font array internally.

utf8_GFX::write(char *s) decodes utf8 sequences to find the 16-bit glyph number for each symbol in the utf8-encoded string. Glyph numbers are passed to utf8_GFX::write(uint16_t) which handle cursor movement, and subsequently uses utf8_GFX::drawChar() to draw on the display. drawChar selects the correct sub-font from the current font set, and draws the glyph. A single string may contain symbols from several subfonts.

For an example of use, see https://github.com/Hafting/moonwatchy The library is used for printing Norwegian text, moon phase symbols and zodiac symbols. Unicode has many interesting symbols.

Non-solution:
Extending Adafruit_GFX print() to handle utf8 directly is possible, and backward compatible with ascii printing. It is not backwards compatible with existing 8-bit printing using cp437 though, and backward compatibility is their prime directive.

Limitation:
Print::print(const __FlashStringHelper *ifsh) don't work with utf-8. This routine picks apart a flash string, and prints it byte by byte. To make this work, it needs to assemble unicode symbols from the extracted bytes. I currently have no use for flash strings, so this is not done.
