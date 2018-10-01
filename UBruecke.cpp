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

//#include "NTypes.h"

#include "NCanUtils.h"
#include "UBruecke.h"

#include <stdio.h>
#include <string.h>

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x640)
  #include <vcl.h>
  #include <System.StrUtils.hpp>
#endif

#include "NUtils.h"
#include "KTcpCanClient.h"
#include "KElsterTable.h"
#include "KCanElster.h"
#include "KScanTable.h"
#include "KCanCommDriver.h"

volatile bool Stop = false;
t_dev dev = dev_simulation;
unsigned short Kennwort = 0;
KCanElster * CanElster = NULL;
#if defined(VCL)
  #if !defined(_DEBUG)
    !!!
  #endif
  // VCL und nicht Debug (Endgueltig) vertragen sich nicht!
  // Delphi Units lassen sich dann nicht einbinden!
  static TComm * CS_Com = NULL; // TComm bietet RS232-Monitor an
  static TComm * Null_CS = NULL;
#else
  static KComm * CS_Com = NULL;
  static KComm * Null_CS = NULL;
#endif
KCanElster * USBtin_Simul = NULL;

static KTcpCanClient CanClient;
static FILE * logFile = NULL;

static const bool UseTimeStamp = true;
static const bool UseTrace = true;

static unsigned char not_connected_dev[256];


void Trace(const char * text)
{
  if (UseTrace)
    printf("%s\n", text);
}

bool GetElsterValue(unsigned short receiver_id, unsigned short elster_index, unsigned short & value)
{
  KComfortFrame send;
  KComfortFrame recv;
  KCanFrame Frame;
  int val = -1;

  bool Ok = Frame.InitElsterFrame(0x680, receiver_id, elster_index);
  //Frame.Len = 7;
  send.GetCanFrame(Frame);
  if (!Bruecke::ElsterComm(send, recv))
    Ok = false;
  if (Ok)
    Ok = recv.SetCanFrame(Frame);

  if (Ok)
    val = Frame.GetValue();
  
  if (val < 0)
    Ok = false;
  else
    value = (unsigned short) val;
  
  return Ok;
}

namespace Bruecke
{

bool LoadScanTable(const char * Filename)
{
  bool Ok = KScanTable::LoadScanTable(Filename);
  printf("Tabelle \"%s\"", Filename);
  if (!Ok)
    printf(" nicht");
  printf(" geladen\n");

  return Ok;
}

void set_log_file(bool set, const char * Filename)
{
  if (set)
  {
    if (!logFile)
    {
      logFile = fopen(Filename, "ab+");
      if (!logFile)
        printf("Log-File \"%s\" konnte nicht geoeffnet werden.\n", Filename);
    }
  } else
  if (logFile)
  {
    fclose(logFile);
    logFile = NULL;
  }
}

void log_frame(bool send, const KComfortFrame & frame)
{
  if (logFile)
  {
    char str[62];

    strcpy(str, send ? "send:" : "recv:");
    for (int i = 0; i < 12; i++)
      sprintf(str + strlen(str), " %2.2x", frame.Data[i]);
    fprintf(logFile, "%s\n", str);
  }
}

bool ConnectedIndex(const KComfortFrame & cs, unsigned char & Index)
{
  KCanFrame Frame;

  bool Ok = cs.SetCanFrame(Frame);
  if (Ok)
    Ok = (Frame.Data[0] & 0x0f) == 1 &&
         Frame.GetElsterIdx() == 0x000b;

  int recv_id = Frame.GetReceiverId();
  if (recv_id < 0)
    Ok = false;
  
  Index = KScanTable::ShortCanId((unsigned short) recv_id);

  return Ok;
}

bool NotConnected(const KComfortFrame & cs)
{
  unsigned char Index;
  bool Ok = ConnectedIndex(cs, Index);

  if (Ok &&
      not_connected_dev[Index] > 1)
    return true;

  return false;
}

unsigned short GetKennwort()
{
  int i = 0;
  int sub = 1; // Für das USB2CAN geht das 1 Telegramm verloren.

  while (KComfortFrame::scan_geraete_id[i].elster_idx)
  {
    unsigned short value;
    unsigned short recv_id = KComfortFrame::scan_geraete_id[i].recv_id;
    unsigned short elster_idx = KComfortFrame::scan_geraete_id[i].elster_idx;

    bool Ok = GetElsterValue(recv_id, elster_idx, value) &&
              value != 0x8000 && value != 0;

    // Vor allem neuere Geräte, wie die WPM3, verwenden 0x0199 und 0x019a
    // als Versionsnummern. Der Wert an der Stelle 0x0199 ist meist grösser
    // als 256 und passt damit nicht in das alte Schema.
    if (Ok && elster_idx == 0x0199)
    {
      value = 0x8000;
    }
    if (Ok && value)
    {
      char str[64];

      SetValueType(str, et_dev_id, value);
      printf("Kennwort: %d  Version: %s\n", value, str);

      return value;
    }
    if (sub)
      sub--;
    else
      i++;
  }
  return 0;
}

bool InitKennwort(const char * Text)
{
  char str[32];
  bool shown = false;

  Kennwort = 0;
  TInt64 d = -1;

  if (dev == dev_nullmodem)
    return false;

  if (strlen(Text) > 0)
    NUtils::GetInt(Text, d);
  if (0 <= d && d <= 0xffff)
  {
    Kennwort = (unsigned short) d;
  } else
  if (dev == dev_can_server || dev == dev_can232 || dev == dev_simulation ||
      dev == dev_usb2can || dev == dev_can232_remote)
  {
    Kennwort = GetKennwort();
    shown = true;
    if (!Kennwort)
    {
      printf("ungueltiges Kennwort\n");
      return false;
    }
  }

  if (!shown)
  {
    SetValueType(str, et_dev_id, Kennwort);
    printf("Kennwort: %d  Version: %s\n", Kennwort, str);
  }
  return true;
}

static void LogElster(const KCanFrame & Frame)
{
  char str[64];
  char buf[128];

  if (FormElsterTable(Frame, str))
  {
    sprintf(buf, "Elster %3.3x.%4.4x.%4.4x: %s\n",
            Frame.Id, Frame.GetElsterIdx(), (unsigned short) Frame.GetValue(), str);
    printf("%s", buf);
    if (logFile)
      fprintf(logFile, "%s", buf);
  }
}

static void LogElster(const KComfortFrame & cf)
{
  KCanFrame Frame;

  if (cf.SetCanFrame(Frame))
  {
    LogElster(Frame);
  }
}

static void LogSend(const KComfortFrame & cs_send)
{
  char str[64];

  cs_send.SetToString(str);
  printf("send CS: %s\n", str);
  if (logFile)
    fprintf(logFile, "send CS: %s\n", str);
}

static void LogRecv(const KComfortFrame & cs_recv)
{
  char str[64];

  cs_recv.SetToString(str);
  printf("recv CS: %s\n", str);
  if (logFile)
    fprintf(logFile, "recv CS: %s\n", str);
}

void WriteToCS(KComfortFrame * cs_recv, bool log)
{
  if (dev != dev_nullmodem)
  #if defined(VCL)
    CS_Com->WriteBufferToCOM(cs_recv->Data, 12);
  #else
    CS_Com->WriteBufferToCOM((const char *) (cs_recv->Data), 12);
  #endif
  if (log)
  {
    LogRecv(*cs_recv);
  }
}

#if defined(VCL)
  void Init_Simple_Com(TComm * & Com)
#else
  void Init_Simple_Com(KComm * & Com)
#endif
{
#if defined(VCL)
  Com = new TComm((TComponent *) NULL);
  Com->UseTimeStamp = UseTimeStamp;
#else
  Com = new KComm;
#endif
  Com->SetDTR(true);
  Com->SetRTS(true);
  Com->SetParity(2);
  Com->SetStopBits(2);
  Com->SetBaudrate(2400);
  Com->SetRtsFlowControl(false);
  Com->NewScreenSize(90, 2000);
  Com->SetTimerInterval(10);
  HideError = true;
}

void Init_CS_Com()
{
  Init_Simple_Com(CS_Com);
}

void Init_Null_Com()
{
  Init_Simple_Com(Null_CS);
}

bool ElsterComm(const KComfortFrame & send, KComfortFrame & recv)
{
  bool Ok = false;
#if defined(VCL)
  try
#endif
  {
    switch (dev)
    {
      case dev_nullmodem:
        return true;

      case dev_can_server:
        Ok = CanClient.SendComfortFrame(send, recv);
        break;

      case dev_can232:
      case dev_usb2can:
      case dev_can232_remote:
        if (!send.SetCanFrame(CanElster->SendFrame) ||
            !CanElster->Send(1, true))
          return false;
        return recv.GetCanFrame(CanElster->RecvFrame);

      case dev_simulation:
      {
        Ok = KScanTable::Get_scan_value(send, recv);
        KComfortFrame::msg_type type = send.GetMsgType();
        if (send.Data[0] != 0x0d) // Sender muss Id 680 haben
          type = KComfortFrame::mt_invalid;
        if (Ok && type == KComfortFrame::mt_change)
          recv.SetOk(true);
        NUtils::SleepMs(Ok ? 50 : 100);
        break;
      }
      default:
        break;
    }
  }
#if defined(VCL)
  catch (Exception &exception)
  {
    Application->ShowException(&exception);
    Ok = false;
    if (dev == dev_can_server)
      CanClient.Disconnect();
    CS_Com->ExitCOM();
  }
#endif
  return Ok;
}

bool InitCom(t_dev new_dev, int sec_com_nr, int timeout_ms,
             const char * remote_ip, const char * usb2can_name,
             bool binary_protocol, void * Form)
{
  bool Ok = true;

  dev = new_dev;

  if (dev == dev_can232 || dev == dev_usb2can)
  {
    char port[256];
    CanElster = new KCanElster;

  #if defined(__WINDOWS__)
    if (dev == dev_usb2can)
    {
      strcpy(port, usb2can_name);
      CanElster->driver_type = NCanUtils::dt_8dev;
    } else {
  #endif
      sprintf(port, "COM%d", sec_com_nr);
      CanElster->driver_type = NCanUtils::dt_can232;
  #if defined(__WINDOWS__)
    }
  #endif
    if (!CanElster->Init(port))
    {
      delete CanElster;
      CanElster = NULL;
      return false;
    }
    CanElster->Trace = true;
    CanElster->Resume();

    return true;
  }

  if (dev == dev_can232_remote)
  {
    CanElster = new KCanElster;
    CanElster->driver_type = NCanUtils::dt_can232_remote;

    if (!CanElster->Init(remote_ip))
    {
      delete CanElster;
      CanElster = NULL;
      return false;
    }
    CanElster->Trace = true;
    CanElster->Resume();

    return true;
  }

  if (dev == dev_can_server)
  {
    CanClient.mReadTimeout = timeout_ms;
	CanClient.SetRemoteHost(remote_ip);
    CanClient.SetRemotePort(5524);
    CanClient.UseBinaryProtocol = binary_protocol;
    CanClient.mTrace = true;
    CanClient.IsServer = false;

    return true;
  }

  if (dev == dev_nullmodem)
  {
    if (!Null_CS)
    {
      Init_Null_Com();
    }
  #if defined(VCL)
    try
  #endif
    {
      Null_CS->SetComPortNr(sec_com_nr);
      Ok = Null_CS->InitCOM();
    }
  #if defined(VCL)
    catch (Exception &exception)
    {
      Application->ShowException(&exception);
      Ok = false;
    }
  #endif
  }

  if (dev == dev_simulation)
    return true;

  return Ok;
}

void CloseCom()
{
  CS_Com->ExitCOM();
  if (CanElster)
  {
    CanElster->Halt();
  #if defined(VCL)
    delete CanElster;
  #endif
    CanElster = NULL;
  }
  if (Null_CS)
    Null_CS->ExitCOM();
  delete Null_CS;
  Null_CS = NULL;
  if (USBtin_Simul)
  {
    USBtin_Simul->Halt();
    NUtils::SleepMs(1000);
    delete USBtin_Simul;
    USBtin_Simul = NULL;
  }
}

bool Start(int com_nr)
{
  Stop = false;
  if (com_nr > 0)
    CS_Com->SetComPortNr(com_nr);
#if defined(VCL)
  try
#endif
  {
    CS_Com->InitCOM();
  }
#if defined(VCL)
  catch (Exception &exception)
  {
    Application->ShowException(&exception);
  }
#endif
  if (!CS_Com->COM_INITIALISIERT())
  {
    CS_Com->Show();
    CS_Com->write(" serielle Schnittstelle konnte nicht geoeffnet werden!  ");
    CloseCom();
    return false;
  }

  return true;
}

#if defined(VCL)
static void ChangeRTS(TComm * Com, bool NewRTS)
#else
static void ChangeRTS(KComm * Com, bool NewRTS)
#endif
{
  Com->SetRTS(NewRTS);
  Com->SetDTR(NewRTS);
  Com->SetComParameters();
}

void Execute(bool & cbxUseElsterTable,
             bool & cbxRecvTelegram, // CAN Empfangstelegramm
             bool & cbxCanTelegram,  // CAN Sendetelegramm
             bool & cbNotChange,
             bool & cbxCS_Telegram,  // CS Sendetelegram
             bool & cbxRecvCS)       // CS Empfangstelegramm
{
  Trace("start Execute");
  ChangeRTS(CS_Com, false);

  if (dev == dev_nullmodem)
  {
    int recv_count = 0;
    int send_count = 0;
    KComfortFrame cs_recv;
    KComfortFrame cs_send;
    int cs_recv_data = 0;
    int cs_send_data = 0;
    int count = 0;

    Null_CS->SetDTR(false);
    Null_CS->SetRTS(true);
    Null_CS->SetComParameters();
    Trace("start null modem");
    do
    {
      int res;
      bool first = count < 20;
      count++;

      while ((res = CS_Com->ReadFromCOM()) != NULL_CHAR)
      {
        if (UseTrace)
        {
          char s[32];
          sprintf(s, "cs received %4.4x", res & 0xffff);
          Trace(s);
        }
        if (res < 0x100)
        {
          count = 0;
          if (UseTimeStamp && first)
            CS_Com->WriteNow();
          Null_CS->WriteToCOM(res);
          cs_send.Data[cs_send_data++] = (unsigned short) res;
          if (cs_send_data >= 12)
          {
            if (cbxCanTelegram)
            {
              KCanFrame fr;
              char str[128];

              cs_send.SetCanFrame(fr);
              fr.FormatFrame(false, str);
              printf("send: %s", str);
              if (logFile)
                fprintf(logFile, "send: %s", str);
            }
            if (cbxCS_Telegram)
            {
              LogSend(cs_send);
            }
            cs_send_data = 0;
          }
        } else
        if ((res & 0xffff) == 0x10)
        {
          Trace("send break");
          CS_Com->SendBreak(20);
          Null_CS->SendBreak(20);
        }
        first = false;
      }

      first = count < 20;
      while ((res = Null_CS->ReadFromCOM()) != NULL_CHAR)
      {
        if (UseTrace)
        {
          char s[32];
          sprintf(s, "opt. received %4.4x", res & 0xffff);
          Trace(s);
        }
        if (res < 0x100)
        {
          count = 0;
          if (UseTimeStamp && first)
            Null_CS->WriteNow();
          CS_Com->WriteToCOM(res);
          cs_recv.Data[cs_recv_data++] = (unsigned short) res;
          if (cs_recv_data >= 12)
          {
            if (cbxUseElsterTable)
            {
              LogElster(cs_recv);
            }
            if (cbxRecvTelegram && cs_recv.Data[0] != 0x0d)
            {
              KCanFrame fr;
              char str[128];

              cs_recv.SetCanFrame(fr);
              fr.FormatFrame(false, str);
              printf("recv: %s", str);
              if (logFile)
                fprintf(logFile, "recv: %s", str);
            }
            if (cbxRecvCS && cs_recv.Data[0] != 0x0d)
            {
              LogRecv(cs_recv);
            }
            cs_recv_data = 0;
          }
        } else
        if ((res & 0xffff) == 0x10)
        {
          Trace("receive break");
          CS_Com->SendBreak(20);
        }
        recv_count = 0;
        first = false;
      }
      recv_count++;
      if (recv_count > 40)
      {
        recv_count = cs_recv_data = 0;
      }
      send_count++;
      if (send_count > 40)
      {
        send_count = cs_recv_data = 0;
      }
      NUtils::SleepMs(1);
    #if defined(VCL) || (defined(__BORLANDC__) && (__BORLANDC__ >= 0x640))
      Application->ProcessMessages();
    #endif
    } while (CS_Com->COM_INITIALISIERT() && !Stop);
    printf("== stop\n");
    CloseCom();

    return;
  }

  int res;
  while ((res = CS_Com->ReadFromCOM()) != NULL_CHAR)   // Puffer leeren
    if (dev == dev_simulation)
      CS_Com->WriteToCOM((char) res);

  KComfortFrame cs;
  KComfortFrame cs_recv;
  bool written = false;
  bool CTS = false;
  bool DSR = false;
  int len_cs = 0;
  do
  {
  #if defined(__USBTIN__)
    if (USBtin_Simul)
      USBtin_Simul->ServeUSBtin();
  #endif
    if (CS_Com->CTS() != CTS)
    {
      CTS = !CTS;
      if (CTS)
      {
        CS_Com->write(" <cts true>");
        //CS_Com->WriteToCOM(0x00);
        //CS_Com->WriteToCOM(0x00);
      } else
        CS_Com->write(" <cts false>");
    }
    if (CS_Com->DSR() != DSR)
    {
      DSR = !DSR;
      if (DSR)
        CS_Com->write(" <dsr true>");
      else
        CS_Com->write(" <dsr false>");
    }

    CS_Com->CommMonitor(NULL);
    res = CS_Com->ReadFromCOM();
    if (res < 0x100)
    {
      if (dev == dev_simulation)
        CS_Com->WriteToCOM((char) res);

      cs.Data[len_cs++] = char(res);
      if (cs.Data[0] != 0x0d)
        len_cs = 0;

      if (len_cs == 12)
      {
        KCanFrame SendFrame;
        char str[64];
        KComfortFrame::msg_type cf_type;

        log_frame(true, cs);

        // cs ==> SendFrame und cf_type
        if (!cs.SetCanFrame(SendFrame) ||
            cs.Data[0] != 0x0d) // Sender muss Id 680 haben
          cf_type = KComfortFrame::mt_invalid;
        else
          cf_type = cs.GetMsgType();
        if (cf_type == KComfortFrame::mt_short_change)
          cf_type = KComfortFrame::mt_change;

        // show send frame
        if (cf_type != KComfortFrame::mt_invalid)
        {
          if (cbxCanTelegram ||
              (cf_type == KComfortFrame::mt_change && !cbNotChange))
          {
            SendFrame.FormatFrame(false, str);
            printf("send: %s", str);
            if (logFile)
              fprintf(logFile, "send: %s", str);
          } else
          if (cbxCS_Telegram)
          {
            LogSend(cs);
          }
        }

        if ((cf_type == KComfortFrame::mt_change && cbNotChange))
        {
          LogElster(SendFrame);
        } else
        if (cf_type == KComfortFrame::mt_invalid)
        {
          cs.SetToString(str);
          printf("invalid: %s\n", str);
          cs_recv.SetOk(false);
          WriteToCS(&cs_recv, cbxRecvCS);
        } else
        if (cf_type == KComfortFrame::mt_init)
        {
          cs_recv.SetOkValue(Kennwort);
          WriteToCS(&cs_recv, cbxRecvCS);
        } else
        if (cf_type == KComfortFrame::mt_get &&
            NotConnected(cs))
        {
          cs_recv.SetOk(false);
          WriteToCS(&cs_recv, cbxRecvCS);
        } else
        if (ElsterComm(cs, cs_recv)) // send and receive / interpret
        {
          log_frame(false, cs_recv);

          char buf[128];
          KCanFrame Frame;
          bool show_recv = false;
          bool recv_ok = cs_recv.SetCanFrame(Frame);
          if (cbxUseElsterTable &&
              cf_type == KComfortFrame::mt_change)
          {
            FormElsterTable(SendFrame, str);
            sprintf(buf, "Elster (sent %s) %3.3x.%4.4x.%4.4x: %s\n",
                    cs_recv.Data[0] == 0x55 ? "ok" : "failed",
                    (unsigned short) SendFrame.GetReceiverId(),
                    SendFrame.GetElsterIdx(),
                    (unsigned short) SendFrame.GetValue(), str);
            printf("%s", buf);
            if (logFile)
              fprintf(logFile, "%s", buf);
          } else
          if (cbxUseElsterTable)
          {
            if (recv_ok)
            {
              LogElster(cs_recv);
            } else
            if (cs_recv.Data[0] != 0xaa && cs_recv.Data[0] != 0x55 &&
                !cbxRecvTelegram && cf_type != KComfortFrame::mt_init)
              show_recv = true;
          }
          if (cbxRecvTelegram || cf_type == KComfortFrame::mt_init || show_recv)
          {
            if (recv_ok && cf_type != KComfortFrame::mt_init)
              Frame.FormatFrame(false, str);
            else {
              cs_recv.SetToString(str);
              strcat(str, "\n");
            }
            if (cf_type == KComfortFrame::mt_init)
              printf("init: %s", str);
            else
              printf("recv: %s", str);
          }
          unsigned char con_ind;
          if (ConnectedIndex(cs, con_ind))
          {
            if (cs_recv.Data[0] == 0xaa)
              not_connected_dev[con_ind]++;
          }
          WriteToCS(&cs_recv, cbxRecvCS);
        } // ElsterComm
        len_cs = 0;
        written = true;
      }
      if (written)
      {
      #if defined(VCL) || (defined(__BORLANDC__) && (__BORLANDC__ >= 0x640))
        Application->ProcessMessages();
      #endif
        CS_Com->writeln();
        written = false;
      }
    } else
    if (res != NULL_CHAR)
    {
      len_cs = 0;
      CS_Com->WriteChar(13);
      CS_Com->WriteChar(10);
    } else
      NUtils::SleepMs(1);
  #if defined(VCL) || (defined(__BORLANDC__) && (__BORLANDC__ >= 0x640))
    Application->ProcessMessages();
  #endif
  } while (CS_Com->COM_INITIALISIERT() && !Stop);
  printf("== stop\n");
  CloseCom();
}

void ExitProg()
{
#if defined(VCL)
  if (CanElster)
    delete CanElster;
  if (CS_Com)
    delete CS_Com;
#endif
  if (Null_CS)
    delete Null_CS;
}

void Send(const KCanFrame & Frame)
{
  if (USBtin_Simul)
    USBtin_Simul->SendData(Frame);
}

}

////////////////////////////////////////////////////////////////////////////////

// USB2CAN ausgetestet mit:
//
// - mac: ./stiebel_simul tty.usbserial-FTK1S17H trace
// - win: cs_bruecke_dos.exe COM8 ED000200 trace
// - ComfortSoft verbinden

#if defined(__NO_VCL__)

#if defined(__WINDOWS__)
  #if !defined(__GCC__)
    bool LoadCanalLibrary();
  #else
    #include "../special/Kusb2can.h" // MinGW-Compiler macht damit Zicken
  #endif
#endif

bool IsNumber(const char * num)
{
  if (!NUtils::IsDigit(num[0]))
    return false;

  for (int i = 0; i < (int) strlen(num); i++)
    if (!NUtils::IsDigit(num[i]))
      return false;
  
  return true;
}

bool IsCOM(const char * dev)
{
  if (strncmp(dev, "com", 3) && strncmp(dev, "COM", 3))
    return false;
  
  return IsNumber(dev + 3);
}

bool IsUSB2CAN(const char * dev)
{
  // "ED..."
  if ((dev[0] == 'e' || dev[0] == 'E') &&
      (dev[1] == 'd' || dev[1] == 'D'))
    return false;
  
  return IsNumber(dev + 2);
}

int main(int argc, char* argv[])
{
#if defined(__WINDOWS__)
  printf("cs_bruecke\n"
         "copyright (c) 2017 Juerg Mueller, CH-5524\n\n");
#else
  printf("cs_bruecke (simulation only)\n"
         "copyright (c) 2017 Jürg Müller, CH-5524\n\n");
#endif
  
  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;
  
  int iarg = 2;
  int com_nr = 1;
  const char * can_dev = "";
#if defined(__WINDOWS__)
  if (argc >= 3 && (IsCOM(argv[2]) || IsUSB2CAN(argv[2])))
  {
    can_dev = argv[2];
    iarg++;
    argc--;
    
    if (IsUSB2CAN(can_dev))
      dev = dev_usb2can;
    else {
      dev = dev_can232;
      com_nr = 0;
      for (int i = 3; i < (int) strlen(can_dev); i++)
        com_nr = 10*com_nr + NUtils::GetDigit(can_dev[i]);
      can_dev = "";
    }
  }
#endif
  
  if (argc < 2 || argc > 3)
  {
    printf("usage:\n\n"
  #if defined(__WINDOWS__)
           "  cs_bruecke_dos <serial port to ComfortSoft> [ <CAN port> ] [ <scan_data table> ]\n\n"
           "  cs_bruecke_dos COM4 \"..\\scan_data.inc\"\n\n");
  #else
           "  ./cs_bruecke <serial port to ComfortSoft> [ <scan_data table> ]\n\n"
           "  ./cs_bruecke ttyABC ../scan_data.inc\n\n");
  #endif
    return 1;
  }

  if (argc >= 3 &&
      !Bruecke::LoadScanTable(argv[iarg]))
    return -1;

 // Bruecke::set_log_file(true, "log.txt");

#if defined(__WINDOWS__)
  if (dev == dev_usb2can && !LoadCanalLibrary())
  {
    return -1;
  }
#endif
  Bruecke::Init_CS_Com();
  if (!Bruecke::InitCom(dev,
                        com_nr, /* COM1 / second port not used */
                        500, /* timeout_ms */
                        "", can_dev, false))
  {
    Bruecke::CloseCom();
    return -3;
  }
  
  if (dev != dev_nullmodem &&
      !Bruecke::InitKennwort(""))
  {
    Bruecke::CloseCom();
    return -2;
  }

#if defined(__USBTIN__) // nicht ausgetestet!
  USBtin_Simul = new KCanElster;
  if (!USBtin_Simul->InitUSBtin("com6", true))
    return -5;
#endif

  CS_Com->SetComDev(argv[1]);
  if (!Bruecke::Start(0))
  {
    printf("not started\n");
    return -4;
  }
  
  printf("\n");

  bool flgUseElsterTable = true;
  bool flgRecvTelegram   = false;
  bool flgCanTelegram    = false;
  bool flgNotChange      = false;
  bool flgCS_Telegram    = false;
  bool flgRecvCS         = false;

  Bruecke::Execute(flgUseElsterTable,
                   flgRecvTelegram,
                   flgCanTelegram,
                   flgNotChange,
                   flgCS_Telegram,
                   flgRecvCS);

  return 0;
}

#endif

