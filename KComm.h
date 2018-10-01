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

#if !defined(KComm_H)

  #define KComm_H

  #if defined(__WINDOWS__)
    #include <windows.h>
  #endif

  #define MaxReadBuffer   100   /* length of type ahead buffer */
  #define NULL_CHAR     0x1000  /* no character in buffer */

  typedef enum
  {
    T_EV_BREAK = 1,
    T_EV_CTS = 2,
    T_EV_DSR = 4,
    T_EV_ERR = 8,
    T_EV_RING = 0x10,
    T_EV_RLSD = 0x20
  } TComEvent;


  class KComm
  {
    protected:
      int FComPortNr;
      int FBaudrate;
      // Codierung der Parität:   0: None
      //                          1: Odd
      //                          2: Even
      //                          3: Mark
      //                          4: Space
      int FParity;
      int FByteSize;
      // Codierung der Stopp-Bits: 0:   1 Bit
      //                           1: 1.5 Bits
      //                           2:   2 Bits
      int FStopBits;
      bool FDTR, FRTS;
      bool FRtsFlowControl;
      // Zum Lesen wird ein Puffer verwendet.
      // Zeichen können an der Stelle "ReadBuffer[ReadBufferPtr]" gelesen
      // werden, falls "ReadBufferPtr < ReadBufferLen".
      unsigned char FReadBuffer[MaxReadBuffer];
      int FReadBufferLen;
      int FReadBufferPtr;
      char dev[64];

  #if defined(__WINDOWS__)
      TComEvent FEventMask;
      unsigned long FEvMask;
      // Die serielle Schnittstelle ist geöffnet, falls COM_Handle >= 0;
      // Fehler, falls < 0
      OVERLAPPED osWrite;
      OVERLAPPED osRead;
      OVERLAPPED osPostEvent;
      HANDLE FCOM_Handle;
      DCB FDCB;

      void SetEventMask(TComEvent EventMask);
      // VCL
      //procedure WM_MonMsg(var _Message : TMessage); message(MonWm_NOTIFY);
  #endif
  #if defined(__LINUX__)
      int FCOM_Handle;
  #endif
    public:
      bool ErrorOccured;

      KComm();
      ~KComm();

      void ExitCOM();
      bool COM_INITIALISIERT();
      bool SetComParameters();
      void SetComPortNr(int Nr);
      void SetBaudrate(int Baud);// NoClose : boolean = false);
      void SetComDev(const char * tty_dev);
      bool InitCOM();
      void ClearBuffer();
      int  ReadFromCOM();
      int  ReadWithTimeOut(int TimeMs);
      bool WriteToCOM(char ch);
      bool WriteBufferToCOM(const char * str, int len);

    #if defined(__WINDOWS__)
      unsigned long GetCommModemStatus();
    #endif
      unsigned WaitCommEvent();
      unsigned Event();
      bool CTS();  // clear-to-send signal is on
      bool DSR();  // data-set-ready signal is on
      bool RING(); // ring-indicator signal is on
    #if defined(__WINDOWS__)
      bool DCD();  // receive-line-signal-detect
    #endif
      void SetDTR(bool NewDTR);
      void SetRTS(bool NewRTS);
      void SetRtsFlowControl(bool NewRTS);
      void SetBits(int NewBits);
      void SetStopBits(int NewStopBits);
      void SetParity(int NewParity);
      int  SendBreak(int Duration);

      virtual void PutMonitorBuffer(int msg) {}
      virtual void NewScreenSize(int x, int y) {}
      virtual void SetTimerInterval(int time) {}
      virtual void Show() {}
      void CommMonitor(void *) {}
      virtual void write(const char * str) {}
      virtual void writeln() {}
      virtual void WriteChar(char c) {}
      virtual void WriteNow() {}
  };
#if !defined(VCL)
  extern bool HideError;
#endif
#endif

