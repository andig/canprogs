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

#if !defined(KCommVCL_H)

  #define KCommVCL_H

  #include "KComm.h"
  #include "Comm.hpp"
  #include "Comm_dlg.hpp"

  #define MaxMonitorBuffer 0x400

  class KCommVCL : public KComm
  {
    public:
      volatile int FInCommMonitor;
      int FMonitorBuffer[MaxMonitorBuffer];
      int FMonitorBufferIn;
      int FMonitorBufferOut;
      TConsoleWnd * ConsoleWnd;
      TTimer * FTimer;
      bool FInDialog;
      _OVERLAPPED * osWrite;
      _OVERLAPPED * osRead;
      _OVERLAPPED * osPostEvent;
      unsigned FCOM_Handle;
    #pragma pack(push, 1)
      _DCB FDCB;
    #pragma pack(pop)
      _NOTIFYICONDATAA FNotifyData;
      Console::TPopUpMenuClass* pmLeiste;

    public:
      bool LFMonitor;
      bool HexMonitor;
      bool DezMonitor;
      bool ErrorOccured;

      KCommVCL(Classes::TComponent * Owner);
      ~KCommVCL();

      int GetMonitorChar();
      bool ParameterDialog();

      void __fastcall mnShowClick(System::TObject* Sender);
      void __fastcall mnConfigClick(System::TObject* Sender);
	    void __fastcall CommMonitor(System::TObject* Sender);

      virtual void PutMonitorBuffer(int msg);
      virtual void NewScreenSize(int x, int y);
      virtual void SetTimerInterval(int time);
      virtual void Show();
      virtual void write(const char * str);
      virtual void writeln();
      virtual void WriteChar(char c);
  };

#endif
