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

#if !defined(KCanCommDriver_H)

  #define KCanCommDriver_H

  #include "KComm.h"
  #include "KCanDriver.h"

  // Treiber zu den seriellen Schnittstellen verwenden KComm;
  // SocketCAN verwendet KCanDriver.
  //
  // Das optische Interface verwendet für die Daten die Struktur KComfortFrame;
  // alle andern KCanFrame.

#if defined(VCL)
  #include "win/KCommVCL.h"

  class KCanCommDriver : public KCommVCL, public KCanDriver
#else
  class KCanCommDriver : public KComm, public KCanDriver
#endif
  {
    protected:
      unsigned time_out;
      unsigned time_stamp;
      unsigned tel_len;
      unsigned char tel[128];

    public:
    #if defined(VCL)
      KCanCommDriver(Classes::TComponent * owner = NULL);
    #else
      KCanCommDriver();
    #endif
    
      virtual ~KCanCommDriver();
    
      int ReadFromCOM();

      virtual void Close();
      virtual bool Connect();

      virtual bool ReadData(KCanFrame & Frame) { return false; }
      virtual bool SendData(const KCanFrame & Frame) { return false; }
      virtual bool SendData(const KComfortFrame & Frame) { return false; }
  };

  class KCan232 : public KCanCommDriver
  {
    private:
      KCanFrame SentFrame;
      volatile bool WaitSentFrame;

    public:

      virtual bool Init(const char * CanDev);

      virtual bool ReceiveData(KCanFrame & Frame);
      virtual bool SendData(const KCanFrame & Frame);
      virtual bool SendData(const KComfortFrame & Frame) { return false;}
  };

  class KCanCS : public KCanCommDriver
  {
    private:
      KCanFrame SentFrame;
      volatile bool WaitSentFrame;

    public:
      bool SenderIsCS;
      unsigned Delay;
    
      virtual bool Init(const char * CanDev);

      virtual bool ReceiveData(KCanFrame & Frame);
      virtual bool ReceiveData(KComfortFrame & Frame);
      virtual bool SendData(const KComfortFrame & Frame);
      virtual bool SendData(const KCanFrame & Frame);
  };

#endif

