#ifndef _FONT_H_
#define _FONT_H_

//#include "compiler.h"
//#include "fonts/dejavu_numerals_24.h"
// #include "fonts/ume_tgo5_18.h"

#include "region.h"


// small bitfield system font
#define FONT_CHARW 8
#define FONT_CHARH 6
// glyph table doesn't include the initial non-ascii chars
#define FONT_ASCII_OFFSET 0x20

// #define FONT_AA font_ume_tgo5_18
// #define FONT_AA_CHARW 	FONT_UME_TGO5_18_W
// #define FONT_AA_CHARH 	FONT_UME_TGO5_18_H

// Добавляем константы для второго шрифта
// #define FONT2_CHARW 12
// #define FONT2_CHARH 14
#define FONT2_CHARW 9
#define FONT2_CHARH 12
#define FONT2_ASCII_OFFSET 0x20

//---------------------------
//---- variables

typedef struct _glyph {
  uint8_t    first     ;       // column inset from left side in proportional mode
  uint8_t    last      ;       // column inset from left side in proportional mode
  uint8_t    data[FONT_CHARW]; // column data 
} glyph_t;

typedef struct _glyph2 {
  uint8_t    first;
  uint8_t    last; 
  uint16_t    data[FONT2_CHARW];
} glyph2_t;

extern const glyph_t font_data[];
extern const glyph2_t font2_data[];

extern const uint32_t font_nglyphs;
extern const uint32_t font2_nglyphs;

//-------------------------------
//--- functions

// render a single glyph to a buffer, rastering
// given pointer, row length, foreground, background
// returns count of columns
extern uint8_t font_glyph(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
// fixed-width variant
extern uint8_t font_glyph_fixed(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
// same as font_glyph, double size
/// -- -fixme: these still return updated buf pointers... no width checking
extern uint8_t* font_glyph_big(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
// same as font_glyph, 4x size
extern uint8_t* font_glyph_bigbig(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
// render a string of packed glyphs to a buffer
extern uint8_t* font_string(const char* str, uint8_t* buf, uint32_t size, uint8_t w, uint8_t a, uint8_t b);
// same as font_string, double size
extern uint8_t* font_string_big(const char* str, uint8_t* buf, uint32_t size, uint8_t w, uint8_t a, uint8_t b);
// same as font_string, 4x size
extern uint8_t* font_string_bigbig(const char* str, uint8_t* buf, uint32_t size, uint8_t w, uint8_t a, uint8_t b);

extern uint8_t font2_glyph(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
extern uint8_t font2_glyph_fixed(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
extern uint8_t* font2_string(const char* str, uint8_t* buf, uint32_t size, uint8_t w, uint8_t a, uint8_t b);

uint8_t font_string_position(const char* str, uint8_t pos);
uint8_t font_string_pixels(const char* str);
// render a font string to a region.
// this allows for smarter bounds handling
extern void font_string_region_wrap(region* reg, const char* str, uint8_t x, uint8_t y, uint8_t fg, uint8_t bg);
extern void font_string_region_clip(region* reg, const char* str, uint8_t x, uint8_t y, uint8_t fg, uint8_t bg);
extern void font_string_region_clip_tab(region* reg, const char* str, uint8_t x, uint8_t y, uint8_t fg, uint8_t bg);
extern void font_string_region_clip_right(region* reg, const char* str, uint8_t x, uint8_t y, uint8_t fg, uint8_t bg);
extern void font_string_region_clip_hi(region* reg, const char* str, uint8_t x, uint8_t y, uint8_t fg, uint8_t bg, uint8_t hi);
extern void font_string_region_clip_hid(region* reg, const char* str, uint8_t x, uint8_t y, uint8_t fg, uint8_t bg, uint8_t hi, uint8_t hid);

extern void region_string_font2(region* reg, const char* str, uint8_t x, uint8_t y, uint8_t fg, uint8_t bg);

///--- anti-aliased

// render an anti-aliased (4-bit) glyph to a buffer
// arguments are character, buffer, target row size, invert flag
// extern uint8_t* font_glyph_aa(char ch, uint8_t* buf, uint8_t w, uint8_t invert);

// render a string of ant-aliased glyphs to a buffer
// extern uint8_t* font_string_aa(const char* str, uint8_t* buf, uint32_t size, uint8_t w, uint8_t invert);

#endif // header guard
