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

// KCommVCL ist nur mit der Borland VCL verfügbar.
//
// In einem Fenster kann die Kommunikation mitverfolgt werden. Das Fenster wird
// mittels Menü im "System Tray" geöffnet.
//
// Das Icon ist leider für die KCommVLC-Klasse noch nicht sichtbar. Für die
// Verbindung zur ComfortSoft wird die Klasse TComm (Delphi-Klasse) verwendet
// und da ist das Icon sichtbar.

#include "UBruecke.h"

#include <stdio.h>
#include <string.h>

#if defined(VCL)

#include "KCommVCL.h"
#include "NUtils.h"

extern "C" void MyCallBack(int x, int y, void * self)
{
  KCommVCL * my = (KCommVCL *) self;
  my->pmLeiste->PopUpMenu->Popup(x, y);
}

KCommVCL::KCommVCL(Classes::TComponent * Owner)
{
  ErrorOccured = false;
  LFMonitor = false;
  HexMonitor = false;
  DezMonitor = false;
  FInDialog = false;

  ConsoleWnd = new TConsoleWnd(Owner);
  // Monitor-Instanz initialisieren
  ConsoleWnd->Caption = "RS232 Monitor";
  ConsoleWnd->Visible = false;
  ConsoleWnd->NewScreenSize(88, 40);

  // Die serielle Schnittstelle ist geöffnet, falls COM_Handle >= 0;
  // Fehler, falls < 0
  osWrite = new _OVERLAPPED;
  osWrite->Internal = 0;
  osWrite->InternalHigh = 0;
  osWrite->Offset = 0;
  osWrite->OffsetHigh = 0;
  osWrite->hEvent = 0;

  osRead = new _OVERLAPPED;
  osRead->Internal = 0;
  osRead->InternalHigh = 0;
  osRead->Offset = 0;
  osRead->OffsetHigh = 0;
  osRead->hEvent = 0;
  osPostEvent = new _OVERLAPPED;
  osPostEvent->Internal = 0;
  osPostEvent->InternalHigh = 0;
  osPostEvent->Offset = 0;
  osPostEvent->OffsetHigh = 0;
  osPostEvent->hEvent = 0;

  memset(FMonitorBuffer, 0, sizeof(FMonitorBuffer));
  FMonitorBufferIn = 0;
  FMonitorBufferOut = 0;

  // create I/O event used for overlapped reads / writes
  osRead->hEvent =
    CreateEvent(NULL,    // no security
                TRUE,    // explicit reset req
                FALSE,   // initial event reset
                NULL);   // no name
  osWrite->hEvent =
    CreateEvent(NULL,    // no security
                TRUE,    // explicit reset req
                FALSE,   // initial event reset
                NULL);   // no name
  osPostEvent->hEvent =
    CreateEvent(NULL,    // no security
                TRUE,    // explicit reset req
                TRUE,    // initial event reset
                NULL);   // no name

  // Funktioniert nur unter VCL: Handle ist sonst nicht verfügbar!
  FNotifyData.cbSize = sizeof(FNotifyData);
  FNotifyData.hWnd = ConsoleWnd->Handle;  // Ist nur unter VCL verfügbar!
  FNotifyData.uID = 0;
  FNotifyData.uFlags = NIF_MESSAGE || NIF_ICON || NIF_TIP;
  FNotifyData.uCallbackMessage = WM_APP + 110;
  FNotifyData.hIcon = ConsoleWnd->Icon->Handle;
    //LoadIcon(0, MAKEINTRESOURCE(IDI_HAND));

  Shell_NotifyIcon(NIM_ADD, &FNotifyData);

  pmLeiste = new TPopUpMenuClass(ConsoleWnd);

  Menus::TMenuItem* MenuItem = pmLeiste->AddItem("&Show monitor", true);
  MenuItem->OnClick = mnShowClick;
#if !defined(__NOCOMMDLG__)
  MenuItem = pmLeiste->AddItem("&Port configuration", true);
  MenuItem->OnClick = mnConfigClick;
#endif

  FInCommMonitor = 1;
  FTimer = new TTimer(ConsoleWnd);
  FTimer->OnTimer = CommMonitor;
  FTimer->Interval = 200; // 200 ms

  ConsoleWnd->FCallingC = this;
  Console::ConsoleCallBack = &MyCallBack;
}

KCommVCL::~KCommVCL()
{
  delete FTimer;

  if (osRead->hEvent)
    CloseHandle(osRead->hEvent);
  if (osWrite->hEvent)
    CloseHandle(osWrite->hEvent);
  if (osPostEvent->hEvent)
    CloseHandle(osPostEvent->hEvent);

  Shell_NotifyIcon(NIM_DELETE, &FNotifyData);

  delete pmLeiste;

  delete osRead;
  delete osWrite;
  delete ConsoleWnd;
}

void KCommVCL::PutMonitorBuffer(int msg)
{
  int OutPtr = (FMonitorBufferIn + 1) % MaxMonitorBuffer;
  // Bei einem Buffer-overrun: Ein Zeichen auslesen
  if (OutPtr == FMonitorBufferOut)
    GetMonitorChar();
  FMonitorBuffer[FMonitorBufferIn] = msg;
  FMonitorBufferIn = OutPtr;
}

int KCommVCL::GetMonitorChar()
{
  int result = 0;
  if (FMonitorBufferOut != FMonitorBufferIn)
  {
    result = FMonitorBuffer[FMonitorBufferOut];
    FMonitorBufferOut = (FMonitorBufferOut + 1) % MaxMonitorBuffer;
  }
  return result;
}

void KCommVCL::NewScreenSize(int x, int y)
{
  ConsoleWnd->NewScreenSize(x, y);
}

void KCommVCL::SetTimerInterval(int time)
{
}

void KCommVCL::Show()
{
  ConsoleWnd->Show();
}

void KCommVCL::write(const char * str)
{
  while (*str)
    ConsoleWnd->WriteChar(*str++);
}

void KCommVCL::writeln()
{
  write("\r\n");
}

void KCommVCL::WriteChar(char c)
{
  ConsoleWnd->WriteChar(c);
}

void __fastcall KCommVCL::CommMonitor(System::TObject* Sender)
{
  FInCommMonitor--;
  if (FInCommMonitor >= 0)
  {
    char str[128];
    int i = GetMonitorChar();
    while (i)
    {
      unsigned char ch = char(i);
      if ((i & 0xE0000000) == 0xE0000000)
      {
        sprintf(str, " <baud %d>", i & 0xffff);
        write(str);
        writeln();
      } else
      if ((i & 0xD0000000) == 0xD0000000)
      {
          sprintf(str, " <close %d>", i & 0xffff);
          write(str);
          writeln();
      } else
      if ((i & 0xC0000000) == 0xC0000000)
      {
          i &= 0xffff;
          sprintf(str, " <open %d / COM%d>", i, FComPortNr);
          write(str);
          if (i)
            writeln();
      } else
      if ((i & 0xA0000000) == 0xA0000000)
      {
        i &= 0xffff;
        switch (i)
        {
          case 0:
            write(" <rts false>");
            break;

          case 1:
            write(" <rts true>");
            break;

          case 2:
            write(" <dtr false>");
            break;

          case 3:
            write(" <dtr true>");
            break;

          default:
            if (i <= 0x01 && i <= 0x3f)
            {
              write(" <");
              if (i < 0x20)
                write("stopbits");
              else
              if (i < 0x30)
                write("parity");
              else
                write("bits");
              sprintf(str, "%d>", i & 0xf);
            } else
            if (i >= 0x10000 && i < 0x20000)
            {
              sprintf(str, " <delay %d>", i & 0xffff);
            }
            write(str);
            break;
        }
      } else
      if (i & 0x80000000)   // Fehler
      {
        i &= 0xffff;
        sprintf(str, "<err 0x%x:", i);
        write(str);
#if defined(MSWINDOWS)  && 0
        if (i and CE_RXOVER) <> 0 then
          write(' RXOVER');            // Receive Queue overflow
        if (i and CE_OVERRUN) <> 0 then
          write(' OVERRUN');           // Receive Overrun Error
        if (i and CE_RXPARITY) <> 0 then
          write(' RXPARITY');          // Receive Parity Error
        if (i and CE_FRAME) <> 0 then
          write(' FRAME');             // Receive Framing error
        if (i and CE_BREAK) <> 0 then
          write(' BREAK');             // Break Detected
        if (i and CE_TXFULL) <> 0 then
          write(' TXFULL');            // TX Queue is full
#endif
        write(">");
        writeln();
      } else {
        if (i & 0x40000000)
          WriteChar(' ');
        else
          WriteChar('.');
        if ((ch == 13 || ch == 10) && LFMonitor)
          WriteChar((char) ch);
        else
        if (HexMonitor)
        {
          sprintf(str, "%2.2x", (int) ch);
          write(str);
        } else
        if (ch <= ' ' || ch >= 127 || DezMonitor)
        {
          sprintf(str, "<%d>", (int) ch);
          write(str);
        } else
          WriteChar(ch);
      }
      i = GetMonitorChar();
    }
  }
  FInCommMonitor++;
}

void __fastcall KCommVCL::mnShowClick(System::TObject* Sender)
{
  ConsoleWnd->Visible = true;
  ConsoleWnd->WindowState = wsMaximized;
  ConsoleWnd->SetFocus();
}

void __fastcall KCommVCL::mnConfigClick(System::TObject* Sender)
{
  int Nr = FComPortNr;
  if (ParameterDialog())
  {
    if (FComPortNr == 0)
      ExitCOM();
    else
    if (Nr != FComPortNr)
      InitCOM();
    else
      SetComParameters();
  }
}

bool KCommVCL::ParameterDialog()
{
#if !defined(__NOCOMMDLG__)
  TdlgCOMM * dlgCOMM;
  TInt64 i;
  char str[32];
  bool result = false;

  if (FInDialog)
    return false;

  FInDialog = true;
  dlgCOMM = new TdlgCOMM((TComponent *) NULL);
  if (FComPortNr > 0)
    dlgCOMM->cbPort->ItemIndex = FComPortNr;
  else
    dlgCOMM->cbPort->ItemIndex = 1;      // default: COM1
  dlgCOMM->cbDTR->ItemIndex = (int) FDTR;
  dlgCOMM->cbRTS->ItemIndex = (int) FRTS;
  sprintf(str, "%d", FBaudrate);
  dlgCOMM->cbBaudrate->Text = str;
  dlgCOMM->cbParity->ItemIndex = FParity;
  dlgCOMM->cbStopBits->ItemIndex = FStopBits;
  dlgCOMM->cbBits->ItemIndex = FByteSize - 4;
  result = dlgCOMM->ShowModal() == mrOk;
  if (result)
  {
    FComPortNr = dlgCOMM->cbPort->ItemIndex;
    FDTR = dlgCOMM->cbDTR->ItemIndex == 1;
    FRTS = dlgCOMM->cbRTS->ItemIndex == 1;
    strcpy(str, string(dlgCOMM->cbBaudrate->Text.c_str()));
    char * p = str;
    if (NUtils::GetInt(p, i))
      FBaudrate = (int) i;
    FParity = dlgCOMM->cbParity->ItemIndex;
    FStopBits = dlgCOMM->cbStopBits->ItemIndex;
    FByteSize = dlgCOMM->cbBits->ItemIndex + 4;
  }
  delete dlgCOMM;

  FInDialog = false;

  return result;
#else
  return false;
#endif
}

#endif

