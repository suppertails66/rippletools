#include "ripple/RippleCmp.h"
#include "nes/NesPattern.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>

using namespace BlackT;

namespace Nes {


void RippleCmp::decmpGrp(BlackT::TStream& ifs, BlackT::TStream& ofs,
                         int numtiles) {
//  TBufStream buffer(decmpBufferSize);
  for (int i = 0; i < numtiles; i++) {
//    buffer.seek(0);
    decmpGrpPlane(ifs, ofs);
    decmpGrpPlane(ifs, ofs);
//    NesPattern pattern;
  }
}

void RippleCmp::decmpGrpWithLimit(BlackT::TStream& ifs, BlackT::TStream& ofs,
                     int endpos) {
  while (ifs.tell() < endpos) {
    decmpGrpPlane(ifs, ofs);
    decmpGrpPlane(ifs, ofs);
  }
}

void RippleCmp::cmpGrp(BlackT::TStream& ifs, BlackT::TStream& ofs) {
  while (!ifs.eof()) {
    cmpGrpPlane(ifs, ofs);
    cmpGrpPlane(ifs, ofs);
    
    ifs.get();
    if (ifs.eof()) break;
    ifs.unget();
  }
}

void RippleCmp::decmpGrpPlane(BlackT::TStream& ifs, BlackT::TStream& ofs) {
  TBufStream buf(planeBufferSize);

  TByte command = ifs.get();
  TByte prev = ifs.get();
  buf.put(prev);
  
  for (int mask = 0x40; mask > 0; mask >>= 1) {
    if ((command & mask) != 0) {
      prev = ifs.get();
    }
    
    buf.put(prev);
  }
  
  if ((command & 0x80) != 0) {
    // un-bitpack
    int getmask = 0x80;
    for (int i = 0; i < 8; i++) {
    
      buf.seek(0);
      TByte value = 0;
      for (int putmask = 0x80; putmask > 0; putmask >>= 1) {
        if ((buf.get() & getmask) != 0) value |= putmask;
      }
      
      ofs.put(value);
      getmask >>= 1;
    }
  }
  else {
    buf.seek(0);
    for (int i = 0; i < 8; i++) {
      ofs.put(buf.get());
    }
  }
}

void RippleCmp::cmpGrpPlane(BlackT::TStream& ifs, BlackT::TStream& ofs) {
  // create stream for source data
  TBufStream src(planeBufferSize);
  src.writeFrom(ifs, 8);
  
  // buffers for compressed content
  TBufStream standardCmpBuf(cmpBufferSize);
  TBufStream packedCmpBuf(cmpBufferSize);
  
  src.seek(0);
  cmpStandard(src, standardCmpBuf);
  src.seek(0);
  cmpPacked(src, packedCmpBuf);
  
  standardCmpBuf.seek(0);
  packedCmpBuf.seek(0);
  if (standardCmpBuf.size() <= packedCmpBuf.size()) {
    ofs.write(standardCmpBuf.data().data(), standardCmpBuf.size());
  }
  else {
    ofs.write(packedCmpBuf.data().data(), packedCmpBuf.size());
  }
}
void RippleCmp::cmpStandard(BlackT::TStream& ifs, BlackT::TStream& ofs) {
  int command = 0x00;
  
  // placeholder for command byte
  ofs.put(0x00);
  TByte prev = ifs.get();
  ofs.put(prev);
  
  for (int mask = 0x40; mask > 0; mask >>= 1) {
    TByte next = ifs.get();
    if (next == prev) {
      
    }
    else {
      ofs.put(next);
      prev = next;
      command |= mask;
    }
  }
  
  // fill in command byte
  ofs.seek(0);
  ofs.writeu8(command);
}

void RippleCmp::cmpPacked(BlackT::TStream& ifs, BlackT::TStream& ofs) {
  int ifspos = ifs.tell();
  int ofspos = ofs.tell();

  // create stream for source data
  TBufStream src(8);
  for (int mask = 0x80; mask > 0; mask >>= 1) {
    // iterate over next 8 bytes
    ifs.seek(ifspos);
    
    // pack all bits of same position into a single byte
    TByte packed = 0x00;
    for (int i = 0; i < 8; i++) {
      packed <<= 1;
      if ((ifs.get() & mask) != 0) packed |= 0x01;
    }
    
    src.put(packed);
  }
  
  // compress as normal
  src.seek(0);
  cmpStandard(src, ofs);
  
  // add packing flag
  ofs.seek(ofspos);
  TByte command = ofs.get();
  command |= 0x80;
  ofs.seek(ofspos);
  ofs.put(command);
}


}
