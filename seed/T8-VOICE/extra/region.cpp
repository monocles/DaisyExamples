#include "region.h"
#include <cstring>
#include "font.h"

// Add TAB_POSITION definition
#define TAB_POSITION 48

Region::Region(uint8_t w, uint8_t h, uint8_t x, uint8_t y)
    : w_(w), h_(h), x_(x), y_(y), dirty_(false) {
    RegionAlloc();
}

Region::~Region() {
    delete[] data_;
}

void Region::RegionAlloc() {
    len_ = w_ * h_;
    data_ = (uint8_t*)malloc(len_);
    
    uint32_t i;
    for(i = 0; i < len_; i++) {
        data_[i] = 0;
    }
    dirty_ = 0;
}

void Region::RegionFill(uint8_t c) {
    uint32_t i;
    uint8_t* p = data_;
    for(i = 0; i < len_; ++i) {
        *p++ = c;
    }
    dirty_ = true;
}

void Region::RegionFillPart(uint32_t start, uint32_t len, uint8_t color) {
    uint32_t i;
    uint8_t* p = data_ + start;
    for(i = 0; i < len; i++) {
        *p++ = color;
    }
    dirty_ = true;
}

void Region::RegionHl(uint8_t c, uint8_t thresh) {
    uint32_t i;
    uint8_t* p = data_;
    for(i = 0; i < len_; i++) {
        if(*p < thresh) {
            *p = c;
        }
        p++;
    }
    dirty_ = true;
}

void Region::RegionMax(uint8_t max) {
    uint32_t i;
    uint8_t* p = data_;
    for(i = 0; i < len_; i++) {
        if(*p > max) {
            *p = max;
        }
        p++;
    }
    dirty_ = true;
}

void Region::RegionString(const char* str, uint8_t x, uint8_t y, uint8_t a, uint8_t b, uint8_t sz) {
    uint32_t bytes = x + ((uint16_t)(w_) * (uint16_t)(y));
    
    switch(sz) {
        case 0:
            font_string(str, data_ + bytes, len_ - bytes, w_, a, b);
            break;
        case 1:
            font_string_big(str, data_ + bytes, len_ - bytes, w_, a, b); 
            break;
        case 2:
            font_string_bigbig(str, data_ + bytes, len_ - bytes, w_, a, b);
            break;
        default:
            font_string(str, data_ + bytes, len_ - bytes, w_, a, b);
            break;
    }
    
    dirty_ = true;
}

void Region::RegionStringWrap(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    uint8_t* buf = data_ + xoff + (uint32_t)(w_) * (uint32_t)yoff;
    uint8_t* max = data_ + len_;
    uint32_t xmax = w_ - 7; // padding
    uint8_t dx = 0;
    
    while(buf < max) {
        // break on end of string
        if(*str == 0) { break; }    
        dx = font_glyph(*str, buf, w_, fg, bg) + 1;
        buf += dx;
        xoff += dx;
        ++str;
        // wrap lines
        if(xoff > xmax) { 
            xoff = 0;
            yoff += 8; // Move down one line
            buf = data_ + xoff + (uint32_t)(w_) * (uint32_t)yoff;
        }
    }
    dirty_ = true;
}

void Region::RegionStringClip(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    uint8_t* buf = data_ + xoff + (uint32_t)(w_) * (uint32_t)yoff;
    uint8_t* max = data_ + len_;
    uint32_t xmax = w_ - 7;
    uint8_t dx = 0;

    while(buf < max) {
        if(*str == 0) { break; }    
        dx = font_glyph(*str, buf, w_, fg, bg) + 1;
        buf += dx;
        xoff += dx;
        ++str;
        if(xoff > xmax) { break; }
    }
    dirty_ = true;
}

void Region::RegionStringClipTab(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    uint8_t* buf = data_ + xoff + (uint32_t)(w_) * (uint32_t)yoff;
    uint8_t* max = data_ + len_;
    uint32_t xmax = w_ - 7; // padding
    uint8_t dx = 0;
    while(buf < max) {
        // break on end of string
        if(*str == 0) { break; }
        if(*str == '|') {
            buf = data_ + TAB_POSITION + (uint32_t)(w_) * (uint32_t)yoff;
        }    
        else {
            dx = font_glyph(*str, buf, w_, fg, bg) + 1;
            buf += dx;
            xoff += dx;
        }
        ++str;
        // wrap lines
        if(xoff > xmax) { 
            return; 
        }
    }
    dirty_ = true;
}

void Region::RegionStringClipRight(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg) {
    int32_t x = xoff - font_string_pixels(str);
    if(x < 0) x = 0;
    uint8_t* buf = data_ + x + (uint32_t)(w_) * (uint32_t)yoff;
    uint8_t* max = data_ + len_;
    uint32_t xmax = w_ - 6; // padding changed from 7 to 6 (only using 4 wide nums anyway)
    uint8_t dx = 0;
    
    // Clear line before drawing text
    RegionFillPart(x + (uint32_t)(w_) * (uint32_t)yoff, w_ * FONT_CHARH, bg);

    while(buf < max) {
        // break on end of string
        if(*str == 0) { break; }    
        dx = font_glyph(*str, buf, w_, fg, bg) + 1;
        buf += dx;
        x += dx;
        ++str;
        // clip at right edge
        if((uint32_t)x > xmax) { 
            return; 
        }
    }
    dirty_ = true;
}

void Region::RegionStringClipHi(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg, uint8_t hi) {
    RegionStringClipHid(str, xoff, yoff, fg, bg, hi, 1);
}

void Region::RegionStringClipHid(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg, uint8_t hi, uint8_t hid) {
    uint8_t* buf = data_ + xoff + (uint32_t)(w_) * (uint32_t)yoff;
    uint8_t* max = data_ + len_;
    uint32_t xmax = w_ - 7;
    uint8_t dx = 0;
    uint8_t i = 0;
    
    while(buf < max) {
        // break on end of string
        if(*str == 0) { break; }
        
        if(i == hi) bg += hid;
        dx = font_glyph(*str, buf, w_, fg, bg) + 1;
        if(i == hi) bg -= hid;
        
        buf += dx;
        xoff += dx;
        ++str;
        
        // clip at right edge
        if(xoff > xmax) {
            return;
        }
        i++;
    }
    dirty_ = true;
}

void Region::RegionDraw(DisplayController& display) {
    if(dirty_) {
        display.DrawRegion(x_, y_, w_, h_, data_);
        dirty_ = false;
    }
}

void Scroll::ScrollInit(Region* reg) {
    reg_ = reg;
    line_h_ = 8; // Font height 
    line_bytes_ = line_h_ * reg->GetWidth();
    line_count_ = reg->GetHeight() / line_h_;
    byte_off_ = 0;
    y_off_ = 0;
    draw_space_ = 0;
    max_byte_off_ = reg->GetWidth() * reg->GetHeight() - line_bytes_;
    reg->RegionFill(0); // Clear region
}

void Scroll::ScrollIncLine() {
    uint32_t byte_off = byte_off_ + line_bytes_;  // Изменено на uint32_t
    uint8_t y_off = y_off_ + line_h_;
    if(byte_off > max_byte_off_) {
        byte_off = 0;
        y_off = 0;
    }
    byte_off_ = byte_off;
    y_off_ = y_off;
}

void Scroll::ScrollDecLine() {
    int32_t byte_off = byte_off_ - line_bytes_;
    int8_t y_off = y_off_ - line_h_;
    if(byte_off < 0) {
        byte_off = max_byte_off_;
        y_off = line_count_ * line_h_;
    }
    byte_off_ = byte_off;
    y_off_ = y_off;
}

void Scroll::ScrollRegionFront(Region* reg) {
    uint8_t* dst = reg_->GetData() + byte_off_;
    uint8_t* src = reg->GetData();
    uint32_t i;

    // copy to current line
    for(i = 0; i < line_bytes_; ++i) {
        *dst = *src;
        ++src;
        ++dst;
    }
    // advance after write
    ScrollIncLine();
    reg_->SetDirty(true);  // Используем публичный метод
}

void Scroll::ScrollRegionBack(Region* reg) {
    uint32_t i;
    uint8_t* dst;
    uint8_t* src = reg->GetData();

    // decrease before write
    ScrollDecLine();
    // setup copy
    dst = reg_->GetData() + byte_off_;
    for(i = 0; i < line_bytes_; ++i) {
        *dst = *src;
        ++dst;
        ++src;
    }
    reg_->SetDirty(true);  // Используем публичный метод
}

void Scroll::ScrollDraw(DisplayController& display) {
    display.DrawRegionOffset(0, 0, reg_->GetWidth(), reg_->GetHeight(),
                           reg_->GetLength(), reg_->GetData(),
                           byte_off_ + draw_space_);
    reg_->SetDirty(false);
}

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
// ... добавить аналогичные реализации для остальных font_string_region_* функций ...
        }
    }
    reg->SetDirty(true);
}

// ... добавить аналогичные реализации для остальных font_string_region_* функций ...

