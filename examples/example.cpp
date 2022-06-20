/*
	This example is not complete. I assume you know how to make a program that uses Adafruit_GFX to print ascii on whatever display you have.
	If you don't know, start with one of their examples.

	 */

//To use the utf8 library, include its header along with other includes:
#include "gfx-utf8.h"

/*
Use fontconvert, make a set of fonts containing the unicode symbols you want to print. See unicode.txt for an example. You need a .ttf font file, FreeSans and many others can be found on the web.

Why make several font files? Because a single file with all of unicode, will need more memory than your microcontroller has!  By extracting the few ranges you want to use, and nothing in between those ranges, you save lots of memory. 

Ascii+latin1+dingbats needs only 3 times the memory of ascii only, if you extract 3 subfonts covering the 3 ranges. (8kB or so) Extract the whole range as one font, and you might need 250kB for a little 8pt font. This will also fail, as the font file uses 16-bit numbers so a single file cannot be larger than 64kB.   
	 */

//To use extracted fonts, include them like this:
#include "Freesans8pt32-126.h"
#include "Freesans8pt160-383.h"
#include "Freesans8pt8592-8703.h"
#include "Freesans8pt10048.h"
//If this gives you errors like "redefinition of 'const GFXfont ...'", re-read unicode.txt. You may need to change some names!

/*
Write code that puts something on the display, using Adafruit_GFX. Refer to their examples, if you need help.
	 */

/*
First, prepare a "font set". That is just an array of the sub-fonts you want to use for unicode printing. Example using the Freesans font with 4 ranges of symbols: 
	 */

const GFXfont *Freesans8pt[4] = {&FreeSans8pt7b, &FreeSansA8pt16b, &FreeSansB8pt16b, &FreeSansC8pt16b};

/*
I assume you have an object called "display", that is used for drawing.
You also need an object of type utf8_GFX, for unicode printing:
	 */

//create u8display, connected to "display"
utf8_GFX u8display(&display); 

//select the fontset declared above. Without it, no printing:
u8display.setFontSet(Freesans8pt, sizeof(Freesans8pt)/sizeof(const GFXfont *) );

//Or simpler: u8display.setFontSet(Freesans8pt, 4); //4 fonts in this set.

/* Now some printing: */
uint16_t FG = 0xFF00; //or some color that works for your display
u8display.drawChar(50, 100, 65, FG, 2); //prints a large "A"
u8display.drawChar(125, 100, 10048, FG, 1); //prints a flower dingbat

//utf8 string in source code. You need a decent unicode font to read this:
display.setCursor(50, 125);
u8display.setTextColor(FG);
u8display.print("Test æøå ❀ ŝšů ↑↔ ⇒ ↲ Q"); 
/*
The string above contains some European letters outside of ascii, 
a flower dingbat, and some arrows. Also the ascii parts "Test" and "Q"
One of the arrows are not in FreeSans, and prints as a framed question mark instead.

u8display.drawChar puts a single symbol where you want it.
u8display.print prints a string, containing any unicode symbols from the basic multilingual plane. (Assuming you have a font containing these symbols..)

 */





