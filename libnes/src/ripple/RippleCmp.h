#ifndef RIPPLECMP_H
#define RIPPLECMP_H


#include "util/TStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TThingyTable.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace Nes {


class RippleCmp {
public:
  static void decmpGrp(BlackT::TStream& ifs, BlackT::TStream& ofs,
                       int numtiles);
  static void decmpGrpWithLimit(BlackT::TStream& ifs, BlackT::TStream& ofs,
                       int endpos);
  static void cmpGrp(BlackT::TStream& ifs, BlackT::TStream& ofs);
  
  static void decmpGrpPlane(BlackT::TStream& ifs, BlackT::TStream& ofs);
  static void cmpGrpPlane(BlackT::TStream& ifs, BlackT::TStream& ofs);
protected:
  const static int planeBufferSize = 0x10;
  const static int decmpBufferSize = planeBufferSize * 2;
  const static int cmpBufferSize = 0x10;
  
  static void cmpStandard(BlackT::TStream& ifs, BlackT::TStream& ofs);
  static void cmpPacked(BlackT::TStream& ifs, BlackT::TStream& ofs);
};


}


#endif
