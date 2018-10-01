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

#ifndef KTcpServer_H

  #define KTcpServer_H

  #if defined(__LINUX__) || defined(__MAC__)
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
  #endif

  #include "KIpSocket.h"
#if defined(__CAN__)
  #include "KTcpCanClient.h"
#else
  #include "KTcpClient.h"
#endif
  #include "KThread.h"


  //////////////////////////////////////////////////////////////////////////////
  //
  // KTcpServer
  //
  //////////////////////////////////////////////////////////////////////////////

  class KCriticalSection;

  // TcpServer als aktives Objekt hat zur Aufgabe, an der Schnittstelle
  // kontinuierlich zu horchen. TcpServer bedient höchstens eine Verbindung.
  class KTcpServer : private KThread, private KIpSocket
  {
    private:
      #if defined(__LINUX__) || defined(__MAC__)
        // Mutex/CriticalSection wrappers
        KCriticalSection * mThreadLock;
      #endif
      TSocketBlockModeEnum mServerBlockMode;
      bool mListening;

      bool Accept();
      void SetServerBlockMode(TSocketBlockModeEnum aValue);
      bool WaitForConnection(int aTimeOut);

    protected:
      unsigned mTryToConnectTimeout;
      unsigned mConnectionSleep;

    public:
      KTcpServer();
      ~KTcpServer();

      bool Open();
      bool Opened() const { return mListening; }
      void Close();
      void Halt();
      bool IsTerminated() { return Terminated(); }

      bool IsListening() const { return mListening; }
      void SetListening(bool Listening) { mListening = Listening; }
      void TryToConnect(int aTimeOut);

      unsigned short GetLocalPort() const { return KIpSocket::GetLocalPort(); }
      void SetLocalPort(unsigned short Value);
      bool ReadyForReading(int aTimeOut);

      virtual void Execute();
      // Datenaustausch nach einem Verbindungsaufbau.
    #if defined(__CAN__)
      virtual void ServeConnection(KTcpCanClient & aClientSocket)=0;
    #else
      virtual void ServeConnection(KTcpClient & aClientSocket)=0;
    #endif
  };

#endif

