/*
 *
 *  Copyright (C) 2016 Jürg Müller, CH-5524
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

#include <string.h>
#include <sys/stat.h>

#include "NTypes.h"
#include "NUtils.h"

#include "KCanLogFile.h"
#include "KElsterTable.h"

// Aus einem log-File, welches durch can_logger erzeugt wurde, einen Array von
// KCanFrame erstellen.

KCanLogFile::KCanLogFile()
{
  Frames = NULL;
  count = 0;
  pos = 0;
}

KCanLogFile::~KCanLogFile()
{
  Init();
}

void KCanLogFile::Init()
{
  if (Frames)
    delete [] Frames;
  Frames = NULL;
  pos = 0;
}

unsigned KCanLogFile::LogFileToCanFrames(const KStream & stream, KCanFrame * Frames)
{
  KCanFrame frame;
  unsigned long pos = 0;
  unsigned count = 0;
  unsigned long size = stream.Size();
  
  while (pos < size)
  {
    while (pos < size && stream.GetByte(pos) < ' ')
      pos++;

    if (pos+10 >= size)
      break;
      
    unsigned long off = pos + 1;
    while (off < size && stream.GetByte(off) >= ' ')
      off++;

    if (frame.InitFromLog(stream.GetCharMemory() + pos))
    {
      if (Frames != NULL)
        Frames[count] = frame;
      count++;
    }
    pos = off;
  }
  return count;
}

KCanFrame * KCanLogFile::BuildCanFrames(const KStream & stream)
{
  Init();
  
  count = LogFileToCanFrames(stream, NULL);
  if (count)
  {
    Frames = new KCanFrame[count];
    LogFileToCanFrames(stream, Frames);
  }
  
  return Frames;
}

bool KCanLogFile::ReadFile(const char * Filename)
{
  KStream stream;
  
  if (!stream.ReadFile(Filename))
  {
    printf("\nlog file \"%s\" not loaded\n", Filename);
    return false;
  }  
  
  printf("\nreading log file \"%s\" ...\n", Filename);
  BuildCanFrames(stream);
  printf("log file read: frame count %d\n\n", count);
  
  return Frames != NULL;
}

void KCanLogFile::PrintScanTable() const
{
  if (Frames)
  {
    unsigned short id_table[200];
    unsigned id_count = 0;
    
    // CAN-IDs ermitteln und im Array "id_table" speichern. 
    for (unsigned i = 0; i < count; i++)
    {
      bool found = false;
      for (unsigned j = 0; !found && j < id_count; j++)
        found = id_table[j] == Frames[i].GetIdToValue();
      
      if (!found && id_count < 200)
        id_table[id_count++] = Frames[i].GetIdToValue();
    }
    // Array "id_table" sortieren.
    for (unsigned i = 0; i < id_count-1; i++)
      for (unsigned k = i+1; k < id_count; k++)
        if (id_table[i] > id_table[k])
        {
          unsigned short u = id_table[i];
          id_table[i] = id_table[k];
          id_table[k] = u;
        }
    
    printf("list of valid can id's:\n\n");
    for (unsigned i = 0; i < id_count; i++)
      printf("  %3.3x\n", id_table[i]);
    
    bool * idx = new bool[0x10000];
    unsigned short * val = new unsigned short[0x10000];
    for (unsigned i = 0; i < id_count; i++)
    {
      for (unsigned k = 0; k < 0x10000; k++)
      {
        idx[k] = false;
      }
  
      // alle Werte zu "id_table[i]" in "val" speichern.
      for (unsigned k = 0; k < count; k++)
      {
        if (Frames[k].GetIdToValue() == id_table[i] &&
            (Frames[k].Data[0] & 0x0f) != 0x01)
        {
          int val_ = Frames[k].GetValue();
          int idx_ = Frames[k].GetElsterIdx();
          if (val_ >= 0 && idx_ >= 0 && !idx[idx_])
          {
            idx[idx_] = true;
            val[idx_] = val_;
          }
        }
      }
      
      printf("\n");
      for (unsigned k = 0; k < 0x10000; k++)
        if (idx[k])
        {
          printf("  { 0x%3.3x, 0x%4.4x, 0x%4.4x},", id_table[i], k, val[k]);
          const ElsterIndex * elst_idx = GetElsterIndex(k);
          if (elst_idx)
          {
            char buff[200];
            
            SetValueType(buff, elst_idx->Type, val[k]);
            
            printf("  // %s: %s", elst_idx->Name, buff);
          }
          printf("\n");
        }
    }
    delete [] idx;
    delete [] val;
  }  
}

unsigned KCanLogFile::SetPos()
{
  if (count)
  {
    int day, time_ms;
    NUtils::Time(day, time_ms);
    
    pos = 0;
    last_day = day;
    while (pos < count &&
           time_ms > Frames[pos].TimeStampMs)
      pos++;
  }
  return pos;
}

bool KCanLogFile::GetNextFrame(KCanFrame & Frame)
{
  if (count)
  {
    int day, time_ms;
    NUtils::Time(day, time_ms);
    
    if (day > last_day)
    {
      last_day = day;
      pos = 0;
    }
    if (pos < count &&
        time_ms >= Frames[pos].TimeStampMs)
    {
      Frame = Frames[pos++];
      return true;    
    }
  }
  return false;
}
