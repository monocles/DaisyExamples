#include "font.h"

// Font data from original font.c
const glyph_t font_data[]= {
  { /* 0x00020UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
  { /* 0x00021UL, 0, 0UL, */ 3, 2, { 0x00, 0x00, 0x00, 0x5e, 0x00, 0x00 } },
  { /* 0x00022UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x06, 0x00, 0x06, 0x00 } },
  { /* 0x00023UL, 0, 0UL, */ 1, 0, { 0x00, 0x24, 0x7e, 0x24, 0x7e, 0x24 } },
  { /* 0x00024UL, 0, 0UL, */ 1, 0, { 0x00, 0x48, 0x54, 0xfe, 0x54, 0x24 } },
  { /* 0x00025UL, 0, 0UL, */ 1, 1, { 0x00, 0x48, 0x20, 0x10, 0x48, 0x00 } },
  { /* 0x00026UL, 0, 0UL, */ 1, 0, { 0x00, 0x34, 0x4a, 0x54, 0x20, 0x50 } },
  { /* 0x00027UL, 0, 0UL, */ 3, 2, { 0x00, 0x00, 0x00, 0x06, 0x00, 0x00 } },
  { /* 0x00028UL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x3c, 0x42, 0x00, 0x00 } },
  { /* 0x00029UL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x42, 0x3c, 0x00, 0x00 } },
  { /* 0x0002aUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x54, 0x38, 0x54, 0x00 } },
  { /* 0x0002bUL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x10, 0x38, 0x10, 0x00 } },
  { /* 0x0002cUL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x80, 0x60, 0x00, 0x00 } },
  { /* 0x0002dUL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x10, 0x10, 0x00, 0x00 } },
  { /* 0x0002eUL, 0, 0UL, */ 3, 2, { 0x00, 0x00, 0x00, 0x40, 0x00, 0x00 } },
  { /* 0x0002fUL, 0, 0UL, */ 1, 2, { 0x00, 0x60, 0x18, 0x06, 0x00, 0x00 } },
  { /* 0x00030UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x3c, 0x5A, 0x3c, 0x00 } },
  { /* 0x00031UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x04, 0x7e, 0x00, 0x00 } },
  { /* 0x00032UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x62, 0x52, 0x4c, 0x00 } },
  { /* 0x00033UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x42, 0x4a, 0x36, 0x00 } },
  { /* 0x00034UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x1c, 0x10, 0x7e, 0x00 } },
  { /* 0x00035UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x4e, 0x4a, 0x32, 0x00 } },
  { /* 0x00036UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x3c, 0x4a, 0x32, 0x00 } },
  { /* 0x00037UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x02, 0x72, 0x0e, 0x00 } },
  { /* 0x00038UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x34, 0x4a, 0x34, 0x00 } },
  { /* 0x00039UL, 0, 2UL, */ 2, 1, { 0x00, 0x00, 0x4c, 0x52, 0x3c, 0x00 } },
  { /* 0x0003aUL, 0, 0UL, */ 3, 2, { 0x00, 0x00, 0x00, 0x48, 0x00, 0x00 } },
  { /* 0x0003bUL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x80, 0x64, 0x00, 0x00 } },
  { /* 0x0003cUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x10, 0x28, 0x44, 0x00 } },
  { /* 0x0003dUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x28, 0x28, 0x28, 0x00 } },
  { /* 0x0003eUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x44, 0x28, 0x10, 0x00 } },
  { /* 0x0003fUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x52, 0x0a, 0x04, 0x00 } },
  { /* 0x00040UL, 0, 0UL, */ 1, 0, { 0x00, 0x7c, 0x82, 0x92, 0xaa, 0x3c } },
  { /* 0x00041UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7c, 0x12, 0x7c, 0x00 } },
  { /* 0x00042UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x4a, 0x34, 0x00 } },
  { /* 0x00043UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x3c, 0x42, 0x42, 0x00 } },
  { /* 0x00044UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x42, 0x3c, 0x00 } },
  { /* 0x00045UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x4a, 0x42, 0x00 } },
  { /* 0x00046UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x0a, 0x02, 0x00 } },
  { /* 0x00047UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x3c, 0x42, 0x72, 0x00 } },
  { /* 0x00048UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x08, 0x7e, 0x00 } },
  //  { /* 0x00049UL, 0, 0UL, */ 3, 2, { 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00 } },
  // there are 2 uppercase I's... this one has serifs
  { /* 0x00049UL, 0, 3UL, */ 2, 1, { 0x00, 0x00, 0x42, 0x7e, 0x42, 0x00 } },
  { /* 0x0004aUL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x80, 0x7e, 0x00, 0x00 } },
  { /* 0x0004bUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x08, 0x76, 0x00 } },
  { /* 0x0004cUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x40, 0x40, 0x00 } },
  { /* 0x0004dUL, 0, 0UL, */ 1, 0, { 0x00, 0x7e, 0x04, 0x08, 0x04, 0x7e } },
  { /* 0x0004eUL, 0, 0UL, */ 1, 1, { 0x00, 0x7e, 0x04, 0x08, 0x7e, 0x00 } },
  { /* 0x0004fUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x3c, 0x42, 0x3c, 0x00 } },
  { /* 0x00050UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x12, 0x0c, 0x00 } },
  { /* 0x00051UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x3c, 0xc2, 0xbc, 0x00 } },
  { /* 0x00052UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x12, 0x6c, 0x00 } },
  { /* 0x00053UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x44, 0x4a, 0x32, 0x00 } },
  { /* 0x00054UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x02, 0x7e, 0x02, 0x00 } },
  { /* 0x00055UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x3e, 0x40, 0x7e, 0x00 } },
  { /* 0x00056UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x20, 0x1e, 0x00 } },
  { /* 0x00057UL, 0, 0UL, */ 1, 0, { 0x00, 0x3e, 0x40, 0x30, 0x40, 0x3e } },
  { /* 0x00058UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x66, 0x18, 0x66, 0x00 } },
  { /* 0x00059UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x0e, 0x70, 0x0e, 0x00 } },
  { /* 0x0005aUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x62, 0x5a, 0x46, 0x00 } },
  { /* 0x0005bUL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x7e, 0x42, 0x00, 0x00 } },
  { /* 0x0005cUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x06, 0x18, 0x60, 0x00 } },
  { /* 0x0005dUL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x42, 0x7e, 0x00, 0x00 } },
  { /* 0x0005eUL, 0, 0UL, */ 1, 0, { 0x00, 0x08, 0x04, 0x7e, 0x04, 0x08 } },
  { /* 0x0005fUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x80, 0x80, 0x80, 0x00 } },
  { /* 0x00060UL, 0, 0UL, */ 1, 1, { 0x00, 0x48, 0x7c, 0x4a, 0x42, 0x00 } },
  { /* 0x00061UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x20, 0x54, 0x78, 0x00 } },
  { /* 0x00062UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x44, 0x38, 0x00 } },
  { /* 0x00063UL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x38, 0x44, 0x00, 0x00 } },
  { /* 0x00064UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x38, 0x44, 0x7e, 0x00 } },
  { /* 0x00065UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x38, 0x54, 0x4c, 0x00 } },
  { /* 0x00066UL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x7c, 0x0a, 0x00, 0x00 } },
  { /* 0x00067UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x98, 0xa4, 0x7c, 0x00 } },
  { /* 0x00068UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x04, 0x78, 0x00 } },
  { /* 0x00069UL, 0, 0UL, */ 3, 2, { 0x00, 0x00, 0x00, 0x7a, 0x00, 0x00 } },
  { /* 0x0006aUL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x80, 0x7a, 0x00, 0x00 } },
  { /* 0x0006bUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7e, 0x18, 0x64, 0x00 } },
  { /* 0x0006cUL, 0, 0UL, */ 3, 2, { 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00 } },
  { /* 0x0006dUL, 0, 0UL, */ 1, 0, { 0x00, 0x7c, 0x04, 0x7c, 0x04, 0x78 } },
  { /* 0x0006eUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7c, 0x04, 0x78, 0x00 } },
  { /* 0x0006fUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x38, 0x44, 0x38, 0x00 } },
  { /* 0x00070UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0xfc, 0x44, 0x78, 0x00 } },
  { /* 0x00071UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x38, 0x24, 0xfc, 0x00 } },
  { /* 0x00072UL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x78, 0x04, 0x00, 0x00 } },
  { /* 0x00073UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x48, 0x54, 0x24, 0x00 } },
  { /* 0x00074UL, 0, 0UL, */ 2, 2, { 0x00, 0x00, 0x3e, 0x44, 0x00, 0x00 } },
  { /* 0x00075UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x3c, 0x40, 0x7c, 0x00 } },
  { /* 0x00076UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x7c, 0x20, 0x1c, 0x00 } },
  { /* 0x00077UL, 0, 0UL, */ 1, 0, { 0x00, 0x3c, 0x40, 0x38, 0x40, 0x3c } },
  { /* 0x00078UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x6c, 0x10, 0x6c, 0x00 } },
  { /* 0x00079UL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x1c, 0xa0, 0x7c, 0x00 } },
  { /* 0x0007aUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x64, 0x54, 0x4c, 0x00 } },
  { /* 0x0007bUL, 0, 0UL, */ 2, 1, { 0x00, 0x00, 0x08, 0x76, 0x42, 0x00 } },
  { /* 0x0007cUL, 0, 0UL, */ 3, 2, { 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00 } },
  { /* 0x0007dUL, 0, 0UL, */ 1, 2, { 0x00, 0x42, 0x76, 0x08, 0x00, 0x00 } },
  { /* 0x0007eUL, 0, 0UL, */ 1, 1, { 0x00, 0x04, 0x02, 0x04, 0x02, 0x00 } },
};

const uint32_t font_nglyphs = sizeof(font_data)/sizeof(glyph_t) - 1;

uint8_t font_glyph(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b) {
    uint8_t i = 0;
    uint8_t j;
    uint8_t* p = buf;
    const glyph_t* gl = &(font_data[ch - FONT_ASCII_OFFSET]);

    // columns to draw
    uint8_t cols = FONT_CHARW - gl->first - gl->last;
    while(i < cols) {
        for(j = 0; j < FONT_CHARH; j++) {
            *p = gl->data[i + gl->first] & (1 << j) ? a : b;
            // point at next row
            p += w;
        }
        // increment column count
        i++;
        // reset pointer to row
        p = buf + i;
    }
    return cols;
}

uint8_t font_glyph_fixed(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b) {
    uint8_t i = 0;
    uint8_t j;
    uint8_t* p = buf;
    const glyph_t* gl = &(font_data[ch - FONT_ASCII_OFFSET]);

    // columns to draw
    while(i < FONT_CHARW) {
        for(j = 0; j < FONT_CHARH; j++) {
            *p = gl->data[i + gl->first] & (1 << j) ? a : b;
            // point at next row
            p += w;
        }
        // increment column count
        i++;
        // reset pointer to row
        p = buf + i;
    }
    return FONT_CHARW;
}

// same as font_glyph, double size
uint8_t* font_glyph_big(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b) {
    uint8_t i = 0, j, val;
    uint8_t* p = buf;
    const glyph_t* gl = &(font_data[ch - FONT_ASCII_OFFSET]);
    // columns to draw
    uint8_t cols = (FONT_CHARW - gl->first - gl->last);
    while(i < cols) {
        for(j = 0; j < FONT_CHARH; j++) {
            val = gl->data[i + gl->first] & (1 << j) ? a : b;
            *p = val;
            *(p + 1) = val;
            // point at next row
            p += w;
            // fill the next row as well
            *p = val;
            *(p + 1) = val;
            // point at next row
            p += w;
        }
        // increment column count
        i++;
        // set pointer to next (pixel*2) in first row
        p = buf + (i * 2);
    }
    return p;
}

// same as font_glyph, 4x size 
uint8_t* font_glyph_bigbig(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b) {
    uint8_t i = 0, j, val;
    uint8_t* p = buf;
    const glyph_t* gl = &(font_data[ch - FONT_ASCII_OFFSET]);
    // columns to draw
    uint8_t cols = (FONT_CHARW - gl->first - gl->last);
    while(i < cols) {
        for(j = 0; j < FONT_CHARH; j++) {
            val = gl->data[i + gl->first] & (1 << j) ? a : b;
            *p = val;
            *(p + 1) = val;
            *(p + 2) = val;
            *(p + 3) = val;
            // point at next row
            p += w;
            // fill the next rows as well
            *p = val;
            *(p + 1) = val;
            *(p + 2) = val;
            *(p + 3) = val;
            p += w;
            *p = val;
            *(p + 1) = val;
            *(p + 2) = val;
            *(p + 3) = val;
            p += w;
            *p = val;
            *(p + 1) = val;
            *(p + 2) = val;
            *(p + 3) = val;
            p += w;
        }
        // increment column count
        i++;
        // set pointer to next (pixel*4) in first row
        p = buf + (i << 2);
    }
    return p;
}

uint8_t font_string_position(const char* str, uint8_t pos) {
    uint8_t i = 0, n = 0;

    while(i < pos) {
        n += FONT_CHARW - font_data[str[i] - FONT_ASCII_OFFSET].first 
                       - font_data[str[i] - FONT_ASCII_OFFSET].last;
        n++;
        i++;
    }

    return n;
}

uint8_t font_string_pixels(const char* str) {
    uint8_t i = 0, n = 0;

    while(str[i]) {
        n += FONT_CHARW - font_data[str[i] - FONT_ASCII_OFFSET].first 
                       - font_data[str[i] - FONT_ASCII_OFFSET].last;
        n++;
        i++;
    }

    return n;
}

uint8_t* font_string(const char* str, uint8_t* buf, uint32_t size, uint8_t w, uint8_t a, uint8_t b) {
    uint8_t* max = buf + size - 8; // pad 1 character width on right edge

    while(buf < max) {
        if(*str == 0) {
            // end of string
            break;
        }
        buf += font_glyph(*str, buf, w, a, b);
        // 1-column space between chars
        ++buf;
        ++str;
    }
    return buf;
}

#define TAB_POSITION 48

void font_string_region_wrap(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    uint8_t* buf = reg->GetData() + xoff + (uint32_t)(reg->GetWidth()) * (uint32_t)yoff;
    uint8_t* max = buf + size;
    uint8_t* max = reg->GetData() + reg->GetLength();
    uint32_t xmax = reg->GetWidth() - 7; // padding
    uint8_t dx = 0;
    while(buf < max) {
        // break on end of string
        if(*str == 0) { break; }    
        dx = font_glyph(*str, buf, reg->GetWidth(), fg, bg) + 1;
        buf += dx;
        xoff += dx;
        ++str;
        // wrap lines
        if(xoff > xmax) { 
            xoff = 0; 
            buf += (reg->GetWidth() - xoff); 
        }
    } 
    // reg->SetDirty(true);
}

void font_string_region_clip(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    uint8_t* buf = reg->GetData() + xoff + (uint32_t)(reg->GetWidth()) * (uint32_t)yoff;
    uint8_t* max = reg->GetData() + reg->GetLength();
    uint32_t xmax = reg->GetWidth() - 7; // padding
    uint8_t dx = 0;
    while(buf < max) {
        // break on end of string
        if(*str == 0) { break; }    
        dx = font_glyph(*str, buf, reg->GetWidth(), fg, bg) + 1;
        buf += dx;
        xoff += dx;
        ++str;
        // clip at right edge
        if(xoff > xmax) { 
            break; 
        }
    }
    // reg->SetDirty(true);
}

void font_string_region_clip_tab(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    uint8_t* buf = reg->GetData() + xoff + (uint32_t)(reg->GetWidth()) * (uint32_t)yoff;
    uint8_t* max = reg->GetData() + reg->GetLength();
    uint32_t xmax = reg->GetWidth() - 7; // padding
    uint8_t dx = 0;
    while(buf < max) {
        // break on end of string
        if(*str == 0) { break; }
        if(*str == '|') {
            buf = reg->GetData() + TAB_POSITION + (uint32_t)(reg->GetWidth()) * (uint32_t)yoff;
        }    
        else {
            dx = font_glyph(*str, buf, reg->GetWidth(), fg, bg) + 1;
            buf += dx;
            xoff += dx;
        }
        ++str;
        // clip at right edge
        if(xoff > xmax) { 
            break; 
        }
    }
    // reg->SetDirty(true);
}

void font_string_region_clip_right(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    int8_t x = xoff - font_string_pixels(str);
    if(x < 0) x = 0;
    uint8_t* buf = reg->GetData() + x + (uint32_t)(reg->GetWidth()) * (uint32_t)yoff;
    uint8_t* max = reg->GetData() + reg->GetLength();
    uint32_t xmax = reg->GetWidth() - 6; // padding changed from 7 to 6 (only using 4 wide nums anyway)
    uint8_t dx = 0;
    while(buf < max) {
        // break on end of string
        if(*str == 0) { break; }    
        dx = font_glyph(*str, buf, reg->GetWidth(), fg, bg) + 1;
        buf += dx;
        x += dx;
        ++str;
        // clip at right edge
        if(x > xmax) { 
            break; 
        }
    }
    // reg->SetDirty(true);
}
