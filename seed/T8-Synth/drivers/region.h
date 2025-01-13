/* regions.h 
   
   draw and otherwise manipulate arbitrary grids of pixels.

 */

#ifndef _REGION_H_
#define _REGION_H_

#include <cstdint>

// Forward declaration
class DisplayController;
extern DisplayController display;

// data type for screen regions
typedef struct _region { 
  // width
  uint8_t w;
  // height
  uint8_t h;
  // size (store for speed)
  uint32_t len;
  // x offset
  uint8_t x;
  // y offset
  uint8_t y;  
  // dirty flag
  uint8_t dirty;
  // data
  uint8_t * data;
} region;

// datatype for a text scroller
typedef struct _scroll {
  // pointer to region
  region* reg;
  // current byte offset into region
  uint32_t byteOff;
  // pixels per line of text
  uint32_t lineBytes;
  // max byte offset before wrapping
  uint32_t maxByteOff;
  // height of a single row of text
  uint8_t lineH;
  // current line offset
  //  uint8_t lineOff;
  // faster to use y-offset in px
  uint32_t yOff;
  // how many lines of text will fit in the region
  uint8_t lineCount; 
  // offset for actual rendering:
  // if zero(default), most recent text on last line
  uint32_t drawSpace;
} scroll;

// allocate and initialize a text scroller
extern void scroll_init(scroll* scr, region* reg);
// draw text to front of scroll
extern void scroll_string_front(scroll* scr, char* str);
// draw text to back of scroll
extern void scroll_string_back(scroll* scr, char* str);
// draw pixels to front of scroll
/// assumes data has correct dimensions!
extern void scroll_region_front(scroll* scr, region* reg);
// draw pixel to back of scroll
/// assumes data has correct dimensions!
extern void scroll_region_back(scroll* scr, region* reg);

// draw scroll to screen
extern void scroll_draw(scroll* scr);


// allocate and initialize a screen region
extern void region_alloc(region* reg);

/// ha
/*  void region_free(region* reg) */

// render a string to a region with offset, using system font
extern void region_string(
		   region* reg,	 // region
		   const char* str,// string
		   uint8_t x, uint8_t y, 	 // offset
		   uint8_t a, uint8_t b, 	 // colors
		   uint8_t sz);  // size levels (dimensions multiplied by 2**sz)

// render a string to a region using the default anti-aliased font.
// extern void region_string_aa(
// 		   region* reg,	 // region
// 		   const char* str,// string
// 		   uint8_t x, uint8_t y, 	 // offset
// 		   uint8_t inv ); // inversion flag
 
// fill a region with given color
extern void region_fill(region* reg, uint8_t c);
// fill a contiguous portion of a region with given color
extern void region_fill_part(region* reg, uint32_t start, uint32_t len, uint8_t color );

/// copy-or
  

// hilight a region with given color and threshold
extern void region_hl(region* reg, uint8_t c, uint8_t thresh);

// limit the value in a region (dimming it)
extern void region_max(region* reg, uint8_t max);

// draw region to screen
extern void region_draw(region* reg);
 



#endif // h guard
