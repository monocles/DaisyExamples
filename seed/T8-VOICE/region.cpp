// #include "print_funcs.h"

#include "font.h"
#include "display_controller.h"
#include "region.h"

// Global display controller instance
extern DisplayController display;

///=================
///===== static

// increment scroll line
static void scroll_inc_line(scroll* scr) {
  int32_t byteoff = scr->byteOff + scr->lineBytes;
  int8_t yoff = scr->yOff + FONT_CHARH;
  if(byteoff > scr->maxByteOff) {
    byteoff = 0;
    yoff = 0;
  }
  scr->byteOff = byteoff;
  scr->yOff = yoff;
}

// decrement scroll line 
static void scroll_dec_line(scroll* scr) {
  int32_t byteoff = scr->byteOff - scr->lineBytes;
  int8_t yoff = (int8_t)(scr->yOff) - (int8_t)FONT_CHARH;
  if(byteoff < 0) {
    byteoff = scr->maxByteOff;
    yoff = scr->lineCount * FONT_CHARH;
  }
  scr->byteOff = byteoff;
  scr->yOff = yoff;
}

//=========================
//==== extern

// allocate buffer
void region_alloc(region* reg) {
  uint32_t i;
  reg->len = reg->w * reg->h;
  // reg->data = (uint8_t*)alloc_mem(reg->len);
  reg->data = (uint8_t*)malloc(reg->len);
  
  //  print_dbg("\r\n zeroing region data... ");
  for(i=0; i<reg->len; i++) {
    reg->data[i] = 0; 
  }
  reg->dirty = 0;
}

/*  void region_free(region* reg) { */
/*   //... haha */
/* } */

// render a string to a region with offset
void region_string(
		   region* reg,	 // region
		   const char* str,// string
		   uint8_t x, uint8_t y, 	 // offset within region
		   uint8_t a, uint8_t b, 	 // colors
		   uint8_t sz)  // size levels (dimensions multiplied by 2**sz)
{
  uint32_t bytes = x + ((uint16_t)(reg->w) * (uint16_t)(y));
  if(sz == 0) {
    font_string(str, reg->data + bytes, reg->len - bytes, reg->w, a, b);
  } else if (sz == 1) {
    font_string_big(str, reg->data + bytes, reg->len - bytes, reg->w, a, b);
  } else if (sz == 2) {
    font_string_bigbig(str, reg->data + bytes, reg->len - bytes, reg->w, a, b);
  }
  reg->dirty = 1;
}


// render a string to a region using the default anti-aliased font.
// void region_string_aa(
// 		   region* reg,	 // region
// 		   const char* str,// string
// 		   uint8_t x, uint8_t y, 	 // offset
// 		   uint8_t inv ) // inversion flag
 
// {
//   font_string_aa(str, reg->data + (uint32_t)reg->w * (uint32_t)y + (uint32_t)x, reg->len, reg->w, inv);
//   reg->dirty = 1;
// }


// fill a region with given color
void region_fill(region* reg, uint8_t c) {
  uint32_t i;
  for(i=0; i<reg->len; ++i) {
    reg->data[i] = c; 
  }
  reg->dirty = 1;
}

// fill a contiguous portion of a region with given color
extern void region_fill_part(region* reg, uint32_t start, uint32_t len, uint8_t color) {
 uint32_t i;
 uint8_t* p = (reg->data) + start;
  for(i=0; i<len; i++) {
    *p++ = color;
  }
  reg->dirty = 1;
}


// hilight a region with given color and threshold
void region_hl(region* reg, uint8_t c, uint8_t thresh) {
  uint32_t i;
  for(i=0; i<reg->len; i++) {
    if ( reg->data[i] < thresh) { reg->data[i] = c; }
  }
  reg->dirty = 1;
}

// limit the value in a region (dimming it)
void region_max(region* reg, uint8_t max) {
  uint32_t i;
  for(i=0; i<reg->len; i++) {
    if ( reg->data[i] > max) { reg->data[i] = max; }
  }
  reg->dirty = 1;
}


///---- scrolling stuff

// initialize a text scroller at memory
extern void scroll_init(scroll* scr, region* reg) {
  scr->reg = reg;
  scr->lineCount = 0;
  scr->yOff = 0;
  scr->byteOff = 0;
  scr->lineBytes = FONT_CHARH * reg->w;
  scr->lineCount = reg->h / FONT_CHARH;
  scr->drawSpace = 0;
  // offset of last row
  scr->maxByteOff = reg->w * reg->h - scr->lineBytes;
  region_fill(reg, 0x0);
}

// render text to front of scroll
extern void scroll_string_front(scroll* scr, char* str) {
  /// clear current line
  region_fill_part(scr->reg, scr->byteOff, scr->lineBytes, 0x0);
  // draw text to region at current offset, using system font
  region_string(scr->reg, str,
		0, scr->yOff, 0xf, 0, 0);
  // advance after writexs
  scroll_inc_line(scr);
  scr->reg->dirty = 1;
}

// render text to back of scroll
extern void scroll_string_back(scroll* scr, char* str) {
  // advance before write
  scroll_dec_line(scr);
  // draw text to region at new offset, using system font
  region_string(scr->reg, str,
		0, scr->yOff, 0xf, 0, 0);
  //// render happens separately,
  //  so we can e.g. trigger from timer based on dirty flag
  scr->reg->dirty = 1;
}


// copy 1 line worth of bytes from region to front of scroll
void scroll_region_front(scroll* scr, region* reg) {
  uint8_t* dst = scr->reg->data + scr->byteOff;
  uint8_t* src = reg->data;
  //  int32_t byteOff;
  uint32_t i;
    // copy to current line
  for(i=0; i<scr->lineBytes; ++i) {
    *dst = *src;
    ++src;
    ++dst;
  }
  // advance after write
  scroll_inc_line(scr);
  scr->reg->dirty = 1;
}

// copy region to back of scroll
// region should be the correct width!
void scroll_region_back(scroll* scr, region* reg) {
  //  int32_t byteoff;
  uint32_t i;
  uint8_t* dst;
  uint8_t* src = reg->data;
  // decrease before write
  scroll_dec_line(scr);
  // setup copy
  dst = scr->reg->data + scr->byteOff;
  for(i=0; i<scr->lineBytes; ++i) {
    *dst = *src;
    ++dst;
    ++src;
  }

  scr->reg->dirty = 1;
}

// draw scroll to screen
extern void scroll_draw(scroll* scr) {
  display.DrawRegionOffset(0, 0, scr->reg->w, scr->reg->h, scr->reg->len, 
                        scr->reg->data, scr->byteOff + scr->drawSpace);
  scr->reg->dirty = 0;
}

// draw region to screen
extern void region_draw(region* r) {
  if(r->dirty) {
    display.DrawRegion(r->x, r->y, r->w, r->h, r->data);
    r->dirty = 0;
  }
}
