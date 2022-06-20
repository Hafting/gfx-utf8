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

//Supports printnumber and such
size_t utf8_GFX::write(uint8_t const c) {
	return write((uint16_t) c); //works for numbers
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
			(int16_t)textsize_y * (uint8_t)pgm_read_byte(&cur_font[0]->yAdvance);
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

void utf8_GFX::setTextSize(uint8_t sx, uint8_t sy) {
	textsize_x = (sx > 0) ? sx : 1;
	textsize_y = (sy > 0) ? sy : 1;
	display->setTextSize(textsize_x, textsize_y);
}

/*
	Helper for print() and getTextBounds()
	Returns the next valid unicode symbol from the string.
	The string itself is advanced to the byte after this symbol
	Silently skips invalid utf-8 and overlong forms, as well as
	4-byte utf-8 which don't fit the 16-bit return type.
	Will never skip past the string's terminating zero.

	Normally used from a loop, for decoding a utf-8 string
	The loop can terminate when the string is advanced to
	its terminating zero, or the return value is zero.

	The characters themselves are const
	The pointer to the character string pointer is const.
	The char pointer itself is not const, it will move through the string
	 */
uint16_t utf8_GFX::decode_utf8(unsigned char const * * const s) {
	uint16_t sym;
	//Loop because we may need to skip garbage first
	while (**s) {
  	if (**s < 128) return *(*s)++;       //Ascii subset
		if (**s < 0xc2) ++(*s);              //skip invalid utf-8
		else if (**s < 0xe0) {               //starts two-byte sequence
			sym = (*(*s)++ & 0x1f) << 6;
			//Valid second byte?
			if (**s >= 0x80 && **s <= 0xbf) return sym | (*(*s)++ & 0x3f);
		}	else if (**s < 0xf0) {             //starts three-byte seq
			sym = (**s & 0x0f) << 12;
			unsigned char first = *(*s)++;
			//Valid second
			if (**s <= 0xbf && (**s >= 0xa0 || (**s >= 0x80 && first > 0xe0))) {
				//Valid second byte
				sym |= (*(*s)++ & 0x3f) << 6;
				//Third byte valid?
				if (**s >= 0x80 && **s <= 0xbf) return sym | (*(*s)++ & 0x3f);
			} //invalid second of 3, let the loop handle it
		}	//invalid or 4-byte utf-8, treat as invalid
	} //end of string
	return 0;
}

/*
	 write loops through the bytes in the string, converting them to unicode numbers. For ascii, the unicode number is the ascii code. Beyond ascii, the unicode number is assembled from 2 or more bytes. 

  Unicode numbers are then printed.

  The printing mechanism is basically drawChar+write from the Adafruit GFX library, extended to print uint16_t characters. (GFXfont has 16 bit glyph numbers, no need to extend the font format.) 
	 */

size_t utf8_GFX::write(const uint8_t *buffer, size_t size) {
	if (!cur_font) return 0; //No fontset, no print
	unsigned char const *s = buffer;
	while (s-buffer < size) {
		write(decode_utf8(&s));
	}
}

/*
  Straight from Adafruit_GFX, except:
	 - the c parameter is a 16-bit unicode character
	 - font_lookup is used to find the correct font
	 - special casing for "default font" removed.
*/
void utf8_GFX::charBounds(uint16_t c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy) {
	if (!cur_font) return; //No fontset, no bounds
	if (c == '\n') { // Newline? No font lookup, as \n have no glyph.
		*x = 0;        // Reset x to zero, advance y by one line
		*y += textsize_y * (uint8_t)pgm_read_byte(&cur_font[0]->yAdvance);
	} else if (c != '\r') { // Not a carriage return; is normal char
		GFXfont const * const gfxFont = font_lookup(c);
		if (!gfxFont) return; //unprintable, no bounds
		uint8_t first = pgm_read_byte(&gfxFont->first);
		GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
		uint8_t gw = pgm_read_byte(&glyph->width),
						gh = pgm_read_byte(&glyph->height),
						xa = pgm_read_byte(&glyph->xAdvance);
		int8_t xo = pgm_read_byte(&glyph->xOffset),
					 yo = pgm_read_byte(&glyph->yOffset);
		if (wrap && ((*x + (((int16_t)xo + gw) * textsize_x)) > display->width())) {
			*x = 0; // Reset x to zero, advance y by one line
			*y += textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
		}
		int16_t tsx = (int16_t)textsize_x, tsy = (int16_t)textsize_y,
						x1 = *x + xo * tsx, y1 = *y + yo * tsy, x2 = x1 + gw * tsx - 1,
						y2 = y1 + gh * tsy - 1;
		if (x1 < *minx)
			*minx = x1;
		if (y1 < *miny)
			*miny = y1;
		if (x2 > *maxx)
			*maxx = x2;
		if (y2 > *maxy)
			*maxy = y2;
		*x += xa * tsx;
	}
}

/*
	 Text bounds for a utf8 string. Picks out & validates unicode symbols,
	 then runs them through charBounds()
	 Meant to be compatible with Adafruit_GFX::getTextBounds()
*/ 
void utf8_GFX::getTextBounds(char const *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
	int16_t minx = 0x7FFF, miny = 0x7FFF, maxx = -1, maxy = -1; // Bound rect
	*x1 = x;
	*y1 = y;
	*w = *h = 0;
	while (*s) charBounds(decode_utf8((unsigned char const **)&s), &x, &y, &minx, &miny, &maxx, &maxy);
	if (maxx >= minx) {     // If legit string bounds were found...
		*x1 = minx;           // Update x1 to least X coord,
		*w = maxx - minx + 1; // And w to bound rect width
	}
	if (maxy >= miny) { // Same for height
		*y1 = miny;
		*h = maxy - miny + 1;
	}	
}
