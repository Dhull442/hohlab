#pragma once

#include "util/io.h"

namespace serial{

  static inline bool is_transmitter_ready(io_t baseport){
    //insert your code here
    uint8_t lr = io::read8(baseport,5);
    return ((lr & 32) == 32);
  }

  static inline void writechar(uint8_t c, io_t baseport){
    //insert your code here
    io::write8(baseport, 0, c);
  }

} //end serial
