#include "util/io.h"

// Color codes ============
// 0x0 - Black
// 0x1 - Blue
// 0x2 - Green
// 0x3 - Cyan
// 0x4 - Red
// 0x5 - Magenta
// 0x6 - Brown
// 0x7 - White

namespace vgatext{

   static inline void writechar(int loc, uint8_t c, uint8_t bg, uint8_t fg, addr_t base){
     //your code goes here
      mmio::write8(base,2*loc,c);
      mmio::write8(base,2*loc+1,(bg<<4)+fg);
   }

}//namespace vgatext
