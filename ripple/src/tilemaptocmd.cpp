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



int main(int argc, char* argv[]) {
  if (argc < 5) {
    cout << "Ripple Island tilemap PPU transfer converter" << endl;
    cout << "Usage: " << argv[0]
      << " <infile> <ppuaddr> <bytespertransfer> <outfile>" << endl;
    
    return 0;
  }
  
  
  
  return 0;
}
