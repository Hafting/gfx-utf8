#include "gfx-utf8.h"

//Pointer magic that might be needed on some platforms:
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



utf8_GFX::utf8_GFX(Adafruit_GFX *displ) { 
	display = displ;
	cur_font = (GFXfont const **) 0;
	cur_font_size = 0;
	textsize_x = textsize_y = 1;
	textcolor = 0xFFFF;
	wrap = true;
}

void utf8_GFX::setFontSet(const GFXfont **fset, int n) {
	cur_font = fset;
	cur_font_size = n;
}

/*
	Search to fontset, find which font has a glyph for this character.
	Returns the first match found, or 0 if no match.
	 */
GFXfont const *utf8_GFX::font_lookup(uint16_t c) {
	if (cur_font) for (int i = 0; i < cur_font_size; ++i) {
		GFXfont const *font = cur_font[i];
		if (font->first <= c && font->last >= c) return font;
	}
	return (GFXfont const *) 0;
} 
/*
  Draw a single unicode glyph. Searches the fontset to see which font, if any, contains the glyph. Then uses that font to print it. 
	Similar to Adafruit GFX library drawChar, but no 'classic font' and therefore no background color.
	 */
void utf8_GFX::drawChar(int16_t x, int16_t y, uint16_t c, uint16_t color, uint8_t size) {
	drawChar(x, y, c, color, size, size);
}

void utf8_GFX::drawChar(int16_t x, int16_t y, uint16_t c, uint16_t color, uint8_t size_x, uint8_t size_y) {
	GFXfont const *gfxFont = font_lookup(c);
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


//write a single character at the current position, handling \n etc
size_t utf8_GFX::write(uint16_t const c) {
	if (!cur_font) return 0; //No fontset, no printing
	if (c == '\r') return 1; //ignore - compatibility with Adafruit_GFX
	int16_t cursor_x = display->getCursorX();
	int16_t cursor_y = display->getCursorY();
	if (c == '\n') {
		//Use cur_font[0], normally no glyph for '\n', but we need yAdvance 
		cursor_x = 0;
		cursor_y +=
			(int16_t)textsize_y * (uint8_t)pgm_read_byte(cur_font[0]->yAdvance);
	} else {
		//There must be a printable glyph now
		GFXfont const * const gfxFont = font_lookup(c);
		if (!gfxFont) return 0; //Not printable, no action.
		uint16_t first = pgm_read_word(&gfxFont->first);
		GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
		uint8_t w = pgm_read_byte(&glyph->width),
						h = pgm_read_byte(&glyph->height);
		if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
			int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
			if (wrap && ((cursor_x + textsize_x * (xo + w)) > display->width())) {
				cursor_x = 0;
				cursor_y += (int16_t)textsize_y *
					(uint8_t)pgm_read_byte(&gfxFont->yAdvance);
			}
			drawChar(cursor_x, cursor_y, c, textcolor, textsize_x, textsize_y);
		}
		cursor_x +=
			(uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
	}
	display->setCursor(cursor_x, cursor_y);
	return 1;
}

/*
	 print loops through the bytes in the string, converting them to unicode numbers. For ascii, the unicode number is the ascii code. Beyond ascii, the unicode number is assembled from 2 or possibly more bytes. 

  Unicode numbers are then printed.

  The printing mechanism is basically drawChar+print from the Adafruit GFX library, extended to print uint16_t characters. (GFXfont has 16 bit glyph numbers, no need to extend the font format.) 
	 */
void utf8_GFX::print(char *s) {
	if (!cur_font) return; //No fontset, no print!
	//incomplete
}


