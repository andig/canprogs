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

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#include <stdio.h>
#include <string.h>

#if defined(__LINUX__)
	#include <utime.h>
	#include <unistd.h>   // für usleep
	#include <stdlib.h>
#endif

#include <sys/stat.h>
#if !defined(__VC__) && !defined(_MSC_VER)
  #include <dirent.h>
#endif

#include "NTypes.h"

#include "NUtils.h"

using namespace NUtils;

#include "VList.h"

typedef int * intp;

KSortItem::~KSortItem()
{
  if (Name)
    delete [] Name;
}

void KSortItem::SetName(const char * NewName)
{
  if (!NewName || !strlen(NewName))
    return;
  
  if (Name)
    delete [] Name;
  
  Name = new char[strlen(NewName) + 1];
  strcpy((char *) Name, NewName);
}

void KSortItemList::Clear()
{
  Reset();
  
  if (SortItemList)
    delete [] (intp *) SortItemList;
  
  SortItemList = NULL;
  MaxItemCount = 0;
}

void KSortItemList::Reset()
{
  for (unsigned i = 0; i < SortItemCount; i++)
  {
    delete SortItemList[i];
  }
  SortItemCount = 0;
}

KSortItemList::~KSortItemList()
{
  Clear();
}

const KSortItem * KSortItemList::GetItem(unsigned Index) const
{
  if (Index >= SortItemCount)
    return NULL;
  
  return SortItemList[Index];
}

const char * KSortItemList::GetName(unsigned Index) const
{
  const KSortItem * Item = GetItem(Index);
  if (!Item)
    return NULL;
  
  return Item->Name;
}

unsigned KSortItemList::AppendItem(const KSortItem * Item)
{
  IsSorted = false;
  if (MaxItemCount <= SortItemCount)
  {
    
    unsigned NewCount = MaxItemCount ? 2*MaxItemCount : 2000;
    const KSortItem ** NewList = (const KSortItem **) new intp[NewCount];
    
    for (unsigned i = 0; i < SortItemCount; i++)
      NewList[i] = SortItemList[i];
    
    if (SortItemList)
      delete [] (intp *) SortItemList;
    
    SortItemList = NewList;
    MaxItemCount = NewCount;
  }
  SortItemList[SortItemCount++] = Item;
  
  return SortItemCount-1;
}

bool KSortItemList::Append(const char * NewName)
{
  if (!NewName || !*NewName || FindItem(NewName) >= 0)
    return false;
  
  KSortItem * Item = new KSortItem;
  Item->SetName(NewName);
  AppendItem(Item);
  
  return true;
}

int KSortItemList::Compare(unsigned Index, const char * FindName) const
{
  if (Index >= SortItemCount || !FindName)
    return 0;
  
  return strcmp(SortItemList[Index]->Name, FindName);
}

int KSortItemList::Compare(unsigned Index1, unsigned Index2) const
{
  if (Index1 >= SortItemCount || Index2 >= SortItemCount)
    return 0;
  
  return stricmp(SortItemList[Index1]->Name, SortItemList[Index2]->Name);
}

bool KSortItemList::Sort()
{
  if (SortItemCount < 1)
    return true;
  
  unsigned n = SortItemCount;
  do
  {
    unsigned newn = 0;
    for (unsigned i = 0; i < n-1; ++i)
    {
      if (Compare(i, i+1) > 0)
      {
        // swap i and (i+1)
        const KSortItem * Temp = SortItemList[i];
        SortItemList[i] = SortItemList[i+1];
        SortItemList[i+1] = Temp;
        newn = i+1;
      } 
    }
    n = newn;
  } while (n > 0);
  IsSorted = true;
  
  return SortItemCount > 0;
}

int KSortItemList::FindItem(const char * FindName) const
{
  if (!FindName || !SortItemCount)
    return -1;
  
  if (IsSorted)
  {
    int middle;
    int left = 0;
    int right = SortItemCount - 1;
    
    while (left <= right)
    {
      middle = (left + right) / 2;
      
      int Comp = Compare(middle, FindName);
      if (Comp == 0)
        return middle;
      else
      if (Comp > 0)
        right = middle - 1;
      else
        left = middle + 1;
    }
    return -1;
  }
  
  for (unsigned i = 0; i < SortItemCount; i++)
    if (!strcmp(SortItemList[i]->Name, FindName))
      return (int) i;
  
  return -1;
}

bool KSortItemList::ReadFile(const char * Filename)
{
  KStream stream;
  
  if (!stream.ReadFile(Filename) ||
      !stream.Size())
    return false;
  
  Reset();
  
  const char * start = stream.GetCharMemory();
  long pos = 0;
  do
  {
    while (pos < stream.Size() && (unsigned char) start[pos] < ' ')
      pos++;
    
    if (pos+1 < stream.Size())
    {
      long pos1 = pos + 1;
      while (pos1 < stream.Size() && (unsigned char) start[pos1] >= ' ')
        pos1++;
    
      stream.SetByte(pos1, 0);
      
      Append(start + pos);
      pos = pos1 + 1;
    }    
  } while (pos < stream.Size());
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

/*

bool VList::AddString(const char * string)
{
  if (!string)
    return false;

  if (FindString(string) >= 0)
    return false;

  unsigned long len = strlen(string) + 1;
  unsigned long old_size = mSize;
  unsigned long new_size = mSize;
  if (!string_count)
    new_size = len < 10000 ? 10000 : 2*len;
  else
    new_size += 2*len;
  SetSize(new_size);
  mSize = old_size;

  memcpy(mMemory + mSize, string, len);
  string_count++;
  mSize += len;
  mMemory[mSize] = 0;
  
  is_sorted = false;

  return true;
}

int VList::FindString(const char * string) const
{
  if (!mMemory || !string)
    return -1;

  unsigned index = 0;
  const char * res = (const char *) (mMemory);
  while (res < (const char *)mMemory + mSize)
  {
    if (!strcmp(res, string))
      return index;

    res += strlen(res) + 1;
    index++;
  }
  return -1;
}

bool VList::Sort()
{
  KStream stream;
  
  stream.SetSize(2000);
  char * stri = (char *) mMemory;
  while (stri < (char *) mMemory + mSize)
  {   
    const char * str = stri;
    const char * s = stri;
    s += strlen(s) + 1;
    while (s < (char *) mMemory + mSize)
    {
      
      if (stricmp(str, s) > 0)
      {
        str = s;
      }
      s += strlen(s) + 1;
    }
    if (stri < str)
    {
      // str an die Stelle stri verschieben.
      long len = strlen(str) + 1;
      stream.SetSize(len);
      memmove((char *)stream.GetMemory(), str, len);
      
      memmove((char *)stri + len, stri, str - stri);
      memmove((char *)stri, stream.GetMemory(), len);
      
      mMemory[mSize] = 0;
    }
    stri += strlen(stri) + 1;
  }

  is_sorted = true;

  return true;
}

const char * VList::GetString(unsigned index)
{
  if (index >= string_count || !mMemory)
    return NULL;

  const char * res = (const char *) (mMemory);
  while (index)
  {
    res += strlen(res) + 1;
    index--;
  }
  return res;
}

bool VList::DeleteString(unsigned index)
{
  const char * pos = GetString(index);
  if (!pos)
    return false;
  
  long p = pos - (const char *) mMemory;
  long len = strlen(pos);
  
  memmove((char *) pos, pos+len+1, mSize-p+1);
  string_count--;
  mSize -= len+1;
  mMemory[mSize] = 0;
  
  return true;  
}

bool VList::InsertString(unsigned index, const char * string)
{
  if (!string ||
      FindString(string) >= 0)
    return false;
  
  if (index >= string_count)
    return AddString(string);
  
  const char * p = GetString(index);
  if (!p)
    return false;
  
  long old_size = mSize;
  long len = strlen(string) + 1;
  
  SetSize(old_size + len);
  
  memmove((char *)p + len, p, mSize - (p - (const char *)mMemory));
  memmove((char *)p, string, len);
  string_count++;
  
  return true;
}

bool VList::ReadFile(const char * Filename, unsigned long Max)
{
  KStream stream;
  if (!stream.ReadFile(Filename, Max))
    return false;
  
  for (long p = 0; p < (long) stream.Size(); p++)
    if (stream.GetByte(p) < ' ')
    {
      stream.SetByte(p, 0);
    }
  
  SetSize(0);
  for (long p = 0; p < (long) stream.Size(); )
  {
    const char * ptr = stream.GetCharMemory() + p;
    long l = strlen(ptr);
    if (l > 0)
      AddString(ptr);
    p += l + 1;
  }
  return true;
}

bool VList::SaveFile(const char * Filename) const
{
  KStream stream;
  
  if (mSize == 0)
    return false;
  
  stream.SetSize(mSize + 1);
  memcpy((char *) stream.GetMemory(), GetMemory(), Size());
  *((char *) stream.GetMemory() + Size()) = 0;
  
  for (long p = 0; p < (long) stream.Size(); p++)
    if (stream.GetByte(p) == 0)
    {
      stream.SetByte(p, '\n');
    }
  
  return stream.SaveFile(Filename);  
}

 */

////////////////////////////////////////////////////////////////////////////////

bool VExploreDir::Start(const char * StartFromDir)
{
  strcpy(FromPath, StartFromDir);
  MakeStandardPath(FromPath);

  RelPath[0] = 0;
  Level = 0;

  if (!NUtils::DirExists(FromPath))
  {
    printf("directory \"%s\" does not exist\n", FromPath);
    return true;
  }

  return ExploreDir("");
}

bool VExploreDir::Start(const char StartFromDirList[][1000])
{
  bool Ok = false;
  int i = 0;
  Level = 0;
  while (StartFromDirList[i] && *(StartFromDirList[i]))
  {
    strcpy(FromPath, StartFromDirList[i]);
    MakeStandardPath(FromPath);
    
    RelPath[0] = 0;
    
    if (NUtils::DirExists(FromPath) && ExploreDir(""))
    {
      Ok = true;
    }
     
    i++;
  }
  return Ok;
}

bool VExploreDir::ExploreSortedList(bool dirs)
{
  bool Ok = true;
  VListFileDir ListFile;
  KSortItemList sorted_list;

  if (ListFile.Open(FromPath, RelPath))
  {
    while (ListFile.Next())
    {
      if ((ListFile.Name[0] != '.' || (!dirs && use_hidden_files)) &&
          ListFile.Name[0] != '@')
      {
        if ((!dirs && ListFile.IsFile()) ||
            (dirs && ListFile.IsDir()))
        {
          if (!sorted_list.Append(ListFile.Name))
            Ok = false;
        }
      }
    }
    ListFile.Close();
  }
  if (!sorted_list.Sort())
    Ok = false;

  if (Ok)
  {
    long pos = strlen(RelPath);
    for (int i = 0; i < (int) sorted_list.GetCount(); i++)
    {
      const char * name = sorted_list.GetName(i);
      if (!dirs)
      {
        if (!ExploreFile(name))
          Ok = false;
      } else
      if (pos + strlen(name) + 2 < sizeof(RelPath))
      {
        strcat(RelPath, name);
        strcat(RelPath, "/");
        if (!ExploreDir(name))
          Ok = false;
        RelPath[pos] = 0;
      }
    }
  }

  return Ok;
}

bool VExploreDir::ExploreDir(const char * DirName)
{
  Level++;
  bool Ok = ExploreSortedList(dir_first);
  if (!ExploreSortedList(!dir_first))
    Ok = false;

  Level--;
  return Ok;
}

VListFileDir::VListFileDir()
{
#if defined(__VC__) || defined(_MSC_VER)
  FindFileData = new WIN32_FIND_DATA;
  FirstRead = false;
#endif
  dir = NULL;
}

VListFileDir::~VListFileDir()
{
  Close();
#if defined(__VC__) || defined(_MSC_VER)
  delete FindFileData;
#endif
}

bool VListFileDir::Open(const char * Path)
{
  if (strlen(Path) + 10 >= sizeof(FullPath))
    return false;

  strcpy(FullPath, Path);
  if (FullPath[0] && FullPath[strlen(FullPath)-1] != '/')
    strcat(FullPath, "/");
  PathLen = strlen(FullPath);
#if defined(__VC__) || defined(_MSC_VER)
  strcat(FullPath, "*.*");
  dir = FindFirstFileA(FullPath, (WIN32_FIND_DATA *)FindFileData);
  FirstRead = true;
  FullPath[PathLen] = 0;
#else
  dir = opendir(Path);
#endif

  return dir != NULL;
}

bool VListFileDir::Open(const char * Path, const char * RelPath)
{
#if defined(__WINDOWS__)
  char Dir[2048];
#else
  char Dir[strlen(Path) + strlen(RelPath) + 2];
#endif

  strcpy(Dir, Path);
  strcat(Dir, RelPath);

  return Open(Dir);
}

void VListFileDir::Close()
{
#if defined(__VC__) || defined(_MSC_VER)
  if (dir)
    FindClose((HANDLE)dir);
#else
  if (dir)
    closedir((DIR *)dir);
#endif
  dir = NULL;
}

bool VListFileDir::Next()
{
  if (!dir)
    return false;

#if defined(__VC__) || defined(_MSC_VER)
  if (!FirstRead)
  {
    if (!FindNextFile((HANDLE) dir, (WIN32_FIND_DATA *)FindFileData))
      return false;
  }
  FirstRead = false;
  strcpy(Name, ((WIN32_FIND_DATA *)FindFileData)->cFileName);
#else
  dirent * dire = readdir((DIR *)dir);
  if (!dire)
    return false;

  strcpy(Name, dire->d_name);
#endif
  if (PathLen + strlen(Name) + 1 >= sizeof(FullPath))
    return false;
  strcpy(FullPath + PathLen, Name);

  mode = size = time = 0;
  struct stat Stat;
  if (!stat(FullPath, &Stat))
  {
    mode = Stat.st_mode;
    size = Stat.st_size;
    time = Stat.st_mtime;
  }

  return true;
}

bool VListFileDir::IsFile() const
{
#if defined(__VC__) || defined(_MSC_VER)
  if (((WIN32_FIND_DATA *)FindFileData)->dwFileAttributes &
      (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY))
    return false;
#endif
#if defined(__ARM__)
  if (!mode)
    return true;
#endif
  return !(mode & S_IFDIR) &&
         (mode & S_IFREG);
}

bool VListFileDir::IsDir() const
{
#if defined(__VC__) || defined(_MSC_VER)
  if (((WIN32_FIND_DATA *)FindFileData)->dwFileAttributes &
      (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY))
    return false;
#endif
  return (mode & S_IFDIR) != 0;
}
