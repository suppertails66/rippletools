#include "util/TThingyTable.h"
#include "util/TStringConversion.h"
#include "util/TCharFmt.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cctype>

using namespace std;

namespace BlackT {


TThingyTable::TThingyTable() { }

TThingyTable::TThingyTable(string filename) {
  readGeneric(filename);
}
  
void TThingyTable::readGeneric(std::string filename) {
  ifstream ifs(filename.c_str(), ios_base::binary);
  
  while (ifs.good()) {
    string numberString;
    
    // skip whitespace
    while (ifs.good() && isspace(ifs.peek())) ifs.get();
    
    // ignore comment lines
    char next = ifs.get();
    if ((next == '/') && (ifs.peek() == '/')) {
      getline(ifs, numberString);
      continue;
    }
    else {
      ifs.unget();
    }
    
    getline(ifs, numberString, '=');
    if (!ifs.good()) {
      break;
    }
    
    // add hex ID for int conversion
    numberString = string("0x") + numberString;
    int rawID = TStringConversion::stringToInt(numberString);
    
    string charString;
    getline(ifs, charString);
    
    // motherfucking windows
    while (charString.size() >= 1
            && ((charString[charString.size() - 1] == '\n')
            || (charString[charString.size() - 1] == '\r'))) {
      charString = charString.substr(0, charString.size() - 1);
    }
    
//    std::cerr << numberString << " " << charString << std::endl;
    entries[rawID] = TableEntry(charString);
  }
  
  generateEntriesByLength();
}
  
void TThingyTable::readSjis(std::string filename) {
  readGeneric(filename);
}
  
void TThingyTable::readUtf8(std::string filename) {
  ifstream ifs(filename.c_str(), ios_base::binary);
  
  while (ifs.good()) {
    string numberString;
    
    // skip whitespace
    while (ifs.good() && isspace(ifs.peek())) ifs.get();
    
    // ignore comment lines
    char next = ifs.get();
    if ((next == '/') && (ifs.peek() == '/')) {
      getline(ifs, numberString);
      continue;
    }
    else {
      ifs.unget();
    }
    
    getline(ifs, numberString, '=');
    if (!ifs.good()) {
      break;
    }
    
    // add hex ID for int conversion
    numberString = string("0x") + numberString;
    int rawID = TStringConversion::stringToInt(numberString);
    
    string charString;
    getline(ifs, charString);
    entries[rawID] = TableEntry(charString);
    
    TUtf16Chars chars;
    TCharFmt::utf8To16(charString, chars);
    if (chars.size() > 0) {
      revEntries[chars[0]] = rawID;
    }
  }
  
  generateEntriesByLength();
}

string TThingyTable::getEntry(int charID) const {
  RawTable::const_iterator it = entries.find(charID);
  
  if (it == entries.cend()) {
//    return string("?");
    std::string str = TStringConversion::intToString(charID,
                        TStringConversion::baseHex).substr(2,
                                std::string::npos);
    while (str.size() < 2) str = "0" + str;
    return ("<$" + str + ">");
  }
  
  return it->second;
}
  
int TThingyTable::getRevEntry(int charID) const {
  ReverseMap::const_iterator it = revEntries.find(charID);
  
  if (it == revEntries.cend()) {
    return -1;
  }
  
  return it->second;
}

TThingyTable::MatchResult
    TThingyTable::matchTableEntry(TableEntry entry) const {
  MatchResult result;
  result.id = -1;
  result.size = -1;
  for (LengthCollection::const_iterator it = entriesByLength.cbegin();
       it != entriesByLength.cend();
       ++it) {
    TableEntry str = entry.substr(0, it->entry.size());
    if (it->entry.compare(str) == 0) {
      result.id = it->id;
      result.size = it->entry.size();
      return result;
    }
  }
  
  return result;
}

TThingyTable::MatchResult
    TThingyTable::matchTableEntry(BlackT::TStream& ifs) const {
  MatchResult result;
  result.id = -1;
  result.size = -1;
  
  int pos = ifs.tell();
  
  // TODO: search longest-to-shortest starting with longest possible string.
  // currently, this does shortest-to-longest, which will cause problems
  // with partially-matched prefixes
  std::string matchStr;
  while ((matchStr.size() < 16) && (!ifs.eof())) {
    matchStr += ifs.get();
    MatchResult temp = matchTableEntry(matchStr);
    if (temp.id != -1) return temp;
  }
  
  ifs.clear();
  ifs.seek(pos);
  return result;
}
  
bool TThingyTable::hasEntry(int charID) const {
  RawTable::const_iterator it = entries.find(charID);
  
  if (it == entries.cend()) {
    return false;
  }
  
  return true;
}

void TThingyTable::generateEntriesByLength() {
  entriesByLength.resize(entries.size());
  int pos = 0;
  for (RawTable::iterator it = entries.begin();
       it != entries.end();
       ++it) {
    entriesByLength[pos].entry = it->second;
    entriesByLength[pos].id = it->first;
    
    ++pos;
  }
  
  std::sort(entriesByLength.begin(), entriesByLength.end(), sortByLength);
}

bool TThingyTable::sortByLength(const TableEntryInfo& first,
                         const TableEntryInfo& second) {
  if (second.entry.size() < first.entry.size()) return true;
  return false;
}


} 
