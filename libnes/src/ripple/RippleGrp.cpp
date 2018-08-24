#include "ripple/RippleGrp.h"
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


void RippleMetatile::read(BlackT::TStream& ifs) {
  ul = ifs.readu8();
  ll = ifs.readu8();
  ur = ifs.readu8();
  lr = ifs.readu8();
}

void RippleMetatile
    ::toTilemap(NesTilemap& dst, int metatileX, int metatileY) const {
  dst.setData((metatileX * 2) + 0, (metatileY * 2) + 0, ul);
  dst.setData((metatileX * 2) + 1, (metatileY * 2) + 0, ur);
  dst.setData((metatileX * 2) + 0, (metatileY * 2) + 1, ll);
  dst.setData((metatileX * 2) + 1, (metatileY * 2) + 1, lr);
}

void RippleMetatileRow::read(BlackT::TStream& ifs, int numMetatiles) {
  metatileIds.resize(numMetatiles);
  for (int i = 0; i < numMetatiles; i++) {
    metatileIds[i] = ifs.readu8();
  }
}

void RippleMetatileRowSet
    ::read(BlackT::TStream& ifs, int numRows, int width__) {
  width = width__;
  for (int i = 0; i < numRows; i++) {
    RippleMetatileRow row;
    row.read(ifs, width__);
    metatileRows.push_back(row);
  }
}

void RippleMetatileSet
    ::read(BlackT::TStream& ifs, int numMetatiles) {
  metatiles.resize(numMetatiles);
  for (int i = 0; i < numMetatiles; i++) {
    metatiles[i].read(ifs);
  }
}

void RippleMetatileMap
    ::fromRowIdSet(BlackT::TStream& ifs, int numRows,
                  const RippleMetatileRowSet& metatileRowSet) {
  metatileIds.resize(metatileRowSet.width, numRows);
  for (int i = 0; i < numRows; i++) {
    int rownum = ifs.readu8();
    
    if (rownum >= metatileRowSet.metatileRows.size()) {
      throw TGenericException(T_SRCANDLINE,
                              "RippleMetatileMap::fromRowIdSet()",
                              "Out-of-range row index: "
                                 + TStringConversion::intToString(rownum));
    }
    
    const RippleMetatileRow& row = metatileRowSet.metatileRows[rownum];
    for (int j = 0; j < row.metatileIds.size(); j++) {
      metatileIds.data(j, i) = row.metatileIds[j];
    }
  }
}

void RippleMetatileMap
    ::toTilemap(const RippleMetatileSet& metatiles,
                NesTilemap& dst) const {
  dst.resize(metatileIds.w() * 2, metatileIds.h() * 2);
  
  for (unsigned int j = 0; j < metatileIds.h(); j++) {
//    const RippleMetatileRow& row = metatileRows[j];
    
    for (unsigned int i = 0; i < metatileIds.w(); i++) {
//      const RippleMetatile& metatile = row.metatiles[i];
      int id = metatileIds.data(i, j);
      if (id >= metatiles.metatiles.size()) {
        throw TGenericException(T_SRCANDLINE,
                                "RippleMetatileMap::fromRowIdSet()",
                                "Out-of-range metatile ID "
                                   + TStringConversion::intToString(id)
                                   + " at ("
                                   + TStringConversion::intToString(i)
                                   + ", "
                                   + TStringConversion::intToString(j)
                                   + ")");
      }
      
      metatiles.metatiles[id].toTilemap(dst, i, j);
    }
  }
}

/*void RippleMetatileRowSet
    ::toTilemap(NesTilemap& dst) const {
  dst.resize(width * 2, metatileRows.size() * 2);
  
  for (unsigned int j = 0; j < metatileRows.size(); j++) {
    const RippleMetatileRow& row = metatileRows[j];
    
    for (unsigned int i = 0; i < row.metatiles.size(); i++) {
      const RippleMetatile& metatile = row.metatiles[i];
      
      dst.setData((i * 2) + 0, (j * 2) + 0, metatile.ul);
      dst.setData((i * 2) + 1, (j * 2) + 0, metatile.ur);
      dst.setData((i * 2) + 0, (j * 2) + 1, metatile.ll);
      dst.setData((i * 2) + 1, (j * 2) + 1, metatile.lr);
    }
  }
} */


}
