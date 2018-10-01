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

#if !defined(KRPiCanDriver_H) && defined(__LINUX__)

  #define KRPiCanDriver_H

  #include "KCanDriver.h"
  #include "KIpSocket.h"

  class KRPiCanDriver : public KIpSocket, public KCanDriver
  {
    private:
      unsigned WantTimeStamp;
      struct sockaddr_can * addr;

    public:
      KRPiCanDriver();
      virtual ~KRPiCanDriver();

      virtual bool Init(const char * CanDev); // can0, can1, all (only for reading)
      virtual void Close() {}
      virtual bool Connect();
      bool Bind();

      virtual bool ReceiveData(KCanFrame & Frame);
      virtual bool ReadData(KCanFrame & Frame);
      virtual bool SendData(const KCanFrame & Frame);
  };

#endif

