/*
 *
 *  Copyright (C) 2014 Jürg Müller, CH-5524
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "NTypes.h"

#include "KStream.h"
#include "NUtils.h"
#include "NCanUtils.h"
#include "KScanTable.h"

#include "scan_data.inc"

static struct
{
  int start_index;
  int length;
} scan_table[256]; // use ShortCanId

static scan_data * _scan_data_table = scan_data_table;
static int scan_tab_len = High(scan_data_table) + 1;
static bool IsInitalized = KScanTable::Init_scan_table();

bool KScanTable::Init_scan_table()
{
  for (int i = 0; i <= (int) High(scan_table); i++)
  {
    scan_table[i].start_index = -1;
    scan_table[i].length = 0;
  }
  for (int i = 0; i < scan_tab_len; i++)
  {
    int id = KScanTable::ShortCanId(_scan_data_table[i].id);
    if (scan_table[id].start_index == -1)
    {
      scan_table[id].start_index = i;
    }
    scan_table[id].length++;
  }
  IsInitalized = true;

  Init_Kennwort();
  return true;
}

bool KScanTable::Get_scan_value(const KComfortFrame & send, KComfortFrame & recv)
{
  KCanFrame SendFrame;
  KCanFrame RecvFrame;

  if (!IsInitalized)
    return false;

  bool Ok = send.SetCanFrame(SendFrame);
  KComfortFrame::msg_type type = send.GetMsgType();
  if (send.Data[0] != 0x0d) // Sender muss Id 68x haben
    type = KComfortFrame::mt_invalid;

  int recv_id = SendFrame.GetReceiverId();
  if (recv_id < 0)
    return false;
  
  unsigned char id = ShortCanId((unsigned short) recv_id);

  int table_ind = scan_table[id].start_index;
  if (!Ok || table_ind == -1 || type == KComfortFrame::mt_invalid)
    return false;

  unsigned short ind = SendFrame.GetElsterIdx();
  RecvFrame = SendFrame;
  RecvFrame.Id = (unsigned short) recv_id;
  RecvFrame.Data[0] = 0xd2;
  RecvFrame.SetReceiverId(SendFrame.Id);
  
  int data_index = -1;
  for (int i = 0; i < scan_table[id].length; i++)
    if (_scan_data_table[table_ind + i].index == ind)
    {
      data_index = table_ind + i;
      break;
    }

  unsigned short val = 0x8000;
  if (data_index != -1)
    val = _scan_data_table[data_index].value;

  if (type == KComfortFrame::mt_get)
  {
    RecvFrame.SetValue(val);
    return recv.GetCanFrame(RecvFrame);
  }
  if (type == KComfortFrame::mt_change ||
      type == KComfortFrame::mt_short_change)
  {
    int Value = SendFrame.GetValue();
    if (val != 0x8000 && Value >= 0)
    {
      _scan_data_table[data_index].value = (unsigned short) Value;
      recv.SetOk(true);
    } else
      recv.SetOk(false);
    return true;
  }
  return false;
}

bool KScanTable::LoadScanTable(const char * Filename)
{
  if (_scan_data_table &&
      _scan_data_table != scan_data_table)
    delete [] _scan_data_table;

  _scan_data_table = NULL;
  IsInitalized = false;
  scan_tab_len = 0;

  FILE * fin = fopen(Filename, "rb");
  if (!fin)
    return false;

  long flen = 0;
  if (!fseek(fin, 0, SEEK_END))
  {
    flen = ftell(fin);
    if (fseek(fin, 0, SEEK_SET))
      flen = 0;
  }
  if (flen <= 0)
  {
    fclose(fin);
    return false;
  }

  unsigned char * buffer = new unsigned char [flen+1];
  buffer[flen] = 0;
  long pos = 0;
  while (pos < flen)
  {

    long l = fread(buffer + pos, 1, flen - pos > 10000 ? 10000 : flen - pos, fin);
    if (l <= 0)
      break;
    pos += l;
  }

  bool Ok = flen == pos;
  int count = 0;
  if (Ok)
  {
    pos = 0;
    while (pos < flen)
    {
      count++;
      while (pos < flen && (buffer[pos] != '\r' && buffer[pos] != '\n'))
        pos++;
      if (pos < flen && (buffer[pos] == '\r' || buffer[pos] == '\n'))
        pos++;
    }

    _scan_data_table = new scan_data[count];
    pos = 0;
    while (pos < flen)
    {
      scan_data newtab;
      KCanFrame Frame;

      while (pos < flen && (buffer[pos] != '\r' && buffer[pos] != '\n'))
        pos++;
      while (buffer[pos] == '\r' || buffer[pos] == '\n')
        pos++;
      while (pos < flen && buffer[pos] == ' ') pos++;

      if (pos >= flen)
        break;

      if (Frame.SetElsterFrame2((const char *) buffer + pos))
      {
        newtab.set_scan_data(Frame);
        _scan_data_table[scan_tab_len++] = newtab;
      }
    }
  }

  delete [] buffer;
  fclose(fin);

  if (Ok)
    Ok = Init_scan_table();

  return Ok;
}

static bool write(const char * str, FILE * fout)
{
  unsigned long len = strlen(str);
  return fwrite(str, 1, len, fout) == len;
}

bool KScanTable::SaveScanTable(const char * Filename)
{
  if (!_scan_data_table || !IsInitalized || scan_tab_len <= 0)
    return false;

  FILE * fout = fopen(Filename, "wb");
  if (!fout)
    return false;

  unsigned short last = 0;
  // static scan_data scan_data_table[] =
  // {
  //   { 0x000, 0x0a26, 0x0001},
  bool Ok = write("static scan_data scan_data_table[] =\n"
                  "{\n", fout);
  for (int i = 0; Ok && i < scan_tab_len; i++)
  {
    char str[64];
    if (last != _scan_data_table[i].id && i > 0)
      write("\n", fout);
    sprintf(str, "  { 0x%3.3x, 0x%4.4x, 0x%4.4x}%c\n",
            _scan_data_table[i].id,
            _scan_data_table[i].index,
            _scan_data_table[i].value,
            i != scan_tab_len-1 ? ',' : ' ');
    Ok = write(str, fout);
    last = _scan_data_table[i].id;
  }
  if (Ok)
    Ok = write("};\n", fout);

  fclose(fout);

  return Ok;
}

// use short can id for scan_table

unsigned char KScanTable::ShortCanId(unsigned short id)
{
  return (unsigned char)((id & 0x780) / 8 + (id & 0xf));
}

unsigned short KScanTable::LongCanId(unsigned char id)
{
  return (unsigned short)(8*(id & 0xf0) + (id & 0xf));
}

bool KScanTable::GetValue(unsigned short receiver_id, unsigned short elster_idx, unsigned short & Value)
{
  KComfortFrame send;
  KComfortFrame recv;
  KCanFrame Frame;
  int val = -1;

  bool Ok = Frame.InitElsterFrame(0x680, receiver_id, elster_idx);
  if (!send.GetCanFrame(Frame))
    Ok = false;

  if(Ok)
    Ok = Get_scan_value(send, recv);

  if (Ok)
    Ok = recv.SetCanFrame(Frame);

  val = Frame.GetValue();
  if (val < 0)
    Ok = false;
  else
    Value = (unsigned short) val;
  
  return Ok;
}

bool KScanTable::SetValue(const KCanFrame & frame)
{
  int ind = frame.GetElsterIdx();
  int val = frame.GetValue();
  int id = KScanTable::ShortCanId(frame.Id);
  int table_ind = scan_table[id].start_index;
  if (table_ind == -1 || val < 0 || ind < 0)
    return false;
  
  int data_index = -1;
  for (int i = 0; i < scan_table[id].length; i++)
    if (_scan_data_table[table_ind + i].index == (unsigned short) ind)
      {
        data_index = table_ind + i;
        break;
      }
    
  if (data_index != -1)
  {
    _scan_data_table[data_index].value = (unsigned short) val;
    return true;
  }
  return false;
}

unsigned KScanTable::Init_Kennwort()
{
  int i = 0;

  while (KComfortFrame::scan_geraete_id[i].elster_idx)
  {
    unsigned short Value;
    unsigned short recv_id = KComfortFrame::scan_geraete_id[i].recv_id;
    unsigned short elster_idx = KComfortFrame::scan_geraete_id[i].elster_idx;

    bool Ok = KScanTable::GetValue(recv_id, elster_idx, Value) &&
                          Value != 0x8000 && Value != 0;
    if (Ok && elster_idx == 0x0199)
    {
      unsigned short Value2;
      Ok = GetValue(recv_id, 0x019a, Value2) &&
                    Value2 != 0x8000;
      if (!(Value & 0xff) && !(Value2 & 0xff)) // little endian (bei älteren Geräten)
      {
        Value = Value >> 8;
        Value2 = Value2 >> 8;
      }
      Value = 256*(Value & 0xff) + (Value2 & 0xff); // value kann grösser als 0xff sein

      if (Ok && Value && Value != 0x8000)
      {
        KComfortFrame::InitId = Value;
        return Value;
      }
    }
    i++;
  }
  return 0;
}

void scan_data::set_scan_data(const KCanFrame & Frame)
{
  id = Frame.Id;
  index = Frame.GetElsterIdx();
  value = Frame.GetValue();
}

int scan_data::Compare(const scan_data & data) const
{
  if (id != data.id)
    return (int) id - (int) data.id;
 
  return (int) index - (int) data.index;
}
