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
#include "NCanUtils.h"
#include "KCriticalSection.h"
#include "KRPiCanDriver.h"
#include "KCanCommDriver.h"
#include "KCanTcpDriver.h"

#if defined(__WINDOWS__)
  #include "special/Kusb2can.h"
#endif

#include "KCanServer.h"


KCanServer::KCanServer()
{
  DriverType = NCanUtils::dt_can;
  AsSimulation = false;

  CanDriver = NULL;
#if defined (__LINUX__)
  CanSendDriver = NULL;
#endif
  FrameCount = 0;
  IsInitalized = false;
  RingHead = RingTail = 0;
  RingBufferSize = 10240;
  RingBuffer = new KCanFrame[RingBufferSize];
  memset(RingBuffer, 0, sizeof(KCanFrame)*RingBufferSize);
  Semaphore = new KCriticalSection;
}

KCanServer::~KCanServer()
{
  if (CanDriver)
  {
    CanDriver->Close();
    delete CanDriver;
  }
#if defined (__LINUX__)
  if (CanSendDriver)
  {
    CanSendDriver->Close();
    delete CanSendDriver;
  }
#endif
  delete Semaphore;
  delete [] RingBuffer;
}

bool KCanServer::Init(const char * CanDev)
{
  if (IsInitalized)
    return false;

  IsInitalized = true;
  if (!CanDriver)
  {
    enum NCanUtils::driver_type new_dt;
      
    if (DriverType == NCanUtils::dt_cs)
      new_dt = DriverType;
    else
      new_dt = NCanUtils::GetDriverType(CanDev);
 
    if (new_dt == NCanUtils::dt_unknown && KIpSocket::IsHostAddr(CanDev))
      new_dt = NCanUtils::dt_can232_remote;
      
    if (new_dt != NCanUtils::dt_unknown)
      DriverType = new_dt;
   
    switch (DriverType)
    {
      case NCanUtils::dt_can:
      #if defined (__LINUX__)
        CanDriver = new KRPiCanDriver;
        #if !defined(__UVR__)
          CanSendDriver = new KRPiCanDriver;
        #endif
      #endif
        break;
     
      case NCanUtils::dt_can232_remote:
        CanDriver = new KCan232Tcp;
        break;
        
      case NCanUtils::dt_cs:
        CanDriver = new KCanCS;
        break;
        
      case NCanUtils::dt_can232:
        CanDriver = new KCan232;
        break;
        
    #if defined(__WINDOWS__)    
      case NCanUtils::dt_8dev:
        CanDriver = new Kusb2canDriver;
        break;
    #endif
        
      default:
        return false;
    }
  }
  CanDriver->AsSimulation = AsSimulation;
  if (!CanDriver->Init(CanDev))
    return false;
  
#if defined (__LINUX__)
  if (CanSendDriver)
  {
    if (!CanSendDriver->Init(CanDev) ||
        !CanSendDriver->Connect())
      return false;
  }
#endif
  return true;
}

void KCanServer::Halt()
{
  Terminate();
  while (!Terminated())
    ;

  NUtils::SleepMs(10);
}

int  KCanServer::RingCount() const
{
  return (RingHead - RingTail + RingBufferSize) % RingBufferSize;
}

bool KCanServer::RingBufferFull() const
{
  return RingCount() > 3*(RingBufferSize / 4);
}

void KCanServer::IncRingPtr(volatile int & RingPtr)
{
  RingPtr = (RingPtr + RingBufferSize + 1) % RingBufferSize;
}

void KCanServer::PutFrame(const KCanFrame & Frame)
{
  if (Semaphore->Acquire())
  {
    IncRingPtr(RingHead);
    if (RingTail == RingHead)
      IncRingPtr(RingTail);
    RingBuffer[RingHead] = Frame;

    Semaphore->Release();
  }
}

bool KCanServer::GetFrame(KCanFrame & Frame)
{
  bool Ok = false;
  if (RingHead == RingTail)
    return Ok;

  if (Semaphore->Acquire())
  {
    if (RingTail != RingHead)
    {
      Ok = true;
      IncRingPtr(RingTail);
      Frame = RingBuffer[RingTail];
    }
    Semaphore->Release();
  }
  return Ok;
}

bool KCanServer::SendData(const KCanFrame & Frame)
{
#if defined (__LINUX__)
  if (CanSendDriver)
    return CanSendDriver->SendData(Frame);
#endif

  if (!CanDriver)
    return false;

  return CanDriver->SendData(Frame);
}

bool KCanServer::SendData(const KComfortFrame & Frame)
{
#if defined (__LINUX__)
  if (CanSendDriver)
    return CanSendDriver->SendData(Frame);
#endif

  if (!CanDriver)
    return false;

  return CanDriver->SendData(Frame);
}

bool KCanServer::ReceiveData(KCanFrame & Frame)
{
  if (CanDriver)
  {
    return CanDriver->ReceiveData(Frame);
  }
  return false;
}

void KCanServer::SetTrace(bool trace)
{
  Trace = trace;
  if (CanDriver)
    CanDriver->Trace = trace;
}

void KCanServer::Execute()
{
#if defined(__CONSOLE__) && defined(__DEBUG__)
  printf("start CanServer\n");
#endif
  Init(NULL);
  if (!CanDriver->Connect())
    Terminate();

  while (!Terminated())
  {
    KCanFrame Frame;

    if (CanDriver->ReceiveData(Frame))
    {
      Frame.Counter = (int)FrameCount++;
      PutFrame(Frame);
    }
  }
#if defined(__CONSOLE__) && defined(__DEBUG__)
  printf("stop CanServer\n");
#endif
}

