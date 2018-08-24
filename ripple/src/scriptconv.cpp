#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TGraphic.h"
#include "util/TStringConversion.h"
#include "util/TPngConversion.h"
#include "util/TCsv.h"
#include "util/TThingyTable.h"
#include "nes/NesPattern.h"
#include "ripple/RippleScriptReader.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nes;

const static int numScriptRegions = 8;
const static int splitPoint = 3;

const static int baseAddr = 0x8000;
  
std::vector<RippleScriptReader::ResultCollection> regions;

void generateRegionSubset(int start, int end, TStream& ofs) {
  // skip region pointer table to fill in later
  int regionTablePos = ofs.tell();
  ofs.seekoff(numScriptRegions * 2);
  
  // compute local offsets of all strings
  
  std::vector< std::vector<int> > regionStringOffsets;
  regionStringOffsets.resize(numScriptRegions);
    
  int stringOffset = 0;
  int stringPointerCount = 0;
  for (int i = 0; i < numScriptRegions; i++) {
    if ((i < start) || (i >= end)) {
      continue;
    }
  
//    TBufStream stringOfs(0x10000);
    RippleScriptReader::ResultCollection& srcStrings
      = regions[i];
    
    std::vector<int>& rawStringOffsets = regionStringOffsets[i];
//    rawStringOffsets.resize(srcStrings.size() + 1);
    rawStringOffsets.resize(srcStrings.size());
    
    // dummy zero entry
//    rawStringOffsets[0] = stringOffset;
//    ++stringPointerCount;
    
    for (unsigned int j = 0; j < srcStrings.size(); j++) {
//      rawStringOffsets[j + 1] = stringOffset;
      rawStringOffsets[j] = stringOffset;
      stringOffset += srcStrings[j].str.size();
      
      ++stringPointerCount;
    }
  }
  
  // table of region start offsets
  
  std::vector<int> stringTableOffsets;
  stringTableOffsets.resize(numScriptRegions);
  
  // write string offset tables
  
  ofs.seek(numScriptRegions * 2);
  for (int i = 0; i < numScriptRegions; i++) {
    if ((i < start) || (i >= end)) {
//      stringTableOffsets[i] = 0xFFFF;
      stringTableOffsets[i] = ofs.tell();
      continue;
    }
    
    stringTableOffsets[i] = ofs.tell();
    std::vector<int>& stringOffsets = regionStringOffsets[i];
    
    for (unsigned int j = 0; j < stringOffsets.size(); j++) {
      // write string offset
      ofs.writeu16le(stringOffsets[j]
                     + baseAddr
                     + (numScriptRegions * 2)
                     + (stringPointerCount * 2));
    }
  }
  
  // write strings
  
  for (int i = 0; i < numScriptRegions; i++) {
    if ((i < start) || (i >= end)) {
      continue;
    }
    
//    std::cerr << regionStringOffsets[0].size() << endl;
    
    RippleScriptReader::ResultCollection& srcStrings
      = regions[i];
    
    for (unsigned int j = 0; j < srcStrings.size(); j++) {
      for (unsigned int k = 0; k < srcStrings[j].str.size(); k++) {
        ofs.put(srcStrings[j].str[k]);
      }
    }
  }
  
  // write region start offset table
  
  ofs.seek(regionTablePos);
  for (int i = 0; i < numScriptRegions; i++) {
    if ((i < start) || (i >= end)) {
//      ofs.writeu16le(0xFFFF);
      ofs.writeu16le(stringTableOffsets[i]
                     + baseAddr);
      continue;
    }
    
//    for (unsigned int j = 0; j < stringTableOffsets.size(); j++) {
      ofs.writeu16le(stringTableOffsets[i]
                     + baseAddr);
//    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Ripple Island script converter" << endl;
    cout << "Usage: " << argv[0]
      << " <inprefix> <table> <outprefix>" << endl;
    
    return 0;
  }
  
  std::string prefix = std::string(argv[1]);
  std::string outprefix = std::string(argv[3]);
  
  TThingyTable table;
//  table.readUtf8(string(argv[2]));
  table.readSjis(string(argv[2]));
  
  regions.resize(numScriptRegions);
  
  for (int i = 0; i < numScriptRegions; i++) {
    TIfstream ifs((prefix + "region_" + TStringConversion::intToString(i)
                    + ".txt").c_str(), ios_base::binary);
    
    RippleScriptReader(ifs, regions[i], table)();
  }
  
  TBufStream ofs1(0x10000);
  TBufStream ofs2(0x10000);
  TBufStream ofs3(0x10000);
  TBufStream ofs4(0x10000);
  
//  generateRegionSubset(0, splitPoint, ofs1);
//  generateRegionSubset(splitPoint, numScriptRegions, ofs2);
  generateRegionSubset(0, 2, ofs1);
  generateRegionSubset(2, 3, ofs2);
  generateRegionSubset(3, 5, ofs3);
  generateRegionSubset(5, numScriptRegions, ofs4);
  
  ofs1.save((outprefix + "script_0.bin").c_str());
  ofs2.save((outprefix + "script_1.bin").c_str());
  ofs3.save((outprefix + "script_2.bin").c_str());
  ofs4.save((outprefix + "script_3.bin").c_str());
  
/*  TOfstream ofs(argv[3], ios_base::binary);
  for (RippleScriptReader::ResultCollection::iterator it = results.begin();
       it != results.end();
       ++it) {
    string result = it->str;
    for (unsigned int i = 0; i < result.size(); i++) {
      ofs.put(result[i]);
    }
  } */
  
  return 0;
}
