#include "gfx-utf8.h"

//Pointer magic that might be needed:
#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

inline GFXglyph *pgm_read_glyph_ptr(const GFXfont *gfxFont, uint16_t c) {
#ifdef __AVR__
  return &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
#else
  // expression in __AVR__ section may generate "dereferencing type-punned
  // pointer will break strict-aliasing rules" warning In fact, on other
  // platforms (such as STM32) there is no need to do this pointer magic as
  // program memory may be read in a usual way So expression may be simplified
  return gfxFont->glyph + c;
#endif //__AVR__
}

inline uint8_t *pgm_read_bitmap_ptr(const GFXfont *gfxFont) {
#ifdef __AVR__
  return (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
#else
  // expression in __AVR__ section generates "dereferencing type-punned pointer
  // will break strict-aliasing rules" warning In fact, on other platforms (such
  // as STM32) there is no need to do this pointer magic as program memory may
  // be read in a usual way So expression may be simplified
  return gfxFont->bitmap;
#endif //__AVR__
}



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
	drawChar_utf8(x, y, c, color, size, size);
}

void utf8::drawChar_utf8(int16_t x, int16_t y, uint16_t c, uint16_t color, uint8_t size_x, uint8_t size_y) {
	GFXfont *gfxFont = font_lookup(c);
	if (!gfxFont) return; //No font with this symbol, no printing!

	//From Adafruit drawChar, with int8 replaced with int16 as needed
	c -= (uint16_t)pgm_read_word(&gfxFont->first);
	GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c);
	uint8_t *bitmap = pgm_read_bitmap_ptr(gfxFont);
	

	uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
	uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
	int8_t xo = pgm_read_byte(&glyph->xOffset),
	       yo = pgm_read_byte(&glyph->yOffset);
	uint8_t xx, yy, bits = 0, bit = 0;
	int16_t xo16 = 0, yo16 = 0;

	if (size_x > 1 || size_y > 1) {
		xo16 = xo;
		yo16 = yo;
	}
	
	display->startWrite();
	for (yy = 0; yy < h; yy++) {
		for (xx = 0; xx < w; xx++) {
			if (!(bit++ & 7)) {
				bits = pgm_read_byte(&bitmap[bo++]);
			}
			if (bits & 0x80) {
				if (size_x == 1 && size_y == 1) {
					display->writePixel(x + xo + xx, y + yo + yy, color);
				} else {
					display->writeFillRect(x + (xo16 + xx) * size_x, y + (yo16 + yy) * size_y,
					              size_x, size_y, color);
				}
			}
			bits <<= 1;
		}
	}
	display->endWrite();
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


