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

const static int regionPointerTableOffset = 0x10000;
const static int regionStringTableOffset = 0x10010;
const static int textBank = 4;
const static int numRegions = 8;
const static int endOfScriptDataOffset = 0x13FE8;

const static int dictionaryOffset = 0x1F760;
const static int dictionaryStartIndex = 0xC6;
const static int numDictionaryEntries = (0x100 - dictionaryStartIndex);
const static int maxDictionaryEntryLen = 8;
const static int dictionaryEntryTerminator = 0xFF;
const static int linebreakChar = 0xC5;

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
  return (addr % 0x4000) + (textBank * 0x4000);
}

int absPointerToRel(int ptr) {
  return (ptr % 0x4000) + 0x8000;
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Ripple Island script extractor" << endl;
    cout << "Usage: " << argv[0]
      << " <rom> <table> <outprefix>" << endl;
    
    return 0;
  }
  
  NesRom rom = NesRom(std::string(argv[1]));
  TBufStream ifs(rom.size());
  rom.toStream(ifs);
  
  table.readSjis(std::string(argv[2]));
  
  ifs.seek(regionPointerTableOffset);
  ifs.seek(relPointerToAbs(ifs.readu16le()));
  int firstStringOffset = relPointerToAbs(ifs.readu16le());
  
  ifs.seek(regionPointerTableOffset);
  regionRanges.resize(numRegions);
  regionRanges[numRegions - 1].upper = firstStringOffset;
  for (int i = 0; i < numRegions; i++) {
    regionRanges[i].lower = relPointerToAbs(ifs.readu16le());
  }
  for (int i = 0; i < numRegions - 1; i++) {
    regionRanges[i].upper = regionRanges[i + 1].lower;
  }
  
//  for (int i = 0; i < numRegions; i++) {
//    cout << hex << regionRanges[i].lower << " " << regionRanges[i].upper << endl;
//  }

  // read dictionary
  
  ifs.seek(dictionaryOffset);
  for (int i = 0; i < numDictionaryEntries; i++) {
    int pos = ifs.tell();
    
    std::string entry;
    for (int j = 0; j < maxDictionaryEntryLen; j++) {
      if ((unsigned char)ifs.peek()
            == (unsigned char)dictionaryEntryTerminator) break;
      entry += ifs.get();
    }
    dictionary[i] = entry;
    
    ifs.seek(pos + maxDictionaryEntryLen);
  }
  
  // read strings
  
  int numStrings = ((firstStringOffset - regionStringTableOffset) / 2) - 1;
  regions.resize(numRegions);
  ifs.seek(regionStringTableOffset);
  for (int i = 0; i < numStrings; i++) {
    int regionNum = -1;
    for (unsigned int j = 0; j < regionRanges.size(); j++) {
      if ((ifs.tell() >= regionRanges[j].lower)
          && (ifs.tell() < regionRanges[j].upper)) {
        regionNum = j;
        break;
      }
    }
    
    if (regionNum == -1) {
      cerr << "bad region num at " << ifs.tell() << endl;
      return 1;
    }
  
    int stringPtr = relPointerToAbs(ifs.readu16le());
    int pos = ifs.tell();
    int size = relPointerToAbs(ifs.readu16le()) - stringPtr;
    
    ifs.seek(stringPtr);
    std::string rawstr;
    for (int j = 0; j < size; j++) {
//      rawstr += ifs.get();
      unsigned char next = ifs.get();
      if (next >= dictionaryStartIndex) {
        int index = next - dictionaryStartIndex;
        rawstr += dictionary[index];
      }
      else {
        rawstr += (char)next;
      }
    }
    
    RegionString result;
    result.str = rawstr;
    result.addr = stringPtr;
    regions[regionNum].push_back(result);
    
    // seek to next string pointer
    ifs.seek(pos);
  }
  
  for (int i = 0; i < regions.size(); i++) {
    vector<RegionString>& region = regions[i];
//    cerr << region.size() << endl;
    
    std::ofstream ofs(string(argv[3])
                      + string("region_") + TStringConversion::intToString(i)
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
      int xpos = 0;
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
        ++xpos;
        
        if ((xpos >= 24)) {
          // a manual linebreak at the point where automatic wrapping would
          // occur results in only one linebreak
          if ((k < regstr.str.size() - 1)
              && ((unsigned char)regstr.str[k + 1] == linebreakChar)) {
            
          }
          else {
            ofs << endl;
            ofs << "// ";
            xpos = 0;
          }
        }
        else if ((next == linebreakChar)) {
          ofs << endl;
          ofs << "// ";
          xpos = 0;
        }
      }
      
      ofs << endl << endl << endl;
      ofs << "#END()";
      ofs << endl << endl;
    }
  }
  
  return 0;
}
