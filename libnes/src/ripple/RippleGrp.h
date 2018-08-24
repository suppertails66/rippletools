#ifndef RIPPLEGRP_H
#define RIPPLEGRP_H


#include "util/TStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TThingyTable.h"
#include "util/TTwoDArray.h"
#include "nes/NesTilemap.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace Nes {


class RippleMetatile {
public:
  BlackT::TByte ul;
  BlackT::TByte ll;
  BlackT::TByte ur;
  BlackT::TByte lr;
  
  void read(BlackT::TStream& ifs);
  void toTilemap(NesTilemap& dst, int metatileX, int metatileY) const;
};

typedef std::vector<RippleMetatile> RippleMetatileCollection;

class RippleMetatileSet {
public:
  RippleMetatileCollection metatiles;
  
  void read(BlackT::TStream& ifs, int numMetatiles);
};

class RippleMetatileRow {
public:
//  RippleMetatileCollection metatiles;
  std::vector<int> metatileIds;
  
  void read(BlackT::TStream& ifs, int numMetatiles);
};

typedef std::vector<RippleMetatileRow> RippleMetatileRowCollection;

class RippleMetatileRowSet {
public:
  RippleMetatileRowCollection metatileRows;
  int width;
  
  void read(BlackT::TStream& ifs, int numRows, int width__);
//  void toTilemap(NesTilemap& dst) const;
};

class RippleMetatileMap {
public:
  BlackT::TTwoDArray<int> metatileIds;
  
  void fromRowIdSet(BlackT::TStream& ifs, int numRows,
                    const RippleMetatileRowSet& metatileRowSet);
  void toTilemap(const RippleMetatileSet& metatiles,
                 NesTilemap& dst) const;
};


}


#endif
