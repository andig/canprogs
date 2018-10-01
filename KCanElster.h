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

#if !defined(KCanElster_H) && !defined(__UVR__)

  #define KCanElster_H

  #include "NCanUtils.h"
  #include "KCanServer.h"
  #include "KSniffedFrame.h"

  class KCanElster : public KCanServer
  {
    private:
      bool InitSendFrame(unsigned short receiver_id, unsigned short elster_idx);
      bool Send_Frame();
      bool SniffedFrame();
      bool Get_Frame();
      bool GetKennwortValue(unsigned short recv_id, unsigned short elster_idx,
                            unsigned short & Value,
                            unsigned short & Value1, unsigned short & Value2);

    public:
      unsigned short SendCanId;
      KCanFrame SendFrame;
      KCanFrame RecvFrame;
    #if defined(KSniffedFrame_H)
      KSniffedData SniffedData;
      const KSniffedFrame * GetFirstSniffedFrame();
    #endif
      enum NCanUtils::driver_type driver_type;
      KCanElster();
      virtual ~KCanElster();

      bool Init(const char * CanDev, unsigned short DefaultId = 0x700);
      bool Send(unsigned count = 3, bool WaitForAnswer = true, int inner_delay = 50);
      bool GetValue(unsigned short receiver_id, unsigned short elster_idx, unsigned short & Value);
      bool SetValue(unsigned short receiver_id, unsigned short elster_idx, unsigned short Value);
      bool SetValue();
      void Scanner(unsigned short receiver_id, bool get_all);
      void GetValidDevices(); // try with Elster-Index 0x000b; if it aswers, it is a valid device
      void EmptyServer();
      bool GetInitCS_Id(); // result is in: KComfortFrame::InitId
      bool GetDoubleValue(unsigned short first_val, unsigned scan_can_id,
                          unsigned short elster_idx, unsigned char elster_type, double & result);

      void PrintSendFrame();
      void PrintRecvFrame();
    #if defined(__USBTIN__)
      bool InitUSBtin(const char * com_dev, bool trace);
      bool ServeUSBtin();
    #endif
  };

#endif

