#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TThingyTable.h"
#include "nes/NesRom.h"
#include "nes/NesPattern.h"
#include "ripple/RippleCmp.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;
using namespace BlackT;
using namespace Nes;

// constants

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Ripple Island graphics compressor" << endl;
    cout << "Usage: " << argv[0]
      << " <infile> <outfile>" << endl;
    
    return 0;
  }
  
/*  TIfstream ifs(argv[1], ios_base::binary);
  ifs.seek(TStringConversion::stringToInt(string(argv[2])));
  int numtiles = TStringConversion::stringToInt(string(argv[3]));
  TOfstream ofs(argv[4], ios_base::binary);
  
  int startPos = ifs.tell();
  cout << "Decompressing graphic at "
       << TStringConversion::intToString(startPos,
                  TStringConversion::baseHex) << endl;
  
  RippleCmp::decmpGrp(ifs, ofs, numtiles);
  
  int endPos = ifs.tell();
  cout << "  Compressed size: "
       << TStringConversion::intToString(endPos - startPos,
                  TStringConversion::baseHex) << " bytes" << endl; */
  
  TIfstream ifs(argv[1], ios_base::binary);
  TOfstream ofs(argv[2], ios_base::binary);
  
  cout << "Compressing " << argv[1] << endl;
  RippleCmp::cmpGrp(ifs, ofs);
  cout << "  Compressed size: " << TStringConversion::intToString(ofs.size(),
                  TStringConversion::baseHex) << endl;
  
  // orig: 5EB
  
  return 0;
}
