#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "nes/NesRom.h"
#include "nes/NesPattern.h"
#include <string>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nes;

const static int fontOffset = 0xB3A0;
const static int numFontChars = 0xC6;
const static int charW = 8;
const static int charH = 16;
const static int charsPerRow = 16;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Ripple Island font extractor" << endl;
    cout << "Usage: " << argv[0]
      << " <rom> <outfile>" << endl;
    
    return 0;
  }
  
  NesRom rom = NesRom(std::string(argv[1]));
  TBufStream ifs(rom.size());
  rom.toStream(ifs);
  
  int outW = charW * charsPerRow;
  int outH = numFontChars / charsPerRow;
  if ((numFontChars % charsPerRow) != 0) ++outH;
  outH *= charH;
  
  TGraphic g(outW, outH);
  g.clearTransparent();
  
  ifs.seek(fontOffset);
  for (int i = 0; i < numFontChars; i++) {
    
    for (int j = 0; j < charH; j++) {
    
      int mask = 0x80;
      int value = ifs.readu8();
      for (int k = 0; k < charW; k++) {
        int x = ((i % charsPerRow) * charW) + k;
        int y = ((i / charsPerRow) * charH) + j;
        
        if ((mask & value) != 0) {
          g.setPixel(x, y, TColor(0xFF, 0xFF, 0xFF, TColor::fullAlphaOpacity));
        }
        else {
          g.setPixel(x, y, TColor(0x00, 0x00, 0x00, TColor::fullAlphaOpacity));
        }
        
        mask >>= 1;
      }
      
    }
    
  }
  
  TPngConversion::graphicToRGBAPng(std::string(argv[2]), g);
  
  return 0;
}
