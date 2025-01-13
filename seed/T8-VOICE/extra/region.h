#pragma once
#include <cstdint>
#include "display_controller.h"
#include "font.h"

// Region-specific font function declarations
void font_string_region_clip_hid(Region* reg, const char* str, uint8_t xoff, uint8_t yoff,
                                uint8_t fg, uint8_t bg, uint8_t hi, uint8_t hid);
void font_string_region_clip_hi(Region* reg, const char* str, uint8_t xoff, uint8_t yoff,
                               uint8_t fg, uint8_t bg, uint8_t hi);
void font_string_region_wrap(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, 
                           uint8_t fg, uint8_t bg);
void font_string_region_clip(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, 
                           uint8_t fg, uint8_t bg);
void font_string_region_clip_tab(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, 
                                uint8_t fg, uint8_t bg);
void font_string_region_clip_right(Region* reg, const char* str, uint8_t xoff, uint8_t yoff, 
                                  uint8_t fg, uint8_t bg);

// Add TAB_POSITION definition
#define TAB_POSITION 48

class Region {
public:
    Region() {}
    Region(uint8_t w, uint8_t h, uint8_t x = 0, uint8_t y = 0);
    ~Region();

    // Region manipulation
    void RegionAlloc();
    void RegionFill(uint8_t c);
    void RegionFillPart(uint32_t start, uint32_t len, uint8_t color);
    void RegionHl(uint8_t c, uint8_t thresh);
    void RegionMax(uint8_t max);
    void RegionDraw(DisplayController& display);

    // Text handling
    void RegionString(const char* str, uint8_t x, uint8_t y, 
                     uint8_t a, uint8_t b, uint8_t sz = 0);

    // Font rendering functions
    uint8_t FontGlyph(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);
    uint8_t FontGlyphFixed(char ch, uint8_t* buf, uint8_t w, uint8_t a, uint8_t b);

    // Region text rendering functions
    void RegionStringWrap(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg);
    void RegionStringClip(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg);
    void RegionStringClipTab(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg);
    void RegionStringClipRight(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg);
    void RegionStringClipHi(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg, uint8_t hi);
    void RegionStringClipHid(const char* str, uint8_t xoff, uint8_t yoff, uint8_t fg, uint8_t bg, uint8_t hi, uint8_t hid);

    // Data access
    uint8_t* GetData() { return data_; }
    uint32_t GetLength() const { return len_; }
    uint8_t GetWidth() const { return w_; }
    uint8_t GetHeight() const { return h_; }
    uint8_t GetX() const { return x_; }
    uint8_t GetY() const { return y_; }
    bool IsDirty() const { return dirty_; }
    void ClearDirty() { dirty_ = false; }
    void SetDirty(bool dirty) { dirty_ = dirty; }

private:
    uint8_t w_ = 0;        // width
    uint8_t h_ = 0;        // height
    uint32_t len_ = 0;     // size (stored for speed)
    uint8_t x_ = 0;        // x offset
    uint8_t y_ = 0;        // y offset
    bool dirty_ = false;    // dirty flag
    uint8_t* data_ = nullptr;  // data
};

class Scroll {
public:
    void ScrollInit(Region* reg);
    void ScrollDraw(DisplayController& display);
    void ScrollRegionBack(Region* reg);
    void ScrollRegionFront(Region* reg);

private:
    void ScrollIncLine();
    void ScrollDecLine();

    Region* reg_;
    uint32_t byte_off_;          // current byte offset into region
    uint32_t line_bytes_;        // bytes per text line 
    uint32_t max_byte_off_;      // max offset before wrap
    uint8_t line_h_;            // height of single row
    uint32_t y_off_;            // y-offset in pixels
    uint8_t line_count_;        // lines that fit in region
    uint32_t draw_space_;       // offset for rendering
};

