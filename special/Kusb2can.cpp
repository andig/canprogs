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

#if defined(__WINDOWS__)

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "canal.h"
#include "../NUtils.h"
#include "../KScanTable.h"
#include "Kusb2can.h"

static const bool Simulation = false;
static KComfortFrame LastFrame;

static HINSTANCE mInstance = NULL;

extern "C"
{
  typedef long (WINAPI * CanalOpenProc)( const char *pDevice, unsigned long flags );
  typedef int (WINAPI * CanalCloseProc)( long handle );
  typedef int (WINAPI * CanalReceiveProc)( long handle, PCANALMSG pCanalMsg );
  typedef int (WINAPI * CanalBlockingReceiveProc)( long handle, PCANALMSG pCanalMsg, unsigned long timeout);
  typedef int (WINAPI * CanalSendProc)( long handle, const PCANALMSG pCanalMsg );
  typedef int (WINAPI * CanalBlockingSendProc)( long handle, const PCANALMSG pCanalMsg, unsigned long timeout);
  typedef int (WINAPI * CanalDataAvailableProc)( long handle );
  typedef int (WINAPI * CanalGetStatusProc)( long handle, PCANALSTATUS pCanalStatus );
}

static CanalOpenProc pCanalOpen = NULL;
static CanalCloseProc pCanalClose = NULL;
static CanalReceiveProc pCanalReceive = NULL;
static CanalBlockingReceiveProc pCanalBlockingReceive = NULL;
static CanalSendProc pCanalSend = NULL;
static CanalBlockingSendProc pCanalBlockingSend = NULL;
static CanalDataAvailableProc pCanalDataAvailable = NULL;
static CanalGetStatusProc pCanalGetStatus = NULL;

bool LoadCanalLibrary()
{
  if (mInstance || Simulation)
    return true;

  mInstance = LoadLibrary("usb2can.dll"); 
  if (!mInstance && (GetVersion() && 0xff) > 5 /* Win XP */)
  {
    mInstance = LoadLibrary("vscpl1_usb2candrv.dll");
  }
  if (mInstance)
  {
    pCanalOpen = (CanalOpenProc) GetProcAddress(mInstance, "CanalOpen");
    pCanalClose = (CanalCloseProc) GetProcAddress(mInstance, "CanalClose");
    pCanalReceive = (CanalReceiveProc) GetProcAddress(mInstance, "CanalReceive");
    pCanalBlockingReceive = (CanalBlockingReceiveProc) GetProcAddress(mInstance, "CanalBlockingReceive");
    pCanalSend = (CanalSendProc) GetProcAddress(mInstance, "CanalSend");
    pCanalBlockingSend = (CanalBlockingSendProc) GetProcAddress(mInstance, "CanalBlockingSend");
    pCanalDataAvailable = (CanalDataAvailableProc) GetProcAddress(mInstance, "CanalDataAvailable");
    pCanalGetStatus = (CanalGetStatusProc) GetProcAddress(mInstance, "CanalGetStatus");
  }
  if (!mInstance ||
      !pCanalOpen ||
      !pCanalClose ||
      !pCanalReceive ||
      !pCanalBlockingReceive ||
      !pCanalSend ||
      !pCanalBlockingSend ||
      !pCanalDataAvailable ||
      !pCanalGetStatus)
  {
    printf("usb2can.dll not loaded\n");
    return false;
  }
  return true;
}


bool ConvertToCanalMsg(canalMsg & Msg, const KCanFrame & Frame)
{
  memset(&Msg, 0, sizeof(Msg));
  Msg.flags = Frame.Flags;
  Msg.id = Frame.Id;
  Msg.sizeData = Frame.Len;
  for (int i = 0; i < Frame.Len && i < 8; i++)
    Msg.data[i] = Frame.Data[i];
  
  return true;
}

bool ConvertToCanFrame(KCanFrame & Frame, const canalMsg & Msg)
{
  Frame.Init();
  Frame.Flags = Msg.flags;
  Frame.Id = Msg.id;
  Frame.Len = Msg.sizeData;
  for (int i = 0; i < Msg.sizeData && i < 8; i++)
    Frame.Data[i] = Msg.data[i];

  NUtils::Time(Frame.TimeStampDay, Frame.TimeStampMs);
 
  return true;
}

Kusb2canDriver::Kusb2canDriver()
{
  CanalHandle = 0;
}

Kusb2canDriver::~Kusb2canDriver()
{
  Close();
}

bool Kusb2canDriver::Init(const char * CanDev) // "E12345678"
{
  char InitStr[140];

  if (!CanDev || strlen(CanDev) > 20)
    return false;
  
  if (!LoadCanalLibrary())
    return false;
    
  Close();

  strcpy(dev, CanDev);
  strcpy(InitStr, dev);
  strcat(InitStr, "; 0; 13; 2; 1; 100"); // 20 kbit/s

  if (pCanalOpen)
    CanalHandle = pCanalOpen(InitStr, 0);

  return CanalHandle > 0 || Simulation;
}

void Kusb2canDriver::Close()
{
  LastFrame.SetOk(false);
  if (CanalHandle)
    pCanalClose(CanalHandle);

  CanalHandle = 0;
}

bool Kusb2canDriver::Connect()
{
  return CanalHandle != 0 || Simulation;
}

bool Kusb2canDriver::ReceiveData(KCanFrame & Frame)
{
  if (Simulation)
  {
    if (LastFrame.Data[0] == 0xAA)
      return false;

    LastFrame.SetCanFrame(Frame);
    LastFrame.SetOk(false);

    return true;
  }
  canalMsg Msg;
  
  if (pCanalReceive(CanalHandle, &Msg) != CANAL_ERROR_SUCCESS)
    return false;
  
  return ConvertToCanFrame(Frame, Msg);
}

bool Kusb2canDriver::ReadData(KCanFrame & Frame)
{
  return ReceiveData(Frame);
}

bool Kusb2canDriver::SendData(const KCanFrame & Frame)
{
  if (Simulation)
  {
    KComfortFrame cf;

    if (!cf.GetCanFrame(Frame))
      return false;

    return SendData(cf);
  }
  canalMsg Msg;
  int res;

  ConvertToCanalMsg(Msg, Frame);

  if (!CanalHandle)
    return false;

  res = pCanalBlockingSend(CanalHandle, &Msg, 20);

  return res == CANAL_ERROR_SUCCESS;
}

bool Kusb2canDriver::SendData(const KComfortFrame & Frame)
{
  if (!Simulation)
    return KCanDriver::SendData(Frame);

  if (!KScanTable::Get_scan_value(Frame, LastFrame))
  {
    LastFrame.SetOk(false);
  }
  NUtils::SleepMs(1);
  return true;
}

#endif

