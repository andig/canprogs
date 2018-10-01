/*
 *
 *  Copyright (C) 2015 Jürg Müller, CH-5524
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see http://www.gnu.org/licenses/ .
 */

#if !defined(VList_H)

  #define VList_H

  #include <time.h>
  #include "KStream.h"

  class KSortItem
  {
    public:
      const char * Name;
      
      KSortItem() { Name = NULL; }
      ~KSortItem();
      void SetName(const char * NewName);
  
  };

  class KSortItemList
  {
    private:
      bool IsSorted;
      unsigned SortItemCount;
      unsigned MaxItemCount;
      const KSortItem ** SortItemList;
    public:
      KSortItemList() { IsSorted = false; SortItemCount = 0; MaxItemCount = 0; SortItemList = NULL; }
      ~KSortItemList();
    
      void Clear();
      void Reset();
      unsigned GetCount() const { return SortItemCount; }
      const KSortItem * GetItem(unsigned Index) const;
      const char * GetName(unsigned Index) const;
      unsigned AppendItem(const KSortItem * Item);
      bool Append(const char * NewName);
      int FindItem(const char * FindName) const;
      virtual int Compare(unsigned Index, const char * FindName) const;
      virtual int Compare(unsigned Index1, unsigned Index2) const;
      bool ReadFile(const char * Filename);
    
      bool Sort();
  };
/*
  class VList : public KStream
  {
    private:
      bool is_sorted;
    public:
      unsigned string_count;

      VList()
      {
        Init();
      }
      void Init()
      {
        string_count = 0;
        is_sorted = false;
        mSize = 0;
      }
      bool AddString(const char * string);
      bool Sort();
      const char * GetString(unsigned index);
      int FindString(const char * string) const;
      bool DeleteString(unsigned index);
      bool InsertString(unsigned index, const char *string);
    
      virtual bool ReadFile(const char * Filename, unsigned long Max = 0);
      virtual bool SaveFile(const char * Filename) const;
  };
*/
  class VExploreDir
  {
    public:
      int Level;
      bool dir_first;
      bool use_hidden_files;
      char FromPath[2048];
      char RelPath[2048];

      VExploreDir()
      {
        Level = 0;
        RelPath[0] = 0;
        dir_first = true;
        use_hidden_files = false;
      }
      virtual bool ExploreFile(const char * Filename) = 0;
      virtual bool ExploreDir(const char * DirName);
      bool ExploreSortedList(bool dirs);
      bool Start(const char * StartFromDir);
      bool Start(const char StartFromDirList[][1000]);
  };

  class VListFileDir
  {
    private:
      void * dir;
      long PathLen;
#if defined(__VC__) || defined(_MSC_VER)
      void * FindFileData;
      bool FirstRead;
#endif
    public:
      char Name[256];
      char FullPath[2048];
      long mode;
      long size;
      time_t time;

      VListFileDir();
      ~VListFileDir();

      virtual bool Open(const char * Path);
      bool Open(const char * Path, const char * RelPath);
      virtual void Close();
      virtual bool Next();
      bool IsFile() const;
      bool IsDir() const;
  };

#endif

