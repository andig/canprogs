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
#include <sys/timeb.h>

#include "NTypes.h"

#include "NUtils.h"
#include "KCanCommDriver.h"

#if defined(VCL)
KCanCommDriver::KCanCommDriver(TComponent * Owner) : KCommVCL(Owner)
#else
KCanCommDriver::KCanCommDriver()
#endif
{
  tel_len = 0;
  time_out = 50; // 50 msec
}

KCanCommDriver::~KCanCommDriver()
{
  Close();
}

int KCanCommDriver::ReadFromCOM()
{
  int res = KComm::ReadFromCOM();

  if (res > NULL_CHAR)
  {
    tel_len = 0;
    return res;
  }
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

bool KCanCommDriver::Connect()
{
  return COM_INITIALISIERT();
}

void KCanCommDriver::Close()
{
  ExitCOM();
}


////////////////////////////////////////////////////////////////////////////////

#if defined(__TEST_MICROCHIP__)
  // S4: 125 kbit/s
  #define INIT_STR  "\rC\rX1\rV\rS4\rO\r"
#elseif defined(__UVR__)
  // S1: 50kbit/s
  #define INIT_STR  "\rC\rX1\rV\rS2\rO\r"
#else
  // S1: 20kbit/s
  #define INIT_STR  "\rC\rX1\rV\rS1\rO\r"
#endif

bool KCan232::Init(const char * CanDev)
{
  tel_len = 0;
  WaitSentFrame = false;
  SetBaudrate(115200);
  SetStopBits(1);
  SetParity(0);
  SetDTR(true);
  SetRTS(true);
  SetRtsFlowControl(false);
  SetComDev(CanDev);
  KCanDriver::Init(CanDev);
#if defined(__WINDOWS__)
  if (!strncmp("com", KCanDriver::dev, 3))
  {
    const char * ptr = KCanDriver::dev + 3;
    TInt64 n;
    if (NUtils::GetInt(ptr, n))
      SetComPortNr((int) n);
  }
#endif
  
  if (InitCOM())
  {
    if (AsSimulation)
      return true;
      
    const char * init = INIT_STR;
    while (init)
    {
      const char * p = strstr(init, "\r");
      if (!p)
        break;
      WriteBufferToCOM(init, (int)(p - init) + 1);
      NUtils::SleepMs(100);
      init += p - init + 1;
    }
    return true;
  }
  return false;
}

bool KCan232::ReceiveData(KCanFrame & Frame)
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
            WriteBufferToCOM("V1324\r", 6);
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

bool KCan232::SendData(const KCanFrame & Frame)
{
  WaitSentFrame = false;
  KCan232Frame can232;
  if (can232.GetCanFrame(Frame) &&
      WriteBufferToCOM(can232.msg, (int) strlen(can232.msg)))
  {
    tel_len = 0;
    WaitSentFrame = true;
    if (WriteBufferToCOM("\r", 1))
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


////////////////////////////////////////////////////////////////////////////////

// optisches Interface: 2400 Bit/s, even, 2 Stopp-Bits, RTS
// jede Message ist 12 Byte lang (vgl. KComfortFrame)

bool KCanCS::Init(const char * CanDev)
{
  tel_len = 0;
  WaitSentFrame = false;
  SenderIsCS = false;
  Delay = 0;
  // ComfortSoft standard setting
  SetBaudrate(2400);
  SetStopBits(2);
  SetParity(2); // even
  SetDTR(false); // optisches Original-Interface ist false
  SetRTS(true);
  SetRtsFlowControl(false);
  SetComDev(CanDev);
  KCanDriver::Init(CanDev);
#if defined(__WINDOWS__)
  if (!strncmp("com", KCanDriver::dev, 3))
  {
    const char * ptr = KCanDriver::dev + 3;
    TInt64 n;
    if (NUtils::GetInt(ptr, n))
      SetComPortNr((int) n);
  }
#endif

  return InitCOM();
}

bool KCanCS::ReceiveData(KComfortFrame & Frame)
{
  int res = ReadFromCOM();
  while (res != NULL_CHAR)
  {
    if (res < NULL_CHAR)
    {
      if (SenderIsCS && tel_len == 1 && tel[0] != 0x0d)
      {
        tel_len = 0;
      } else
      if (tel_len >= 12)
      {
        for (int i = 0; i < 12; i++)
          Frame.Data[i] = tel[i];

        tel_len = 0;
        if (Frame.CheckSum())
        {
#if defined(__SIMULATION__)
          if (Frame.Data[0] < 0x0d)
#endif
            return true;
        }
      }
    } else
      tel_len = 0;

    res = ReadFromCOM();
  }
  Delay++;
  NUtils::SleepMs(1);
  if (Delay > 200)
    tel_len = 0;
  if (tel_len == 0)
    Delay = 0;
  return false;
}

bool KCanCS::ReceiveData(KCanFrame & Frame)
{
  KComfortFrame cf;

  if (!ReceiveData(cf))
    return false;

  if (cf.CheckSum() &&
      cf.SetCanFrame(Frame))
  {
    NUtils::Time(Frame.TimeStampDay, Frame.TimeStampMs);
  } else
    Frame.Len = 0;

  return true;
}

bool KCanCS::SendData(const KCanFrame & Frame)
{
  KComfortFrame CF;

  if (!CF.GetCanFrame(Frame))
    return false;

  return SendData(CF);
}

bool KCanCS::SendData(const KComfortFrame & Frame)
{
  bool Ok = WriteBufferToCOM((char *)Frame.Data, 12);

  tel_len = 0;

  return Ok;
}


