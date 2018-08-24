#include "ripple/RippleScriptReader.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>

using namespace BlackT;

namespace Nes {


const static int scriptBufferCapacity = 0x10000;

RippleScriptReader::RippleScriptReader(
                  BlackT::TStream& src__,
                  ResultCollection& dst__,
                  const BlackT::TThingyTable& thingy__)
  : src(src__),
    dst(dst__),
    thingy(thingy__),
    lineNum(0),
    currentScriptBuffer(scriptBufferCapacity) {
  loadThingy(thingy__);
//  spaceOfs.open((outprefix + "msg_space.txt").c_str());
//  indexOfs.open((outprefix + "msg_index.txt").c_str());
}

void RippleScriptReader::operator()() {
  while (!src.eof()) {
    std::string line;
    src.getLine(line);
    ++lineNum;
    
    if (line.size() <= 0) continue;
    
    // discard lines containing only ASCII spaces and tabs
    bool onlySpace = true;
    for (int i = 0; i < line.size(); i++) {
      if ((line[i] != ' ')
          && (line[i] != '\t')) {
        onlySpace = false;
        break;
      }
    }
    if (onlySpace) continue;
    
    TBufStream ifs(line.size());
    ifs.write(line.c_str(), line.size());
    ifs.seek(0);
    
    // check for special stuff
    if (ifs.peek() == '#') {
      // directives
      ifs.get();
      processDirective(ifs);
      continue;
    }
    
    while (!ifs.eof()) {
      // check for comments
      if ((ifs.remaining() >= 2)
          && (ifs.peek() == '/')) {
        ifs.get();
        if (ifs.peek() == '/') break;
        else ifs.unget();
      }
      
      outputNextSymbol(ifs);
    }
  }
}
  
void RippleScriptReader::loadThingy(const BlackT::TThingyTable& thingy__) {
  thingy = thingy__;

  // To efficiently unmap script values, we order the thingy's strings
  // by length
/*  thingiesBySize.clear();
  thingiesBySize.resize(thingy.entries.size());
  int pos = 0;
  for (TThingyTable::RawTable::iterator it = thingy.entries.begin();
       it != thingy.entries.end();
       ++it) {
    ThingyValueAndKey t = { it->second, it->first };
    thingiesBySize[pos++] = t;
  }
  std::sort(thingiesBySize.begin(), thingiesBySize.end()); */
}
  
void RippleScriptReader::outputNextSymbol(TStream& ifs) {
  // literal value
  if ((ifs.remaining() >= 5)
      && (ifs.peek() == '<')) {
    int pos = ifs.tell();
    
    ifs.get();
    if (ifs.peek() == '$') {
      ifs.get();
      std::string valuestr = "0x";
      valuestr += ifs.get();
      valuestr += ifs.get();
      
      if (ifs.peek() == '>') {
        ifs.get();
        int value = TStringConversion::stringToInt(valuestr);
        
//        dst.writeu8(value);
        currentScriptBuffer.writeu8(value);

        return;
      }
    }
    
    // not a literal value
    ifs.seek(pos);
  }
  
  

/*  for (int i = 0; i < thingiesBySize.size(); i++) {
    if (checkSymbol(ifs, thingiesBySize[i].value)) {
      int symbolSize;
      if (thingiesBySize[i].key <= 0xFF) symbolSize = 1;
      else if (thingiesBySize[i].key <= 0xFFFF) symbolSize = 2;
      else if (thingiesBySize[i].key <= 0xFFFFFF) symbolSize = 3;
      else symbolSize = 4;
      
//      dst.writeInt(thingiesBySize[i].key, symbolSize,
//        EndiannessTypes::big, SignednessTypes::nosign);
      currentScriptBuffer.writeInt(thingiesBySize[i].key, symbolSize,
        EndiannessTypes::big, SignednessTypes::nosign);
        
      // when terminator reached, flush script to ROM stream
//      if (thingiesBySize[i].key == 0) {
//        flushActiveScript();
//      }
      
      return;
    }
  } */
  
  TThingyTable::MatchResult result = thingy.matchTableEntry(ifs);
  
  if (result.id != -1) {
//    std::cerr << std::dec << lineNum << " " << std::hex << result.id << " " << result.size << std::endl;
  
    int symbolSize;
    if (result.id <= 0xFF) symbolSize = 1;
    else if (result.id <= 0xFFFF) symbolSize = 2;
    else if (result.id <= 0xFFFFFF) symbolSize = 3;
    else symbolSize = 4;
    
    currentScriptBuffer.writeInt(result.id, symbolSize,
      EndiannessTypes::big, SignednessTypes::nosign);
    
    return;
  }
  
  // XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
  if ((unsigned char)ifs.peek() >= 0x80) {
    ifs.get();
    ifs.get();
  }
  else {
    ifs.get();
  }
  currentScriptBuffer.writeInt(0x01, 1,
    EndiannessTypes::big, SignednessTypes::nosign);
  return;
  // XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
  
  std::string remainder;
  ifs.getLine(remainder);
  
  // if we reached end of file, this is not an error: we're done
  if (ifs.eof()) return;
  
  throw TGenericException(T_SRCANDLINE,
                          "RippleScriptReader::outputNextSymbol()",
                          "Line "
                            + TStringConversion::intToString(lineNum)
                            + ":\n  Couldn't match symbol at: '"
                            + remainder
                            + "'");
}
  
void RippleScriptReader::flushActiveScript() {
  int outputSize = currentScriptBuffer.size();
  
  // output message binary
/*  std::string blockStartString = TStringConversion::intToString(blockStart,
                 TStringConversion::baseHex);
  currentScriptBuffer.seek(0);
  std::string filename =
    outprefix
    + std::string("msg_")
    + blockStartString
    + ".bin";
  TOfstream ofs(filename.c_str(), std::ios_base::binary);
  ofs.write(currentScriptBuffer.data().data(), currentScriptBuffer.size()); */
  
  ResultString result;
  currentScriptBuffer.seek(0);
  while (!currentScriptBuffer.eof()) {
    result.str += currentScriptBuffer.get();
  }
  
  dst.push_back(result);
  
  // clear script buffer
  currentScriptBuffer = TBufStream(scriptBufferCapacity);
}
  
bool RippleScriptReader::checkSymbol(BlackT::TStream& ifs, std::string& symbol) {
  if (symbol.size() > ifs.remaining()) return false;
  
  int startpos = ifs.tell();
  for (int i = 0; i < symbol.size(); i++) {
    if (symbol[i] != ifs.get()) {
      ifs.seek(startpos);
      return false;
    }
  }
  
  return true;
}
  
void RippleScriptReader::processDirective(BlackT::TStream& ifs) {
  skipSpace(ifs);
  
  std::string name = matchName(ifs);
  matchChar(ifs, '(');
  
  for (int i = 0; i < name.size(); i++) {
    name[i] = toupper(name[i]);
  }
  
  if (name.compare("LOADTABLE") == 0) {
    processLoadTable(ifs);
  }
  else if (name.compare("END") == 0) {
    processEndMsg(ifs);
  }
  else {
    throw TGenericException(T_SRCANDLINE,
                            "RippleScriptReader::processDirective()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unknown directive: "
                              + name);
  }
  
  matchChar(ifs, ')');
}

void RippleScriptReader::processLoadTable(BlackT::TStream& ifs) {
  std::string tableName = matchString(ifs);
  TThingyTable table(tableName);
  loadThingy(table);
}

void RippleScriptReader::processEndMsg(BlackT::TStream& ifs) {
  flushActiveScript();
}

void RippleScriptReader::skipSpace(BlackT::TStream& ifs) const {
  ifs.skipSpace();
}

bool RippleScriptReader::checkString(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  
  if (!ifs.eof() && (ifs.peek() == '"')) return true;
  return false;
}

bool RippleScriptReader::checkInt(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  
  if (!ifs.eof()
      && (isdigit(ifs.peek()) || ifs.peek() == '$')) return true;
  return false;
}

bool RippleScriptReader::checkChar(BlackT::TStream& ifs, char c) const {
  skipSpace(ifs);
  
  if (!ifs.eof() && (ifs.peek() == c)) return true;
  return false;
}

std::string RippleScriptReader::matchString(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "RippleScriptReader::matchString()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!checkString(ifs)) {
    throw TGenericException(T_SRCANDLINE,
                            "RippleScriptReader::matchString()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected non-string at: "
                              + getRemainingContent(ifs));
  }
  
  ifs.get();
  
  std::string result;
  while (!ifs.eof() && (ifs.peek() != '"')) result += ifs.get();
  
  matchChar(ifs, '"');
  
  return result;
}

int RippleScriptReader::matchInt(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "RippleScriptReader::matchInt()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!checkInt(ifs)) {
    throw TGenericException(T_SRCANDLINE,
                            "RippleScriptReader::matchInt()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected non-int at: "
                              + getRemainingContent(ifs));
  }
  
  std::string numstr;
  // get first character (this covers the case of an initial '$' for
  // hexadecimal)
  numstr += ifs.get();
  // handle possible initial "0x"
  if ((numstr[0] == '0') && (tolower(ifs.peek()) == 'x')) numstr += ifs.get();
  
  char next = ifs.peek();
  while (!ifs.eof()
         && (isdigit(next)
          || (tolower(next) == 'a')
          || (tolower(next) == 'b')
          || (tolower(next) == 'c')
          || (tolower(next) == 'd')
          || (tolower(next) == 'e')
          || (tolower(next) == 'f'))) {
    numstr += ifs.get();
    next = ifs.peek();
  }
  
  return TStringConversion::stringToInt(numstr);
}

void RippleScriptReader::matchChar(BlackT::TStream& ifs, char c) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "RippleScriptReader::matchChar()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (ifs.peek() != c) {
    throw TGenericException(T_SRCANDLINE,
                            "RippleScriptReader::matchChar()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Expected '"
                              + c
                              + "', got '"
                              + ifs.get()
                              + "'");
  }
  
  ifs.get();
}
  
std::string RippleScriptReader
  ::getRemainingContent(BlackT::TStream& ifs) const {
  std::string content;
  while (!ifs.eof()) content += ifs.get();
  return content;
}

std::string RippleScriptReader::matchName(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "RippleScriptReader::matchName()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!isalpha(ifs.peek())) {
    throw TGenericException(T_SRCANDLINE,
                            "RippleScriptReader::matchName()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Couldn't read name at: "
                              + getRemainingContent(ifs));
  }
  
  std::string result;
  result += ifs.get();
  while (!ifs.eof()
         && (isalnum(ifs.peek()) || (ifs.peek() == '_'))) {
    result += ifs.get();
  }
  
  return result;
}


}
