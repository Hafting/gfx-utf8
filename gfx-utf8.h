#ifndef GFXUTF8
#define GFXUTF8

#include <Adafruit_GFX.h>

/*
	A font array is one or several regular fonts with non-overlapping glyph ranges, used for utf8 printing. This saves memory by not storing unused glyph ranges. 

	For sane linefeeds, all fonts in the array should have the same height.
	 */

/*
	 The class that prints utf8 strings,
	 will eventually also measure them.
	 */
class utf8_GFX {
	private:
		Adafruit_GFX *display;
		const GFXfont **cur_font; //Array of fonts for utf8 printing
		int cur_font_size;//array size

		GFXfont const *font_lookup(uint16_t c);
	public:
		utf8_GFX(Adafruit_GFX *displ);
		void setFontSet(const GFXfont **fset, int n);
		void drawChar(int16_t x, int16_t y, uint16_t c, uint16_t color, uint8_t size);
		void drawChar(int16_t x, int16_t y, uint16_t c, uint16_t color, uint8_t size_x, uint8_t size_y);
		size_t write(uint16_t c); //print utf8 character, handling \n
		void print(char *s); //print utf8 string on display
		//getTextBounds(char *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h); //future
};

#endif
