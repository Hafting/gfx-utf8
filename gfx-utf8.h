#ifndef GFXUTF8
#define GFXUTF8

#include <Adafruit_GFX.h>
#include <Print.h>
/*
	A font array is one or several regular fonts with non-overlapping glyph ranges, used for utf8 printing. This saves memory by not storing unused glyph ranges. 

	For sane linefeeds, all fonts in the array should have the same height.
	 */

/*
	 The class that prints utf8 strings,
	 will eventually also measure them.
	 */
class utf8_GFX : public Print {
	protected:
		Adafruit_GFX *display;
		const GFXfont **cur_font; //Array of fonts for utf8 printing
		int cur_font_size;//array size

		//may get cursor_x, cursor_y, rotation, _width, _height from display
		//may NOT get textcolor, textsize_x, textsize_y, wrap. So reimplement.
		//cursor position & text properties for print/write
		uint8_t textsize_x;   ///< Desired magnification in X-axis of text to print()
		uint8_t textsize_y;   ///< Desired magnification in Y-axis of text to print()
		bool wrap;            ///< If set, 'wrap' text at right edge of display
		uint16_t textcolor;

		GFXfont const *font_lookup(uint16_t c);
		void charBounds(uint16_t c, int16_t *x, int16_t *y, int16_t *minx,
                  int16_t *miny, int16_t *maxx, int16_t *maxy);
		uint16_t decode_utf8(unsigned char const ** const s);
	public:
		utf8_GFX(Adafruit_GFX *displ);
		void setFontSet(GFXfont const **fset, int n);
		void drawChar(int16_t x, int16_t y, uint16_t c, uint16_t color, uint8_t size);
		void drawChar(int16_t x, int16_t y, uint16_t c, uint16_t color, uint8_t size_x, uint8_t size_y);
		//Support printnumber:		
		size_t write(uint8_t c);
		//Support strings, which may use utf-8:
		size_t write(const uint8_t *buffer, size_t size);

		//Write a unicode character number. 	
		size_t write(uint16_t c); //print utf8 character, handling \n
		void getTextBounds(char const *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h); 

		void setTextColor(uint16_t c) { textcolor = c; }
		uint16_t getTextColor() { return textcolor; }
		void setTextWrap(bool w) { display->setTextWrap(wrap = w); }
		bool getTextWrap() { return wrap; }
		void setTextSize(uint8_t s) { setTextSize(s, s); }
		void setTextSize(uint8_t sx, uint8_t sy);

};

#endif
