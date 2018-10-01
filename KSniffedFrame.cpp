/*
 *
 *  Copyright (C) 2017 Jürg Müller, CH-5524
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

#include "NTypes.h"
#include "KSniffedFrame.h"
#include "KScanTable.h"

void KSniffedFrame::Init(unsigned short Id, unsigned short Recv_id, unsigned short idx)
{
  id = Id;
  recv_id = Recv_id;
  elster_idx = idx;
  value = 0x8000;
  TimeStampDay = 0;
  TimeStampMs = 0;
  
  State = st_invalid;
}

KSniffedFrame::KSniffedFrame(unsigned short Id, unsigned short Recv_id, unsigned short idx)
{
  Init(Id, Recv_id, idx);
  
  Next = NULL;
}

KSniffedFrame::KSniffedFrame(const KCanFrame & Frame)
{
  Init(Frame.Id, Frame.GetReceiverId(), Frame.GetElsterIdx());
  
  Next = NULL;
}

bool KSniffedFrame::SetValue(const KCanFrame & Frame)
{
  if (!Frame.HasValue())
    return false;
  
  value = Frame.GetValue();
  TimeStampDay = Frame.TimeStampDay;
  TimeStampMs = Frame.TimeStampMs;
  
  switch (State)
  {
    case st_invalid:
      State = st_value_changed;
    break;
    
    case st_valid:
    case st_only_time_changed:
      if (Frame.GetValue() == value)
        State = st_only_time_changed;
      else
        State = st_value_changed;
    break;
    
    case st_value_changed:
      break;    
  }
  
  return true;
}

KSniffedData::KSniffedData()
{
  for (int i = 0; i <= (int) High(Sniffed); i++)
    Sniffed[i] = NULL;
  
  for (int i = 0; i <= (int) High(UsedCanId); i++)
    UsedCanId[i] = false;
  
}

KSniffedData::~KSniffedData()
{
  for (int i = 0; i <= (int) High(Sniffed); i++)
  {
    while (Sniffed[i])
    {
      KSniffedFrame * SF = Sniffed[i]->Next;
      
      delete Sniffed[i];
      Sniffed[i] = SF;
    }
  }
}

void KSniffedData::SetUsedCanId(unsigned char Id)
{
  UsedCanId[Id] = true;
}

bool KSniffedData::AddToSniffed(const KCanFrame & Frame)
{
  unsigned char Index = Frame.Data[0];
  if ((Frame.Len != 3 && Frame.Len != 5) ||
      (Index & 0xf) == 1)
    return false;
 
  if (SearchSniffedFrame(Frame))
    return false;

  KSniffedFrame * SF = new KSniffedFrame(Frame);

  unsigned char ShortId = KScanTable::ShortCanId((unsigned short) Frame.GetReceiverId());
  SF->Next = Sniffed[ShortId];
  Sniffed[ShortId] = SF;

  return true;
}

void KSniffedData::ReadAll()
{
  for (int i = High(Sniffed); i >= 0; i--)
  {
    KSniffedFrame * SF = Sniffed[i];
    while (SF)
    {
      if (!SF->State != KSniffedFrame::st_invalid)
      {
        SF->State = KSniffedFrame::st_value_changed;
      }
      SF = SF->Next;
    }
  }
}

const KSniffedFrame * KSniffedData::SearchSniffedFrame(unsigned short id, unsigned short elster_idx)
{
  unsigned char ShortId = KScanTable::ShortCanId(id);
  KSniffedFrame * SF = Sniffed[ShortId];
  
  while (SF)
  {
    if (elster_idx == SF->elster_idx)
    {
        return SF;
    }
    SF = SF->Next;
  }
  return NULL;
}

const KSniffedFrame * KSniffedData::SearchSniffedFrame(const KCanFrame & Frame)
{
  return SearchSniffedFrame(Frame.Id, Frame.GetElsterIdx());
}

const KSniffedFrame * KSniffedData::GetFirstSniffedFrame()
{
  for (int i = High(Sniffed); i >= 0; i--)
  {
    KSniffedFrame * SF = Sniffed[i];
    while (SF)
    {
      if (SF->State >= KSniffedFrame::st_only_time_changed)
      {
        SF->State = KSniffedFrame::st_valid;
        return SF;
      }
      SF = SF->Next;
    }
  }
  return NULL;
}

bool KSniffedData::GetFirstSniffedValue(unsigned short & Value, unsigned short & id, unsigned short & idx)
// Get first changed sniffed value.
{
  const KSniffedFrame * SF = GetFirstSniffedFrame();
  
  if (SF)
  {
    Value = SF->value;
    id = SF->id;
    idx = SF->elster_idx;
    return true;
  }
  return false;
}

bool KSniffedData::ClearSniffedValue(const KCanFrame & Frame)
{
  KSniffedFrame * SF = (KSniffedFrame *) SearchSniffedFrame(Frame);
  if (SF && SF->State != KSniffedFrame::st_invalid)
  {
    SF->State = KSniffedFrame::st_valid;
  }
  return SF != NULL;
}

