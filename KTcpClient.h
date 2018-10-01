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

#ifndef KTcpClient_H

  #define KTcpClient_H

  #if defined(__LINUX__) || defined(__MAC__)
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
  #endif

  #include "KIpSocket.h"


  //////////////////////////////////////////////////////////////////////////////
  //
  // KTcpClient
  //
  //////////////////////////////////////////////////////////////////////////////

  // Erste Timeout-Zeit
  #define cTimeOutSynch 2000

  class KTcpClient : public KIpSocket
  {
    protected:
      bool mConnected;
      char mRemotePath[256];

    public:
      friend class KTcpServer;

      int  mReadTimeout;
      int  mSendTimeout;
      bool mTrace;

      KTcpClient();

      virtual ~KTcpClient();

      bool Connect(int aTimeout = 0);
      bool Connected() { return mConnected; }
      void CloseConnection() { mConnected = false; }
      void Disconnect();

      bool ReceiveBuffer(char * Buffer, int BufferLen);
      // Liest "aToRead"-Bytes in den Puffer "aBuffer". Es kann sein, dass dazu
      // mehrere Pakete gelesen werden müssen.
      bool ReadBuffer(char* aBuffer, int aToRead);

      bool SendBuffer(const char * aBuffer, int aToSend, int TimeOut = 1000);

      bool ReadLine(char * aString, int aLength);
      bool WriteString(const char * aBuffer);
      bool WriteLine(const char * aString);

      // CheckConnection wird verwendet, um alle TCP/IP-Adressen zu scannen.
      bool CheckConnection(int aTimeout);

  };

  //////////////////////////////////////////////////////////////////////////////
  //
  // KHttp
  //
  //////////////////////////////////////////////////////////////////////////////

  class KHttp
  {
    protected:
      bool mIpError;

    public:
      enum THttpType
      {
        htOptions,
        htGet,
        htHead,
        htPost,
        htPut,
        htDelete,
        htTrace,
        htConnect,
        htUnknown
      };

      int mShowLevel;

      static const char * Methods[htUnknown];

      static const char * StatusCode(int aCode);

      KHttp();

      bool ReadLine(KTcpClient & aClientSocket, char * aString, int aLength, bool aShow = false);
      bool WriteLine(KTcpClient & aClientSocket, const char * aString, bool aShow = false);
  };

#endif

