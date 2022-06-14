#ifndef GFXUTF8
#define GFXUTF8

#include <Adafruit_GFX.h>

/*
	A fontset is one or several regular fonts with non-overlapping glyph ranges, used for utf8 printing. This saves memory by not storing unused glyph ranges. 

	Also, print_utf8() uses print(), so a single font cannot have more than 256 glyphs anyway. More glyphs are supported by using a fontset with multiple fonts.

	size must match the number of fonts in the array.
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
	public:
		utf8(Adafruit_GFX *displ);

		//setFontSet(

		void print_utf8(char *s); //print utf8 string on display
		//getTextBounds_utf8(char *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h); //future
};

#endif
