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

#if !defined(KTcpElsterServer_H) && !defined(__CAN_CLIENT__)

  #define KTcpElsterServer_H

  #include "KCanElster.h"
  #include "KTcpCanClient.h"
  #include "KTcpServer.h"

  class KTcpElsterServer : public KTcpServer
  {
    private:
      KCanElster * CanElster;

      void ServeToDevice(const KComfortFrame & cf, KComfortFrame & cfRes);
      void ServeSimulation(const KComfortFrame & cf, KComfortFrame & cfRes);
      bool ReadComfortFrame(KTcpCanClient & aClientSocket, KComfortFrame & cf);

    public:
      bool Trace;
      bool Simulation;
      bool UseBinaryProtocol;

      KTcpElsterServer();

      bool Init(const char * CanDev, bool trace);
      static bool ReadElster(KTcpCanClient & aClientSocket, char * Buffer);
      virtual void ServeConnection(KTcpCanClient & aClientSocket);
      const char * GetCanDev() const;
  };

#endif

