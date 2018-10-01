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

#if defined(__WINDOWS__)
  #if defined(__CAN__)
    #include <winsock.h>
  #endif
  #include <windows.h>
#endif

#include "NTypes.h"

#if defined(__LINUX__)
  // Definition von sockaddr_in (Socket address, internet style)
  #include <netinet/in.h>
  // KTcpServer
  #include "KCriticalSection.h"
#endif

#include "NUtils.h"
using namespace NUtils;

#include "KTcpServer.h"


////////////////////////////////////////////////////////////////////////////////
//
// KTcpServer
//
////////////////////////////////////////////////////////////////////////////////

KTcpServer::KTcpServer()
{
  #if defined(__LINUX__)
    mThreadLock = new KCriticalSection;
  #endif

  mServerBlockMode = bmThreadBlocking;
  mListening = false;
  SetServerBlockMode(bmNonBlocking);
  mTryToConnectTimeout = 100;
  mConnectionSleep = 1;
  #if !defined(__NO_MULTITHREADS__)
    Resume();
  #endif
}

KTcpServer::~KTcpServer()
{
  Close();
  #if !defined(__NO_MULTITHREADS__)
    Terminate();
  #endif
  #if defined(__LINUX__)
    mThreadLock->Acquire();
    try
    {
    }
    catch (...)
    {
    #if defined(__CONSOLE__)
      printf("exception caught in \"KTcpServer::~KTcpServer\"\n");
    #endif
    }
    mThreadLock->Release();
  #endif
  #if !defined(__NO_MULTITHREADS__)
    int TimeOut = 2000;
    while (!Finished() && TimeOut > 0)
    {
      NUtils::SleepMs(10);
      TimeOut -= 10;
    }
  #endif

  #if defined(__LINUX__)
    delete mThreadLock;
  #endif

  KIpSocket::Close();
}

void KTcpServer::SetLocalPort(unsigned short Value)
{
  if (!Value)
    Value = mDefaultTcpIpPort;

  KIpSocket::SetLocalPort(Value);
}

bool KTcpServer::Accept()
{
  int Sock;
  sockaddr_in Addr;
#if defined(__LINUX__) && !defined(__CYGWIN__)
  unsigned int Len;
#else
  int Len;
#endif
  bool Ok = true;
#if defined(__CAN__)
  KTcpCanClient ClientSocket;
#else
  KTcpClient ClientSocket;
#endif

  if (!mActive)
    return false;

  Len = sizeof(Addr);
  memset(&Addr, 0, sizeof(Addr));
  try
  {
    Sock = ErrorCheck(accept(mSocket, (sockaddr *)(&Addr), &Len), "accept");
  }
  catch (...)
  {
    Sock = INVALID_SOCKET;
  #if defined(__CONSOLE__)
    printf("exception caught in \"KTcpServer::Accept\" (1)\n");
  #endif
  }
  if (Sock != INVALID_SOCKET && Sock != SOCKET_ERROR)
  {
    try
    {
      ClientSocket.mActive = true;
      ClientSocket.mConnected = true;
      ClientSocket.mSocket = Sock;
      ClientSocket.mDomain = mDomain;
      ClientSocket.mSocketType = mSocketType;
      ClientSocket.mProtocol = mProtocol;
      ClientSocket.mBlockMode = bmBlocking; //mBlockMode; Änderung vom 7. 9. 2004
      ClientSocket.StoreRemote(Addr);
      ClientSocket.SetBlockMode(bmNonBlocking);

      ServeConnection(ClientSocket);
    }
    catch(...)
    {
      Ok = false;
    #if defined(__CONSOLE__)
      printf("exception caught in \"KTcpServer::Accept\" (2)\n");
    #endif
    }
    ClientSocket.Disconnect(); // Wird auch vom Destructor aufgerufen
    return Ok;
  }

  return false;
}

void KTcpServer::Close()
{
  mListening = false;
  // Gibt unter Linux einen "segmentation fault", wenn der TcpServerDialog
  // geschlossen wird!
  // Ohne Close kann der Server nicht aus- und dann wieder eingeschaltet werden.
//#if defined(__WINDOWS__)
  KIpSocket::Close();
//#endif
}

void KTcpServer::Execute()
{
  while (!Terminated())
  {
    TryToConnect(mTryToConnectTimeout);
    NUtils::SleepMs(mConnectionSleep);
  }
}

bool KTcpServer::Open()
{
  Close();
  KIpSocket::Open();
  if (Bind())
  {
     mListening = ErrorCheck(listen(mSocket, SOMAXCONN), "listen") == 0;
  }
  return mListening;
}

void KTcpServer::SetServerBlockMode(TSocketBlockModeEnum aValue)
{
  if (aValue != mServerBlockMode)
  {
    mServerBlockMode = aValue;
    if (aValue == bmThreadBlocking)
      SetBlockMode(bmBlocking);
    else
      SetBlockMode(aValue);
  }
}

void KTcpServer::TryToConnect(int aTimeOut)
{
  if (WaitForConnection(aTimeOut))
  {
    #if !defined(__NO_MULTITHREADS__)
      while (!Terminated() && mActive)
    #endif
      {
        if (!Accept())
          return;
        NUtils::SleepMs(2);
      }
  }
}

bool KTcpServer::ReadyForReading(int aTimeOut)
{
  bool ReadReady;
  bool ExceptFlag;
  bool Result = false;

  if (Select(&ReadReady, NULL, &ExceptFlag, aTimeOut))
    Result = ReadReady && !ExceptFlag;

  return Result;
}

bool KTcpServer::WaitForConnection(int aTimeOut)
{

  bool Result = false;

  if (!mListening)
    return false;

#if defined(__LINUX__)
  if (mBlockMode == bmThreadBlocking)
  {
    // Hack to avoid server thread block forever in linux
    mThreadLock->Acquire();
    try
    {
      Result = ReadyForReading(aTimeOut);
    }
    catch(...)
    {
      Result = false;
    #if defined(__CONSOLE__)
      printf("exception caught in \"KTcpServer::WaitForConnection\"\n");
    #endif
    }
    mThreadLock->Release();
  }
  else
#endif
    Result = ReadyForReading(aTimeOut);

  return Result;
}

void KTcpServer::Halt()
{
  Terminate();
  mListening = false;
  while (!Terminated())
    ;
  
  NUtils::SleepMs(300);
}

