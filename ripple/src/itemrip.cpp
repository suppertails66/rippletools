#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TThingyTable.h"
#include "nes/NesRom.h"
#include "nes/NesPattern.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;
using namespace BlackT;
using namespace Nes;

// constants

//const static int regionPointerTableOffset = 0x10000;
//const static int regionStringTableOffset = 0x10010;
//const static int textBank = 4;
//const static int numItemPointerTableEntries = 8;
//const static int endOfScriptDataOffset = 0x13FE8;

const static int dictionaryOffset = 0x1F760;
const static int dictionaryStartIndex = 0xC6;
const static int numDictionaryEntries = (0x100 - dictionaryStartIndex);
const static int maxDictionaryEntryLen = 8;
const static int dictionaryEntryTerminator = 0xFF;
const static int linebreakChar = 0xC5;

const static int itemPointerTableOffset = 0x1EF94;
const static int itemListStart = 0x1EF9E;
const static int numItemPointerTableEntries = 5;
const static int maxItemListEntryLen = 8;
const static int itemListBank = 7;
const static int itemListTerminator = 0xFF;

struct RegionRange {
  int lower;
  int upper;
};

struct RegionString {
  int addr;
  string str;
};

// globals

map<int, string> dictionary;
vector<RegionRange> regionRanges;
TThingyTable table;
vector< vector<RegionString> > regions;

// functions

int relPointerToAbs(int addr) {
  return (addr % 0x4000) + (itemListBank * 0x4000);
}

int absPointerToRel(int ptr) {
  return (ptr % 0x4000) + 0xC000;
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Ripple Island item name extractor" << endl;
    cout << "Usage: " << argv[0]
      << " <rom> <table> <outprefix>" << endl;
    
    return 0;
  }
  
  NesRom rom = NesRom(std::string(argv[1]));
  TBufStream ifs(rom.size());
  rom.toStream(ifs);
  
  table.readSjis(std::string(argv[2]));
  
  ifs.seek(itemPointerTableOffset);
//  ifs.seek(relPointerToAbs(ifs.readu16le()));
  
  regionRanges.resize(numItemPointerTableEntries);
  regionRanges[numItemPointerTableEntries - 1].upper = 0x1F07A;
  for (int i = 0; i < numItemPointerTableEntries; i++) {
    regionRanges[i].lower = relPointerToAbs(ifs.readu16le());
  }
  for (int i = 0; i < numItemPointerTableEntries - 1; i++) {
    regionRanges[i].upper = regionRanges[i + 1].lower;
  }
  
//  for (int i = 0; i < numItemPointerTableEntries; i++) {
//    cout << hex << regionRanges[i].lower << " " << regionRanges[i].upper << endl;
//  }
  
  // read strings
  
//  int numStrings = ((firstStringOffset - regionStringTableOffset) / 2) - 1;
  regions.resize(numItemPointerTableEntries);
  for (int i = 0; i < regionRanges.size(); i++) {
    int regionNum = i;
    
    if (regionNum == -1) {
      cerr << "bad region num at " << ifs.tell() << endl;
      return 1;
    }
    
    ifs.seek(regionRanges[i].lower);
    int pos = ifs.tell();
    
    while ((unsigned char)ifs.peek() != itemListTerminator) {
      std::string rawstr;
      for (int j = 0; j < maxItemListEntryLen; j++) {
  //      rawstr += ifs.get();
        unsigned char next = ifs.get();
        if (next == itemListTerminator) break;
        else rawstr += (char)next;
      }
      
      RegionString result;
      result.str = rawstr;
      result.addr = pos;
      regions[regionNum].push_back(result);
      
      pos += maxItemListEntryLen;
      ifs.seek(pos);
    }
    
    // seek to next string pointer
    ifs.seek(pos);
  }
  
  for (int i = 0; i < regions.size(); i++) {
    vector<RegionString>& region = regions[i];
//    cerr << region.size() << endl;
    
    std::ofstream ofs(string(argv[3])
                      + string("items_") + TStringConversion::intToString(i)
                      + ".txt",
                      ios_base::binary);
    for (int j = 0; j < region.size(); j++) {
      RegionString& regstr = region[j];
      
      ofs << "// String " << i << "-" << j << " ("
        << TStringConversion::intToString(regstr.addr,
                  TStringConversion::baseHex)
        << ")"
        << endl;
      ofs << "// ";
//      int xpos = 0;
      for (unsigned int k = 0; k < regstr.str.size(); k++) {
        int next = (unsigned char)(regstr.str[k]);
        
        if (!table.hasEntry(next)) {
          cerr << "No match for character "
            << TStringConversion::intToString(next,
                  TStringConversion::baseHex)
            << " in string at "
            << TStringConversion::intToString(regstr.addr,
                  TStringConversion::baseHex)
            << " (pos: "
            << TStringConversion::intToString(regstr.addr + k,
                  TStringConversion::baseHex)
            << ")"
            << endl;
          return 1;
        }
        
        ofs << table.getEntry(next);
//        ++xpos;
        
//        if ((next == linebreakChar)
//            || (xpos >= 24)) {
//          ofs << endl;
//          xpos = 0;
//        }
      }
      
      ofs << endl << endl << endl;
      ofs << "#END()";
      ofs << endl << endl;
    }
    
    // terminator
    ofs << "// List terminator" << endl;
    ofs << "<$FF>";
    ofs << endl << endl;
    ofs << "#END()";
    ofs << endl << endl;
  }
  
  return 0;
}
