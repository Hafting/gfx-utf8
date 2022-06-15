#include "gfx-utf8.h"

utf8::utf8(Adafruit_GFX *displ) { 
	display = displ;
	cur_fonts = 0;
}

void utf8::setFontSet(fontset *fset) {
	cur_fonts = fset;
}

void utf8::print_utf8(char *s) {
	if (!cur_fonts) return;
	//incomplete
}


