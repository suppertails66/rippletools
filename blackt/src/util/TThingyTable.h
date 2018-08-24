#ifndef TTHINGYTABLE_H
#define TTHINGYTABLE_H


#include "util/TStream.h"
#include <string>
#include <map>
#include <vector>

namespace BlackT {


struct TThingyTable {   
public:
  typedef std::string TableEntry;
  
  struct TableEntryInfo {
    TableEntry entry;
    int id;
  };
  
  typedef std::map<int, TableEntry> RawTable;
  typedef std::map<int, int> ReverseMap;
  typedef std::vector<TableEntryInfo> LengthCollection;

  TThingyTable();
  TThingyTable(std::string filename);
  
  void readGeneric(std::string filename);
  void readSjis(std::string filename);
  void readUtf8(std::string filename);
  
  std::string getEntry(int charID) const;
  
  int getRevEntry(int charID) const;
  
  bool hasEntry(int charID) const;
  
  struct MatchResult {
    int id;
    int size;
  };
  
  MatchResult matchTableEntry(TableEntry entry) const;
  MatchResult matchTableEntry(BlackT::TStream& ifs) const;
  
  RawTable entries;
  ReverseMap revEntries;
  LengthCollection entriesByLength;

  void generateEntriesByLength();
  
  static bool sortByLength(const TableEntryInfo& first,
                           const TableEntryInfo& second);
  
};


}


#endif 
