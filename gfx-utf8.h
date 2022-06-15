#ifndef GFXUTF8
#define GFXUTF8

#include <Adafruit_GFX.h>

/*
	A fontset is one or several regular fonts with non-overlapping glyph ranges, used for utf8 printing. This saves memory by not storing unused glyph ranges. 

	size must match the number of fonts in the array.

	For sane linefeeds, all fonts in a set should have the same height.
	 */
struct fontset {
	GFXfont font[];
	int size;
};

/*
	 The class that prints utf8 strings,
	 will eventually also measure them.
	 */
class utf8 {
	private:
		Adafruit_GFX *display;
		fontset *cur_fonts;

		GFXfont *font_lookup(uint16_t c);
	public:
		utf8(Adafruit_GFX *displ);
		void setFontSet(fontset *fset);
		void drawChar_utf8(int16_t x, int16_t y, uint16_t c, uint16_t color, uint16_t bg, uint8_t size);
		void drawChar_utf8(int16_t x, int16_t y, uint16_t c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);

		void print_utf8(char *s); //print utf8 string on display
		//getTextBounds_utf8(char *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h); //future
};

#endif
