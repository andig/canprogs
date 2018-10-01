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

#include <stdio.h>
#include <string.h>

#include "NTypes.h"

#include "NUtils.h"
#include "KElsterTable.h"
#include "KScanTable.h"

#include "KTcpElsterServer.h"

#if !defined(__CAN_CLIENT__)

KTcpElsterServer::KTcpElsterServer()
{
  Trace = false;
  Simulation = false;
  CanElster = NULL;
  UseBinaryProtocol = false;
  SetLocalPort(5524);
  mTryToConnectTimeout = 2000;
  mConnectionSleep = 200;
}

bool KTcpElsterServer::Init(const char * CanDev, bool trace)
{
  Trace = trace;

  if (!Simulation)
  {
    if (!CanElster)
      CanElster = new KCanElster;

    if (!CanElster->Init(CanDev, 0x680))
      return false;

    CanElster->Trace = trace;

    CanElster->Resume();

    NUtils::SleepMs(100);

    return CanElster->GetInitCS_Id();
  }
  return true;
}

const char * KTcpElsterServer::GetCanDev() const
{
  if (CanElster)
    return CanElster->GetDev();

  return "simulation";
}

void KTcpElsterServer::ServeToDevice(const KComfortFrame & cf, KComfortFrame & cfRes)
{
  if (CanElster->SendFrame.Id != 0x680)
  {
    cfRes.SetOk(false);
    if (!Trace)
    {
      printf("wrong id> ");
      CanElster->PrintSendFrame();
    }
  } else {
    KComfortFrame::msg_type msg = cf.GetMsgType();
    if (cf.Data[0] != 0x0d) // Sender muss Id 680 haben
      msg = KComfortFrame::mt_invalid;
    switch (msg)
    {
      case KComfortFrame::mt_get:
        if (CanElster->Send(1, true, 50))
          cfRes.GetCanFrame(CanElster->RecvFrame);
        break;
        
      case KComfortFrame::mt_short_change:
      case KComfortFrame::mt_change:
      {
        unsigned char u = CanElster->SendFrame.Data[0] & 0xf0;
        CanElster->SendFrame.Data[0] = u + 2;
        bool Ok = CanElster->SetValue();
        cfRes.SetOk(Ok);
        if (!Trace)
        {
          char str[128];
          
          printf("set param (%s)> ", Ok ? "true" : "false");
          if (FormElsterTable(CanElster->SendFrame, str))
          {
            printf("%s\n", str);
          }
          CanElster->PrintSendFrame();
        }
        break;
      }
        
      default:
        if (!Trace)
        {
          printf("no answer> ");
          CanElster->PrintSendFrame();
        }
        cfRes.SetOk(false);
        break;
    }    
  }
}

void KTcpElsterServer::ServeSimulation(const KComfortFrame & cf, KComfortFrame & cfRes)
{
  cfRes.SetOk(false);
  KScanTable::Get_scan_value(cf, cfRes);
}

bool KTcpElsterServer::ReadComfortFrame(KTcpCanClient & aClientSocket, KComfortFrame & cf)
{
  char Buffer[1024];

  if (UseBinaryProtocol)
  {
    if (!aClientSocket.ReadElster(cf))
      return false;
    if (Trace)
    {
      cf.SetToString(Buffer);
      printf("recv: %s\n", Buffer);
    }
  } else {
    if (!aClientSocket.ReadElster(Buffer))
      return false;

    if (Trace)
      printf("recv: %s", Buffer);
    if (!cf.SetFromString(Buffer, false))
    {
      if (Trace)
        printf("- invalid Comfort format\n");
      return false;
    }
  }
  return true;
}

void KTcpElsterServer::ServeConnection(KTcpCanClient & aClientSocket)
{
  char Res[128];
  KComfortFrame cf;
  KComfortFrame cfRes;

  if (Trace)
    aClientSocket.mTrace = true;
  while (!IsTerminated() &&
         (!CanElster || !CanElster->Terminated()) &&
         ReadComfortFrame(aClientSocket, cf))
  {
    cfRes.SetOk(false);
    if (cf.IsInitFrame())
    {
      cfRes.SetInitOk();
    } else
    if (CanElster && !cf.SetCanFrame(CanElster->SendFrame))
    {
      if (Trace)
        printf("- cannot convert to can frame\n");
    } else
    if (!Simulation)
      ServeToDevice(cf, cfRes);
    else
      ServeSimulation(cf, cfRes);

    cfRes.SetToString(Res);
    if (Trace)
      printf("send: %s\n", Res);

    if (UseBinaryProtocol)
    {
      if (!aClientSocket.SendElster(cfRes, 100))
        break;
    } else {
      if (!aClientSocket.SendElster(Res, 100))
        break;
    }
    ReadyForReading(50);
  }
}

#endif

#if defined(__CAN_SERVER__)

#if defined(__LINUX__)
  #include <sys/signal.h>
#endif

int main(int argc, char* argv[])
{
  KTcpElsterServer Server;
  bool start_ok;

#if defined(__WINDOWS__)
  // DOS tabelle cp850
  printf("Elster-Kromschr\x94 \bder TCP/IP server\n"
         "copyright (c) 2014 J\x81rg M\x81ller, CH-5524\n");
#else
  printf("Elster-Kromschröder TCP/IP server\n"
         "copyright (c) 2015 Jürg Müller, CH-5524\n");
#endif

  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;

  Server.UseBinaryProtocol = !strcmp(argv[argc-1], "binary_protocol");
  if (Server.UseBinaryProtocol)
    argc--;

  start_ok = 2 <= argc && argc <= 3;
  if (argc >= 2)
  {
    if (!strncmp(argv[1], "simul", 5))
    {
      Server.Simulation = true;

      if (argc == 3)
      {
        if (!KScanTable::LoadScanTable(argv[2]))
        {
          printf("\ncould scan table \"%s\" not load\n", argv[2]);
          start_ok = false;
        }
      } else
        KScanTable::Init_scan_table();
    } else
    if (argc > 2)
      start_ok = false;
  }

  if (!start_ok)
  {
    printf("\nusage:\n"
           "  can_server ( <can device> | \"simulation\" ) [ <scan table name> ] [ \"binary_protocol\" ] [ \"trace\" ]\n\n"
#if defined(__WINDOWS__)
           "example: can_server COM1 \n\n");
#else
           "example: ./can_server can0 \n\n");
#endif
    if (argc >= 2)
      return -1;
  }

  if (!Server.Init(argc == 2 ? argv[1] : NULL, trace))
  {
    printf("can (%s) not opened\n\n", Server.GetCanDev());
    return -1;
  }

  char Addr[128];
  char str[128];
  KIpSocket::LocalHostAddr(Addr, High(Addr));
  printf("\ndevice: %s\n", Server.GetCanDev());
  SetValueType(str, et_dev_id, KComfortFrame::InitId);
  printf("Kennwort: 0x%4.4x  Version: %s\n", KComfortFrame::InitId, str);

#if defined(SIGPIPE)
  signal(SIGPIPE, SIG_IGN);
#endif
  if (!NUtils::set_term_signal())
  {
    return -1;
  }

  if (!Server.Open())
  {
    printf("TCP/IP server not established\n\n");
    return -1;
  }
  printf("server established: %s:%d\n\n", Addr, Server.GetLocalPort());
  
  while (Server.Opened() && !Server.IsTerminated() &&
         !NUtils::term_signal_is_set())
  {
    NUtils::SleepMs(100);
  }
  
  Server.Halt();
  
  printf("\nbye-bye\n");

  return 0;
}

#endif

#if defined(__CAN_CLIENT__)

#include "KTcpCanClient.h"

int main(int argc, char* argv[])
{
  KTcpCanClient Client;
  char Buffer[1024];

#if defined(__WINDOWS__)
  // DOS tabelle cp850
  printf("Elster-Kromschr\x94 \bder TCP/IP client\n"
         "copyright (c) 2014 J\x81rg M\x81ller, CH-5524\n\n");
#else
  printf("Elster-Kromschröder TCP/IP client\n"
         "copyright (c) 2014 Jürg Müller, CH-5524\n\n");
#endif
  Client.SetRemotePort(5524);
  Client.SetRemoteHost(argc == 2 ? argv[1] : "localhost");
  Client.SetBlockMode(bmBlocking);
  
  bool UseBinaryProtocol = !strcmp(argv[argc-1], "binary_protocol");
  if (UseBinaryProtocol)
    argc--;

  if (argc != 2)
  {
    printf("usage:\n"
           "  can_client <ip address host> [ \"binary_protocol\" ]\n\n");
  }

  printf("used IP address %s\n\n", Client.GetRemoteHost());

  while (true)
  {
    printf(">> ");
    if (fgets(Buffer, High(Buffer), stdin))
    {
      while (*Buffer == '\n' ||
             *Buffer == '\r' ||
             *Buffer == ' ')
        memmove(Buffer, Buffer+1, strlen(Buffer));

      if (!strncmp(Buffer, "en", 2) ||
          *Buffer == 'q' ||
          *Buffer == 0)
        break;

      int len = (int) strlen(Buffer);
      while (len > 0 &&
             (Buffer[len-1] == ' ' || Buffer[len-1] == '.' ||
              Buffer[len-1] == '\n' || Buffer[len-1] == '\r'))
        len--;
      Buffer[len] = 0;

      int DigitCount = 0;
      for (int i = 0; i < len; i++)
        if (NUtils::IsHexDigit(Buffer[i]))
          DigitCount++;

      if (DigitCount == 20) // Check Summe anfügen
      {
        KComfortFrame cf;

        cf.SetFromString(Buffer, true);
        sprintf(Buffer + strlen(Buffer), " %2.2x %2.2x", cf.Data[10], cf.Data[11]);
      }
      if (Client.Connect(2000))
      {
        printf("connected\n");

        if (Client.SendElster(Buffer, 100)) // 100 ms timeout
        {
          if (Client.ReadElster(Buffer))
            printf("%s", Buffer);
        }
        Client.Disconnect();
      } else
        printf("not connected\n");
    }
  }
  printf("bye-bye\n\n");
  return 0;
}

#endif

