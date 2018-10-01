/*
 *
 *  Copyright (C) 2007-2014 Jürg Müller, CH-5524
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

#ifndef KIpSocket_H

  #define KIpSocket_H

  #include "NTypes.h"

  #if defined(__LINUX__) || defined(__MAC__)
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
  #endif

  enum TSocketBlockModeEnum
  {
    bmBlocking,
    bmNonBlocking,
    bmThreadBlocking
  };

  enum TSocketTypeEnum
  {
    stStream,
    stDgram,
    stRaw,
    stRdm,
    stSeqPacket
  };

  class KIpSocket
  {
    public:
      bool mActive;
    #if defined(__WINDOWS__)
      unsigned int  mSocket; // WINDOWS uses unsigned
    #else
      int mSocket;
    #endif
      int mDomain;
      TSocketTypeEnum mSocketType;
      unsigned short  mProtocol;
      TSocketBlockModeEnum mBlockMode;
      int mLastError;

      char mLocalHost[256];
      unsigned short mLocalPort;
      char mRemoteHost[256];
      unsigned short mRemotePort;

      bool Open();
      void Close();

    public:
      friend class KTcpServer;

      static const unsigned short mDefaultTcpIpPort;

      KIpSocket();
      virtual ~KIpSocket();

      int  ErrorCheck(long rc, const char * aPlace);

      int  PeekBuf(char & Buf, int BufSize);
      int  ReceiveBuf(char* aBuf, int aBufSize, int aFlags = 0, int aTimeout = 0);
      bool Select(bool * ReadReady, bool * WriteReady, bool * ExceptFlag,
                  int TimeOut = 0);
      int  SendBuf(const char * aBuf, int aBufSize, int aFlags = 0,
                   int aTimeout = 0);
    #if defined(__LINUX__) && !defined(__ARM__)
      bool SendFile(int file, TInt64 offset, TInt64 length);
    #endif
      TSocketBlockModeEnum GetBlockMode() const { return mBlockMode; }
      void SetBlockMode(TSocketBlockModeEnum aValue);

      bool WaitForData(int TimeOut = 0);

      virtual bool Bind();

      void Deactivate() { mActive = false; }
      bool IsActive() { return mActive; }

      static void GetSocketAddr(struct sockaddr_in & aResult, char * aHost,
                                unsigned short aPort);
      static void LocalDomainName(char * aResult, int aResultSize);
      static void LocalHostAddr(char * aResult, int aResultSize);
      static void LocalHostName(char * aResult, int aResultSize);
      static void LookupHostAddr(const char * aHn, char * aResult,
                                 int aResultSize);
      static bool IsHostAddr(const char * aHn);
      static void LookupHostName(const char * ipaddr, char * aResult,
                                 int aResultSize);
      static unsigned short LookupPort(const char * aSn, char* aPn = 0);
      static unsigned short LookupProtocol(const char * pn);

      int ReceiveFrom(char* buf, int bufsize, struct sockaddr ToAddr, int & len,
                      int flags = 0, int aTimeout = 0);
      int SendTo(char* buf, int bufsize, sockaddr ToAddr, int flags = 0,
                 int aTimeout = 0);
      unsigned short GetLocalPort() const;
      void SetLocalPort(unsigned short Value);
      void SetLocalHost(const char * Value);
      void SetRemoteHost(const char * Value);
      void SetRemotePort(unsigned short Value);
      const char * GetRemoteHost() const { return mRemoteHost; }
      unsigned short GetRemotePort() const { return mRemotePort; }

      // stores the values to mPort and mRemoteHost
      bool StoreRemote(const sockaddr_in & aAddr);
  };

#endif

