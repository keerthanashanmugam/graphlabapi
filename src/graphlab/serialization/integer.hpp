#ifndef INTEGER_HPP
#define INTEGER_HPP
#include <stdint.h>
#include <graphlab/logger/assertions.hpp>
namespace graphlab {
inline unsigned char compress_int(uint64_t u, char output[10]) {
  // if 1st bit of u is set. could be negative,
  // flip all the bits if it is
  uint64_t isneg = (int64_t)(u) >> 63;
  // if first bit of u is set, isneg = -1.......
  // otherwise isneg = 0....
  u = (u ^ isneg) - isneg;
  
  // get the largest bit
  int nbits = 0;
  if (u != 0) nbits = (64 - __builtin_clzll(u));
  
  //std::cout << (int)nbits << "\t"; 
  //std::cout.flush();
  // divide by 7. quickly
  // this is the number of bytes I need. The second one is actually rather ingenious
  // "lookup table" faster than "magic number" faster than "division"
  //unsigned char c = (nbits + 6) / 7 + (nbits == 0);
  //unsigned char c = (nbits >> 3) + 1 + ((0xfefcf8f0e0c08000ULL & (1ULL << nbits)) > 0);
  static const unsigned char lookuptable[64] = {1,1,1,1,1,1,1,1,
                                                2,2,2,2,2,2,2,
                                                3,3,3,3,3,3,3,
                                                4,4,4,4,4,4,4,
                                                5,5,5,5,5,5,5,
                                                6,6,6,6,6,6,6,
                                                7,7,7,7,7,7,7,
                                                8,8,8,8,8,8,8,
                                                9,9,9,9,9,9,9};
  unsigned char c = lookuptable[nbits];

  switch(c) {
    case 9:
      output[1] = (u & (0x7FLL << 56)) >> 56;
    case 8:
      output[2] = (u & (0x7FLL << 49)) >> 49;
    case 7:
      output[3] = (u & (0x7FLL << 42)) >> 42;
    case 6:
      output[4] = (u & (0x7FLL << 35)) >> 35;
    case 5:
      output[5] = (u & (0x7FLL << 28)) >> 28;
    case 4:
      output[6] = (u & (0x7FLL << 21)) >> 21; 
    case 3:
      output[7] = (u & (0x7FLL << 14)) >> 14;
    case 2:
      output[8] = (u & (0x7FLL << 7)) >> 7;
    case 1:
      output[9] = (u & 0x7F) | 0x80; // set the bit in the least significant 
  }
  
  if (isneg == 0) {
    return c;
  }
  else {
    output[9 - c] = 0;
    return c + 1;
  }
}



template <typename IntType>
inline void decompress_int(char* arr, IntType &ret) {
  // if 1st bit of u is set. could be negative,
  // flip all the bits if it is
  bool isneg = (arr[0] == 0);
  if (isneg) ++arr;
  
  ret = 0;
  while(1) {
    ret = (ret << 7) | ((*arr) & 0x7F);
    if ((*arr) & 0x80) break;
    ++arr;
  };
  if (isneg)  ret = -ret;
}

template <typename IntType>
inline void decompress_int(std::istream &strm, IntType &ret) {
  // if 1st bit of u is set. could be negative,
  // flip all the bits if it is
  char c = strm.peek();
  bool isneg = (c == 0);
  if (isneg) strm.get(c);
  
  ret = 0;
  while(1) {
    strm.get(c);
    ret = (ret << 7) | (c & 0x7F);
    if (c & 0x80) break;
  };
  if (isneg)  ret = -ret;
}

}



#endif
