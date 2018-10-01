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

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#if defined(__LINUX__)
  #include <fcntl.h>
  #include <unistd.h>
  #include <termios.h>
  #include <sys/ioctl.h>

  #define COM_HANDLE_CLOSE -1
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/timeb.h>

#include "NTypes.h"

#include "NUtils.h"
#include "KComm.h"

#if defined(__WINDOWS__)
  #define COM_HANDLE_CLOSE INVALID_HANDLE_VALUE
  // VCL
  static const int MonWm_NOTIFY  = WM_APP + 100;
#endif

static const int RXQUEUE = 512;
static const int TXQUEUE = RXQUEUE;
static const int DefaultBaudRate  = 4800;

#if !defined(VCL)
  bool HideError = false;
#endif

KComm::KComm()
{
  FBaudrate = DefaultBaudRate;
  FByteSize = 8;
  FStopBits = 0; // 1 Stop bit
  FDTR = false;
  FParity = 0;
  FRTS = false;
  ErrorOccured = false;

  FComPortNr = 0;
  FCOM_Handle = COM_HANDLE_CLOSE;

  memset(FReadBuffer, 0, sizeof(FReadBuffer));
  FReadBufferLen = 0;
  FReadBufferPtr = 0;
  dev[0] = 0;

#if defined(__WINDOWS__)
  FEventMask = (TComEvent) 0;
  FEvMask = 0;

  // Die serielle Schnittstelle ist geöffnet, falls COM_Handle >= 0;
  // Fehler, falls < 0
  memset(&osWrite, 0, sizeof(osWrite));
  memset(&osRead, 0, sizeof(osRead));
  memset(&osPostEvent, 0, sizeof(osPostEvent));

  // create I/O event used for overlapped reads / writes
  osRead.hEvent =
    CreateEvent(NULL,    // no security
                true,    // explicit reset req
                false,   // initial event reset
                NULL);   // no name
  osWrite.hEvent =
    CreateEvent(NULL,    // no security
                true,    // explicit reset req
                false,   // initial event reset
                NULL);   // no name
  osPostEvent.hEvent =
    CreateEvent(NULL,    // no security
                true,    // explicit reset req
                true,    // initial event reset
                NULL);   // no name
#endif
}

KComm::~KComm()
{
  ExitCOM();
}

bool KComm::COM_INITIALISIERT()
{
  return FCOM_Handle != COM_HANDLE_CLOSE;
}
void ShowLastErr(unsigned err);

#if defined(__WINDOWS__)
void ShowLastErr(unsigned Err)
{
  char MsgBuf[500];
  if (FormatMessageA(
       FORMAT_MESSAGE_FROM_SYSTEM,
       NULL,
       Err,
       LANG_USER_DEFAULT,
       MsgBuf,
       High(MsgBuf),
       NULL) != 0)
  {
    printf("%s\n", MsgBuf);
  } else
    printf("GetLastError: %d\n", Err);
}
#endif

void KComm::ExitCOM()
// Schliesst die serielle Schnittstelle. RTS und DTR werden auf -12V
// zurückgesetzt.
{
  // FDCB.Flag : LongInt
  // Bits:    FBinary ist das LSB
  // 0 DWORD fBinary: 1;          // binary mode, no EOF check
  // 1 DWORD fParity: 1;          // enable parity checking
  // 2 DWORD fOutxCtsFlow:1;      // CTS output flow control
  // 3 DWORD fOutxDsrFlow:1;      // DSR output flow control
  // 4 DWORD fDtrControl:2;       // DTR flow control type
  // 6 DWORD fDsrSensitivity:1;   // DSR sensitivity
  // 7 DWORD fTXContinueOnXoff:1; // XOFF continues Tx

  // 8 DWORD fOutX: 1;        // XON/XOFF out flow control
  // 9 DWORD fInX: 1;         // XON/XOFF in flow control
  // 10 DWORD fErrorChar: 1;   // enable error replacement
  // 11 DWORD fNull: 1;        // enable null stripping
  // 12 DWORD fRtsControl:2;   // RTS flow control
  // 14 DWORD fAbortOnError:1; // abort reads/writes on error
  // 31 DWORD fDummy2:17;      // reserved
  if (COM_INITIALISIERT())
  {
#if defined(__WINDOWS__)
    // RTS und DTR werden auf -12V zurückgestellt: Das bedeutet
    // für das serielle Interface, dass die Schnittstelle inaktiv
    // ist.
    FDCB.fDtrControl = DTR_CONTROL_DISABLE;
    FDCB.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(FCOM_Handle, &FDCB))
      ShowLastErr(GetLastError());
    if (!CloseHandle(FCOM_Handle))
      ShowLastErr(GetLastError());
#endif
#if defined(__LINUX__)
    close(FCOM_Handle);
#endif
    FCOM_Handle = COM_HANDLE_CLOSE;
    PutMonitorBuffer(0xD0000000 | FBaudrate);
  }
}

bool KComm::SetComParameters()
{
// Setzt die richtigen Parameter der seriellen Schnittstelle.
  bool result = true;
  if (COM_INITIALISIERT())
  {
#if defined(__WINDOWS__)
    // Id;
    FDCB.BaudRate = FBaudrate;
    FDCB.ByteSize = FByteSize;
    FDCB.Parity = FParity;
    FDCB.StopBits = FStopBits;

    FDCB.fBinary = true;
    FDCB.fParity = (FParity != NOPARITY);
    FDCB.fOutxCtsFlow = 0;
    FDCB.fOutxDsrFlow = 0;

    FDCB.fDtrControl = FDTR;
    if (FRtsFlowControl)
    {
      FDCB.fOutxCtsFlow = true;
      FDCB.fRtsControl = RTS_CONTROL_HANDSHAKE;
    } else
    if (FRTS)
      FDCB.fRtsControl = RTS_CONTROL_ENABLE;
    else
      FDCB.fRtsControl = RTS_CONTROL_DISABLE;

    FDCB.XonChar = 0;
    FDCB.XoffChar =  0;
    FDCB.XonLim = RXQUEUE / 2;
    FDCB.XoffLim = RXQUEUE / 2;
    FDCB.EofChar = 0;
    FDCB.EvtChar = 0;
    // TxDelay;
    // Parameter setzen
    result = SetCommState(FCOM_Handle, &FDCB);
    if (!result)
      ShowLastErr(GetLastError());
#endif
#if defined(__LINUX__)

    termios newtio;
    int ErrorNr;
    int Speed;

    memset(&newtio, 0, sizeof(newtio));
    if (tcgetattr(FCOM_Handle, &newtio) < 0)
      result = false;

    newtio.c_cflag = newtio.c_cflag &
                     ~(CS5 | CS6 | CS7 | CS8 | PARODD | PARENB | INPCK |
                       HUPCL /*| CIBAUD */ | CRTSCTS | CSTOPB | CREAD | CLOCAL);
    newtio.c_iflag = 0;

    Speed = 0;
    if (FBaudrate <= 300)
    {
      Speed = B300;
      FBaudrate = 300;
    } else
    if (FBaudrate <= 600)
    {
      Speed = B600;
      FBaudrate = 600;
    } else
    if (FBaudrate <= 1200)
    {
      Speed = B1200;
      FBaudrate = 1200;
    } else
    if (FBaudrate <= 2400)
    {
      Speed = B2400;
      FBaudrate = 2400;
    } else
    if (FBaudrate <= 4800)
    {
      Speed = B4800;
      FBaudrate = 4800;
    } else
    if (FBaudrate <= 9600)
    {
      Speed = B9600;
      FBaudrate = 9600;
    } else
    if (FBaudrate <= 19200)
    {
      Speed = B19200;
      FBaudrate = 19200;
    } else
    if (FBaudrate <= 38400)
    {
      Speed = B38400;
      FBaudrate = 38400;
    } else
    if (FBaudrate <= 57600)
    {
      Speed = B57600;
      FBaudrate = 57600;
    } else
    if (FBaudrate <= 115200)
    {
      Speed = B115200;
      FBaudrate = 115200;
    } else
      result = false;
    if (result)
      cfsetspeed(&newtio, Speed);

    switch (FByteSize)
    {
      case 5:
        newtio.c_cflag |= CS5;  // 5 bits
        break;

      case 6:
        newtio.c_cflag |= CS6;  // 6 bits
        break;

      case 7:
        newtio.c_cflag |= CS7;  // 7 bits
        break;

      case 8:
        newtio.c_cflag |= CS8;  // 8 bits
        break;

      default:
        result = false;
    }

    switch (FParity)
    {
      case 0:
        break; // none

      case 1:
        newtio.c_cflag |= PARODD | PARENB; // odd
        newtio.c_iflag |= INPCK;  // input parity checking
        break;

      case 2:
        newtio.c_cflag |= PARENB; // even
        newtio.c_iflag |= INPCK;
        break;

      default:
        result = false;
    }

    // Setting Raw Input and Defaults
    //newtio.c_cflag := newtio.c_cflag or CSTOPB;
    newtio.c_cflag |= CREAD | CLOCAL;
    newtio.c_iflag |= BRKINT;
    //newtio.c_lflag:= newtio.c_lflag and not (ICANON or ECHO or ISTRIP);
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;

    newtio.c_cc[VMIN] = 1;
    newtio.c_cc[VTIME] = 0;

    if (result)
    {
      ErrorNr = tcflush(FCOM_Handle, TCIOFLUSH); // Flush the serial port
      result = ErrorNr == 0;
    }
    if (result)
    {
      ErrorNr = tcsetattr(FCOM_Handle, TCSANOW, &newtio); // Set the parameters
      result = ErrorNr == 0;
    }
#endif
  }
  return result;
}

void KComm::SetDTR(bool NewDTR)
{
  FDTR = NewDTR;
  SetComParameters();
  if (COM_INITIALISIERT())
    PutMonitorBuffer(0xA0000000 | int(FDTR) | 2);
}

void KComm::SetRTS(bool NewRTS)
{
  FRTS = NewRTS;
  SetComParameters();
  if (COM_INITIALISIERT())
    PutMonitorBuffer(0xA0000000 | int(FRTS));
}

void KComm::SetRtsFlowControl(bool NewRTS)
{
  FRtsFlowControl = NewRTS;
  SetComParameters();
}

void KComm::SetBits(int NewBits)
{
  FByteSize = NewBits;
  SetComParameters();
  if (COM_INITIALISIERT())
    PutMonitorBuffer(0xA0000000 | 0x30 | (FByteSize & 0xf));
}

void KComm::SetParity(int NewParity)
{
  FParity = NewParity;
  SetComParameters();
  if (COM_INITIALISIERT())
    PutMonitorBuffer(0xA0000000 | 0x20 | (FParity & 0xf));
}

void KComm::SetStopBits(int NewStopBits)
{
  FStopBits = NewStopBits;
  SetComParameters();
  if (COM_INITIALISIERT())
    PutMonitorBuffer(0xA0000000 | 0x10 | (FStopBits & 0xf));
}

int KComm::SendBreak(int Duration)
{
  if (!COM_INITIALISIERT())
    return 0;

#if defined(__WINDOWS__)
  SetCommBreak(FCOM_Handle);
  if (Duration > 0)
  {
    NUtils::SleepMs(Duration);
  } else
    Duration = 0;
  ClearCommBreak(FCOM_Handle);
  PutMonitorBuffer(0xA0000000 | EV_BREAK);
  return Duration;
#else
  return tcsendbreak(FCOM_Handle, Duration);
#endif
}


#if defined(__WINDOWS__)
bool KComm::InitCOM()
{
// Eröffnet die serielle Schnittstelle("FCOM_Handle" >= 0) und
// setzt die Parameter(Baudrate, RTS und DTR).
  COMMTIMEOUTS CommTimeOuts;
  char Nr[16];
  bool result = true;

  FReadBufferLen = 0;
  FReadBufferPtr = 0;
  if (COM_INITIALISIERT())
    ExitCOM();

  if (!COM_INITIALISIERT() && (FComPortNr != 0 || dev[0]))
  {
    if (dev[0] && strlen(dev) < 8)
      strcpy(Nr, dev);
    else
      sprintf(Nr, "COM%d", FComPortNr);
    FCOM_Handle =
      CreateFileA(Nr, GENERIC_READ | GENERIC_WRITE,
                 0,                    // exclusive access
                 NULL,                  // no security attrs
                 OPEN_EXISTING,
                 FILE_ATTRIBUTE_NORMAL |
                 FILE_FLAG_OVERLAPPED, // overlapped I/O
                 0);
    if (COM_INITIALISIERT())
    {
      // get any early notifications
      SetEventMask(FEventMask);

      // setup device buffers
      SetupComm(FCOM_Handle, 4096, 4096) ;

      // set up for overlapped non-blocking I/O
      CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF ;
      CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.ReadTotalTimeoutConstant = 0 ;
      CommTimeOuts.WriteTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.WriteTotalTimeoutConstant = 5000 ;
      SetCommTimeouts(FCOM_Handle, &CommTimeOuts);
    } else {
      ShowLastErr(GetLastError());
    }
    if (!COM_INITIALISIERT())
      result = false;
  }
  if (COM_INITIALISIERT())
  {
    memset(&FDCB, 0, sizeof(FDCB));
    FDCB.DCBlength = sizeof(FDCB);
    if (!GetCommState(FCOM_Handle, &FDCB))
    {
      // war nicht erfolgreich: alles zur¸ck
      ShowLastErr(GetLastError());
      ExitCOM();
    } else
    if (!SetComParameters())
    {
      // war nicht erfolgreich: alles zur¸ck
      ExitCOM();
    } else {
      PutMonitorBuffer(0xC0000000 | FBaudrate);
    }
  }
  return result;
}
#endif

#if defined(__LINUX__)
bool KComm::InitCOM()
{
// Eröffnet die serielle Schnittstelle("FCOM_Handle" >= 0) und
// setzt die Parameter(Baudrate, RTS und DTR).
  fd_set rfds;
  timeval tv;
  char Nr[128];
  bool result = true;

  FReadBufferLen = 0;
  FReadBufferPtr = 0;
  if (COM_INITIALISIERT())
    ExitCOM();
  if (!COM_INITIALISIERT() &&
      (FComPortNr != 0 || strlen(dev) > 0))
  {
    if (strlen(dev))
    {
      Nr[0] = 0;
      if (strncmp(dev, "/dev/", 5))
        strcpy(Nr, "/dev/");
      strcat(Nr, dev);
    } else
      sprintf(Nr, "/dev/ttyS%d", FComPortNr-1); // vor Version 2.2: /dev/cuaX
    FCOM_Handle = open(Nr, O_RDWR | O_NOCTTY | O_NONBLOCK);  // Try user input depending on port
    if (FCOM_Handle <= 0)
      FCOM_Handle = COM_HANDLE_CLOSE;
    // printf("dev: %s  opened: %d\n", Nr, (int)COM_INITIALISIERT());
    if (COM_INITIALISIERT())
    {
      if (!SetComParameters())
        // war nicht erfolgreich: alles zurück
        ExitCOM();
      else {
        // WORKAROUND TO STRANGE READER BEHAVIOUR ON FIRST READING OPERATION
        tv.tv_sec = 0;
        tv.tv_usec = 20000;

        FD_ZERO(&rfds);
        FD_SET(FCOM_Handle, &rfds);

        select(FCOM_Handle+1, &rfds, NULL, NULL, &tv);
        tcflush(FCOM_Handle, TCIFLUSH);
        PutMonitorBuffer(0xC0000000 | FBaudrate);
      }
    } else
      result = false;
  }
  return result;
}
#endif

void KComm::SetComPortNr(int Nr)
{
  ExitCOM();
  if (Nr >= 0 && Nr <= 15)
    FComPortNr = Nr;
}

void KComm::SetBaudrate(int Baud)
{
  const bool NoClose = false;

  if (NoClose && COM_INITIALISIERT())
  {
    FBaudrate = Baud;
    PutMonitorBuffer(0xE0000000 | Baud);
    SetComParameters();
  } else {
    ExitCOM();
    FBaudrate = Baud;
  }
}

void KComm::SetComDev(const char * tty_dev)
{
  if (!this)
    return;

  if (!tty_dev)
    dev[0] = 0;
  else
  if (strlen(tty_dev) < High(dev))
    strcpy(dev, tty_dev);
}

#if defined(__WINDOWS__)
int KComm::ReadFromCOM()
{
// Ein Zeichen aus dem Puffer auslesen. Ist der Puffer leer, so wird
// NULL_CHAR (0x1000) zurückgegeben.
// Die von der Prozedur "ReadComm" gelesenen Zeichen werden im Puffer
// "ReadBuffer" zwischengespeichert. Das erlaubt es, dass ein
// einzelnes gelesenes Zeichen wieder in den Puffer zurückge-
// speichert werden kann.
  bool fReadStat;
  COMSTAT ComStat;
  unsigned Err;
  long unsigned dwErrorFlags;
  long unsigned dwLength;

  // Wenn der ReadBuffer leer ist: ReadBuffer wieder füllen.
  if (COM_INITIALISIERT() && FReadBufferLen <= FReadBufferPtr)
  {
    // 17. 2. 98
    dwLength = 0;
    if (ClearCommError(FCOM_Handle, &dwErrorFlags, &ComStat))
    {
      if (dwErrorFlags != 0)
      {
        PutMonitorBuffer(dwErrorFlags | 0x80000000);
        ErrorOccured = true;
        return dwErrorFlags | NULL_CHAR;
      }
      dwLength = ComStat.cbInQue;
    } else
      ShowLastErr(GetLastError());
    if (dwLength >= MaxReadBuffer)
      dwLength = MaxReadBuffer - 1;
    if (dwLength > 0)
    {
      fReadStat = ReadFile(FCOM_Handle, FReadBuffer, MaxReadBuffer,
                           &dwLength, &osRead);
      if (!fReadStat)
      {
        Err = GetLastError();
        if (Err == ERROR_IO_PENDING)
        {
          // wait for a second for this transmission to complete
          if (WaitForSingleObject(osRead.hEvent, 1000) != 0)
          {
            ShowLastErr(GetLastError());
            dwLength = 0;
          } else {
            GetOverlappedResult(FCOM_Handle, &osRead, &dwLength, false);
            osRead.Offset += dwLength;
          }
        } else
          // some other error occurred
          ShowLastErr(Err);
        dwLength = 0;
      }
    }
    FReadBufferLen = dwLength;
    FReadBufferPtr = 0;

    for (int i = 0; i < FReadBufferLen; i++)
      PutMonitorBuffer(FReadBuffer[i] | 0x20000000);
  }
  if (FReadBufferLen > FReadBufferPtr)
    return FReadBuffer[FReadBufferPtr++];

  return NULL_CHAR;
}
#endif

#if defined(__LINUX__)
int KComm::ReadFromCOM()
{
  fd_set rfds;
  fd_set efds;
  timeval tv;
  unsigned char c[4];

  int result = NULL_CHAR;
  if (COM_INITIALISIERT())
  {
    tv.tv_sec = 0; // sofort antworten
    tv.tv_usec = 0;
    // vgl. Kylix in Team Seite 488
    FD_ZERO(&rfds);             // Löscht das gesamte Set von Deskriptoren.
    FD_SET(FCOM_Handle, &rfds); // Setzt "FCOM_Handle" in "rdfs" ein.
    FD_ZERO(&efds);             // Dasselbe für die Exceptions vorbereiten.
    FD_SET(FCOM_Handle, &efds);
    int rval = select(FCOM_Handle+1, &rfds, NULL, &efds, &tv);
    // Error: rval = -1 und errno ist gesetzt.
    if (rval < 0 || FD_ISSET(FCOM_Handle, &efds))
    {
//      if (rval < 0)
// !!!!!        ShowLastErr(errno);
      tcflush(FCOM_Handle, TCIFLUSH);
    } else  // 0 = time out
    if (FD_ISSET(FCOM_Handle, &rfds))
    {
      c[0] = 0;
      if (read(FCOM_Handle, c, 1) == 1)
      {
        PutMonitorBuffer(c[0] | 0x20000000);
        result = c[0];
      }
    }
  }
  return result;
}
#endif

void KComm::ClearBuffer()
{
  while (ReadFromCOM() != NULL_CHAR)
    ;
}

#if defined(__LINUX__)
bool KComm::WriteToCOM(char ch)
{
  bool Ok = COM_INITIALISIERT();
  if (Ok)
  {
    Ok = ::write(FCOM_Handle, &ch, 1) == 1;
    PutMonitorBuffer((unsigned char)(ch) | 0x48000000);
  }
  return Ok;
}
#endif

bool KComm::WriteBufferToCOM(const char * str, int Len)
{
  if (Len <= 0 || str == NULL)
    return false;

#if defined(__WINDOWS__)
  bool Ok = false;
  if (COM_INITIALISIERT())
  {
    int Err;
    bool fWriteStat;
    unsigned long dwBytesWritten;
    if (FRtsFlowControl)
    {
      int i = 0;
      do
      {
        Ok = false;
        PutMonitorBuffer(str[i] | 0x48000000);

        fWriteStat = WriteFile(FCOM_Handle, str + i, 1, &dwBytesWritten, &osWrite);
        if (!fWriteStat)
        {
          Err = GetLastError();
          if (Err == ERROR_IO_PENDING)
          {
            // wait for a second for this transmission to complete
            if (WaitForSingleObject(osWrite.hEvent, 1000) != 0)
            {
              ShowLastErr(GetLastError());
              dwBytesWritten = 0;
            } else {
              GetOverlappedResult(FCOM_Handle, &osWrite, &dwBytesWritten, false);
              osWrite.Offset += dwBytesWritten;
            }
          } else
            ShowLastErr(Err);
          Ok = dwBytesWritten == 1;
        }
        i++;
      } while (i < Len && Ok);
    } else {
      for (int i = 0; i < Len; i++)
        PutMonitorBuffer(str[i] | 0x48000000);

      fWriteStat = WriteFile(FCOM_Handle, str, Len, &dwBytesWritten, &osWrite);
      if (!fWriteStat)
      {
        Err = GetLastError();
        if (Err == ERROR_IO_PENDING)
        {
          // wait for a second for this transmission to complete
          if (WaitForSingleObject(osWrite.hEvent, 1000) != 0)
          {
            ShowLastErr(GetLastError());
            dwBytesWritten = 0;
          } else {
            GetOverlappedResult(FCOM_Handle, &osWrite, &dwBytesWritten, false);
            osWrite.Offset += dwBytesWritten;
          }
        } else
          ShowLastErr(Err);
      }
      Ok = (int) dwBytesWritten == Len;
    }
  }
  return Ok;
#endif

#if defined(__LINUX__)
  bool Ok = true;
  for (int i = 0; Ok && i < Len; i++)
    Ok = WriteToCOM(str[i]);

  return Ok;
#endif
}

#if defined(__WINDOWS__)
bool KComm::WriteToCOM(char ch)
{
  return WriteBufferToCOM(&ch, 1);
}

unsigned long KComm::GetCommModemStatus()
{
  unsigned long result = 0;
  if (COM_INITIALISIERT())
  {
    unsigned long res;
    if (!::GetCommModemStatus(FCOM_Handle, &res))
      ShowLastErr(GetLastError());
    else
      result = res;
  }
  return result;
}

bool KComm::CTS()  // (clear-to-send) signal is on
// Pin 5 (25) / 8 (9)
{
  return GetCommModemStatus() & MS_CTS_ON;
}

bool KComm::DSR()  // (data-set-ready) signal is on
// Pin 6
{
  return GetCommModemStatus() & MS_DSR_ON;
}

bool KComm::RING() // ring indicator signal is on
// Pin 22 (25)
{
  return GetCommModemStatus() & MS_RING_ON;
}

bool KComm::DCD() // receive-line-signal-detect
// Pin 8 (25) / 1 (9)
{
  return GetCommModemStatus() & MS_RLSD_ON;
}

unsigned KComm::WaitCommEvent()
// Gibt das Ereignis seit der letzten Abfrage zur¸ck.
{
  if (::WaitCommEvent(FCOM_Handle, &FEvMask, &osPostEvent))
    return FEvMask;

  unsigned Err = GetLastError();
  if (Err != ERROR_IO_PENDING)
      ShowLastErr(Err);

  return 0;
}

void KComm::SetEventMask(TComEvent EventMask)
{
  FEventMask = EventMask;
  FEvMask = 0;

  if (COM_INITIALISIERT())
  {
    // get any early notifications
    unsigned Events = 0; //EV_RXCHAR;
    if (T_EV_BREAK & FEventMask) Events |= EV_BREAK;
    if (T_EV_CTS & FEventMask) Events |= EV_CTS;
    if (T_EV_DSR & FEventMask) Events |= EV_DSR;
    if (T_EV_ERR & FEventMask) Events |= EV_ERR;
    if (T_EV_RING & FEventMask) Events |= EV_RING;
    if (T_EV_RLSD & FEventMask) Events |= EV_RLSD;
    if (!SetCommMask(FCOM_Handle, Events))
      ShowLastErr(GetLastError());
    else
    if (FEventMask)
      WaitCommEvent();
  }
}

unsigned KComm::Event()
{
  unsigned result = FEvMask;
  if (result)
    SetEventMask(FEventMask);
  return result;
}

#else

bool KComm::CTS()  // (clear-to-send) signal is on
// Pin 5 (25) / 8 (9)
{
  int s;
  
  ioctl(FCOM_Handle, TIOCMGET, &s);
  return (s & TIOCM_CTS) != 0;
}

bool KComm::DSR()  // (data-set-ready) signal is on
// Pin 6
{
  int s;
  
  ioctl(FCOM_Handle, TIOCMGET, &s);
  return (s & TIOCM_DSR) != 0;
}

bool KComm::RING() // ring indicator signal is on
// Pin 22 (25)
{
  int s;
  
  ioctl(FCOM_Handle, TIOCMGET, &s);
  return (s & TIOCM_RNG) != 0;
}

#endif

