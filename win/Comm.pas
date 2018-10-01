unit Comm;

interface

{$R+,S+}

{$ifdef LINUX}
  {$undef VCL}
{$endif}

uses
{$ifndef VCL}
  {$ifdef MSWINDOWS}
  Windows,
  {$endif}
  {$ifdef LINUX}
  KernelIoctl, Libc,
  {$endif}
  Types, Classes, SysUtils, DateUtils,
  Qt, QGraphics, QControls, QForms, QDialogs, QStdCtrls, QExtCtrls, QMenus, QTypes,
{$else}
  Windows, ShellApi, Messages, SysUtils,
  Classes, Graphics, Controls, Menus, Forms, Dialogs, ExtCtrls,
{$endif}
{$ifndef __NOCOMMDLG__}
  Comm_dlg,
{$endif}
  Console;

const
{$ifdef LINUX}
      COM_HANDLE_CLOSE = -1;
{$endif}
{$ifdef MSWINDOWS}
      COM_HANDLE_CLOSE = INVALID_HANDLE_VALUE;
{$endif}
      RXQUEUE = 512;
      TXQUEUE = RXQUEUE;
      DefaultBaudRate  = 4800;
      _MaxMonitorBuffer = $400;  // length of monitor buffer
      _MaxReadBuffer    = 100;   // length of type ahead buffer
      _NULL_CHAR        = $1000; // no character in buffer

type
  MyBOOL = WordBool;            // as used in Excel
  TEvent = (T_EV_BREAK, T_EV_CTS, T_EV_DSR, T_EV_ERR, T_EV_RING, T_EV_RLSD);
  SEvent = set of TEvent;

//  PComm = ^TComm;
  TComm = class
    procedure FormKeyPress(Sender: TObject; var Key: AnsiChar);
    procedure mnConfigClick(Sender: TObject);
    procedure mnShowClick(Sender: TObject);
  private
    FComPortNr : integer;
    FBaudrate : integer;
    // Codierung der Parität:   0: None
    //                          1: Odd
    //                          2: Even
    //                          3: Mark
    //                          4: Space
    FParity : integer;
    FByteSize : integer;
    // Codierung der Stopp-Bits: 0:   1 Bit
    //                           1: 1.5 Bits
    //                           2:   2 Bits
    FStopBits : integer;
    FDTR, FRTS : boolean;
    FRtsFlowControl : boolean;
{$ifdef MSWINDOWS}
    FEventMask : SEvent;
    FEvMask : DWORD;
{$endif}
    FInDialog : boolean;
    FInCommMonitor : integer;
    FLastStamp : TDateTime;

{$ifdef MSWINDOWS}
    // Die serielle Schnittstelle ist geöffnet, falls COM_Handle >= 0;
    // Fehler, falls < 0
    osWrite : TOVERLAPPED;
    osRead : TOVERLAPPED;
    osPostEvent : TOVERLAPPED;
    FCOM_Handle : THandle;
    FDCB : TDCB;
{$endif}
{$ifdef LINUX}
    FCOM_Handle : integer;
{$endif}
    // MonitorBuffer leer: MonitorBufferIn == MonitorBufferOut
    FMonitorBuffer : array [0.._MaxMonitorBuffer-1] of integer;
    FMonitorBufferIn : integer;
    FMonitorBufferOut : integer;
    // Zum Lesen wird ein Puffer verwendet.
    // Zeichen können an der Stelle "ReadBuffer[ReadBufferPtr]" gelesen
    // werden, falls "ReadBufferPtr < ReadBufferLen".
    FReadBuffer : array [0.._MaxReadBuffer-1] of AnsiChar;
    FReadBufferLen : integer;
    FReadBufferPtr : integer;
    FTimer : TTimer;
{$ifdef VCL}
    FNotifyData : TNotifyIconData;
    FConsoleWnd : TConsoleWnd;
{$endif}
{$ifdef MSWINDOWS}
    procedure SetFlags(Info, Mask : DWORD; Bit : integer);
    procedure SetEventMask(EventMask : SEvent);
{$endif}
  public
    ErrorOccured : boolean;
    DezMonitor : boolean;
    HexMonitor : boolean;
    LFMonitor : boolean;
    pmLeiste : TPopUpMenuClass;
    UseTimeStamp : boolean;

    constructor Create(AOwner: TComponent);
    destructor Destroy; override;
    procedure  MyDestroy;

{$ifdef VCL}
    procedure ChangeCaption(const Titel : string);
{$endif}
    procedure ExitCOM;
    function  COM_INITIALISIERT : boolean;
    function  SetComParameters : MyBOOL;
    procedure SetComPortNr(Nr : integer);
    procedure SetBaudrate(Baud : integer);// NoClose : boolean = false);
    function  InitCOM : MyBOOL;
    function  GetMonitorChar : integer;
    procedure PutMonitorBuffer(ch : integer);
    function  ReadFromCOM : integer;
    procedure PutBackToReadBuffer;
    function  ReadWithTimeOut(TimeMs : integer) : integer;
    procedure ClearBuffer;
    function  WriteToCOM(ch : AnsiChar) : MyBOOL;
    function  WriteStringToCOM(const str : AnsiString) : MyBOOL;
    function  WriteBufferToCOM(str : PAnsiChar; Len : integer) : MyBOOL;
    function  WriteCS(const str : AnsiString) : MyBool;
    function  WriteCS_OK : MyBool;
    function  ParameterDialog : boolean;
    function  GetTimerInterval : integer;
    procedure SetTimerInterval(NewInterval : integer);
    function  SendBreak(Duration : integer) : integer;

{$ifdef MSWINDOWS}
    function  GetCommModemStatus : integer;
    function  WaitCommEvent : DWORD;
    function  Event : DWORD;
    function  CTS : boolean;  // clear-to-send signal is on
    function  DSR : boolean;  // data-set-ready signal is on
    function  RING : boolean; // ring-indicator signal is on
    function  DCD : boolean;  // receive-line-signal-detect
{$endif}
    procedure SetDTR(NewDTR : boolean);
    procedure SetRTS(NewRTS : boolean);
    procedure SetRtsFlowControl(NewRTS : boolean);
    procedure SetBits(NewBits : integer);
    procedure SetStopBits(NewStopBits : integer);
    procedure SetParity(NewParity : integer);

    procedure DelayMs(TimeMs : integer);
    procedure CommMonitor(Sender : TObject);
    procedure TimeStamp;
    property  TimerInterval : integer read GetTimerInterval write SetTimerInterval;

    procedure NewScreenSize(x, y : integer);
    procedure Show;
    procedure WriteChar(ch : char);
    procedure write(const t : string); overload;
    procedure write(const t : PChar); overload;
    procedure writeln; overload;
    procedure writeln(const t : PChar); overload;
    procedure WriteTime(Time : TDateTime);
    procedure WriteNow;
  published
    property Baudrate : integer read FBaudrate write SetBaudrate;
    property Bits : integer read FByteSize write SetBits;
    property ComPortNr : integer read FComPortNr write SetComPortNr;
    property StopBits : integer read FStopBits write SetStopBits;
    property DTR : boolean read FDTR write SetDTR;
    property Parity : integer read FParity write SetParity;
    property RTS : boolean read FRTS write SetRTS;
    property RtsFlowControl : boolean read FRtsFlowControl write SetRtsFlowControl;
{$ifdef MSWINDOWS}
    property EventMask : SEvent read FEventMask write SetEventMask;
{$endif}
  end;

function  MessageLoop(Save : MyBool) : MyBool;
procedure DelayMs(TimeMs : integer);
function  CRC(Buffer : PAnsiChar; Length : integer) : word;

var
  HideError : boolean = false;

implementation

procedure MyCallBack(x, y : integer; this : pointer);
begin
{$ifdef __NOCOMMDLG__}
  TComm(this).mnShowClick(nil);
{$else}
  TComm(this).pmLeiste.PopUpMenu.Popup(x, y);
{$endif}
end;

constructor TComm.Create(AOwner: TComponent);
var i : integer;
{$ifdef VCL}
    Head : string;
{$endif}
begin
  FConsoleWnd:= TConsoleWnd.Create(AOwner);

  with FConsoleWnd do begin
    // Monitor-Instanz initialisieren
    Caption:= 'RS232 Monitor';
    Visible:= false;
    NewScreenSize(88, 40);
    UseHalt:= false;
  end;
  FBaudrate:= DefaultBaudRate;
  FByteSize:= 8;
  FStopBits:= 0; // 1 Stop bit
  FDTR:= false;
  FParity:= 0;
  FRTS:= false;
{$ifdef MSWINDOWS}
  FEventMask:= [];
  FEvMask:= 0;
{$endif}
  FInDialog:= false;

  ErrorOccured:= false;
  DezMonitor:= false;
  HexMonitor:= true;
  LFMonitor:= false;
  UseTimeStamp := false;

  FComPortNr:= 0;

{$ifdef MSWINDOWS}
  // Die serielle Schnittstelle ist geöffnet, falls COM_Handle >= 0;
  // Fehler, falls < 0
  with osWrite do begin
    Internal:= 0;
    InternalHigh:= 0;
    Offset:= 0;
    OffsetHigh:= 0;
    hEvent:= 0;
  end;
  with osRead do begin
    Internal:= 0;
    InternalHigh:= 0;
    Offset:= 0;
    OffsetHigh:= 0;
    hEvent:= 0;
  end;
  osPostEvent:= osRead;
{$endif}

  for i:= 0 to High(FMonitorBuffer) do
    FMonitorBuffer[i]:= 0;
  FMonitorBufferIn:= 0;
  FMonitorBufferOut:= 0;

  for i:= 0 to High(FReadBuffer) do
    FReadBuffer[i]:= #0;
  FReadBufferLen:= 0;
  FReadBufferPtr:= 0;

  FCOM_HANDLE:= COM_HANDLE_CLOSE;

{$ifdef MSWINDOWS}
  // create I/O event used for overlapped reads / writes
  osRead.hEvent:=
    CreateEvent(nil,     // no security
                TRUE,    // explicit reset req
                FALSE,   // initial event reset
                nil);    // no name
  osWrite.hEvent:=
    CreateEvent(nil,     // no security
                TRUE,    // explicit reset req
                FALSE,   // initial event reset
                nil) ;   // no name
  osPostEvent.hEvent:=
    CreateEvent(nil,     // no security
                TRUE,    // explicit reset req
                TRUE,   // initial event reset
                nil) ;   // no name

  {$ifdef VCL}
  // Funktioniert nur unter VCL: Handle ist sonst nicht verfügbar!
  FNotifyData.cbSize:= SizeOf(FNotifyData);
  with FNotifyData do begin
    Wnd:= FConsoleWnd.Handle;  // Ist nur unter VCL verfügbar!
    uID:= 0;
    uFlags:= NIF_MESSAGE or NIF_ICON or NIF_TIP;
    uCallbackMessage:= MonWm_NOTIFY;
    hIcon:= FConsoleWnd.Icon.Handle;
    if hIcon = 0 then
      hIcon:= LoadIcon(0, MAKEINTRESOURCE(IDI_APPLICATION));
    if AOwner is TForm then begin
      with AOwner as TForm do
        Head:= 'RS232-Monitor - ' + Caption + #0;
    end else
      Head:= FConsoleWnd.Caption + #0;
    for i:= 1 to length(Head) do begin
      if i > High(szTip) then
        break;
      szTip[i-1]:= Head[i];
    end;
  end;
  Shell_NotifyIcon(NIM_ADD, @FNotifyData);
  {$endif}
{$endif}
{$ifdef VCL}
  pmLeiste:= TPopUpMenuClass.create(FConsoleWnd);
{$else}
  pmLeiste:= TPopUpMenuClass.create(nil);
{$endif}
  with pmLeiste do begin
    AddItem('&Show monitor', true).OnClick:= mnShowClick;
{$ifndef __NOCOMMDLG__}
    AddItem('&Port configuration', true).OnClick:= mnConfigClick;
{$endif}
  end;

  FInCommMonitor:= 1;
  FTimer:= TTimer.Create(FConsoleWnd);
  FTimer.OnTimer:= CommMonitor;
  FTimer.Interval:= 200; // 200 ms

  FConsoleWnd.FCallingDelphi := self;
  Console.CommCallBack := @MyCallBack;
end;

{$ifdef VCL}
procedure TComm.ChangeCaption(const Titel : string);
var Head : string;
    i : integer;
begin
  FConsoleWnd.Caption:= Titel;
  Head:= Titel + #0;
  with FNotifyData do begin
    for i:= 1 to length(Head) do begin
      if i > High(szTip) then
        break;
      szTip[i-1]:= Head[i];
    end;
  end;
  Shell_NotifyIcon(NIM_MODIFY, @FNotifyData);
end;
{$endif}

procedure TComm.MyDestroy;
begin
  ExitCOM;
  FTimer.Free;
  FTimer:= nil;

{$ifdef MSWINDOWS}
  if (osRead.hEvent <> 0) then
    CloseHandle(osRead.hEvent);
  if (osWrite.hEvent <> 0) then
    CloseHandle(osWrite.hEvent);
  if (osPostEvent.hEvent <> 0) then
    CloseHandle(osPostEvent.hEvent);
{$endif}

{$ifdef VCL}
  Shell_NotifyIcon(NIM_DELETE, @FNotifyData);
{$endif}
  pmLeiste.Free;
  pmLeiste:= nil;

  FConsoleWnd.MyDestroy;
end;

destructor TComm.Destroy;
begin
  MyDestroy;

  FConsoleWnd.Free;
end;

function TComm.COM_INITIALISIERT : boolean;
begin
  result:= (FCOM_Handle <> COM_HANDLE_CLOSE);
end;

function FormatError(Err : DWORD) : string;
{$ifdef MSWINDOWS}
var MsgBuf : array [0..500] of char;
{$endif}
begin
{$ifdef MSWINDOWS}
  if FormatMessage(
       FORMAT_MESSAGE_FROM_SYSTEM,
       nil,
       Err,
       LANG_USER_DEFAULT,
       MsgBuf,
       High(MsgBuf),
       nil) <> 0 then begin
    result := MsgBuf;
  end else
{$endif}
    result := Format('"GetLastError": %d', [Err]);
end;

procedure ShowLastErr(Err : DWORD);
var
  s : string;
begin
  s := FormatError(Err);
  MessageDlg(s, mtError, [mbOk], 0);
end;

function TComm.GetMonitorChar : integer;
begin
  result:= 0;
  if (FMonitorBufferOut <> FMonitorBufferIn) then begin
    result:= FMonitorBuffer[FMonitorBufferOut];
    FMonitorBufferOut:= (FMonitorBufferOut + 1) mod _MaxMonitorBuffer;
  end;
end;

procedure TComm.PutMonitorBuffer(ch : integer);
var OutPtr : integer;
begin
  OutPtr:= (FMonitorBufferIn + 1) mod _MaxMonitorBuffer;
  // Bei einem Buffer-overrun: Ein Zeichen auslesen
  if (OutPtr = FMonitorBufferOut) then
    GetMonitorChar;
  FMonitorBuffer[FMonitorBufferIn]:= ch;
  FMonitorBufferIn:= OutPtr;
end;

{$ifdef MSWINDOWS}
procedure TComm.SetFlags(Info, Mask : DWORD; Bit : integer);
begin
  Info:= (Info and Mask) shl Bit;
  FDCB.Flags:= (FDCB.Flags and not (Mask shl Bit)) or Info;
end;
{$endif}

procedure TComm.ExitCOM;
// Schliesst die serielle Schnittstelle. RTS und DTR werden auf -12V
// zurückgesetzt.
begin
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
  if (COM_INITIALISIERT) then begin
{$ifdef MSWINDOWS}
    // RTS und DTR werden auf -12V zurückgestellt: Das bedeutet
    // für das serielle Interface, dass die Schnittstelle inaktiv
    // ist.
    SetFlags(DTR_CONTROL_DISABLE, 3, 4);  //  FDCB.fDtrControl = DTR_CONTROL_DISABLE;
    SetFlags(RTS_CONTROL_DISABLE, 3, 12); //  FDCB.fRtsControl = RTS_CONTROL_DISABLE;

    if not SetCommState(FCOM_Handle, FDCB) then
      ShowLastErr(GetLastError);
    if not CloseHandle(FCOM_Handle) then
      ShowLastErr(GetLastError);
{$endif}
{$ifdef LINUX}
    Libc.__close(FCOM_Handle);
{$endif}
    FCOM_Handle:= COM_Handle_CLOSE;
    PutMonitorBuffer(integer($D0000000) or FBaudrate);
  end;
end;

{$ifdef LINUX}
function TIOCMODG : cardinal;
begin
  Result:= __IOR(ord('t'), 3, sizeof(integer));
end;
{$endif}

function TComm.SetComParameters : MyBOOL;
// Setzt die richtigen Parameter der seriellen Schnittstelle.
{$ifdef LINUX}
var newtio : termios;
    ErrorNr : integer;
    Speed : integer;
  //  ModemRes : integer;
  //  s : word;
{$endif}
begin
  result:= true;
  if COM_INITIALISIERT then begin
{$ifdef MSWINDOWS}
    // Id;
    FDCB.BaudRate:= FBaudRate;
    FDCB.ByteSize:= FByteSize;
    FDCB.Parity:= FParity;
    FDCB.StopBits:= FStopBits;
    FDCB.Flags:= 0;

    SetFlags(1, 1, 0); //  FDCB.fBinary:= TRUE;
    if (FParity <> NOPARITY) then
      SetFlags(1, 1, 1); //  FDCB.fParity:= (Parity <> NOPARITY);
    //  FDCB.fOutxCtsFlow:= FALSE;
    //  FDCB.fOutxDsrFlow:= FALSE;
    // fDummy;
    if (FDTR) then
      SetFlags(DTR_CONTROL_ENABLE, 3, 4)
    else
      SetFlags(DTR_CONTROL_DISABLE, 3, 4);
    if FRtsFlowControl then begin
      SetFlags(1, 1, 2); //  FDCB.fOutxCtsFlow:= true;
      SetFlags(RTS_CONTROL_HANDSHAKE, 3, 12);
    end else
    if (FRTS) then
      SetFlags(RTS_CONTROL_ENABLE, 3, 12)
    else
      SetFlags(RTS_CONTROL_DISABLE, 3, 12);
    // FDCB.fOutX:= false;
    // FDCB.fInX:= FALSE;
    // FDCB.fNull:= 0;
//    SetFlags(1, 1, 14); //  FDCB.fAbortOnError = 1

    // fDummy2

    FDCB.XonChar:= #0;
    FDCB.XoffChar:= #0;
    FDCB.XonLim:= RXQUEUE div 2;
    FDCB.XoffLim:= RXQUEUE div 2;
    FDCB.EofChar:= #0;
    FDCB.EvtChar:= #0;
    // TxDelay;
    // Parameter setzen
    result:= SetCommState(FCOM_Handle, FDCB);
    if not result then
      ShowLastErr(GetLastError);
{$endif}
{$ifdef LINUX}
    FillChar(newtio, sizeof(newtio), 0);
    if tcgetattr(FCOM_Handle, newtio) < 0 then
      result:= false;

    newtio.c_cflag:= newtio.c_cflag and
      not (CS5 or CS6 or CS7 or CS8 or PARODD or PARENB or INPCK or
           HUPCL or CIBAUD or CRTSCTS or
           CSTOPB or CREAD or CLOCAL);
    newtio.c_iflag:= 0;

    Speed:= 0;
    if FBaudrate <= 300 then begin
      Speed:= B300;
      FBaudrate:= 300;
    end else
    if FBaudrate <= 600 then begin
      Speed:= B600;
      FBaudrate:= 600;
    end else
    if FBaudrate <= 1200 then begin
      Speed:= B1200;
      FBaudrate:= 1200;
    end else
    if FBaudrate <= 2400 then begin
      Speed:= B2400;
      FBaudrate:= 2400;
    end else
    if FBaudrate <= 4800 then begin
      Speed:= B4800;
      FBaudrate:= 4800;
    end else
    if FBaudrate <= 9600 then begin
      Speed:= B9600;
      FBaudrate:= 9600;
    end else
    if FBaudrate <= 19200 then begin
      Speed:= B19200;
      FBaudrate:= 19200;
    end else
    if FBaudrate <= 38400 then begin
      Speed:= B38400;
      FBaudrate:= 38400;
    end else
    if FBaudrate <= 57600 then begin
      Speed:= B57600;
      FBaudrate:= 57600;
    end else
    if FBaudrate <= 115200 then begin
      Speed:= B115200;
      FBaudrate:= 115200;
    end else
      result:= false;
    if result then
      cfsetspeed(newtio, Speed);

    case FByteSize of
      5: newtio.c_cflag:= newtio.c_cflag or CS5;  // 5 bits
      6: newtio.c_cflag:= newtio.c_cflag or CS6;  // 6 bits
      7: newtio.c_cflag:= newtio.c_cflag or CS7;  // 7 bits
      8: newtio.c_cflag:= newtio.c_cflag or CS8;  // 8 bits
      else result:= false;
    end;

    case FParity of
      0: begin end; // none
      1: begin
           newtio.c_cflag:= newtio.c_cflag or PARODD or PARENB; // odd
           newtio.c_iflag:= newtio.c_iflag or INPCK;  // input parity checking
         end;
      2: begin
           newtio.c_cflag:= newtio.c_cflag or PARENB; // even
           newtio.c_iflag:= newtio.c_iflag or INPCK;
         end;
      else result:= false;
    end;

    // Setting Raw Input and Defaults
    //newtio.c_cflag := newtio.c_cflag or CSTOPB;
    newtio.c_cflag:= newtio.c_cflag or CREAD or CLOCAL;
    newtio.c_iflag:= newtio.c_iflag or BRKINT;
    //newtio.c_lflag:= newtio.c_lflag and not (ICANON or ECHO or ISTRIP);
    newtio.c_oflag:= 0;
    newtio.c_lflag:= 0;

    newtio.c_cc[VMIN]:= #1;
    newtio.c_cc[VTIME]:= #0;

    if result then begin
      ErrorNr:= tcflush(FCOM_Handle, TCIOFLUSH); // Flush the serial port
      result:= ErrorNr = 0;
    end;
    if result then begin
      ErrorNr:= tcsetattr(FCOM_Handle, TCSANOW, newtio); // Set the parameters
      result:= ErrorNr = 0;
    end;
    if Result then begin
    {  s:= nrlinux.CLRDTR;
      if FDtr then
        s:= nrlinux.SETDTR;
      EscapeCommFunction(FCOM_Handle, s);
      s:= nrlinux.CLRRTS;
      if FRts then
        s:= nrlinux.SETRTS;
      EscapeCommFunction(FCOM_Handle, s); }
{      ModemRes:= TIOCM_RTS;
      ErrorNr:= ioctl(FCOM_Handle, _IO(ord('t'), 121));
      // TIOCMODG, @ModemRes);
      if ErrorNr <> 0 then
        ErrorNr:= Libc.errno;}
    end;
{$endif}
  end;
end;

procedure TComm.SetDTR(NewDTR : boolean);
begin
  FDTR:= NewDTR;
  SetComParameters;
  if COM_INITIALISIERT then
    PutMonitorBuffer(integer($A0000000) or ord(FDTR) or 2);
end;

procedure TComm.SetRTS(NewRTS : boolean);
begin
  FRTS:= NewRTS;
  SetComParameters;
  if COM_INITIALISIERT then
    PutMonitorBuffer(integer($A0000000) or ord(FRTS));
end;

procedure TComm.SetRtsFlowControl(NewRTS : boolean);
begin
  FRtsFlowControl:= NewRTS;
  SetComParameters;
end;

procedure TComm.SetBits(NewBits : integer);
begin
  FByteSize:= NewBits;
  SetComParameters;
  if COM_INITIALISIERT then
    PutMonitorBuffer(integer($A0000000) or $30 or (FByteSize and $f));
end;

procedure TComm.SetParity(NewParity : integer);
begin
  FParity:= NewParity;
  SetComParameters;
  if COM_INITIALISIERT then
    PutMonitorBuffer(integer($A0000000) or $20 or (FParity and $f));
end;

procedure TComm.SetStopBits(NewStopBits : integer);
begin
  FStopBits:= NewStopBits;
  SetComParameters;
  if COM_INITIALISIERT then
    PutMonitorBuffer(integer($A0000000) or $10 or (FStopBits and $f));
end;

function TComm.ParameterDialog : boolean;
{$ifndef __NOCOMMDLG__}
var dlgCOMM: TdlgCOMM;
    i, Code : integer;
{$endif}
begin
  result:= false;
{$ifndef __NOCOMMDLG__}
  if FInDialog then exit;

  dlgCOMM:= TdlgCOMM.Create(nil);
  try
    FInDialog:= true;
    if FComPortNr > 0 then
      dlgCOMM.cbPort.ItemIndex:= FComPortNr
    else
      dlgCOMM.cbPort.ItemIndex:= 1;      // default: COM1
    dlgCOMM.cbDTR.ItemIndex:= ord(FDTR);
    dlgCOMM.cbRTS.ItemIndex:= ord(FRTS);
    dlgCOMM.cbBaudrate.Text:= Format('%d', [FBaudrate]);
    dlgCOMM.cbParity.ItemIndex:= FParity;
    dlgCOMM.cbStopBits.ItemIndex:= FStopBits;
    dlgCOMM.cbBits.ItemIndex:= FByteSize - 4;
    result:= dlgCOMM.ShowModal = mrOk;
    if result then begin
      FComPortNr:= dlgCOMM.cbPort.ItemIndex;
      FDTR:= dlgCOMM.cbDTR.ItemIndex = 1;
      FRTS:= dlgCOMM.cbRTS.ItemIndex = 1;
{$R-}
      Val(dlgCOMM.cbBaudrate.Text, i, Code);
      if Code = 0 then FBaudrate:= i;
{$R+}
      FParity:= dlgCOMM.cbParity.ItemIndex;
      FStopBits:= dlgCOMM.cbStopBits.ItemIndex;
      FByteSize:= dlgCOMM.cbBits.ItemIndex + 4;
    end;
  finally
    dlgCOMM.free;
    FInDialog:= false;
  end;
{$endif}
end;

function TComm.InitCOM : MyBOOL;
// Eröffnet die serielle Schnittstelle("FCOM_Handle" >= 0) und
// setzt die Parameter(Baudrate, RTS und DTR).
var
{$ifdef MSWINDOWS}
    CommTimeOuts : TCOMMTIMEOUTS;
{$endif}
{$ifdef LINUX}
    rfds : TFdSet;
    tv : TTimeVal;
{$endif}
    Nr : array [0..15] of char;
begin
  result:= true;
  FReadBufferLen:= 0;
  FReadBufferPtr:= 0;
  if (COM_INITIALISIERT) then ExitCOM;
  if FComPortNr = 0 then
    ParameterDialog;
{$ifdef MSWINDOWS}
  if (not COM_INITIALISIERT) and (FComPortNr <> 0) then begin
    StrFmt(Nr, 'COM%d', [FComPortNr]);
    FCOM_Handle:=
      CreateFile(Nr, GENERIC_READ or GENERIC_WRITE,
                 0,                    // exclusive access
                 nil,                  // no security attrs
                 OPEN_EXISTING,
                 FILE_ATTRIBUTE_NORMAL or
//                 FILE_FLAG_NO_BUFFERING or
                 FILE_FLAG_OVERLAPPED, // overlapped I/O
                 0);
    if (COM_INITIALISIERT) then begin
      // get any early notifications
      SetEventMask(FEventMask);

      // setup device buffers
      SetupComm(FCOM_Handle, 4096, 4096 ) ;

      // set up for overlapped non-blocking I/O
      CommTimeOuts.ReadIntervalTimeout:= $FFFFFFFF ;
      CommTimeOuts.ReadTotalTimeoutMultiplier:= 0 ;
      CommTimeOuts.ReadTotalTimeoutConstant:= 0 ;
      CommTimeOuts.WriteTotalTimeoutMultiplier:= 0 ;
      CommTimeOuts.WriteTotalTimeoutConstant:= 5000 ;
      SetCommTimeouts(FCOM_Handle, CommTimeOuts);
    end else begin
      ShowLastErr(GetLastError);
    end;
    if not COM_INITIALISIERT then
      result:= false;
  end;
  {$endif}
{$ifdef LINUX}
  if (not COM_INITIALISIERT) and (FComPortNr <> 0) then begin
    StrFmt(Nr, '/dev/ttyS%d', [FComPortNr-1]); // vor Version 2.2: /dev/cuaX
    FCOM_Handle:= Libc.open(Nr, O_RDWR or O_NOCTTY or O_NONBLOCK);  // Try user input depending on port
    if FCOM_Handle <= 0 then
      FCOM_Handle:= COM_HANDLE_CLOSE;
  end;
{$endif}
  if (COM_INITIALISIERT) then begin
{$ifdef MSWINDOWS}
    FDCB.DCBlength:= sizeof(FDCB);
    if (not GetCommState(FCOM_Handle, FDCB)) then begin
      // war nicht erfolgreich: alles zurück
      ShowLastErr(GetLastError);
      ExitCOM;
    end else
{$endif}
    if not SetCOMParameters then begin
      // war nicht erfolgreich: alles zurück
      ExitCOM;
    end else begin
{$ifdef LINUX}
      // WORKAROUND TO STRANGE READER BEHAVIOUR ON FIRST READING OPERATION
      tv.tv_sec:= 0;
      tv.tv_usec:= 20000;

      FD_ZERO(rfds);
      FD_SET(FCOM_Handle, rfds);

      select(FCOM_Handle+1, @rfds, nil, nil, @tv);
      tcflush(FCOM_Handle, TCIFLUSH);
{$endif}
      PutMonitorBuffer(integer($C0000000) or BaudRate);
    end;
  end;
end;

procedure TComm.SetComPortNr(Nr : integer);
begin
  ExitCOM;
  if (Nr >= 0) and (Nr <= 8) then begin
    FComPortNr:= Nr;
  end;
end;

procedure TComm.SetBaudrate(Baud : integer);//Close : boolean);
const NoClose = false;
begin
  if NoClose and COM_INITIALISIERT then begin
    FBaudRate:= Baud;
    PutMonitorBuffer(integer($E0000000) or Baud);
    SetComParameters;
  end else begin
    ExitCOM;
    FBaudRate:= Baud;
  end;
end;

{$ifdef MSWINDOWS}
function TComm.ReadFromCOM : integer;
// Ein Zeichen aus dem Puffer auslesen. Ist der Puffer leer, so wird
// NULL_CHAR (0x1000) zurückgegeben.
// Die von der Prozedur "ReadComm" gelesenen Zeichen werden im Puffer
// "ReadBuffer" zwischengespeichert. Das erlaubt es, dass ein
// einzelnes gelesenes Zeichen wieder in den Puffer zurückge-
// speichert werden kann.
var fReadStat : BOOL;
    ComStat : TCOMSTAT;
    i : integer;
    Err : DWORD;
    dwErrorFlags, dwLength : DWORD;
begin
  // Wenn der ReadBuffer leer ist: ReadBuffer wieder füllen.
  if (COM_INITIALISIERT and (FReadBufferLen <= FReadBufferPtr)) then begin
    // 17. 2. 98
    dwLength:= 0;
    if ClearCommError(FCOM_Handle, dwErrorFlags, @ComStat) then begin
      if (dwErrorFlags <> 0) then begin
        PutMonitorBuffer(integer(dwErrorFlags or $80000000));
        ErrorOccured:= true;
        result:= dwErrorFlags or _NULL_CHAR;
        exit;
      end;
      dwLength:= ComStat.cbInQue;
    end else
      ShowLastErr(GetLastError);
    if (dwLength >= _MaxReadBuffer) then
      dwLength:= _MaxReadBuffer - 1;
    if (dwLength > 0) then begin
      fReadStat:= ReadFile(FCOM_Handle, FReadBuffer, _MaxReadBuffer,
                           dwLength, @osRead);
      if not fReadStat then begin
        Err:= GetLastError;
        if (Err = ERROR_IO_PENDING) then begin
          // wait for a second for this transmission to complete
          if (WaitForSingleObject(osRead.hEvent, 1000) <> 0) then begin
            ShowLastErr(GetLastError);
            dwLength:= 0;
          end else begin
            GetOverlappedResult(FCOM_Handle, osRead, dwLength, FALSE);
            inc(osRead.Offset, dwLength);
          end;
        end else
          // some other error occurred
          ShowLastErr(Err);
        dwLength:= 0;
      end;
    end;
    FReadBufferLen:= dwLength;
    FReadBufferPtr:= 0;

    for i:= 0 to FReadBufferLen-1 do begin
      PutMonitorBuffer(Byte(FReadBuffer[i]) or $20000000);
    end;
  end;
  result:= _NULL_CHAR;
  if (FReadBufferLen > FReadBufferPtr) then begin
    result:= ord(FReadBuffer[FReadBufferPtr]);
    inc(FReadBufferPtr);
  end;
end;
{$endif}

{$ifdef LINUX}
function TComm.ReadFromCOM : integer;
var rfds, efds : TFdSet;
    tv : TTimeVal;
    rval : integer;
    c : byte;
begin
  result:= NULL_CHAR;
  if COM_INITIALISIERT then begin
    tv.tv_sec:= 0; // sofort antworten
    tv.tv_usec:= 0;
    // vgl. Kylix in Team Seite 488
    FD_ZERO(rfds);             // Löscht das gesamte Set von Deskriptoren.
    FD_SET(FCOM_Handle, rfds); // Setzt "FCOM_Handle" in "rdfs" ein. 
    FD_ZERO(efds);             // Dasselbe für die Exceptions vorbereiten.
    FD_SET(FCOM_Handle, efds);
    rval:= select(FCOM_Handle+1, @rfds, nil, @efds, @tv);
    // Error: rval = -1 und errno ist gesetzt.
    if (rval < 0) or FD_ISSET(FCOM_Handle, efds) then begin
      if rval < 0 then
        ShowLastErr(Libc.errno);
      tcflush(FCOM_Handle, TCIFLUSH);
    end else  // 0 = time out
    if FD_ISSET(FCOM_Handle, rfds) then begin         
      c:= 0;
      if Libc.__read(FCOM_Handle, c, 1) = 1 then begin
        PutMonitorBuffer(Byte(c) or $20000000);
        result:= c;
      end;
    end;
  end;
end;
{$endif}

procedure TComm.PutBackToReadBuffer;
begin
  if (FReadBufferLen >= FReadBufferPtr) and
     (FReadBufferPtr > 0) then
    dec(FReadBufferPtr);
end;

{$ifdef LINUX}
function GetTickCount : cardinal;
var t : TimeVal;
begin
  result:= 0;
  if gettimeofday(t, nil) = 0 then begin
    result:= 1000*(t.tv_sec mod 86400) + t.tv_usec div 1000;
  end;
end;
{$endif}

function TComm.ReadWithTimeOut(TimeMs : integer) : integer;
var t1, t2 : DWORD;
begin
  result:= ReadFromCOM;
  if result = _NULL_CHAR then begin
    // GetTickCount:
    //   The elapsed time is stored as a DWORD value.
    //   Therefore, the time will wrap around to zero if Windows
    //   is run continuously for 49.7 days.
    t1:= GetTickCount;
//    t1:= MilliSecondOfTheDay(time);
    while (TimeMs > 0) and (result = _NULL_CHAR) and COM_INITIALISIERT do begin
      Sleep(0);
      MessageLoop(false);
      result:= ReadFromCOM;
      if result = _NULL_CHAR then begin
        t2:= GetTickCount;
        if t2 > t1 then                // Überlauf abfangen
          dec(TimeMs, t2-t1);
        t1:= t2;
      end;
    end;
  end;
end;

procedure TComm.ClearBuffer;
begin
  while ReadFromCOM <> _NULL_CHAR do begin end;
end;

{$ifdef MSWINDOWS}
function TComm.WriteToCOM(ch : AnsiChar) : MyBOOL;
begin
  result:= WriteBufferToCOM(@ch, 1);
end;
{$endif}
{$ifdef LINUX}
function TComm.WriteToCOM(ch : AnsiChar) : MyBOOL;
begin
  Result:= COM_INITIALISIERT;
  if result then begin
    result:= Libc.__write(FCOM_Handle, ch, 1) = 1;
    PutMonitorBuffer(Byte(ch) or $48000000);
  end;
end;
{$endif}

function TComm.WriteStringToCOM(const str : AnsiString) : MyBOOL;
var Res : MyBool;
    i : integer;
begin
  Res:= true;
  for i:= 1 to length(str) do
    if Res then
      Res:= WriteToCOM(str[i]);
  WriteStringToCOM:= Res;
end;

function TComm.WriteCS(const str : AnsiString) : MyBool;
var
  w : word;
  i : integer;
begin
  w := 0;
  for i := 1 to length(str) do
    inc(w, ord(str[i]));
  result := WriteStringToCOM(str + AnsiChar(w shr 8) + AnsiChar(w and $ff));
end;

function TComm.WriteCS_OK : MyBool;
begin
  result := WriteCS(#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55);
end;

function TComm.WriteBufferToCOM(str : PAnsiChar; Len : integer) : MyBOOL;
var i : integer;
    s : string;
{$ifdef MSWINDOWS}
    fWriteStat : BOOL;
    dwBytesWritten, Err : DWORD;
{$endif}
begin
  Result:= false;
  if (Len <= 0) or (str = nil) then
    exit;
{$ifdef MSWINDOWS}
  if (COM_INITIALISIERT) then begin
    if RtsFlowControl then begin
      i:= 0;
      repeat
        result:= false;
        PutMonitorBuffer(Byte(str[i]) or $48000000);

        fWriteStat:= WriteFile(FCOM_Handle, str[i], 1, dwBytesWritten, @osWrite);
        if not fWriteStat then begin
          Err:= GetLastError;
          if Err = ERROR_IO_PENDING then begin
            // wait for a second for this transmission to complete
            if (WaitForSingleObject(osWrite.hEvent, 1000) <> 0) then begin
              ShowLastErr(GetLastError);
              dwBytesWritten:= 0;
            end else begin
              try
                GetOverlappedResult(FCOM_Handle, osWrite, dwBytesWritten, FALSE);
                inc(osWrite.Offset, dwBytesWritten);
              except
                on E: Exception do
                  write(E.Message);
              end;
            end;
          end else
            ShowLastErr(Err);
          result:= integer(dwBytesWritten) = 1;
        end;
        inc(i);
      until (i = Len) or not result;
    end else begin
      for i:= 0 to Len-1 do
        PutMonitorBuffer(Byte(str[i]) or $48000000);

      fWriteStat:= WriteFile(FCOM_Handle, str[0], Len, dwBytesWritten, @osWrite);
      if not fWriteStat then begin
        Err:= GetLastError;
        if Err = ERROR_IO_PENDING then begin
          // wait for a second for this transmission to complete
          if (WaitForSingleObject(osWrite.hEvent, 1000) <> 0) then begin
            if HideError then begin
              s := FormatError(GetLastError);
              FConsoleWnd.write('<' + s + '>');
            end else
             ShowLastErr(GetLastError);
            dwBytesWritten:= 0;
          end else begin
            try
              GetOverlappedResult(FCOM_Handle, osWrite, dwBytesWritten, FALSE);
              inc(osWrite.Offset, dwBytesWritten);
            except
              on E: Exception do begin
                write(E.Message);
                result := false;
                exit;
              end;
            end;
          end;
        end else
          ShowLastErr(Err);
      end;
      result:= integer(dwBytesWritten) = Len;
    end;
  end;
{$endif}
{$ifdef LINUX}
  Result:= true;
  i:= 0;
  while (i < Len) and result do begin
    result:= WriteToCOM(str[i]);
    inc(i);
  end;
{$endif}
end;

function  TComm.GetTimerInterval : integer;
begin
  if not FTimer.Enabled then
    result:= 0
  else
    result:= FTimer.Interval;
end;

procedure TComm.SetTimerInterval(NewInterval : integer);
begin
  if NewInterval <= 0 then
    FTimer.Enabled:= false
  else
    FTimer.Interval:= NewInterval;
end;

function TComm.SendBreak(Duration : integer) : integer;
begin
  result := 0;
  if (COM_INITIALISIERT) then begin
    if UseTimeStamp then
      WriteNow;
{$ifdef MSWINDOWS}
    SetCommBreak(FCOM_Handle);
    if Duration > 0 then begin
      Sleep(Duration);
      result := Duration;
    end;
    ClearCommBreak(FCOM_Handle);
{$else}
    result := tcsendbreak(FCOM_Handle, Duration);
{$endif}
    PutMonitorBuffer(integer($A0000000) or EV_BREAK);
    if UseTimeStamp then
      WriteNow;
  end;
end;

{$ifdef MSWINDOWS}
function TComm.GetCommModemStatus : integer;
var Res : DWORD;
begin
  result:= 0;
  if (COM_INITIALISIERT) then begin
    if not Windows.GetCommModemStatus(FCOM_Handle, Res) then
      ShowLastErr(GetLastError)
    else
      result:= Res;
  end;
end;
{$endif}

{$ifdef MSWINDOWS}
function  TComm.CTS : boolean;  // (clear-to-send) signal is on
// Pin 5 (25) / 8 (9)
begin
  result:= (GetCommModemStatus and MS_CTS_ON) <> 0;
end;

function  TComm.DSR : boolean;  // (data-set-ready) signal is on
// Pin 6
begin
  result:= (GetCommModemStatus and MS_DSR_ON) <> 0;
end;

function  TComm.RING : boolean; // ring indicator signal is on
// Pin 22 (25)
begin
  result:= (GetCommModemStatus and MS_RING_ON) <> 0;
end;

function  TComm.DCD : boolean; // receive-line-signal-detect
// Pin 8 (25) / 1 (9)
begin
  result:= (GetCommModemStatus and MS_RLSD_ON) <> 0;
end;

function TComm.WaitCommEvent : DWORD;
// Gibt das Ereignis seit der letzten Abfrage zurück.
var Err : DWORD;
begin
  if Windows.WaitCommEvent(FCOM_Handle, FEvMask, @osPostEvent) then
    result:= FEvMask
  else begin
    Err:= GetLastError;
    if Err <> ERROR_IO_PENDING then
      ShowLastErr(Err);
    result:= 0;
  end;
end;

procedure TComm.SetEventMask(EventMask : SEvent);
var Events : DWORD;
begin
  FEventMask:= EventMask;
  FEvMask:= 0;

  if (COM_INITIALISIERT) then begin
    // get any early notifications
    Events:= 0; //EV_RXCHAR;
    if T_EV_BREAK in FEventMask then Events:= Events or EV_BREAK;
    if T_EV_CTS in FEventMask then Events:= Events or EV_CTS;
    if T_EV_DSR in FEventMask then Events:= Events or EV_DSR;
    if T_EV_ERR in FEventMask then Events:= Events or EV_ERR;
    if T_EV_RING in FEventMask then Events:= Events or EV_RING;
    if T_EV_RLSD in FEventMask then Events:= Events or EV_RLSD;
    if not SetCommMask(FCOM_Handle, Events) then begin
      ShowLastErr(GetLastError);
    end else
    if FEventMask <> [] then
      WaitCommEvent;
  end;
end;

function TComm.Event : DWORD;
begin
  result:= FEvMask;
  if result <> 0 then begin
    SetEventMask(FEventMask);
  end;
end;
{$endif}

procedure TComm.DelayMs(TimeMs : integer);
begin
  if COM_INITIALISIERT then
    PutMonitorBuffer(integer($A0010000) or (TimeMs and $ffff));
  Comm.DelayMs(TimeMs);
end;

procedure TComm.FormKeyPress(Sender: TObject; var Key: AnsiChar);
begin
{ !!!!!
  case Key of
    'l', 'L': LFMonitor:= not LFMonitor;
    'h', 'H': HexMonitor:= not HexMonitor;
    'd', 'D': begin
                DezMonitor:= not DezMonitor;
                HexMonitor:= false;
              end;
    else inherited FormKeyPress(Sender, Key);
  end;
  }
end;

procedure TComm.CommMonitor(Sender: TObject);
var i : integer;
    ch : char;
begin
  dec(FInCommMonitor);
  try
    if FInCommMonitor >= 0 then begin
      repeat
        i:= GetMonitorChar;
        if i <> 0 then begin
          ch:= char(i and $FF);
          if UseTimeStamp and (i < 0) then
            WriteNow;
          if ((i and $E0000000) = $E0000000) then begin
            write(SysUtils.Format(' <baud %d>', [i and $FFFFF]));
            writeln;
          end else
          if ((i and $D0000000) = $D0000000) then begin
            write(Format(' <close %d>', [i and $FFFFF]));
            writeln;
          end else
          if ((i and $C0000000) = $C0000000) then begin
            i:= i and $FFFFF;
            write(SysUtils.Format(' <open %d / COM%d>', [i, FComPortNr]));
            if i <> 0 then writeln;
          end else
          if ((i and $A0000000) = $A0000000) then begin
            i:= i and $FFFFF;
            case i of
              0:         write(' <rts false>');
              1:         write(' <rts true>');
              2:         write(' <dtr false>');
              3:         write(' <dtr true>');
              $10..$3f:  begin
                           write(' <');
                           if i < $20 then
                             write('stopbits')
                           else
                           if i < $30 then
                             write('parity')
                           else
                             write('bits');
                           write(SysUtils.Format('%d>', [i and $f]));
                         end;
              $40:       write(' <Break>');
              else       if (i >= $10000) and (i < $20000) then begin
                           write(SysUtils.Format(' <delay %d>', [i and $ffff]));
                         end;
            end;
          end else
          if (i and $80000000) <> 0 then begin { Fehler }
            i:= i and $ffff;
            write(SysUtils.Format('<err 0x%x:', [i]));
{$ifdef MSWINDOWS}
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
{$endif}              
            write('>');
            writeln;
          end else
          begin
            if (i and $40000000) <> 0 then
              write(' ')
            else
              write('.');
            if ((ch = #13) or (ch = #10)) and LFMonitor then
              write(ch)
            else
            if HexMonitor then begin
              write(Format('%2.2x', [integer(ch)]))
            end else
            if (ch <= ' ') or (ch >= #127) or DezMonitor then begin
              write(Format('<%d>', [integer(ch)]));
            end else
              write(ch);
          end;
        end;
      until i = 0;
    end;
  finally
    inc(FInCommMonitor);
  end;
end;

procedure TComm.TimeStamp;
begin
  CommMonitor(nil);
  dec(FInCommMonitor);
  try
    FConsoleWnd.TimeStamp;
  finally
    inc(FInCommMonitor);
  end;
end;

procedure TComm.mnConfigClick(Sender: TObject);
var Nr : integer;
begin
  Nr:= FComPortNr;
  if ParameterDialog then begin
    if FComPortNr = 0 then begin
      ExitCOM;
    end else
    if Nr <> FComPortNr then begin
      InitCOM;
    end else
      SetComParameters;
  end;
end;

procedure TComm.mnShowClick(Sender: TObject);
begin
  FConsoleWnd.Visible:= true;
  FConsoleWnd.WindowState:= wsMaximized;
{$ifdef MSWINDOWS}
  FConsoleWnd.SetFocus;
{$endif}
end;

procedure TComm.NewScreenSize(x, y : integer);
begin
  FConsoleWnd.NewScreenSize(x, y);
end;

procedure TComm.Show;
begin
  FConsoleWnd.Show;
end;

procedure TComm.WriteChar(ch : char);
begin
  FConsoleWnd.WriteChar(AnsiChar(ch));
end;

procedure TComm.write(const t : string);
begin
  FConsoleWnd.write(t);
end;

procedure TComm.write(const t : PChar);
var
  i : integer;
begin
  i := 0;
  if t <> nil then
    while t[i] <> #0 do begin
      WriteChar(t[i]);
      inc(i);
    end;
end;

procedure TComm.writeln;
begin
  FConsoleWnd.writeln;
end;

procedure TComm.writeln(const t : PChar);
begin
  write(t);
  writeln;
end;

procedure TComm.WriteTime(Time : TDateTime);
var
  col : integer;
begin
  if FConsoleWnd.Cursor.X > 0 then
    writeln;
  col := FConsoleWnd.Foreground;
  FConsoleWnd.Foreground := LightMagenta;
  write(SysUtils.FormatDateTime(' nn:ss.zzz', Time));
  FConsoleWnd.Foreground := col;
end;

procedure TComm.WriteNow;
var
  n, m : TDateTime;
begin
  n := now;
  m := 24.0*3600000.0*(n - FLastStamp);
  if abs(round(m)) > 20 then
    WriteTime(n);
  FLastStamp := n;
end;

{$ifdef VCL}
function MessageLoop(Save : MyBool) : MyBool;
// Garantiert den Message-Loop für Windows.
var msg : TMSG;
    hWndCapture, hWndFocus : HWND;
begin
  result:= true;
  if not Save then begin
    Application.ProcessMessages;
  end else begin
    hWndCapture:= 0;
    hWndFocus:= 0;

    // "Capture" und "Focus" garantieren, dass eine Sanduhr erhalten
    // bleibt.
    if (Save) then begin
      hWndCapture:= GetCapture;
      hWndFocus:= GetFocus;
    end;
    // Garantiert den "Message-Loop", wenn gewartet werden muss.
    // Damit wird vor allem der Aufbau der Dialogbox garantiert.
    while (PeekMessage(msg, 0, 0, 0, PM_REMOVE)) do begin
      case (msg.message) of
        WM_QUIT: begin
  //      PostQuitMessage(0);
                  Halt(255);
                  result:= false
                 end;
        else begin
                  TranslateMessage(msg);
                  DispatchMessage(msg);
                 end;
      end;
    end;
    if (hWndFocus <> 0) then
      SetFocus(hWndFocus);
    if (hWndCapture <> 0) then
      SetCapture(hWndCapture);
  end;
end;
{$else}
function MessageLoop(Save : MyBool) : MyBool;
// Garantiert den Message-Loop für Windows.
begin
  result:= true;
  Application.ProcessMessages;
end;
{$endif}

procedure DelayMs(TimeMs : integer);
var t1, t2 : DWORD;
begin
  t1:= GetTickCount;
  while (TimeMs > 0) do begin
    Sleep(0);
    t2:= GetTickCount;
    if t2 > t1 then            // Tagesueberlauf abfangen
      dec(TimeMs, t2-t1);
    t1:= t2;
    if (TimeMs > 0) then
      MessageLoop(false);
  end;
end;

function CRC(Buffer : PAnsiChar; Length : integer) : word;
{  "CRC" bildet den 16-Bit-CRC-Wert von "Length" Bytes des Puffers "Buffer".

   CRC_16 = 0x18005ul oder x^16 + x^15 + x^2 + 1
}
const CRC_FRAC = $8005;
var i, A, B : word;
begin
  A:= 0;
  { Die letzten Bytes müssen 0 sein:
    Ein 0-Byte wird beim Schieben nach links eingeschoben
    und ein zweites 0-Byte wird erzeugt, indem das "*Buffer"-
    Byte jeweils ins höhere Byte von "A" eingeschoben wird.   }
  while Length > 0 do begin
    A:= A xor Swap(word(byte(Buffer^)));
    inc(Buffer);
    i:= 8;
    repeat
      B:= A and $8000;
      inc(A, A);
      { Hier wird das MSB von CRC_16 berücksichtigt. }
      if B <> 0 then
        A:= A xor CRC_FRAC;
      dec(i);
    until i = 0;
    dec(Length);
  end;
  CRC:= A;
end;

end.
