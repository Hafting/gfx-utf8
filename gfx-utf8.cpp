#include "gfx-utf8.h"

utf8::utf8(Adafruit_GFX *displ) { 
	display = displ;
	cur_fonts = 0;
}

void utf8::setFontSet(fontset *fset) {
	cur_fonts = fset;
}

/*
	Search to fontset, find which font has a glyph for this character.
	Returns the first match found, or 0 if no match.
	 */
GFXfont *utf8::font_lookup(uint16_t c) {
	if (cur_fonts) 	for (int i = 0; i < cur_fonts->size; ++i) {
		GFXfont *font = cur_fonts->font + i;
		if (font->first <= c && font->last >= c) return font;
	}
 return (GFXfont *) 0;
} 
/*
  Draw a single unicode glyph. Searches the fontset to see which font, if any, contains the glyph. Then uses that font to print it. 
	Similar to Adafruit GFX library drawChar, but no 'classic font' and therefore no background color.
	 */
void utf8::drawChar_utf8(int16_t x, int16_t y, uint16_t c, uint16_t color, uint8_t size) {
	drawChar(x, y, c, color, size, size);
}

void utf8::drawChar_utf8(int16_t x, int16_t y, uint16_t c, uint16_t color, uint8_t size_x, uint8_t size_y) {
	GFXfont *gfxfont = font_lookup(c);
	if (!gfxfont) return; //No font with this symbol, no printing!

}


/*
	 print_utf8 loops through the bytes in the string, converting them to unicode numbers. For ascii, the unicode number is the ascii code. Beyond ascii, the unicode number is assembled from 2 or possibly more bytes. 

  Unicode numbers are then printed.

  The printing mechanism is basically drawChar+print from the Adafruit GFX library, extended to print uint16_t characters. (GFXfont has 16 bit glyph numbers, no need to extend the font format.) 
	 */
void utf8::print_utf8(char *s) {
	if (!cur_fonts) return; //No fontset, no print!
	//incomplete
}


