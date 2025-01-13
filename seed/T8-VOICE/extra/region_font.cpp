#include "region.h"
#include "font.h"

// Add all Region-dependent font functions here
#define TAB_POSITION 48

void font_string_region_wrap(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    uint8_t* buf = reg->GetData() + xoff + (uint32_t)(reg->GetWidth()) * (uint32_t)yoff;
    uint8_t* max = reg->GetData() + reg->GetLength();
    uint32_t xmax = reg->GetWidth() - 7;
    uint8_t dx = 0;
    while(buf < max) {
        if(*str == 0) { break; }    
        dx = font_glyph(*str, buf, reg->GetWidth(), fg, bg) + 1;
        buf += dx;
        xoff += dx;
        ++str;
        if(xoff > xmax) { 
            xoff = 0;
            buf += (reg->GetWidth() - xoff);
        }
    }
    reg->SetDirty(true);
}

void font_string_region_clip(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    uint8_t* buf = reg->GetData() + xoff + (uint32_t)(reg->GetWidth()) * (uint32_t)yoff;
    uint8_t* max = reg->GetData() + reg->GetLength();
    uint32_t xmax = reg->GetWidth() - 7;
    uint8_t dx = 0;
    
    while(buf < max) {
        if(*str == 0) { break; }    
        dx = font_glyph(*str, buf, reg->GetWidth(), fg, bg) + 1;
        buf += dx;
        xoff += dx;
        ++str;
        if(xoff > xmax) { break; }
    }
}

void font_string_region_clip_tab(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    // ...existing implementation...
}

void font_string_region_clip_right(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    // ...existing implementation...
}

void font_string_region_clip_hid(Region* reg, const char* str, uint8_t xoff, uint8_t yoff,
                                uint8_t fg, uint8_t bg, uint8_t hi, uint8_t hid) {
    // ...existing implementation...
}

void font_string_region_clip_hi(Region* reg, const char* str, uint8_t xoff, uint8_t yoff,
                               uint8_t fg, uint8_t bg, uint8_t hi) {
    font_string_region_clip_hid(reg, str, xoff, yoff, fg, bg, hi, 1);
}
