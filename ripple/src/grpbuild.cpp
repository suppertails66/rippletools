#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TThingyTable.h"
#include "util/TFolderManip.h"
#include "util/TIniFile.h"
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

/*void doBank(TStream& ifs, int ptrTableStart, int numEntries,
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
      indexofs << "src=" << endl;
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
    
    indexofs << "src=" << cmpFilename << endl;
    indexofs << endl;
  }
  
  ripofs << endl;
  buildofs << endl;
} */

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Ripple Island graphics bank builder" << endl;
    cout << "Usage: " << argv[0]
      << " <indexfile> <outfile>" << endl;
    
    return 0;
  }
  
//  TBufStream ifs(1);
//  ifs.open(argv[1]);
//  string outprefix = string(argv[2]);
  TIniFile script = TIniFile(string(argv[1]));
  
  TBufStream files(0x10000);
  vector<int> offsets;
  
  for (SectionKeysMap::const_iterator it = script.cbegin();
       it != script.cend();
       ++it) {
    // iterate over all sections beginning with "Entry"
    string cmpstr = "Entry";
    if (it->first.substr(0, cmpstr.size()).compare(cmpstr) != 0) continue;
    string sectionName = it->first;
    
    string sourceStr;
    
    if (!script.hasKey(sectionName, "source")) {
      offsets.push_back(-1);
      continue;
    }
    sourceStr = script.valueOfKey(sectionName, "source");
    
    if (sourceStr.empty()) {
      offsets.push_back(-1);
      continue;
    }
    
    offsets.push_back(files.tell());
    TBufStream src(1);
    src.open(sourceStr.c_str());
    files.writeFrom(src, src.size());
  }
  
  TBufStream ofs(0x10000);
  
  int dataStart = offsets.size() * 2;
  for (unsigned int i = 0; i < offsets.size(); i++) {
    if (offsets[i] == -1) ofs.writeu16le(0x8000);
    else ofs.writeu16le(offsets[i] + dataStart + 0x8000);
  }
  
  files.seek(0);
  ofs.writeFrom(files, files.size());
  
  if (ofs.size() >= 0x4000) {
    cerr << "Error: built file " << argv[2]
         << " is too large ("
         << TStringConversion::intToString(ofs.size(),
              TStringConversion::baseHex)
         << " bytes, max 0x4000)" << endl;
    return 1;
  }
  
  cout << "Built " << argv[2]
       << " (unpadded size: "
       << TStringConversion::intToString(ofs.size(),
            TStringConversion::baseHex)
       << " bytes)" << endl;
  
//  ofs.alignToWriteBoundary(0x4000);
  
  ofs.save(argv[2]);
  
  return 0;
}
