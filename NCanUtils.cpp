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

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#if defined(__LINUX__)
  #include <sys/types.h>
  #include <sys/time.h>
  #include <unistd.h>
  #include <signal.h>
#endif
#if defined(__WINDOWS__)
  #include <windows.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NTypes.h"

#include "NUtils.h"
#include "NCanUtils.h"
#include "KElsterTable.h"

#if defined(__ROTEX__) && !defined(__USE_SHORT__)
  #define __USE_SHORT__
#endif

using namespace NCanUtils;

enum NCanUtils::driver_type NCanUtils::GetDriverType(const char * dev)
{
  char copy[8];
  
  if (!dev || strlen(dev) < 4)
    return dt_unknown;
  
  strncpy(copy, dev, 3);
  copy[3] = 0;
  
#if !defined(__WINDOWS__)
  if (!stricmp(copy, "tty") ||
      !stricmp(copy, "cu."))
  {
    return dt_can232;
  }
  if (!stricmp(copy, "can"))
  {
    return dt_can;
  }
#else
  if (!stricmp(copy, "com"))
  {
    return dt_can232;
  }
  if (!stricmp(copy, "ED0"))
  {
    return dt_8dev;
  }
#endif
  
  return dt_unknown;
}

KCanFrame::KCanFrame()
{
  Init();
}

void KCanFrame::Init()
{
  memset(this, 0, sizeof(*this));
}

bool KCanFrame::InitElsterFrame(unsigned short sender_id,
                                unsigned short receiver_id,
                                unsigned short elster_idx)
{
  Init();
  Id = sender_id;
  Data[0] = 1; // receive flag
  
  bool Ok = SetReceiverId(receiver_id);
  if (!SetElsterIdx(elster_idx))
    Ok = false;
  
  return Ok;      
}

bool KCanFrame::EqualData(const KCanFrame & Frame) const
{
  if (Len != Frame.Len)
    return false;

  for (int i = 0; i < Len; i++)
    if (Data[i] != Frame.Data[i])
      return false;

  return true;
}

bool KCanFrame::Changed(const KCanFrame & SendFrame) const
{
  if (Len < 5 || !IsAnswerToElsterIndex(SendFrame))
    return false;

  return GetValue() == SendFrame.GetValue();
}

int KCanFrame::GetReceiverId() const
{
  if (Len < 2)
    return -1;
  
  return ((Data[0] & 0xf0) << 3) + (Data[1] & 0x7f);
}

unsigned short KCanFrame::GetIdToValue() const
{
  if (Len < 6)
    return 0;
  
  // Wert wird verändert. Als Id zählt der Empfänger!
  if ((Data[0] & 0x0f) == 0x00)
    return GetReceiverId();
  else
    return Id;
}

bool KCanFrame::SetReceiverId(unsigned short id)
{
  if (id > 0x7ff)
    return false;
  
  Data[0] = (Data[0] & 0xf) | ((id >> 3) & 0xf0);
  Data[1] = id & 0x7f;
  
  return true;
}

bool KCanFrame::IsAnswer(const KCanFrame & Frame) const
{
  // CAN-Ids vergleichen
  return (int) Id == Frame.GetReceiverId() && GetReceiverId() == (int) Frame.Id;
}

bool KCanFrame::IsAnswerToElsterIndex(const KCanFrame & Frame) const
{
  return IsAnswer(Frame) &&
         GetElsterIdx() == Frame.GetElsterIdx();
}

bool KCanFrame::Supported() const
{
  int val = GetValue();
  
  return val >= 0 && val != 0x8000;
}

bool KCanFrame::HasValue() const
{
  return GetValue() >= 0;
}

int KCanFrame::GetValue() const
{
  if (Len < 5 || Len > 7)
    return -1;
  
  if (Data[2] == 0xfa)
  {
    if (Len != 7)
      return -1;
    
    return 256*Data[5] + Data[6];
  }
  
  return 256*Data[3] + Data[4];
}

bool KCanFrame::SetValue(unsigned short value)
{
  if (Len != 5 && Len != 7)
    return false;

  if (Data[2] == 0xfa)
  {
    Data[5] = value >> 8;
    Data[6] = value & 0xff;
    Len = 7;
  } else {
    Data[3] = value >> 8;
    Data[4] = value & 0xff;
  }
  return true;
}

int KCanFrame::GetElsterIdx() const
{
  if (Len > 7 || Len < 3)
    return -1;

  if (Data[2] == 0xfa)
  {
    if (Len < 5)
      return -1;
    
    return 256*Data[3] + Data[4];
  } else
    return Data[2];
}

bool KCanFrame::SetElsterIdx(unsigned short idx)
{
#if defined(__USE_SHORT__) 
  if (idx < 0x100)
  {
    Len = 5;
    Data[2] = (unsigned char) idx;
  } else
#endif
  {
    Len = 7;
    Data[2] = 0xfa;
    Data[3] = (unsigned char) (idx >> 8);
    Data[4] = (unsigned char) idx;
  }
#if defined(__ROTEX__)
  Len = 7;
#endif
  
  return idx != 0xfa;
}

bool KCanFrame::GetDataFromStream(const char * str)
{
  Len = 0;
  for ( ; Len < 8; Len++)
  {
    while (*str == ' ' || *str == '.')
      str++;

    if (!NUtils::GetHexByte(str, Data[Len]))
      break;
  }
  while (*str == ' ' || *str == '.')
    str++;

  return !*str || *str == '\n' || *str == '\r';
}

void KCanFrame::FormMySqlInsert(char * statement) const
{
  char fields[256];
  char values[256];
  char str[64];

  NUtils::PrintMs(TimeStampDay, TimeStampMs, str);
  strcpy(fields, "counter,timestamp,can_id");

  sprintf(values, "%u,%s,%u",
          Counter & ~ 0x80000000, str, Id & ~ 0x80000000);
  for (int i = 0; i < Len; i++)
  {
    sprintf(fields + strlen(fields), ",d%d", i);
    sprintf(values + strlen(values), ",%d", Data[i]);
  }
  sprintf(statement, "INSERT can_log(%s) VALUES(%s);", fields, values);
}

void KCanFrame::FormHttpVariables(char * stream) const
{
  char time_str[64];

  NUtils::PrintMs(TimeStampDay, TimeStampMs, time_str);
  sprintf(stream, "counter=%d&timestamp=%s&can_id=%d",
          Counter & ~ 0x80000000, time_str, Id & ~ 0x80000000);

  for (int i = 0; i < Len; i++)
  {
    sprintf(stream + strlen(stream), "&d%d=%d", i, Data[i]);
  }
}

bool KCanFrame::SetElsterFrame(const char * str)
{
  // format: 302.0016
  unsigned recv_id;
  unsigned elster_idx;
  bool Ok;

  Ok = NUtils::IsHexDigit(*str);
  if (Ok)
  {
    recv_id = NUtils::GetHex(str);
    Ok = *str == '.';
    if (Ok)
    {
      str++;
      Ok = NUtils::IsHexDigit(*str);
    }
  }
  if (Ok)
  {
    elster_idx = NUtils::GetHex(str);
    Ok = !*str;
  }
  if (Ok)
    Ok = elster_idx <= 0xffff && !(recv_id & ~0x787);

  if (Ok)
    Ok = InitElsterFrame(0, recv_id, elster_idx);

  return Ok;
}

static bool GetInteger(unsigned & num, const char * & Line)
{
  while (*Line == ' ' || *Line == ',') Line++;
  
  TInt64 res;
  unsigned inc = NUtils::GetInt(res, Line);
  if (!inc)
    return false;
  
  Line += inc;
  num = (unsigned) res;
  
  return true;
}

bool KCanFrame::SetElsterFrame2(const char * str)
{
  // format: { 0x180, 0x0001, 0x0000},
  Init();
  while (*str && *str != '\n' && *str != '\r' && *str != '{')
    str++;
  
  if (*str != '{')
    return false;
  str++;
  
  if (!GetInteger(Id, str) || Id >= 0x8000)
    return false;
  
  unsigned res;
  if (!GetInteger(res, str) || res >= 0x10000)
    return false;
  if (!SetElsterIdx((unsigned short) res))
    return false;;
  
  if (!GetInteger(res, str) || res >= 0x10000)
    return false;
  SetValue((unsigned short) res);
  
  return true;
}

void KCanFrame::PrintTime(char * OutStr, bool csv) const
{
  NUtils::PrintTime(TimeStampDay, TimeStampMs, OutStr, csv);
}

void KCanFrame::PrintMs(int days, int ms, char * stream)
{
  int time_stamp_1 = days;
  int time_stamp_2;

  time_stamp_1 += 30*365 + 30 / 4; // Tage zwischen 1970 und 2000
  time_stamp_1 *= 24*36; // 86'4 * 10'957 = 9'466'848
  time_stamp_1 += ms / 100000;
  if (ms < 0)
  {
    time_stamp_2 = 100000 + ms % 100000;
    time_stamp_1--;
  } else
    time_stamp_2 = ms % 100000;

  // GMT time_stamp_1 -= 2*36;

  // ./ms2time.sh 1'399'346'245'967  ==> 2014-05-06 05:17:25
  // date -d "2014-05-06 05:17:25" ==>  1'399'346'245  (03:17:25 einsetzen GMT!)
  sprintf(stream, "%d%5.5d", time_stamp_1, time_stamp_2);
}

void KCanFrame::PrintDateTime(int days, int ms, char * stream)
{
  // format: YYYY-MM-DD HH:MM:SS.mmm
  int Day, Month, Year;
  char day_str[32];

  NUtils::GetDateFromDays(days, Day, Month, Year);
  NUtils::PrintDayTime(ms, day_str);

  sprintf(stream, "%4.4d-%2.2d-%2.2d %s", Year, Month, Day, day_str);
}

void KCanFrame::FormatFrame(bool csv, char * OutStr, bool UseElster) const
{
  char TimeStr[64];

  PrintTime(TimeStr, csv);
  if (csv)
  {
    sprintf(OutStr, "%u,%s,%u,%d", Counter, TimeStr,
            Id & 0xffffffff, Flags & 0xff);
    for (int i = 0; i < 8; i++)
    {
      strcat(OutStr, ",");
      if (i < Len)
        sprintf(OutStr + strlen(OutStr), "%d", Data[i]);
      else
        strcat(OutStr, "NULL");
    }
  } else {
    if (!Counter && !TimeStampDay)
      sprintf(OutStr, "%8x %2.2x [%d] ",
              Id & 0x1fffffff, Flags & 0xff, Len);
    else
      sprintf(OutStr, "%3u  %s  %8x %2.2x [%d] ", Counter, TimeStr,
              Id & 0x1fffffff, Flags & 0xff, Len);
    for (int i = 0; i < 8; i++)
      if (i < Len)
        sprintf(OutStr + strlen(OutStr), "%2.2x ", Data[i]);
      else
        sprintf(OutStr + strlen(OutStr), "   ");
    strcat(OutStr, " ");
    if (UseElster)
    {
      int elster_idx = GetElsterIdx();
      if (elster_idx >= 0)
      {
        const ElsterIndex * elst_ind = GetElsterIndex(elster_idx);
        int Value = GetValue();
        int d = Data[0] & 0xf;
        if (Value >= 0 && d != 1) // 1 is request flag
        {
          if (elst_ind)
            SetValueType(OutStr + strlen(OutStr), elst_ind->Type, (unsigned short) Value);
          else
            sprintf(OutStr + strlen(OutStr), "%d", (unsigned) Value);
          strcat(OutStr, " ");
        }
        strcat(OutStr, elst_ind ? elst_ind->Name : "?");
      }
    } else {
      for (int i = 0; i < 8; i++)
        if (i < Len)
          sprintf(OutStr + strlen(OutStr), "%c",
                  ' ' <= Data[i] && Data[i] < 127 ? Data[i] : '.');
    }
  }
  strcat(OutStr, "\n");
}

void KCanFrame::PrintFrame() const
{
  char Buffer[256];
  
  FormatFrame(false, Buffer);
  printf("%s", Buffer);
}

bool KCanFrame::InitFromLog(const char * log)
{
  // 225811  24.7.2014 00:00:05:437       480 00 [7] a1 00 fa 07 a8 00 00     .......
  //
  // alternativ (von CS_Bruecke)
  //
  // send: 0d 00 06 01 02 fa 00 08 00 c5 01 dd
  // recv: 06 02 0d 02 00 fa 00 08 00 c5 01 de
  // Elster 302.0008.00c5: RAUMSOLLTEMP_NACHT = 19.7

  
  bool Ok = true;
  unsigned Day, Month, Year;
  unsigned h, m, s, s1000;
  unsigned u, cnt = 0;
  
  Init();
  
  Counter = 0;
  NUtils::GetUnsigned(log, Counter);
  
  if (!strncmp(log, "Elster", 6))
  {
    return false;
  }
  if (!strncmp(log, "send: ", 6) ||
      !strncmp(log, "recv: ", 6))
  {
    log += 6;    
    
    KComfortFrame cf;
    Ok = cf.SetFromString(log, false);
    if (Ok)
      Ok = cf.SetCanFrame(*this);
    
    return Ok;
  }
  
  // Datum
  NUtils::SkipBlanks(log);
  if (Ok)
    Ok = NUtils::GetUnsigned(log, Day);
  if (Ok)
    Ok = *log == '.';
  if (Ok)
  {
    log++;
    Ok = NUtils::GetUnsigned(log, Month);
  }
  if (Ok)
    Ok = *log == '.';
  if (Ok)
  {
    log++;
    Ok = NUtils::GetUnsigned(log, Year);
  }
  if (Ok)
    NUtils::GetDays(Year, Month, Day, TimeStampDay);
  
  // Zeit
  NUtils::SkipBlanks(log);
  if (Ok)
    Ok = NUtils::GetUnsigned(log, h);
  if (Ok)
    Ok = *log == ':';
  if (Ok)
  {
    log++;
    Ok = NUtils::GetUnsigned(log, m);
  }
  if (Ok)
    Ok = *log == ':';
  if (Ok)
  {
    log++;
    Ok = NUtils::GetUnsigned(log, s);
  }
  if (Ok)
    Ok = *log == ':' || *log == '.';
  if (Ok)
  {
    log++;
    Ok = NUtils::GetUnsigned(log, s1000);
  }
  if (Ok)
    TimeStampMs = 1000*(3600*h + 60*m + s) + s1000;
  
  // Id
  NUtils::SkipBlanks(log);
  if (Ok)
  {
    Id = NUtils::GetHex(log);
    Ok = Id > 0 && Id <= 0x780;
  }
  
  NUtils::SkipBlanks(log);
  NUtils::GetUnsigned(log, u);
  NUtils::SkipBlanks(log);
  if (Ok && *log == '[')
    log++;
  else
    Ok = false;  
  if (Ok)
    Ok = NUtils::GetUnsigned(log, cnt);
  if (Ok)
    Ok = cnt == 5 || cnt == 7;
  Len = (int) cnt;
  if (Ok && *log == ']')
    log++;
  else
    Ok = false;
  
  if (Ok)
  {
    for (int i = 0; i < Len; i++)
    {
      NUtils::SkipBlanks(log);
      Data[i] = NUtils::GetHex(log);
    }
  }  
  
  return Ok;
}

////////////////////////////////////////////////////////////////////////////////

KComfortFrame KComfortFrame::InitFrame = {
  {0x0d, 0x00, 0x0d, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26}
};

const KComfortFrame::t_scan_geraete_id KComfortFrame::scan_geraete_id[] =
{
  { 0x601, 0x000b},
  { 0x301, 0x000b},
  { 0x480, 0x000b},
  { 0x500, 0x000b},
  // http://www.ip-symcon.de/forum/threads/15888-Wolf-CSZ-mit-ISM1
  // eventuell mit direkter RS232-Schnittststelle an WP
  { 0x000, 0x015b}, // GERAETEKONFIGURATION_2
  { 0x601, 0x0199}, // WPMiw (Xanatos) und WPM3 (Wert von 0x019a gehört auch dazu)
  { 0x301, 0x0199},
  { 0x480, 0x0199},
  { 0x500, 0x0199},
  { 0x000, 0x0199},
  { 0x000, 0x0000} // Ende der List
};

unsigned short KComfortFrame::InitId = 0xa51b;

unsigned KComfortFrame::GetCheckSum() const
{
  return 256*Data[10] + Data[11];
}

unsigned KComfortFrame::GetSum() const
{
  unsigned Sum = 0;
  for (int i = 0; i < 10; i++)
    Sum += Data[i];

  return Sum;
}

bool KComfortFrame::CheckSum() const
{
  return GetSum() == GetCheckSum();
}

void KComfortFrame::SetCheckSum()
{
  unsigned Sum = GetSum();

  Data[10] = (unsigned char)(Sum >> 8);
  Data[11] = (unsigned char)(Sum & 0xff);
}

bool KComfortFrame::SetCanFrame(KCanFrame & Frame) const
{
  Frame.Init();
  Frame.Len = 7;

  Frame.Id = ((Data[0] & 0x0f) << 7) + (Data[1] & 0xf);
  Frame.Data[0] = (Data[2] << 4) + (Data[3] & 0x0f);

  for (int i = 0; i < 6; i++)
    Frame.Data[i + 1] = Data[i + 4];

  return CheckSum() &&
         (Data[0] & 0xf0) == 0;  // 11-Bit-Id
}

bool KComfortFrame::GetCanFrame(const KCanFrame & Frame)
{
  Data[0] = (Frame.Id >> 7) & 0x0f;
  Data[1] = Frame.Id & 0x0f;
  Data[2] = Frame.Data[0] >> 4;
  Data[3] = Frame.Data[0] & 0x0f; // Sollte zum Abfragen 1 und für eine Antwort 2 sein!

  for (int i = 0; i < 6; i++)
    Data[i + 4] = Frame.Data[i + 1];

  if (Frame.Len == 5)
  {
    Data[8] = 0x80; // Stopf-Bytes initialisieren (siehe Marc2012)
    Data[9] = 0x08;
  }

  SetCheckSum();

  return (Frame.Len == 7 || Frame.Len == 5) &&
         (Frame.Id & ~0x7ff) == 0;  // 11-Bit-Id
}

void KComfortFrame::SetOk(bool Ok)
{
  for (int i = 0; i < 10; i++)
    Data[i] = Ok ? 0x55 : 0xAA;

  SetCheckSum();
}

void KComfortFrame::SetOkValue(unsigned Value)
{
  SetOk();

  Data[8] = (unsigned char)(Value >> 8);
  Data[9] = (unsigned char)(Value & 0xff);

  SetCheckSum();
}

bool KComfortFrame::EqualData(const KComfortFrame & Frame) const
{
  for (int i = 0; i < 12; i++)
    if (Data[i] != Frame.Data[i])
      return false;

  return true;
}

void KComfortFrame::SetToString(char * Str) const
{
  Str[0] = 0;
  for (int i = 0; i < 12; i++)
    sprintf(Str + strlen(Str), "%2.2x", Data[i]);
}

bool KComfortFrame::SetFromString(const char * Str, bool AppendCheck)
{
  for (int i = 0; i < 12; i++)
  {
    while (*Str == ' ' || *Str == '.') Str++;
    if (!NUtils::GetHexByte(Str, Data[i]))
    {
      if (!AppendCheck ||
          i != 10)
        return false;

      SetCheckSum();
      return true;
    }
  }
  return true;
}

bool KComfortFrame::IsInitFrame() const
{
  // 0d 00 00 00 00 fa 01 5b 00 00 01 63
  if (Data[0] == 0x0d && Data[1] == 0x00 &&
      Data[2] == 0x00 && Data[3] == 0x00 &&
      Data[4] == 0x00 && Data[5] == 0xfa &&
      Data[6] == 0x01 && Data[7] == 0x5b)
  {
    return true;
  }
  return EqualData(InitFrame);
}

int KComfortFrame::OkFrameValue() const
{
  if (!CheckSum())
    return -1;

  for (int i = 0; i < 8; i++)
    if (Data[i] != 0x55)
      return -1;

  return 256*Data[8] + Data[9];
}

void KComfortFrame::SetInitOk()
{
  SetOkValue(InitId);
}

KComfortFrame::msg_type KComfortFrame::GetMsgType() const
{
  if (IsInitFrame())
    return mt_init;

  // Vorsichtsmassnahmen:
  // - nur reine Abfragen zulassen
  // - Abfragen ohne 0xfa an das Gerät mit ID = 000 kann Nebenwirkungen
  //   auslösen! (deshalb nicht zulassen)
  if (Data[3] == 0x01) // Abfrage
  {
    if ((Data[4] & 0xf0) == 0 && // reine Abfragen
        (Data[5] == 0xfa || Data[5] == 0xfb || Data[5] == 0x10 || Data[2] != 0))
      return mt_get;

    return mt_invalid;
  }

  if (Data[3] == 0x00) // change
  {
    // Reset der Fehlerliste: 0d.00.09.00.00.fb.80.00.00.00.01.91   Mudultype: Manager
    // abtauen manual:        0d.00.09.00.00.5f.00.00.00.00.00.75
    if ((Data[4] & 0xf0) == 0 &&
        Data[5] != 0xfa)
      return mt_short_change;

    // Wert ändern
    // 0d 00 03-00-00 fa ...  (marc2012)
    if ((Data[4] & 0xf0) == 0 &&
        Data[5] == 0xfa && Data[2] != 0)
      return mt_change;
  }
  return mt_invalid;
}

////////////////////////////////////////////////////////////////////////////////

KCan232Frame::KCan232Frame(const char * str)
{
  unsigned len = 0;
  if (str)
    len = (unsigned) strlen(str);
  if (len >= High(msg))
    len = High(msg);
  if (len)
    strncpy(msg, str, len);
  msg[len] = 0;
}

bool KCan232Frame::SetCanFrame(KCanFrame & Frame) const
{
  Frame.Init();

  const char * Ptr = msg + 1;
  unsigned char b[4];

  if (!NUtils::GetHexByte(Ptr, b[0]))
    return false;

  switch (msg[0])
  {
    case 't':
    case 'r':
      if (b[0] > 0x7f ||
          !NUtils::IsHexDigit(*Ptr))
        return false;

      Frame.Id = 16*b[0] + NUtils::GetHexDigit(*Ptr++);
      break;

    case 'T':
    case 'R':
      if (b[0] > 0x3f ||
          !NUtils::GetHexByte(Ptr, b[1]) ||
          !NUtils::GetHexByte(Ptr, b[2]) ||
          !NUtils::GetHexByte(Ptr, b[3]))
        return false;

      for (int i = 0; i < 4; i++)
        Frame.Id = 256*Frame.Id + b[i];
      Frame.Id |= 0x80000000;
      break;

    default:
      return false;
  }
  if (msg[0] == 'r' || msg[0] == 'R') // RTR
    Frame.Id |= 0x40000000;

  if (*Ptr < '0' || *Ptr > '8')
    return false;

  Frame.Len = *Ptr++ - '0';

  for (int i = 0; i < Frame.Len; i++)
    if (!NUtils::GetHexByte(Ptr, Frame.Data[i]))
      return false;

  return true;
}

bool KCan232Frame::IsOk() const
{
  KCanFrame Frame;

  return SetCanFrame(Frame);
}

bool KCan232Frame::GetCanFrame(const KCanFrame & Frame)
{
  if (Frame.Id & 0x80000000)
    sprintf(msg, "T%8.8X%1.1X", Frame.Id & 0x3ffffff, Frame.Len & 0xf);
  else
    sprintf(msg, "t%3.3X%1.1X", Frame.Id & 0x7ff, Frame.Len & 0xf);

  // RTR
  if (Frame.Id & 0x40000000)
    msg[0] += 'R' - 'T';

  for (int i = 0; i < Frame.Len; i++)
    sprintf(msg + strlen(msg), "%2.2X", Frame.Data[i]);

  return true;
}

KCan232Frame & KCan232Frame::operator =(const char * str)
{
  unsigned l = 0;
  int m = 0;
  while (str && str[m] && l < High(msg))
  {
    if (str[m] > ' ')
      msg[l++] = str[m];
    m++;
  }
  msg[l] = 0;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////


#if (0)

// Test zu "timestamp" des Volkszählers
int main(int argc, char* argv[])
{
  int Days;

  // 1334416684000.  14. April 2012  17:19
  //
  // ./ms2time.sh 1399346245967  ==> 2014-05-06 03:17:25
  //              1399349845000
  //                   -36
  //              1399353445000
  //              1399334400000
  //                   11800
  // date -d "2014-05-06 05:17:25" ==>  1399346245

  if (NCanUtils::GetArtDays(2012, 4, 14, Days))
  {
    char str[64];
    char mstr[64];

    int sec = 60*(60*15 + 18) + 4;

    NCanUtils::PrintTime((int) Days, 1000*sec, str, false);
    NCanUtils::PrintMs((int) Days, 1000*sec, mstr);
    printf("%d   %s   %s\n", sec, mstr, str);
  }

  return 0;
}

#endif
