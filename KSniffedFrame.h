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

#if !defined(KSniffedFrame_H) && !defined(__UVR__)

  #define KSniffedFrame_H

  #include "NCanUtils.h"

  struct KSniffedFrame
  {
    enum state_type
    {
      st_invalid,
      st_valid,
      st_only_time_changed,
      st_value_changed
    };
    unsigned short id;
    unsigned short recv_id;
    unsigned short elster_idx;
    unsigned short value;
    int TimeStampDay;
    int TimeStampMs;
  
    state_type State;
   
    KSniffedFrame * Next;
  
    void Init(unsigned short Id, unsigned short Recv_id, unsigned short idx);
    KSniffedFrame(unsigned short Id, unsigned short Recv_id, unsigned short idx);
    KSniffedFrame(const KCanFrame & Frame);
    bool SetValue(const KCanFrame & Frame);
  };

  // CAN-Bus-Werte, die am Bus mitzulesen sind, können in KSniffedData hinterlegt
  // werden und später ohne am Bus zu intervenieren als "passive Abfrage"
  // ausgelesen werden.
  class KSniffedData
  {
    private:
      KSniffedFrame * Sniffed[256]; // Der Index ist der "ShortCanId" vom Receiver.
      bool UsedCanId[256];
    
    public:
      KSniffedData();
      ~KSniffedData();
    
      void SetUsedCanId(unsigned char Id);
      bool GetUsedCanId(unsigned char Id) { return UsedCanId[Id]; };
      bool AddToSniffed(const KCanFrame & Frame);
      void ReadAll();
      const KSniffedFrame * SearchSniffedFrame(unsigned short id, unsigned short elster_idx);
      const KSniffedFrame * SearchSniffedFrame(const KCanFrame & Frame);
      const KSniffedFrame * GetFirstSniffedFrame();
      bool GetFirstSniffedValue(unsigned short & Value, unsigned short & Id, unsigned short & idx);
      bool ClearSniffedValue(const KCanFrame & Frame);
  };

#endif

