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

#if !defined(KCanTcpDriver_H)

  #define KCanTcpDriver_H

  #include "KTcpClient.h"
  #include "KCanDriver.h"

  // Treiber zu den seriellen Schnittstellen verwenden KComm;
  // SocketCAN verwendet KCanDriver.
  //
  // Das optische Interface verwendet für die Daten die Struktur KComfortFrame;
  // alle andern KCanFrame.

  class KCanTcpDriver : public KTcpClient, public KCanDriver
  {
    protected:
      unsigned time_out;
      unsigned time_stamp;
      unsigned tel_len;
      unsigned char tel[128];

    public:
 
      KCanTcpDriver();
      virtual ~KCanTcpDriver();
    
      int ReadFromCOM();

      virtual bool Init(const char * TcpAddr);
    
      virtual void Close();
      virtual bool Connect();
      virtual unsigned GetPort() const { return GetRemotePort(); };

      virtual bool ReadData(KCanFrame & Frame) { return false; }
      virtual bool SendData(const KCanFrame & Frame) { return false; }
      virtual bool SendData(const KComfortFrame & Frame) { return false; }
  };

  class KCan232Tcp : public KCanTcpDriver
  {
    private:
      KCanFrame SentFrame;
      volatile bool WaitSentFrame;

    public:

      virtual bool Init(const char * TcpAddr);

      virtual bool ReceiveData(KCanFrame & Frame);
      virtual bool SendData(const KCanFrame & Frame);
      virtual bool SendData(const KComfortFrame & Frame) { return false;}
  };

#endif

