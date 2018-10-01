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

#define NULL_CHAR     0x1000  /* no character in buffer */

#if defined(__LINUX__)
  #include <sys/socket.h>
  #include <arpa/inet.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>

#include "NTypes.h"

#include "NUtils.h"
#include "KCanTcpDriver.h"

KCanTcpDriver::KCanTcpDriver()
{
  tel_len = 0;
  time_out = 50; // 50 msec
}

KCanTcpDriver::~KCanTcpDriver()
{
  Close();
}

bool KCanTcpDriver::Init(const char * TcpAddr)
{
  strcpy(dev, "localhost");
  if (TcpAddr && strlen(TcpAddr) < High(dev)-2)
    strcpy(dev, TcpAddr);
  
  char * port_str = NULL;
  unsigned port_nr = 5524;
  if (strchr(dev, ':')) // port
  {
    port_str = strchr(dev, ':');
    *port_str++ = 0;
    
    const char * p_str = port_str;
    if (!NUtils::GetUnsigned(p_str, port_nr))
      return false;
  }
      

  SetRemoteHost(TcpAddr);
  SetRemotePort(port_nr);
  
  return true;
}

int KCanTcpDriver::ReadFromCOM()
{
  int res;
  char buffer[32];
  
  if (!KTcpClient::ReadBuffer(buffer, 1))
    res = NULL_CHAR;
  else
    res = (unsigned char) buffer[0];
  
  if (res == NULL_CHAR && tel_len == 0)
  {
    NUtils::SleepMs(1);
    return res;
  }

  timeb tb;
	ftime(&tb);
  unsigned now = 1000*(tb.time % 1000000) + tb.millitm;

  int div = now - time_stamp;
  if (div < 0)    // time overflow
    div += 1000000000;
  if (div > (int) time_out)
    tel_len = 0;  // clear buffer after timeout

  if (res == NULL_CHAR)
    return NULL_CHAR;

  if (tel_len+1 >= High(tel))
  {
    tel_len = 0;
    return 0x2000;
  }

  tel[tel_len++] = (unsigned char) res;
  tel[tel_len] = 0;
  time_stamp = now;

  return res;
}

bool KCanTcpDriver::Connect()
{
  return KTcpClient::Connect(100);
}

void KCanTcpDriver::Close()
{
  KTcpClient::Disconnect();
}


////////////////////////////////////////////////////////////////////////////////
//
// socat -d -d pty,raw,echo=0 tcp-listen:5524,debug,reuseaddr
//

#if defined(__TEST_MICROCHIP__)
// S4: 125 kbit/s
#define INIT_STR  "\rC\rX1\rV\rS4\rO\r"
#else
// S1: 20kbit/s
#define INIT_STR  "\rC\rX1\rV\rS1\rO\r"
#endif

bool KCan232Tcp::Init(const char * TcpAddr)
{
  tel_len = 0;
  WaitSentFrame = false;

  KCanTcpDriver::Init(TcpAddr);
   
  if (Connect())
  {
    SetBlockMode(bmNonBlocking);
    if (AsSimulation)
      return true;
      
    const char * init = INIT_STR;
    while (init)
    {
      const char * p = strstr(init, "\r");
      if (!p)
        break;
      SendBuffer(init, (int)(p - init) + 1);
      NUtils::SleepMs(100);
      init += p - init + 1;
    }
    return true;
  }
  return false;
}

bool KCan232Tcp::ReceiveData(KCanFrame & Frame)
{
  int res = ReadFromCOM();
  while (res != NULL_CHAR)
  {
    if (res < NULL_CHAR)
    {
      switch (char(res))
      {
        case 7: // bell means error
          WaitSentFrame = false;
          break;

        case 't':
        case 'T':
        case 'r':
        case 'R':
          tel_len = 1; // synchronize
          tel[0] = (unsigned char) res;
          break;

        case 'V':
          if (AsSimulation)
          {
            tel_len = 0;
            SendBuffer("V1324\r", 6);
            NUtils::SleepMs(10);
          }
          break;

        case '\r':
          if (tel_len == 2)
          {
            // USBtin respose for "t..." is z<CR>
            // CAN232: 'Z' for switching time stamp
            if (tel[0] == 'Z' || tel[0] == 'z')
            {
              if (WaitSentFrame)
              {
                tel_len = 0;
                WaitSentFrame = false;
                Frame = SentFrame;
                return true;
              }
            }
          } else
          if (tel_len > 1)
          {
            tel[tel_len-1] = 0;
            KCan232Frame can232((char *)tel);
            if (can232.SetCanFrame(Frame))
            {
              NUtils::Time(Frame.TimeStampDay, Frame.TimeStampMs);
              tel_len = 0;

              if (Trace && AsSimulation)
              {
                char str[64];
                Frame.PrintTime(str, false);
                printf("recv: %s %s\n", str, tel);
              }
              return true;
            }
          }
          tel_len = 0;
          break;

        default:
          break;
      }
    }
    res = ReadFromCOM();
  }
  NUtils::SleepMs(1);
  return false;
}

bool KCan232Tcp::SendData(const KCanFrame & Frame)
{
  WaitSentFrame = false;
  KCan232Frame can232;
  if (can232.GetCanFrame(Frame) &&
      SendBuffer(can232.msg, (int) strlen(can232.msg)))
  {
    tel_len = 0;
    WaitSentFrame = true;
    if (SendBuffer("\r", 1))
    {
      SentFrame = Frame;
      NUtils::Time(SentFrame.TimeStampDay, SentFrame.TimeStampMs);
      if (Trace && AsSimulation)
      {
        char str[64];
        SentFrame.PrintTime(str, false);
        printf("send: %s %s\n", str, can232.msg);
      }
      return true;
    } else
      WaitSentFrame = false;
  }
  return false;
}


