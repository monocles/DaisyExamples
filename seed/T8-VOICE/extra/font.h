#pragma once
#include <cstdint>

// Core font definitions
#define FONT_CHARW 6
#define FONT_CHARH 8
#define FONT_CHARH_1 7
#define FONT_ASCII_OFFSET 0x20

typedef struct _glyph {
    uint8_t first;     // column inset from left side in proportional mode
    uint8_t last;      // column inset from right side in proportional mode
    uint8_t data[FONT_CHARW]; // column data 
} glyph_t;

extern const glyph_t font_data[];
extern const uint32_t font_nglyphs;

// Core font functions only
uint8_t font_glyph(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
uint8_t font_glyph_fixed(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
uint8_t font_string_position(const char* str, uint8_t pos);
uint8_t font_string_pixels(const char* str);
uint8_t* font_glyph_big(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
uint8_t* font_glyph_bigbig(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
uint8_t* font_string(const char* str, uint8_t* buf, uint32_t size, uint8_t w, uint8_t a, uint8_t b);
uint8_t* font_string_big(const char* str, uint8_t* buf, uint32_t size, uint8_t w, uint8_t a, uint8_t b);
uint8_t* font_string_bigbig(const char* str, uint8_t* buf, uint32_t size, uint8_t w, uint8_t a, uint8_t b);
