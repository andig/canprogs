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

#if !defined(__UVR__)

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "NTypes.h"

#if defined(__MYSQL__)
  #include "KMySql.h"
#endif

#include "NUtils.h"
#include "NCanUtils.h"
#include "KCanElster.h"

#include "KElsterTable.h"
#include "KScanTable.h"

KCanElster::KCanElster()
{
  Trace = false;
  SendCanId = 0x680;

 
}

KCanElster::~KCanElster()
{
 
}

bool KCanElster::Init(const char * CanDev, unsigned short DefaultId)
{
  Trace = false;
  SendCanId = DefaultId;

  return KCanServer::Init(CanDev);
}

void KCanElster::PrintSendFrame()
{
  SendFrame.PrintFrame();
}

void KCanElster::PrintRecvFrame()
{
  RecvFrame.PrintFrame();
}

bool KCanElster::Send_Frame()
{
  bool Ok = /* SendDriver-> */ SendData(SendFrame);

  if (Trace)
    PrintSendFrame();

  return Ok;
}

bool KCanElster::SniffedFrame()
{
  bool Ok = GetFrame(RecvFrame);

  if (Ok)
  {
    KSniffedFrame * SF = (KSniffedFrame *) SniffedData.SearchSniffedFrame(RecvFrame);
    if (SF)
    {
      SF->SetValue(RecvFrame);
    }
  }
  return Ok;
}

#if defined(KSniffedFrame_H)
const KSniffedFrame * KCanElster::GetFirstSniffedFrame()
{
  return SniffedData.GetFirstSniffedFrame();  
}
#endif

void KCanElster::EmptyServer()
{
  bool Ok;
  do
  {
    Ok = SniffedFrame();
    if (Ok && 
        SniffedData.SearchSniffedFrame(RecvFrame))
    {
      if (Trace)
        PrintRecvFrame();
    }
  } while (Ok);
}

bool KCanElster::Get_Frame()
{
  bool Ok = SniffedFrame();

  if (Ok && Trace)
    PrintRecvFrame();

  return Ok;
}

bool KCanElster::Send(unsigned count, bool WaitForAnswer, int inner_delay)
{
  for (int i = 0; (unsigned) i < count; i++)  // 3 Versuche
  {
    int wait_delay = 1000; // zwischen 2 Versuchen
    EmptyServer();

    if (Send_Frame())
    {
      bool SendFrameReceived = driver_type == NCanUtils::dt_cs ||
    #if defined(__WINDOWS__)
                               driver_type == NCanUtils::dt_8dev ||
    #endif
                               driver_type == NCanUtils::dt_can232;
      for (int k = 0; !SendFrameReceived && k < 20; k++)
      {
        NUtils::SleepMs(1);
        if (Get_Frame())
          SendFrameReceived = RecvFrame.EqualData(SendFrame);
      }
      if (SendFrameReceived)
      {
        if (!WaitForAnswer)
          return true;

        int del = count > 1 ? 400 : inner_delay;
        for (int k = 0; k < del; k++)
        {
          while (Get_Frame())
          {
            bool Ok = false;
            if (driver_type == NCanUtils::dt_cs)
            {
              if (RecvFrame.Id == SendFrame.Id) // Echo
                continue;
              
              Ok = RecvFrame.Len > 0;
            } else
            if (RecvFrame.IsAnswerToElsterIndex(SendFrame))
              Ok = true;
          #if !defined(__UVR__)
            if (Ok)
              SniffedData.ClearSniffedValue(RecvFrame);
          #endif
            return Ok;
          }
          NUtils::SleepMs(1);
        }
        wait_delay -= del;
      }
    }
    if ((unsigned)(i+1) < count)
      NUtils::SleepMs(wait_delay);
  }

  return false;
}

bool KCanElster::InitSendFrame(unsigned short receiver_id, unsigned short elster_idx)
{
  return SendFrame.InitElsterFrame(SendCanId, receiver_id, elster_idx);
}

bool KCanElster::GetValue(unsigned short receiver_id, unsigned short elster_idx, unsigned short & Value)
{
  InitSendFrame(receiver_id, elster_idx);

  if (Send() && RecvFrame.Len == 7)
  {
    int val = -1;
    
    val = RecvFrame.GetValue();
    if (val < 0)
      return false;
    
    Value = (unsigned short) val;
    
    return true;
  }

  return false;
}

bool KCanElster::SetValue(unsigned short receiver_id, unsigned short elster_idx, unsigned short Value)
{
  InitSendFrame(receiver_id, elster_idx);

  SendFrame.Data[0]--;
  SendFrame.SetValue(Value);
 
  return SetValue();
}

bool KCanElster::SetValue()
{
  if ((SendFrame.Data[0] & 0x0f) != 0) // change data flag
    return false;

  if (!Send(1, false))
    return false;

  NUtils::SleepMs(50);
  SendFrame.Data[0]++; // receive data flag
  bool Ok = Send();

  if (Ok && !RecvFrame.Changed(SendFrame))
    Ok = false;

  SendFrame.Data[0]--; // make original flag
  
  return Ok;
}

void KCanElster::GetValidDevices()
{
  printf("list of valid can id's:\n\n");
  for (int recv = 0; recv < 0x10; recv++)
    if (recv != SendCanId / 0x80)
    {
      int count = 0;
      for (int i = 0; i < 0x10; i++)
      {
        unsigned short Value;
        unsigned short val1 = 0;
        unsigned short val2 = 0;
        
        if (GetValue(0x80*recv + i, 0x000b, Value))
        {
          char str[64];

          if (Value == 0x8000)
          {
            GetKennwortValue(0x80*recv + i, 0x199, Value, val1, val2);
          }

          count++;
          if (val2 != 0x8000 && val2)
            sprintf(str, "%d-%2.2d-", val1, val2 & 0xff);
          else
            SetValueType(str, et_dev_id, Value);
          printf("  %3.3x (%4.4x = %s)\n",
                 0x80*recv + i, Value, str);
          SniffedData.SetUsedCanId((unsigned char)(16*recv + i));
        }
        if (count < i-2)
          break;
      }
    }
  printf("\n");
}

void KCanElster::Scanner(unsigned short receiver_id, bool get_all)
{
  unsigned short Value;

  for (unsigned elster_idx = 0; elster_idx < 0x10000; elster_idx++)
  {
    if (receiver_id == 0)
      elster_idx = 0x000b;

    const ElsterIndex * Elster = GetElsterIndex(elster_idx);
#if !defined(__ROTEX__)
    if (elster_idx > 0x0200 && !Elster)
    {

    } else
#endif
    if (GetValue(receiver_id, elster_idx, Value) &&
        Value != 0x8000)
    {
      printf("  { 0x%3.3x, 0x%4.4x, 0x%4.4x},", receiver_id, elster_idx, Value);
      if (Elster)
      {
        char Val[32];

        SetValueType(Val, Elster->Type, Value);
        printf("  // %s: %s", Elster->Name, Val);
      }
      printf("\n");
    }
    if (receiver_id == 0)
      break;
  }
}

bool KCanElster::GetKennwortValue(unsigned short recv_id, unsigned short elster_idx,
                            unsigned short & Value,
                            unsigned short &Value1, unsigned short & Value2)
{
  bool Ok = GetValue(recv_id, elster_idx, Value) &&
                     Value != 0x8000 && Value != 0;

  // Vor allem neuere Geräte, wie die WPM3, verwenden 0x0199 und 0x019a
  // als Versionsnummern. Der Wert an der Stelle 0x0199 ist meist grösser
  // als 256 und passt damit nicht in das alte Schema.
   if (Ok && elster_idx == 0x0199)
  {
    // z.B. WPMme
    Ok = GetValue(recv_id, 0x019a, Value2);
    Value1 = Value;
    Value = 0x8000;
  }
  return Ok;
}

bool KCanElster::GetInitCS_Id()
{
  unsigned short Value;
  int i = 0;

  while (KComfortFrame::scan_geraete_id[i].elster_idx)
  {
    unsigned short recv_id = KComfortFrame::scan_geraete_id[i].recv_id;
    unsigned short elster_idx = KComfortFrame::scan_geraete_id[i].elster_idx;
    unsigned short Value1 = 0, Value2 = 0;

    if (GetKennwortValue(recv_id, elster_idx, Value, Value1, Value2))
    {
      KComfortFrame::InitId = Value;
      return true;
    }
    i++;
  }
  return false;
}

bool KCanElster::GetDoubleValue(unsigned short first_val,
                                unsigned scan_can_id,
                                unsigned short elster_idx,
                                unsigned char elster_type,
                                double & result)
{
  if (first_val == 0x8000)
    return false;

  if (elster_type != et_double_val &&
      elster_type != et_triple_val)
    return false;

  unsigned short sec_val;
  unsigned short third_val;
  NUtils::SleepMs(100);
  if (!GetValue(scan_can_id, elster_idx - 1, sec_val))
    return false;

  result = (double)(first_val) + (double)(sec_val) / 1000.0;

  if (elster_type == et_triple_val)
  {
    NUtils::SleepMs(100);
    if (!GetValue(scan_can_id, elster_idx - 2, third_val))
      return false;

    result += (double)(third_val) / 1000000.0;
  }
  return true;
}

#if defined(__USBTIN__)

bool KCanElster::InitUSBtin(const char * com_dev, bool trace)
{
  driver_type = NCanUtils::dt_can232;
  AsSimulation = true;
  if (!Init(com_dev, 0x680))
    return false;

  SetTrace(trace);
  Resume();
  NUtils::SleepMs(100);

  return true;
}

bool KCanElster::ServeUSBtin()
{
  if (Terminated())
    return false;

  KCanFrame Frame;

  if (GetFrame(Frame))
  {
    KComfortFrame cf;
    KComfortFrame cfRes;
    char str[64];

    if ((Frame.Data[0] & 0xf) == 2) // change flag
      Frame.Data[0] &= 0xf0;

    cf.GetCanFrame(Frame);

    if (cf.IsInitFrame())
    {
      cfRes.SetOkValue(KComfortFrame::InitId);
    } else
    if (!KScanTable::Get_scan_value(cf, cfRes))
      cfRes.SetOk(false);

    if (cfRes.SetCanFrame(Frame) &&
        FormElsterTable(Frame, str))
    {
      if (Trace)
        printf("Elster %3.3x.%4.4x.%4.4x: %s\n", Frame.Id, Frame.GetElsterIdx(), (unsigned short) Frame.GetValue(), str);
    }
    if (Frame.Data[0] != 0x55 && Frame.Data[0] != 0xAA)
      SendData(Frame);
  }
  return true;
}

#endif

#if defined(__SCAN__)

void PrintComfortSoftFormat(const KCanFrame & Frame)
{
  char str[128];
  KCanFrame Frame1;
  KComfortFrame cf;
  KCan232Frame can232;

  cf.GetCanFrame(Frame);
  cf.SetToString(str);
  can232.GetCanFrame(Frame);
  printf("ComfortSoft: %s   can232: %s\n", str, can232.msg);
  cf.SetCanFrame(Frame1);

  bool Equal = Frame.Id == Frame1.Id;
  for (int i = 0; i < Frame.Len; i++)
    if (Frame.Data[i] != Frame1.Data[i])
      Equal = false;
  if (!Equal)
  {
    Frame1.FormatFrame(false, str);
    printf("invalid back translated telegram: %s\n", str);
  }
}

int main(int argc, char* argv[])
{
  KCanElster Can;

  printf("elster-kromschroeder can-bus address scanner and test utility\n"
#if defined(__WINDOWS__)
         "copyright (c) 2014 Juerg Mueller, CH-5524\n\n");
#else
         "copyright (c) 2014 Jürg Müller, CH-5524\n\n");
#endif

  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;

  bool cs_com = !strcmp(argv[argc-1], "cs_com");
  if (cs_com)
  {
    argc--;
    Can.driver_type = NCanUtils::dt_cs;
  }

  bool params_ok = argc == 3 || argc == 4;
  bool scan_only = false;
  bool write = false;
  unsigned short elster_idx = 0;
  unsigned short Value = 0;
  const char * Line;
  const char * last_pos;
  unsigned sender_can_id = 0;
  unsigned scan_can_id = 0;

  if (params_ok)
  {
    Line = argv[2];
    sender_can_id = NUtils::GetHex(Line);
    switch (sender_can_id & ~0x1f)
    {
#if defined(__ROTEX__)
      case 0x100:
      case 0x180:
      case 0x300:
#endif
      case 0x680:
      case 0x700:
      case 0x780:
        break;
      default:
        params_ok = false;
        break;
    }
  }

  bool get_total = false;
  if (params_ok && argc == 4)
  {
    get_total = !strcmp("total", argv[3]);
  }
  if (params_ok && argc == 4 && !get_total)
  {
    Line = argv[3];
    last_pos = Line;
    // 000.0000 oder 000.0000.0000
    scan_can_id = NUtils::GetHex(Line);
  #if !defined(__ROTEX__)
    if (Line - last_pos != 3 ||
        (scan_can_id < 0x100 && scan_can_id > 7) ||
        ((scan_can_id & 0xf80) == (sender_can_id & 0xf80)))
    {
      printf("\n%x is not a legal can id!\n\n", scan_can_id);
      params_ok = false;
    }
  #endif
    last_pos = Line;
    scan_only = *Line != '.';
    if (!scan_only)
    {
      elster_idx = NUtils::GetHex(++Line);
      if (Line - last_pos != 5)
        params_ok = false;
    }
    last_pos = Line;
    write = *Line == '.';
    if (write)
    {
      Value = NUtils::GetHex(++Line);
      if (Line - last_pos != 5 ||
          sender_can_id == 0)
        params_ok = false;
    }
  }
  if (!params_ok)
  {
    printf("use scanner\n"
           "  can_scan <can dev> <sender can id> [ \"total\" ] [ \"cs_com\" ] [ \"trace\" ]\n"
           "or read/change value\n"
           "  can_scan <can dev> <sender can id> <receiver can id> [ \".\" <elster index> [ \".\" <new value> ] ] [ \"cs_com\" ] [ \"trace\" ]\n\n"
           "example: can_scan can0 680 180           (scan all elster indices from 0000 to 1fff)\n"
           "or       can_scan can0 680 180.0126      (read minutes at elster index 0126)\n"
           "or       can_scan can0 680 180.0126.0f00 (set minutes to 15)\n\n"
           "allowed sender can id: 680 or 700 or 780\n\n"
           "BE CAREFUL WITH THIS TOOL !!!\n\n");
    return 0;
  }

  if (!Can.Init(argv[1], sender_can_id))
  {
    printf("device \"%s\" not connected\n", argv[1]);
    return -1;
  }
  
  bool Ok = true;
  Can.Trace = trace;
  Can.Resume();

  if (get_total)
  {
    printf("scan on CAN-id: %3.3x\n", sender_can_id);
    Can.GetValidDevices();
    printf("\n");
    for (int i = 0; i < 256; i++)
      if (Can.SniffedData.GetUsedCanId(i))
        Can.Scanner(8*(i & 0xf0) + (i & 0xf), true);
  } else
  if (argc == 3)
  {
    Can.GetValidDevices();
  } else
  if (scan_only)
  {
    Can.Scanner(scan_can_id, false);
  } else {
    if (write)
      Ok = Can.SetValue(scan_can_id, elster_idx, Value);
    else
      Ok = Can.GetValue(scan_can_id, elster_idx, Value);

    if (Ok && trace)
    {
      printf("\nsend ");
      PrintComfortSoftFormat(Can.SendFrame);
      printf("recv ");
      PrintComfortSoftFormat(Can.RecvFrame);
      printf("\n");
    }

    if (!Ok)
    {
      if (write)
        printf("value not set\n");
      else
        printf("no value read\n");
    } else {
      if (write)
        printf("value set\n");

      printf("value: %4.4x", Value);

      const ElsterIndex * elst_ind = GetElsterIndex(elster_idx);
      if (elst_ind)
      {
        char Val[32];
        double d;

        if (!write && Can.GetDoubleValue(Value, scan_can_id, elster_idx, elst_ind->Type, d))
        {
          SetDoubleType(Val, elst_ind->Type, d);
        } else
          SetValueType(Val, elst_ind->Type, Value);
        printf("  (%s  %s)", elst_ind->Name, Val);
      }
      printf("\n");
    }
  }
  Can.Halt();
  
  return Ok ? 0 : 1;
}

#endif

#if defined(__VZ__)

#include <time.h>

#include "XmlParser.h"
#include "KHttpClient.h"

int main(int argc, char* argv[])
{
#if defined(__WINDOWS__)
  printf("Volkszaehler can-bus logger\n"
         "copyright (c) 2014 Juerg Mueller, CH-5524\n\n");
#else
  printf("Volkszaehler can-bus logger\n"
         "copyright (c) 2014 Jürg Müller, CH-5524\n\n");
#endif

  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;

  bool cs_com = !strcmp(argv[argc-1], "cs_com");
  if (cs_com)
    argc--;

  if (argc != 2)
  {
    printf("usage:\n"
           "  can_vz <xml configuration> [ \"trace\" ]\n\n"
           "example: can_vz VZ_config.xml\n\n");
    return 0;
  }

  KXmlNode Root;
  KStream buffer;

  if (!KParser::ParseFile(argv[1], &Root))
    return -1;

  const char * sender = Root.FindNodeAttr("sender", "Name");
  const char * device = Root.FindNodeAttr("device", "Name");
  const char * sub_dev = Root.FindNodeAttr("device", "SubDev"); 
  const char * template_ = Root.FindNodeAttr("template", "Name");
  const char * ip_address = Root.FindNodeAttr("ip_address", "Name");
  const char * secs = Root.FindNodeAttr("sleep", "Name");
  unsigned sender_id = 0x700;
#if defined(__MYSQL__)
  bool use_mysql = Root.Find("mysql") != NULL;
  KMySql MySql;
#else
  bool use_mysql = false;
#endif
  int seconds = 104;

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

  if (sender)
  {
    unsigned id = NUtils::GetHex(sender);
    switch (id)
    {
      case 0x580:
      case 0x680:
      case 0x700:
      case 0x780:
        sender_id = id;
        break;
      default:
        break;
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
    if (sub_dev)
      printf("sub_dev: %s\n", sub_dev);
    printf("sender: %x\n", sender_id);
    printf("sleep: %d secs\n", seconds);
    if (!use_mysql)
      printf("IP address: %s\n", ip_address);
    printf("\n");
  }
  if (!NUtils::set_term_signal())
  {
    return -1;
  }

  buffer.SetSize(4000);
  if (!Root.channel_check(use_mysql, template_, &buffer))
    return -1;

  KCanElster Can;
  bool added = false;

  if (cs_com)
    Can.driver_type = NCanUtils::dt_cs;

  const KXmlNode * Node = NULL;
  // passive (sniffed) Frames bereitstellen (AddToSniffed)
  while (Root.GetNextNode(Node))
  {
    if (!strcmp(Node->mName, "channel"))
    {
      KCanFrame Frame;
      const char * can_msg = Node->FindAttr("can_msg");
      const char * can_inst = Node->FindAttr("can_inst");
      const char * format = Node->FindAttr("format");

      if (can_inst)
        Frame.SetElsterFrame(can_inst);
      else
        Frame.GetDataFromStream(can_msg);
      if (Can.SniffedData.SearchSniffedFrame(Frame))
      {
        printf("can_msg \"%s\" already exists\n", can_msg);
      }
      if (Can.SniffedData.AddToSniffed(Frame) && trace)
      {
        printf("frame can_msg added: %s\n", can_msg);
        added = true;
      }

      if (!format)
      {
        int elster_idx = Frame.GetElsterIdx();
        const ElsterIndex * Idx = elster_idx > 0 ? GetElsterIndex((unsigned short) elster_idx) : NULL;
        if (Idx && Idx->Type != et_default)
        {
          const char * name = ElsterTypeToName(Idx->Type);

          const_cast<KXmlNode *>(Node)->AppendAttr("format", name, (int) strlen(name));
        }
      }
    }
  }
  if (added)
    printf("\n");

  KHttpClient TcpClient;
  time_t timer;
  time_t new_timer;

  TcpClient.mTrace = trace;
  TcpClient.SetBlockMode(bmBlocking);
  TcpClient.SetRemoteHost(ip_address);
  TcpClient.SetRemotePort(80);

  if (sub_dev)
  {
    if (!stricmp(sub_dev, "cs_com"))
      Can.driver_type = NCanUtils::dt_cs;
  }
  if (!Can.Init(device, sender_id))
  {
    printf("device \"%s\" not connected\n", Can.GetDev());

    Can.Halt();

    return -1;
  }

  Can.Trace = trace;
  Can.Resume();

  NUtils::SleepMs(1000);
  while (!Can.Terminated() && !NUtils::term_signal_is_set())
  {
    Node = NULL;
    time(&timer);
    while (Root.GetNextNode(Node) && !NUtils::term_signal_is_set())
    {
      if (!strcmp(Node->mName, "channel"))
      {
        const char * can_msg = Node->FindAttr("can_msg");
        const char * can_inst = Node->FindAttr("can_inst");
        const char * format = Node->FindAttr("format");

        const_cast<KXmlNode *>(Node)->can_value = 0x8000;
        const_cast<KXmlNode *>(Node)->double_value = 0;

        Can.SendFrame.Init();
        if (can_inst)
          Can.SendFrame.SetElsterFrame(can_inst);
        else
          Can.SendFrame.GetDataFromStream(can_msg);
        Can.SendFrame.Id = Can.SendCanId;
        if (!NUtils::term_signal_is_set())
        {
          if ((Can.SendFrame.Data[0] & 0xf) != 1)
          {
            Can.EmptyServer();

            // passive Telegramme auswerten
            const KSniffedFrame * SF = Can.SniffedData.SearchSniffedFrame(Can.SendFrame);
            if (SF)
            {
              const_cast<KXmlNode *>(Node)->can_value = SF->value;
              const_cast<KXmlNode *>(Node)->time_days = Can.SendFrame.TimeStampDay;
              const_cast<KXmlNode *>(Node)->time_ms = Can.SendFrame.TimeStampMs;

           // !!!!!   SF->SetValue_(0x8000);
            }
          } else {
            if (Can.Send())
            {
              const_cast<KXmlNode *>(Node)->can_value = (unsigned short) Can.RecvFrame.GetValue();
              const_cast<KXmlNode *>(Node)->time_days = Can.RecvFrame.TimeStampDay;
              const_cast<KXmlNode *>(Node)->time_ms = Can.RecvFrame.TimeStampMs;

              ElsterType elster_type = GetElsterType(format);
              if (elster_type == et_double_val ||
                  elster_type == et_triple_val)
              {
                if (!Can.GetDoubleValue(Node->can_value,
                                        Can.RecvFrame.Id,
                                        Can.SendFrame.GetElsterIdx(),
                                        elster_type,
                                        const_cast<KXmlNode *>(Node)->double_value))
                  const_cast<KXmlNode *>(Node)->can_value = 0x8000;
              }
            }
            NUtils::SleepMs(1000); // 1 Sek. zwischen 2 Telegrammen abwarten
          }
        }
      }
    }

  #if defined(__MYSQL__)
    if (use_mysql)
    {
      MySql.Open();
    }
  #endif
    Node = NULL;
    while (Root.GetNextNode(Node) && !NUtils::term_signal_is_set())
    {
      if (!strcmp(Node->mName, "channel"))
      {
        if (Node->can_value != 0x8000)
        {
          bool Ok = const_cast<KXmlNode *>(Node)->set_channel_value(&buffer, template_, trace);

          if (!Ok)
          {
            if (trace)
            {
              const char * name = Node->FindAttr("Name");
              printf("\"%s\" has non value\n", name);
            }
          } else
        #if defined(__MYSQL__)
          if (use_mysql)
          {
            MySql.Query(buffer, trace);
          } else
        #endif
            TcpClient.ServeStream(buffer);

          const_cast<KXmlNode *>(Node)->can_value = 0x8000;
        } else
        if (trace)
        {
          const char * name = Node->FindAttr("Name");
          printf("\"%s\" has non or invalid value\n", name);
        }
      }
    }

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
    while (del > 0 && !NUtils::term_signal_is_set())
    {
      int d = del < 10 ? del : 10;

      while (d-- && !NUtils::term_signal_is_set())
        NUtils::SleepMs(1000);
      del = del < 10 ? 0 : del - 10;

      Can.EmptyServer();
    }
  }
  printf("\nbye-bye\n");
  return 0;
}

#endif

#if defined(__SIMULATION__)

int main(int argc, char* argv[])
{
  printf("optical interface simulator\n"
#if defined(__WINDOWS__)
         "copyright (c) 2014 Juerg Mueller, CH-5524\n\n");
#else
         "copyright (c) 2014 Jürg Müller, CH-5524\n\n");
#endif

  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;

  KCanElster CanElster;
  KCanFrame Frame;
  KComfortFrame cf;
  KComfortFrame cfRes;
  char str[256];
  bool start_ok = argc == 2 || argc == 3;

  if (argc == 3)
  {
    if (!KScanTable::LoadScanTable(argv[2]))
    {
      printf("\nscan table \"%s\" not loaded\n", argv[2]);
      start_ok = false;
    }
  } else
    KScanTable::Init_scan_table();

  if (!start_ok)
  {
    printf("\nusage:\n"
#if defined(__WINDOWS__)
           "  can_simul <com dev> [ <scan table name> ] [ \"trace\" ]\n\n"
           "example: can_simul com3 ..\\scan_data.inc \n\n");
#else
           "  can_simul <tty dev> [ <scan table name> ] [ \"trace\" ]\n\n"
           "example: can_simul tty.usbserial-FTK1S17H ../scan_data.inc \n\n");
#endif
    return -1;
  }

  CanElster.driver_type = NCanUtils::dt_cs;
  CanElster.Trace = trace;
  if (!CanElster.Init(argv[1], 0x680))
  {
    printf("initialization failed\n");
    printf("device \"%s\" not opened\n", argv[1]);
    return  -1;
  }
  
  CanElster.Trace = trace;
  CanElster.Resume();
  NUtils::SleepMs(100);
  
  if (CanElster.Terminated())
    printf("terminated\n");

  do
  {
    if (CanElster.GetFrame(Frame))
    {
      cf.GetCanFrame(Frame);

      if (trace)
      {
        cf.SetToString(str);
        printf("recv %s\n", str);
      }
      if (cf.IsInitFrame())
      {
        cfRes.SetOkValue(KComfortFrame::InitId);
      } else
        if (!KScanTable::Get_scan_value(cf, cfRes))
        cfRes.SetOk(false);

      if (trace &&
          cfRes.SetCanFrame(Frame) &&
          FormElsterTable(Frame, str))
      {
        printf("Elster %3.3x.%4.4x.%4.4x: %s\n", Frame.Id, Frame.GetElsterIdx(), (unsigned short) Frame.GetValue(), str);
      }
      cfRes.SetToString(str);
      if (trace)
        printf("send %s\n", str);

      CanElster.SendData(cfRes);
    }
    NUtils::SleepMs(1);
  } while (!CanElster.Terminated());

  CanElster.Halt();
  
  printf("bye-bye\n\n");

  return 0;
}

#endif

#if defined(__USBTIN_SIMULATION__) && defined(__USBTIN__)

/*
 
 Auf dem MAC mit socat testen:
 
 socat -d -d pty,raw,echo=0 pty,raw,echo=0
 
 2017/02/01 15:34:00 socat[36180] N PTY is /dev/ttys001
 2017/02/01 15:34:00 socat[36180] N PTY is /dev/ttys002
 2017/02/01 15:34:00 socat[36180] N starting data transfer loop with FDs [5,5] and [7,7]

 ./usbtin_simul ttys001
 
 und 
 
 ./can_scan tty002 680
 
 -------------------------------------------------------------------------
 
 Simulation von Wärmepumen-Echt-Zeit-Daten
 
 ./usbtin_simul ttys001 mac/can_log_20140724.log
 
 In can_log_20140724.log sind Daten von einem ganzen Tag aufgezeichnet. 
 Die Daten werden zur aufgezeichneten Tageszeit in der Simulation ausgegeben.
 
 Log-File muss die Erweiterung ".log" haben.
 
 Mit der Simulation der Echt-Zeit-Daten soll die passive Datenaufzeichnung
 (Daten, die nicht aktiv angefordert werden) getestet werden.
 
                                                                            /==> Simulationsdaten aus "Scan"
 Anwendung ("can_logger", "can_scan", usw.) <== socat ==> USBtin Simulator <
                                                                            \==> Live-Daten gemäss "Log"
*/

#include "KCanLogFile.h"

int main(int argc, char* argv[])
{
  printf("USBtin simulator\n"
#if defined(__WINDOWS__)
         "copyright (c) 2014 Juerg Mueller, CH-5524\n\n");
#else
         "copyright (c) 2014 Jürg Müller, CH-5524\n\n");
#endif

  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;

  KCanElster CanElster;
  KCanLogFile logfile;
  KCanFrame Frame;
  bool start_ok = argc >= 2 && argc <= 4;
  const char * log_file = NULL;
  const char * inc_file = NULL;
  
  if (start_ok && argc >= 3)
  {
    if (!stricmp(NUtils::ExtractExtension(argv[2]), ".log"))
      log_file = argv[2];
    else
      inc_file = argv[2];
  
    if (argc == 4)
    {
      if (!log_file)
        log_file = argv[3];
      else
        start_ok = false;
    }
  }
  if (start_ok)
  {
    if (inc_file)
    {
      if (!KScanTable::LoadScanTable(inc_file))
      {
        printf("\nscan table \"%s\" not loaded\n", inc_file);
        start_ok = false;
      } else
        printf("\nscan table \"%s\" loaded\n", inc_file);
    } else
      KScanTable::Init_scan_table();
    
    if (log_file)
    {
      if (!logfile.ReadFile(log_file))
        start_ok = false;
    }
  }
  
  if (!start_ok)
  {
    printf("\nusage:\n"
#if defined(__WINDOWS__)
           "  usbtin_simul <com dev> [ <scan table name> ] [ <log file name> ] [ \"trace\" ]\n\n"
           "example: usbtin_simul com3 ..\\scan_data.inc ..\\mac\\can_log_20140724.log \n\n");
#else
           "  can_simul <tty dev> [ <scan table name> ] [ <log file name> ] [ \"trace\" ]\n\n"
           "example: usbtin_simul tty.usbserial-FTK1S17H ../scan_data.inc \n\n");
#endif
    return -1;
  }
 
  logfile.SetPos();
  
  if (!CanElster.InitUSBtin(argv[1], trace))
    return  -1;

  while (CanElster.ServeUSBtin())
  {
    if (logfile.GetNextFrame(CanElster.SendFrame))
    {
      if (CanElster.SendFrame.Id != 0x680 && CanElster.SendFrame.Data[0] != 0xd2)
      {
        if (trace)
          CanElster.PrintSendFrame();
        CanElster.SendData(CanElster.SendFrame);
        KScanTable::SetValue(CanElster.SendFrame);
      }
    }
    NUtils::SleepMs(1);
  }
  
  CanElster.Halt();

  return 0;
}

#endif

#if defined(__STIEBEL_SIMULATION__)

// Mit einem CAN232/USBtin oder USB2CAN (nur für Windows) wird am CAN-Bus
// eine Wärmepumpe von Stiebel-Eltron simuliert.

#include "KCanLogFile.h"

int main(int argc, char* argv[])
{
  printf("Stiebel CAN-BUS simulator\n"
         "(use ComfortSoft with CS_Bruecke)\n"
#if defined(__WINDOWS__)
         "copyright (c) 2017 Juerg Mueller, CH-5524\n\n");
#else
         "copyright (c) 2017 Jürg Müller, CH-5524\n\n");
#endif
  
  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;
  
  KCanElster CanElster;
  KCanLogFile logfile;
  char str[256];
  bool start_ok = argc >= 2 && argc <= 4;
  const unsigned SenderId = 0x680;
  const char * log_file = NULL;
  const char * inc_file = NULL;
  
  if (start_ok && argc >= 3)
  {
    if (!stricmp(NUtils::ExtractExtension(argv[2]), ".log"))
      log_file = argv[2];
    else
      inc_file = argv[2];
  
    if (argc == 4)
    {
      if (!log_file)
        log_file = argv[3];
      else
        start_ok = false;
    }
  }
  if (start_ok)
  {
    if (inc_file)
    {
      if (!KScanTable::LoadScanTable(inc_file))
      {
        printf("\nscan table \"%s\" not loaded\n", inc_file);
        start_ok = false;
      } else
        printf("\nscan table \"%s\" loaded\n", inc_file);
    } else
      KScanTable::Init_scan_table();
    
    if (log_file)
    {
      if (!logfile.ReadFile(log_file))
        start_ok = false;
    }
  }
  
  if (!start_ok)
  {
    printf("\nusage:\n"
#if defined(__WINDOWS__)
           "  stiebel_simul <com dev> [ <scan table name> ] [ <log file name> ] [ \"trace\" ]\n\n"
           "example: stiebel_simul com3 ..\\scan_data.inc ..\\mac\\can_log_20140724.log\n"
           "     or  stiebel_simul ED000200\n\n");
#else
    "  stiebel_simul <tty dev> [ <scan table name> ] [ <log file name> ] [ \"trace\" ]\n\n"
    "example: stiebel_simul tty.usbserial-FTK1S17H ../scan_data.inc \n\n");
#endif
    return -1;
  }
  
  logfile.SetPos();
    
  CanElster.SetTrace(trace);
  if (!CanElster.Init(argv[1], 0x680))
  {
    printf("initialization failed\n");
    printf("device \"%s\" not opened\n", argv[1]);
    return  -1;
  }
  if (trace)
    printf("device \"%s\" opened\n", argv[1]);
  
  CanElster.Trace = trace;
  CanElster.Resume();
  NUtils::SleepMs(100);
  
  if (CanElster.Terminated())
    printf("terminated\n");
  
  do
  {
    KCanFrame Frame;
    KCanFrame resFrame;
    KComfortFrame cf;
    KComfortFrame cfRes;
    
    if (CanElster.GetFrame(Frame))
    {
      cf.GetCanFrame(Frame);
      if (trace)
      {
        printf(Frame.Id == SenderId ? "recv " : "send ");
        Frame.PrintFrame();
      }
      if (KScanTable::Get_scan_value(cf, cfRes) &&
          cfRes.SetCanFrame(resFrame))
      {
        if (trace &&
            FormElsterTable(resFrame, str))
        {
          printf("Elster %3.3x.%4.4x.%4.4x: %s\n", resFrame.Id, resFrame.GetElsterIdx(), (unsigned short) resFrame.GetValue(), str);
        }
        if (Frame.Id == SenderId) // Echo nicht senden
          CanElster.SendData(resFrame);
      }
    }
   
    if (logfile.GetNextFrame(CanElster.SendFrame))
    {
      if (CanElster.SendFrame.Id != 0x680 && CanElster.SendFrame.Data[0] != 0xd2)
      {
        if (trace)
          CanElster.PrintSendFrame();
        CanElster.SendData(CanElster.SendFrame);
        KScanTable::SetValue(CanElster.SendFrame);
      }
    }
    
    NUtils::SleepMs(1);
  } while (!CanElster.Terminated());
  
  CanElster.Halt();
  
  printf("bye-bye\n\n");
  
  return 0;
}

#endif

#endif   // __UVR__
