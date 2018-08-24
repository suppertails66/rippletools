#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TThingyTable.h"
#include "util/TFolderManip.h"
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

void doBank(TStream& ifs, int ptrTableStart, int numEntries,
            string nameprefix, ostream& ripofs, ostream& buildofs) {
  std::ofstream indexofs(
    (string("rsrc/grp/") + nameprefix + "index.txt").c_str());
  
  for (int i = 0; i < numEntries; i++) {
    ifs.seek(ptrTableStart + (i * 2));
    
    string numstr = TStringConversion::intToString(i);
    while (numstr.size() < 2) numstr = "0" + numstr;
    indexofs << "[Entry"
         << numstr
         << "]" << endl;
    
    int ptr = ifs.readu16le();
    if (ptr == 0x8000) {
      indexofs << "source=" << endl;
      indexofs << endl;
      continue;
    }
    
    int addr = (ptr - 0x8000) + ptrTableStart;
    
    int endpos;
    if (i == (numEntries - 1)) {
      endpos = ptrTableStart + 0x4000;
    }
    else {
      int nextptr;
      do nextptr = ifs.readu16le(); while (nextptr == 0x8000);
      int nextaddr = (nextptr - 0x8000) + ptrTableStart;
      endpos = nextaddr;
    }
    
    ifs.seek(addr);
    TBufStream dummy(0x10000);
    RippleCmp::decmpGrpWithLimit(ifs, dummy, endpos);
    int numTiles = dummy.size() / NesPattern::size;
    
    // hack hack hack
    if (endpos == 0xC000) numTiles = 0xC0;
    
    string rawfilename = string("rsrc/grp/raw/") + nameprefix
      + "grp_" + numstr + ".bin";
    string filename = string("rsrc/grp/") + nameprefix
      + "grp_" + numstr + ".png";
    string buildFilename = string("out/grp/raw/")
      + nameprefix + string("grp_") + numstr + ".bin";
    string cmpFilename = string("out/grp/cmp/")
      + nameprefix + string("grp_") + numstr + ".bin";
    
    ripofs << "./grp_decmp ripple_noheader.nes "
      << TStringConversion::intToString(addr, TStringConversion::baseHex)
      << " "
      << TStringConversion::intToString(numTiles, TStringConversion::baseHex)
      << " "
      << rawfilename << endl;
    ripofs << "./nes_tiledmp " << rawfilename << " 0 "
      << filename << endl;
      
    buildofs << "./nes_tileundmp " << filename
      << " "
      << TStringConversion::intToString(numTiles, TStringConversion::baseHex)
      << " "
      << buildFilename
      << endl;
    buildofs << "./grp_cmp " << buildFilename << " "
      << cmpFilename
      << endl;
    
    indexofs << "source=" << cmpFilename << endl;
    indexofs << endl;
  }
  
  ripofs << endl;
  buildofs << endl;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Ripple Island graphics bank extractor" << endl;
    cout << "Usage: " << argv[0]
      << " <rom> <outprefix>" << endl;
    
    return 0;
  }
  
  TBufStream ifs(1);
  ifs.open(argv[1]);
  string outprefix = string(argv[2]);
  
//  std::ofstream ripofs((outprefix + "extract.sh").c_str());
  std::ofstream ripofs("dumpbankgrp.sh");
  std::ofstream buildofs("undumpbankgrp.sh");
  
  doBank(ifs, 0x00000, 0xB, "bank0/", ripofs, buildofs);
  doBank(ifs, 0x04000, 0xC, "bank1/", ripofs, buildofs);
  doBank(ifs, 0x08000, 0xB, "bank2/", ripofs, buildofs);
//  doBank(ifs, 0x0C000, 0xB, "bank3/", ripofs, buildofs);
  
  // decompress graphics data
//  TBufStream grpRaw(0x10000);
//  ifs.seek(titleGrpAddr);
//  RippleCmp::decmpGrp(ifs, grpRaw, titleGrpNumTiles);
  
  // render tilemap using pattern data
//  TGraphic output(titleMetatileRowWidth * 2 * NesPattern::width,
//                  numTitleMetatileRowIndices * 2 * NesPattern::height);
//  tilemap.drawGrayscale(output, patterns, 0, 0, false);
//  TPngConversion::graphicToRGBAPng(string(argv[2]), output);
  
  return 0;
}
