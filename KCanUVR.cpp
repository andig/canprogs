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
 
 Wird nicht länger unterstützt!
 
 */

#if defined(__UVR__)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "NTypes.h"

#include "KCanUVR.h"

#if defined(__MYSQL__)
  #include "KMySql.h"
#endif

#include "KElsterTable.h"
#include "NUtils.h"
#include "KHttpClient.h"

KCanUVR::KCanUVR()
{
  Trace = false;
}

bool KCanUVR::Init(const char * CanDev)
{
  Trace = false;

  return KCanServer::Init(CanDev);
}

int KCanUVR::sniff_index(unsigned can_id)
{
  if ((can_id & 0x7f) > 62 ||
      (can_id & 0x7f) == 0)
    return -1;

  switch (can_id & ~0x7f)
  {
    case 0x180:
      return 0;
    case 0x200:
      return 1;
    case 0x280:
      return 2;
    case 0x300:
      return 3;
    case 0x380:
      return 4;
    default:
      return -1;
  }
}

bool KCanUVR::AddToSniffed(const KCanFrame & Frame)
{
  int idx = sniff_index(Frame.Id);
  if (idx < 0)
    return false;

  sniffed_frame[idx][Frame.Id & 0x3f] = Frame;

  return true;
}

const KCanFrame * KCanUVR::GetSniffFrame(unsigned can_id)
{
  int idx = sniff_index(can_id);
  if (idx < 0)
    return NULL;

  return &(sniffed_frame[idx][can_id & 0x3f]);
}

void KCanUVR::EmptyServer()
{
  bool Ok;
  KCanFrame Frame;
  do
  {
    Ok = GetFrame(Frame);
    if (Ok)
    {
      AddToSniffed(Frame);
      if (Trace)
        Frame.PrintFrame();
    }
  } while (Ok);
}

bool KCanUVR::GetIdAndChan(const char * can_val, unsigned & can_id, int & chan)
{
  can_id = NUtils::GetHex(can_val);
  bool Ok = *can_val == '.';
  if (Ok)
  {
    TInt64 res;
    can_val++;
    Ok = NUtils::GetInt(can_val, res);
    can_id = (unsigned) res;
  }
  if (Ok)
    Ok = *can_val == 0;
  if (Ok)
    Ok = KCanUVR::sniff_index(can_id) >= 0;
  if (Ok)
  {
    if ((can_id & 0xf80) == 0x180)
      Ok = 1 <= chan && chan <= 16;
    else
      Ok = 1 <= chan && chan <= 16;
  }
  return Ok;
}

#if defined(__CAN__) && defined(__SCAN__)

#include <time.h>

#include "XmlParser.h"
#include "KTcpCanClient.h"


int main(int argc, char* argv[])
{
  printf("Volkszaehler can-bus UVR logger\n"
         "copyright (c) 2014 Jürg Müller, CH-5524\n\n");

  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;

  if (argc != 2)
  {
    printf("usage:\n"
           "  can_vz_uvr <xml configuration> [ \"trace\" ]\n\n"
           "example: can_vz_uvr VZ_config.xml\n\n");
    return 0;
  }

  KXmlNode Root;
  const KXmlNode * Node;
  KStream buffer;

  if (!KParser::ParseFile(argv[1], &Root))
    return -1;

  // device Name="can0"
  // template Name=...
  // ip_address Name="...
  // sleep="104"
  // channel   mehrfach
  const char * device = Root.FindNodeAttr("device", "Name");
  const char * template_ = Root.FindNodeAttr("template", "Name");
  const char * ip_address = Root.FindNodeAttr("ip_address", "Name");
  const char * secs = Root.FindNodeAttr("sleep", "Name");
  int seconds = 104;
#if defined(__MYSQL__)
  bool use_mysql = Root.Find("mysql") != NULL;
  KMySql MySql;
#else
  bool use_mysql = false;
#endif

  if (!use_mysql)
  {
    if (!test_template(template_))
      return -1;

    if (!ip_address || strlen(ip_address) == 0)
    {
      printf("IP address is missing\n");
      return -1;
    }
  }

#if defined(__MYSQL__)
  if (use_mysql &&
      !MySql.GetXmlParams(&Root))
    return -1;
#endif

  if (secs)
  {
    TInt64 sec;
    if (NUtils::GetInt(secs, sec) &&
        !*secs &&
        60 <= sec && sec <= 3600)
      seconds = (int) sec;
  }
  if (trace)
  {
    printf("device: %s\n", device);
    printf("sleep: %d secs\n", seconds);
    if (!use_mysql)
      printf("IP address: %s\n", ip_address);
    printf("\n");
  }

  buffer.SetSize(4000);
  if (!Root.channel_check(use_mysql, template_, &buffer))
    return -1;

  KCanUVR Can;

  KHttpClient TcpClient;
  time_t timer;
  time_t new_timer;

  TcpClient.mTrace = trace;
  TcpClient.SetBlockMode(bmBlocking);
  TcpClient.SetRemoteHost(ip_address);
  TcpClient.SetRemotePort(80);

  if (!Can.Init(device))
  {
    printf("device \"%s\" not connected\n", Can.GetDev());

    Can.Halt();

    return -1;
  }

  Can.Trace = trace;
  Can.Resume();

  NUtils::SleepMs(10000);
  while (!Can.Terminated())
  {
    time(&timer);

    Can.EmptyServer();

  #if defined(__MYSQL__)
    if (use_mysql)
    {
      MySql.Open();
    }
  #endif

    // der "log" ist für die Aufdeckung von Fehlerquellen gedacht
    FILE * log = NULL; // fopen("uvr.log", "ab+");
    Node = NULL;
    while (Root.GetNextNode(Node))
    {
      if (!strcmp(Node->mName, "channel"))
      {
        const char * can_val = Node->FindAttr("can_val"); // <can_id>.<1..10> (hex.)
        unsigned can_id = 0;
        int chan;
        bool Ok = Can.GetIdAndChan(can_val, can_id, chan);
        const KCanFrame * Frame = Can.GetSniffFrame(can_id);

        if (Ok)
          Ok = Frame != NULL && Frame->TimeStampDay > 0;
        if (Ok)
        {
          int Days = Frame->TimeStampDay;
          int Ms = Frame->TimeStampMs;

          bool IsNew = Days > Node->time_days;
          if (!IsNew &&
              Days == Node->time_days)
            IsNew = Ms - Node->time_ms > 10;
          if (!IsNew)
            NUtils::Time(Days, Ms);

          const_cast<KXmlNode *>(Node)->time_days = Days;
          const_cast<KXmlNode *>(Node)->time_ms = Ms;
          unsigned short val = 0;
          if ((can_id & 0xf80) == 0x180)
          {
            Ok = Frame->Len >= 2;
            val = Frame->Data[0] + 256*Frame->Data[1];
            if (chan > 1)
              val <<= chan - 1;
            val = (val & 0x8000) ? 1 : 0;
          } else {
            Ok = Frame->Len >= 2*chan;
            val = Frame->Data[2*chan-2] + 256*Frame->Data[2*chan-1];
          }
          const_cast<KXmlNode *>(Node)->can_value = val;
        }
        if (Ok)
          Ok = const_cast<KXmlNode *>(Node)->set_channel_value(&buffer, template_, trace);
        if (!Ok)
        {
        } else
      #if defined(__MYSQL__)
        if (use_mysql)
        {
          Ok = MySql.Query(buffer, trace);

          if (log)
          {
            int nowDays, nowMs;
            char str[64];
            char now[64];

            NUtils::Time(nowDays, nowMs);
            NUtils::PrintDayTime(nowMs, now);

            NUtils::PrintTime(Node->time_days, Node->time_ms, str, false);
            fprintf(log, "%s - now: %s\n", str, now);
            fprintf(log, "%d: %s\n", (int)Ok, (const char *) buffer.GetMemory());
          }
        } else
      #endif
          TcpClient.ServeStream(buffer);
      }
    }
    if (log)
      fclose(log);

  #if defined(__MYSQL__)
    if (use_mysql)
    {
      MySql.Close();
    }
  #endif

    time(&new_timer);
    int del = seconds - (int)(new_timer - timer);
    if (del < 45)
      del = 45;
    while (del > 0)
    {
      int d = del < 10 ? del : 10;

      NUtils::SleepMs(d*1000);
      del = del < 10 ? 0 : del - 10;

      Can.EmptyServer();
    }
  }

  return 0;
}

#endif

#endif // __UVR__