#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TThingyTable.h"
#include "nes/NesPattern.h"
#include "nes/NesPatternTable.h"
#include "nes/NesTilemap.h"
#include "ripple/RippleCmp.h"
#include "ripple/RippleGrp.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;
using namespace BlackT;
using namespace Nes;

const static int titleGrpAddr = 0x142A4;
const static int titleGrpNumTiles = 0xA4;
const static int titleGrpLoadTile = 0x40;

const static int titleMetatileSetAddr = 0x1854A;
const static int numTitleMetatiles = 0x4A;

const static int titleMetatileRowDefsAddr = 0x18672;
const static int numTitleMetatileRowDefs = 0x10;
const static int titleMetatileRowWidth = 0x10;

const static int titleMetatileRowIndexAddr = 0x187C2;
const static int numTitleMetatileRowIndices = 0x0F;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Ripple Island title screen" << endl;
    cout << "Usage: " << argv[0]
      << " <rom> <outfile>" << endl;
    
    return 0;
  }
  
  TBufStream ifs(1);
  ifs.open(argv[1]);
  
  // decompress graphics data
  TBufStream grpRaw(0x10000);
  ifs.seek(titleGrpAddr);
  RippleCmp::decmpGrp(ifs, grpRaw, titleGrpNumTiles);
  
  // convert decompressed graphics data to NesPatterns and load to pattern
  // table
  NesPatternTable patterns;
  grpRaw.seek(0);
  for (int i = 0; i < titleGrpNumTiles; i++) {
    NesPattern pattern;
    pattern.read(grpRaw);
    patterns.tile(titleGrpLoadTile + i) = pattern;
  }
  
  // read metatiles
  RippleMetatileSet metatileSet;
  ifs.seek(titleMetatileSetAddr);
  metatileSet.read(ifs, numTitleMetatiles);
  
  // read metatile row definitions
  RippleMetatileRowSet metatileRowDefs;
  ifs.seek(titleMetatileRowDefsAddr);
  metatileRowDefs.read(ifs, numTitleMetatileRowDefs, titleMetatileRowWidth);
  
  // read title metatile map
  RippleMetatileMap metatileMap;
  ifs.seek(titleMetatileRowIndexAddr);
  metatileMap.fromRowIdSet(ifs, numTitleMetatileRowIndices, metatileRowDefs);
  
  // generate tilemap
  NesTilemap tilemap;
  metatileMap.toTilemap(metatileSet, tilemap);
  
  // temp
//  TOfstream ofs(argv[2], ios_base::binary);
//  tilemap.write(ofs);
  
  // render tilemap using pattern data
  TGraphic output(titleMetatileRowWidth * 2 * NesPattern::width,
                  numTitleMetatileRowIndices * 2 * NesPattern::height);
  tilemap.drawGrayscale(output, patterns, 0, 0, false);
  TPngConversion::graphicToRGBAPng(string(argv[2]), output);
  
  return 0;
}
