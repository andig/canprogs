unit Console;

interface
{$ifdef LINUX}
  {$undef VCL}
{$endif}

uses
{$ifndef VCL}
  SysUtils, Types, Classes, Qt, QGraphics, QControls, QForms, QDialogs, QMenus,
  QStdCtrls, QExtCtrls, QTypes;
{$else}
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  Menus, ExtCtrls;
{$endif}

const
  Black         = $00000000;
  Blue          = $007F0000;
  Green         = $00007F00;
  Cyan          = $007F7F00;
  Red           = $0000007F;
  Magenta       = $007F00FF;
  Brown         = $00007F7F;
  LightGray     = $00C0C0C0;

{ Foreground color constants }

  DarkGray      = $003F3F3F;
  LightBlue     = $00FF0000;
  LightGreen    = $0000FF00;
  LightCyan     = $00FFFF3f;
  LightRed      = $000000FF;
  LightMagenta  = $00FF00FF;
  Yellow        = $0000FFFF;
  White         = $00FFFFFF;

{$ifndef VCL}
  Vk_Escape = Key_Escape;
  Vk_Tab    = Key_Tab;
  Vk_Backspace = Key_Backspace;
  Vk_Return = Key_Return;
  Vk_Enter  = Key_Enter;
  Vk_Up     = Key_Up;
  Vk_Down   = Key_Down;
  Vk_Left   = Key_Left;
  Vk_Right  = Key_Right;
  Vk_Prior  = Key_PageUp;
  Vk_Next   = Key_PageDown;
  Vk_Home   = Key_Home;
  Vk_End    = Key_End;
  Vk_Insert = Key_Insert;
  Vk_Delete = Key_Delete;
  Vk_Cancel = Key_Escape;
  Vk_F1     = Key_F1;
  Vk_F2     = Key_F2;
  Vk_F3     = Key_F3;
  Vk_F4     = Key_F4;
  Vk_F5     = Key_F5;
  Vk_F6     = Key_F6;
  Vk_F7     = Key_F7;
  Vk_F8     = Key_F8;
  Vk_F9     = Key_F9;
  Vk_F10    = Key_F10;
  Vk_F11    = Key_F11;
  Vk_F12    = Key_F12;
{$endif}

{$ifdef VCL}
  MonWm_NOTIFY  = WM_APP + 100;
{$endif}

type
  PColor = ^TColor;

  PMinMaxInfo = ^TMinMaxInfo;
  TMinMaxInfo = array[0..4] of TPoint;

  TConsoleCallBack = procedure (x, y : integer; ptr : pointer); cdecl;
  TCommCallBack = procedure (x, y :integer; ptr : pointer);


///////////////////////////////////////////////////////
//                                                   //
// TConsoleWnd:  allg. Fensterklasse                 //
//                                                   //
///////////////////////////////////////////////////////

  TConsoleWnd = class(TForm)
    procedure FormResize(Sender: TObject);
    procedure FormShow(Sender: TObject);
{$ifndef VCL}
    procedure FormPaint(Sender: TObject);
{$endif}
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure FormKeyPress(Sender: TObject; var Key: Char);
    procedure FormMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormMouseWheel(Sender: TObject; Shift: TShiftState;
      WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);

  private
    FScreenSize : TPoint;
    FCursor : TPoint;
    FWindMin : TPoint;                 // Window upper left coordinates
    FWindMax : TPoint;                 // Window lower right coordinates
    FScrollY : integer;
    FTabWidth : word;                  // Tabulatorweite
{$ifdef MSWINDOWS}
    FChangeOemToAnsi : boolean;
{$endif}
    FNewResize : boolean;

    FForeground : TColor;
    FBackground : TColor;
    FOuterBackground : TColor;
    FCharSize : TPoint;
    OldBackground, OldForeground : TColor;
    FAutoTracking: Boolean;            // Track cursor on Write?
    FCheckEOF: Boolean;                // Allow Ctrl-Z for EOF?
    FCheckBreak: Boolean;              // Allow Ctrl-C for break?
    FReading : boolean;

    FUseHalt : boolean;

{$ifdef VCL}
    DC : HDC;
    SaveFont : HFONT;
{$endif}

    FKeyCount : integer;
    FKeyBuffer: array [0..63] of AnsiChar; // Keyboard type-ahead buffer

    ScreenBuffer : PAnsiChar;
    ScreenForeground : PColor;
    ScreenBackground : PColor;

    procedure ScrollTo(X, Y: Integer);
    procedure CursorTo(x, y : integer);
    procedure ShowText(L, R: Integer);
    procedure TextOutput(X, Y, Laenge : Integer);
    procedure InitDeviceContext(FPainting : boolean);
    procedure DoneDeviceContext(FPainting : boolean);
    procedure TrackCursor;
    procedure UpdateCursor;
    function  ScreenPtr(X, Y: Integer): PAnsiChar;
    function  SaveChar(X, Y : Integer; c : AnsiChar) : boolean;
    function  ColorLength(PColorRef : PColor; Len : Integer) : Integer;
    procedure SetScreenSize(NewSize : TPoint);
    function  ScreenIndex(X, Y : integer) : integer;
    function  GetScreenSizeX : LongInt;
    function  GetScreenSizeY : LongInt;
    procedure WindowScroll(Horz : boolean; Action : integer);
    procedure InvalidateRect(const Rect: TRect; EraseBackground: Boolean);
{$ifdef VCL}
    procedure WindowPaint(var _Message : TWMPaint); message WM_PAINT;
    procedure WM_ConsMsg(var _Message : TMessage); message(WM_APP + 110);
    procedure WM_MonMsg(var _Message : TMessage); message(MonWm_NOTIFY);

{$endif}

 published
    property AutoTracking : boolean read FAutoTracking write FAutoTracking;
    property CheckBreak : boolean read FCheckBreak write FCheckBreak;
    property CheckEOF : boolean read FCheckEOF write FCheckEOF;
    property Foreground : TColor read FForeground write FForeground;
    property Background : TColor read FBackground write FBackground;
    property ScreenSizeX : LongInt read GetScreenSizeX;
    property ScreenSizeY : LongInt read GetScreenSizeY;
    property TabWidth : word read FTabWidth write FTabWidth;
{$ifdef MSWINDOWS}
    property ChangeOemToAnsi : boolean read FChangeOemToAnsi write FChangeOemToAnsi;
{$endif}
    property UseHalt : boolean read FUseHalt write FUseHalt;

  public
    FAutoScroll : boolean;
    FCursorTracking : boolean;
    FDblClickPoint : TPoint;
    FSingleClick : boolean;
    FCallingC : pointer;
    FCallingDelphi : pointer;

    procedure MyDestroy;
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;

    procedure WriteBuf(Buffer: PAnsiChar; Count: Word);
    function  ReadBuf(Buffer: PAnsiChar; Count: Word): Word;

    procedure ClrScr;
    procedure NewScreenSize(x, y : integer);
    procedure WriteChar(Ch: AnsiChar);
    procedure GotoXY(x, y : integer);
    function  KeyPressed : Boolean;
    function  ReadKey : AnsiChar;
    procedure TextBackground(Color : TColor);
    procedure TextColor(Color : TColor);
    procedure LowVideo;
    procedure HighVideo;
    procedure ShowCursor;
    procedure HideCursor;
    procedure Window(XMin, YMin, XMax, YMax : Integer);
    procedure SetNewColor(ForeGrnd, BackGrnd : TColor);

    function  WhereX : Integer;
    function  WhereY : Integer;
    procedure ClrWindow;
    procedure ClrEol;
    procedure write(const t : string); overload;
    procedure write(p : pointer); overload;
    procedure write(d : extended); overload;
    procedure write(i : integer); overload;
    procedure write(const FormatStr : string; const Args: array of const); overload;
    procedure write(const Args: array of const); overload;
    procedure writeln; overload;
    procedure writeln(const t : string); overload;
    procedure writeln(const FormatStr : string; const Args: array of const); overload;
    procedure writeln(const Args: array of const); overload;
    procedure WriteHexByte(b : integer);
    procedure WriteHexWord(w : integer);
    procedure TimeStamp;

    procedure CopyTextToClipboard;
    procedure CharCoordinates(var Coord : TPoint; x, y : integer);

    property Cursor : TPoint read FCursor;
  end;


///////////////////////////////////////////////////////
//                                                   //
// TMenuClass: Menü-Hilfsklasse                      //
//                                                   //
///////////////////////////////////////////////////////

  TMenuHandlerProc = procedure (Sender : TMenuItem; IndexItem, IndexSubItem : integer);

  TBasicMenuClass = class
  private
    FOwner : TMenuItem;
    FHandlerProc : TMenuHandlerProc;
    procedure MenuHandler(Sender : TObject);
  public
    constructor Create(AOwner : TMenuItem);
    procedure SetHandlerProc(Proc : TMenuHandlerProc);
    function AddItem(const Title : string; NotSubItem : boolean) : TMenuItem;
    function Delete(Index : integer) : boolean;
    function AddToItem(const Title : string; AddTo : TMenuItem) : TMenuItem;
  end;

  TMainMenuClass = class(TBasicMenuClass)
  public
    constructor Create(AOwner : TCustomForm);
  end;

  TPopUpMenuClass = class(TBasicMenuClass)
  public
    PopUpMenu : TPopUpMenu;

    constructor Create(AOwner : TCustomForm);
    destructor  Destroy; override;
  end;
  
function TimeToMs(t : double) : AnsiString;

var
  ConsoleCallBack : TConsoleCallBack;
  CommCallBack : TCommCallBack;


implementation

{$ifdef MSWINDOWS}
uses Clipbrd;
{$endif}
{$ifdef LINUX}
uses QClipbrd;
{$endif}

{$ifdef VCL}
  {$R *.DFM}
{$else}
  {$R *.xfm}
{$endif}

function Min(X, Y: Integer): Integer;
begin
  if X < Y then Min:= X else Min:= Y;
end;

function Max(X, Y: Integer): Integer;
begin
  if X > Y then Max:= X else Max:= Y;
end;

function TimeToMs(t : double) : AnsiString;
begin
  t:= frac(t);
  if t < 0 then t:= t + 1.0;
  Result:= AnsiString(SysUtils.TimeToStr(t) +
             Format(':%3.3d', [trunc(24.0*3600.0*1000.0*t) mod 1000]));
end;


///////////////////////////////////////////////////////
//                                                   //
// TConsoleWnd: Methoden                             //
//                                                   //
///////////////////////////////////////////////////////

procedure TConsoleWnd.SetScreenSize(NewSize : TPoint);
{$ifdef VCL}
var X, Y, XS, YS : integer;
    State : TWindowState;
{$endif}
begin
  if ScreenBuffer <> nil then begin
    FreeMem(ScreenBuffer, FScreenSize.X * FScreenSize.Y);
    ScreenBuffer:= nil;
  end;
  if ScreenForeground <> nil then begin
    FreeMem(ScreenForeground,
            FScreenSize.X * FScreenSize.Y*SizeOf(TColor));
    ScreenForeground:= nil;
  end;
  if ScreenBackground <> nil then begin
    FreeMem(ScreenBackground,
            FScreenSize.X * FScreenSize.Y*SizeOf(TColor));
    ScreenBackground:= nil;
  end;
  FScrollY:= 0;
  FScreenSize:= NewSize;
  if (NewSize.x > 0) and (NewSize.y > 0) then begin
    GetMem(ScreenBuffer, FScreenSize.X * FScreenSize.Y);
    GetMem(ScreenForeground,
           FScreenSize.X * FScreenSize.Y*SizeOf(TColor));
    GetMem(ScreenBackground,
           FScreenSize.X * FScreenSize.Y*SizeOf(TColor));
  end else
    exit;

  with Constraints do begin
{$ifndef VCL}
    MinWidth:= Max(0, 5*FCharSize.x);
    MinHeight:= Max(0, 5*FCharSize.y);
    MaxWidth:= Max(0, FScreenSize.X*FCharSize.x)+5;
    MaxHeight:= Max(0, FScreenSize.Y*FCharSize.y)+5;
{$else}
    State:= WindowState;
    XS:= GetSystemMetrics(sm_CXVScroll);
    X:= Min(FScreenSize.X * FCharSize.X + XS,
      GetSystemMetrics(sm_CXScreen)) + GetSystemMetrics(sm_CXFrame) * 2;
    if not VertScrollBar.Visible then
      X:= X - XS;
    YS:= GetSystemMetrics(sm_CYHScroll);
    Y:= 0;
    if Menu <> nil then
      Y:= GetSystemMetrics(SM_CYMENU);
    Y:= Min(FScreenSize.Y * FCharSize.Y + YS + Y +
      GetSystemMetrics(sm_CYCaption), GetSystemMetrics(sm_CYScreen)) +
      GetSystemMetrics(sm_CYFrame) * 2;
    if not HorzScrollBar.Visible then
      Y:= Y - YS;
    MaxWidth:= X;
    MaxHeight:= Y;
    MinWidth:= FCharSize.X * 16 + GetSystemMetrics(sm_CXVScroll) +
               GetSystemMetrics(sm_CXFrame) * 2;
    MinHeight:= FCharSize.Y * 4 + GetSystemMetrics(sm_CYHScroll) +
               GetSystemMetrics(sm_CYFrame) * 2 + GetSystemMetrics(sm_CYCaption);
    WindowState:= State;
{$endif}
  end;
end;

constructor TConsoleWnd.Create(AOwner: TComponent);
begin
  FScreenSize.x:= 80;
  FScreenSize.y:= 25;
  FForeground:= White; OldForeground:= -1;
  FBackground:= Black; OldBackground:= -1;
  FOuterBackground:= FBackground;
  FCursorTracking:= false;
  FAutoTracking:= false;
  FCheckEOF:= false;
  FCheckBreak:= false;
  FDblClickPoint.x:= -1;
  FDblClickPoint.y:= -1;
  FSingleClick:= false;
  FTabWidth:= 8;
{$ifdef MSWINDOWS}
  FChangeOemToAnsi:= false;
{$endif}
  FNewResize:= false;

  FKeyCount:= 0;

  FReading:= false;
  FAutoScroll:= false;

  UseHalt:= true;

  FScrollY:= 0;
  ScreenBuffer:= nil;
  ScreenForeground:= nil;
  ScreenBackground:= nil;

  inherited Create(AOwner);

{$ifdef VCL}
  DoubleBuffered:= true;
{$endif}

  Scaled:= false;
{$ifndef VCL}
  OnPaint:= FormPaint;
  {$ifdef LINUX}
  Font.Size:= 10;
  Font.Name:= 'misc-fixed';
  {$else}
  Font.Name:= 'Fixedsys';
  {$endif}
  Font.Pitch:= fpFixed;
{$endif}

  FCharSize.X:= Canvas.TextWidth('0');
  FCharSize.Y:= Canvas.TextHeight('ABCDÄjyŽ') + 1;
  SetScreenSize(FScreenSize);
  WindowState:= wsMaximized;

{  if AOwner is TForm then
    Icon:= (AOwner as TForm).Icon
  else}
{$ifndef VCL}
  if (Application.Icon <> nil) and (Application.Icon.Handle <> nil) then
{$else}
  if (Application.Icon <> nil) and (Application.Icon.Handle <> 0) then
{$endif}
    Icon:= Application.Icon;
  ClrScr;
end;

procedure TConsoleWnd.MyDestroy;
const NullSize : TPoint = (x : 0; y : 0);
begin
  SetScreenSize(NullSize);
end;

destructor TConsoleWnd.Destroy;
begin
  MyDestroy;
  
  inherited Destroy;
end;

procedure TConsoleWnd.InvalidateRect(const Rect: TRect; EraseBackground: Boolean);
begin
{$ifndef VCL}
  inherited InvalidateRect(Rect, EraseBackground);
{$else}
  Windows.InvalidateRect(Handle, @Rect, EraseBackground);
{$endif}
end;

function  TConsoleWnd.GetScreenSizeX : LongInt;
begin
  Result:= FScreenSize.x;
end;

function  TConsoleWnd.GetScreenSizeY : LongInt;
begin
  Result:= FScreenSize.y;
end;

procedure TConsoleWnd.NewScreenSize(x, y : integer);
var State : TWindowState;
  NewSize : TPoint;
begin
  NewSize.x := x;
  NewSize.y := y;
  if (NewSize.x > 0) and (NewSize.y > 0) and
     ((NewSize.x <> FScreenSize.x) or (NewSize.y <> FScreenSize.y)) then begin
    SetScreenSize(NewSize);
    ClrScr;

    // Resize erzwingen
    if Visible then begin
      State:= WindowState;
      WindowState:= wsMinimized;
      WindowState:= State;
    end else begin
      FNewResize:= true;
    end;
  end else
    ClrScr;
end;

procedure TConsoleWnd.FormShow(Sender: TObject);
var State : TWindowState;
begin
  if FNewResize then begin
    FNewResize:= false;
{$ifdef MSWINDOWS}
    State:= WindowState;
    WindowState:= wsMinimized;
    WindowState:= State;
{$endif}
  end;
end;

procedure TConsoleWnd.TextBackground(Color : TColor);
begin
  FBackground:= Color;
end;

procedure TConsoleWnd.TextColor(Color : TColor);
begin
  FForeground:= Color;
end;

procedure TConsoleWnd.LowVideo;
begin
  if (FForeground and $0000FF) <> 0 then
    FForeground:= FForeground and $00FFFF7F;
  if (FForeground and $00FF00) <> 0 then
    FForeground:= FForeground and $00FF7FFF;
  if (FForeground and $FF0000) <> 0 then
    FForeground:= FForeground and $007FFFFF;
end;

procedure TConsoleWnd.HighVideo;
begin
  if (FForeground and $0000FF) <> 0 then
    FForeground:= FForeground or $00000080;
  if (FForeground and $00FF00) <> 0 then
    FForeground:= FForeground or $00008000;
  if (FForeground and $FF0000) <> 0 then
    FForeground:= FForeground or $00800000;
end;

procedure TConsoleWnd.Window(XMin, YMin, XMax, YMax : Integer);
begin
  FWindMin.X:= Max(1, Min(XMin, FScreenSize.X)) - 1;
  FWindMin.Y:= Max(1, Min(YMin, FScreenSize.Y)) - 1;
  FWindMax.X:= Max(1, Min(XMax, FScreenSize.X)) - 1;
  FWindMax.Y:= Max(1, Min(YMax, FScreenSize.Y)) - 1;
end;

procedure TConsoleWnd.SetNewColor(ForeGrnd, BackGrnd : TColor);
begin
  if ForeGrnd <> OldForeground then begin
    Canvas.Font.Color:= ForeGrnd;
    OldForeground:= ForeGrnd;
  end;
  if BackGrnd <> OldBackground then begin
    Canvas.Brush.Color:= BackGrnd;
    OldBackground:= BackGrnd;
  end;
end;

procedure TConsoleWnd.InitDeviceContext(FPainting : boolean);
begin
{$ifdef VCL}
  if not FPainting then DC:= GetDC(Handle);
  SaveFont:= SelectObject(DC, Font.Handle);
{$endif}
  OldForeground:= -1;
  OldBackground:= -1;
end;

procedure TConsoleWnd.DoneDeviceContext(FPainting : boolean);
begin
{$ifdef VCL}
  SelectObject(DC, SaveFont);
  if not FPainting then ReleaseDC(Handle, DC);
{$endif}
end;

procedure TConsoleWnd.UpdateCursor;
var ind : integer;
begin
  if (ScreenForeground <> nil) and (ScreenBackground <> nil) then begin
    ind:= SizeOf(TColor) * ScreenIndex(FCursor.X, FCursor.Y);
    PColor(LongInt(ScreenForeground) + ind)^:= PColor(LongInt(ScreenForeground) + ind)^ xor $ffffff;
    PColor(LongInt(ScreenBackground) + ind)^:= PColor(LongInt(ScreenBackground) + ind)^ xor $ffffff;
    TextOutput(FCursor.X, FCursor.Y, 1);
    FReading:= not FReading;
    Update;
  end;
end;

procedure TConsoleWnd.ShowCursor;
begin
  if not FReading then
    UpdateCursor;
end;

procedure TConsoleWnd.HideCursor;
begin
  if FReading then
    UpdateCursor;
end;

procedure TConsoleWnd.CursorTo(x, y : integer);
begin
  FCursor.X:= Max(0, Min(X, FWindMax.X - FWindMin.X)) + FWindMin.X;
  FCursor.Y:= Max(0, Min(Y, FWindMax.Y - FWindMin.Y)) + FWindMin.Y;
end;

procedure TConsoleWnd.ScrollTo(X, Y: Integer);
begin
  X := Max(0, Min(X, HorzScrollBar.Range));
  Y := Max(0, Min(Y, VertScrollBar.Range));
  if (X <> HorzScrollBar.Position) or
     (Y <> VertScrollBar.Position) then begin
    HorzScrollBar.Position:= X;
    VertScrollBar.Position:= Y;
  end;
end;

procedure TConsoleWnd.TrackCursor;
begin
  if FCursorTracking then
    ScrollTo(Max((FCursor.X + 1)*FCharSize.x - ClientWidth,
             Min(HorzScrollBar.Position, FCursor.X*FCharSize.x)),
             Max((FCursor.Y + 1)*FCharSize.y - ClientHeight,
             Min(VertScrollBar.Position, FCursor.Y*FCharSize.y)));
end;

function TConsoleWnd.ScreenIndex(X, Y : integer) : integer;
begin
  Result:= Y;
  if (FScreenSize.Y > 1) and (FScrollY > 0) then
    Result:= (Y + FScrollY) mod FScreenSize.Y;
  Result:= Result*FScreenSize.X + X;
end;

function TConsoleWnd.ScreenPtr(X, Y: Integer): PAnsiChar;
begin
  ScreenPtr:= @ScreenBuffer[ScreenIndex(X, Y)];
end;

function TConsoleWnd.SaveChar(X, Y : Integer; c : AnsiChar) : boolean;
var ind : integer;
begin
  result:= false;
  if ScreenBuffer = nil then
    exit;
  ind:= ScreenIndex(X, Y);
  if ScreenBuffer[ind] <> c then begin
    ScreenBuffer[ind]:= c;
    result:= true;
  end;

  // Save Cursor Grounds
  if (ScreenForeground <> nil) and (ScreenBackground <> nil) then begin
    ind:= SizeOf(TColor) * ind;
    if PColor(LongInt(ScreenForeground) + ind)^ <> FForeground then begin
      result:= true;
      PColor(LongInt(ScreenForeground) + ind)^:= FForeground;
    end;
    if PColor(LongInt(ScreenBackground) + ind)^ <> FBackground then begin
      result:= true;
      PColor(LongInt(ScreenBackground) + ind)^:= FBackground;
    end;
  end;
end;

function TConsoleWnd.ColorLength(PColorRef : PColor; Len : Integer) : Integer;
var l : Integer;
    PCol : PColor;
begin
  l:= 1;
  PCol:= PColorRef;
  while l < Len do begin
    PCol:= PColor(LongInt(PCol) + SizeOF(TColor));
    if PColorRef^ <> PCol^ then break;
    l:= l + 1;
  end;
  ColorLength:= l;
end;

procedure TConsoleWnd.TextOutput(X, Y, Laenge : Integer);
var i, l : Integer;
    Xm, Ym : Integer;
    Ind : integer;
    PForG, PBackG : PColor;
    s : WideString;
    k : integer;
    Rect : TRect;
begin
  if (ScreenBuffer = nil) or
     (ScreenForeground = nil) or (ScreenBackground = nil) then exit;

  i:= X;
  Ym:= Min(FScreenSize.Y-1, Y);
  Xm:= Min(FScreenSize.X-1, X);
  Ind:= ScreenIndex(Xm, Ym);
  Rect.Top:= Y*FCharSize.Y - VertScrollBar.Position;
  Rect.Bottom:= Rect.Top + FCharSize.Y;
  while i <= X+Laenge-1 do begin
    Rect.Left:= i*FCharSize.X - HorzScrollBar.Position;
    if (i < FScreenSize.X) and (Y < FScreenSize.Y) then begin
      PForG:= PColor(LongInt(ScreenForeground) + Ind*SizeOf(TColor));
      PBackG:= PColor(LongInt(ScreenBackground) + Ind*SizeOf(TColor));
      SetNewColor(PForG^, PBackG^);
      l:= Min(X+Laenge-i, FScreenSize.X - i);
      l:= Min(ColorLength(PForG, l), ColorLength(PBackG, l));
      s:= '';
      for k:= 0 to l-1 do
        s:= s + char(ScreenBuffer[Ind+k]);
      try
        Rect.Right:= Rect.Left + l*FCharSize.X;
        Canvas.FillRect(Rect);
        Canvas.TextOut(Rect.Left, Rect.Top, s);
      except
        s := '';
      end;
      Inc(Ind, l);
      Inc(i, l);
//      if i = FScreenSize.X then Dec(Ind);
    end else begin
      // Beim Rand mit der Standard-Hintergrundfarbe ausfüllen.
      SetNewColor(OldForeground, FOuterBackground);
      Rect.Right:= Rect.Left + FCharSize.X;
      Canvas.FillRect(Rect);
      Canvas.TextOut(Rect.Left, Rect.Top, ' ');
      inc(i);
//      if i < FScreenSize.X then Inc(Ind);
    end;
  end;
end;

procedure TConsoleWnd.ShowText(L, R: Integer);
{$ifndef VCL}
//var Rect : TRect;
{$endif}
begin
  if (L < R) and Visible and (WindowState <> wsMinimized) then begin
{$ifndef _VCL}
    InitDeviceContext(false);
    TextOutput(L, FCursor.Y, R - L);
    DoneDeviceContext(False);
{$else}
    Rect.Left:= L*FCharSize.X - HorzScrollBar.Position;
    Rect.Right:= Rect.Left + (R - L)*FCharSize.X;
    Rect.Top:= FCursor.Y*FCharSize.Y - VertScrollBar.Position;
    Rect.Bottom:= Rect.Top + FCharSize.Y;
    InvalidateRect(Rect, true);
{$endif}
  end;
end;

procedure TConsoleWnd.WriteBuf(Buffer: PAnsiChar; Count: Word);
var i : integer;
    L, R: Integer;
    ShowNewText : boolean;

  procedure NewLine;
  var i, Len, Ind, IndX : Integer;
      Rect : TRect;
{$ifndef VCL}
      Dest, Source : TRect;
      X1, X2, Y1 : integer;
{$endif}
  begin
    if ShowNewText then
      ShowText(L, R);
    ShowNewText:= false;
    L:= 0;
    R:= 0;
    FCursor.X:= FWindMin.X;
    Inc(FCursor.Y);
    if FCursor.Y >= FWindMax.Y+1 then begin
      Dec(FCursor.Y);
      if (FWindMin.X = 0) and (FWindMin.Y = 0) and
         (FWindMax.X = FScreenSize.X-1) and
         (FWindMax.Y = FScreenSize.Y-1) then begin
        inc(FScrollY);
        if FScreenSize.Y > 1 then
          FScrollY:= FScrollY mod FScreenSize.Y;
      end else begin
        Len:= FWindMax.X-FWindMin.X+1;
        for i:= FWindMin.Y to FWindMax.Y-1 do begin
          Ind:= ScreenIndex(FWindMin.X, i);
          Move(ScreenBuffer[Ind+FScreenSize.X], ScreenBuffer[Ind], Len);
          Ind:= SizeOf(TColor)*Ind;
          IndX:= Ind + FScreenSize.X*SizeOf(TColor);
          Move(PColor(LongInt(ScreenForeground) + IndX)^,
               PColor(LongInt(ScreenForeground) + Ind)^,
               SizeOf(TColor)*Len);
          Move(PColor(LongInt(ScreenBackground) + IndX)^,
               PColor(LongInt(ScreenBackground) + Ind)^,
               SizeOf(TColor)*Len);
        end;
      end;
      for i:= FWindMin.X to FWindMax.X do 
        SaveChar(i, FCursor.Y, ' ');
      if Visible and (WindowState <> wsMinimized) then begin
{$ifndef VCL}
        Dest.left:= FWindMin.X*FCharSize.X - HorzScrollBar.Position;
        Dest.right:= Dest.left + (FWindMax.X - FWindMin.X + 1)*FCharSize.X;
        Dest.top:= FWindMin.Y*FCharSize.Y - VertScrollBar.Position;
        Dest.bottom:= Dest.top + (FWindMax.Y - FWindMin.Y)*FCharSize.Y;
        if Dest.Bottom > ClientRect.Bottom then
          Dest.Bottom:= ClientRect.Bottom;
        if Dest.Right > ClientRect.Right then
          Dest.Right:= ClientRect.Right;
        Source:= Dest;
        Source.Top:= Dest.Top + FCharSize.Y;
        Source.Bottom:= Dest.Bottom + FCharSize.Y;
        if Source.Bottom > ClientRect.Bottom then
          Source.Bottom:= ClientRect.Bottom;
        Canvas.CopyRect(Dest, Canvas, Source);
        Update;
        Rect:= Source;
        Rect.Top:= Source.Bottom - FCharSize.Y;
        //InvalidateRect(Rect, true);
        X1:= Max(0, (Rect.Left + HorzScrollBar.Position) div FCharSize.X);
        X2:= (Rect.Right + FCharSize.X - 1 + HorzScrollBar.Position) div FCharSize.X;
        Y1:= Max(0, (Rect.Top + VertScrollBar.Position) div FCharSize.Y);
        TextOutput(X1, Y1, X2-X1+1);
        Update;
{$else}
        Rect.left:= FWindMin.X*FCharSize.X - HorzScrollBar.Position;
        Rect.top:= FWindMin.Y*FCharSize.Y - VertScrollBar.Position;
        Rect.right:= Rect.left + (FWindMax.X - FWindMin.X + 1)*FCharSize.X;
        Rect.bottom:= Rect.top + (FWindMax.Y - FWindMin.Y + 1)*FCharSize.Y;
        ScrollWindowEx(Handle, 0, -FCharSize.Y, @Rect, @Rect,
                       0, nil, sw_Invalidate);
        UpdateWindow(Handle);
{$endif}
      end;
    end;
  end;

begin
  if Self = nil then
    exit;

  HideCursor;
  ShowNewText:= false;
  L:= FCursor.X;
  R:= FCursor.X;
  while Count > 0 do begin
    case Buffer^ of
      #32..#255:
           begin
{$ifdef VCL}
             if ChangeOemToAnsi then
               OemToAnsiBuff(Buffer, Buffer, 1);
{$endif}
             if SaveChar(FCursor.X, FCursor.Y, Buffer^) then
               ShowNewText:= true;
             Inc(FCursor.X);
             if FCursor.X > R then R := FCursor.X;
             if FCursor.X >= FWindMax.X+1 then NewLine;
           end;
      #13: begin
             ShowText(L, R);
             FCursor.x:= FWindMin.x;
             L:= FCursor.x; R:= FCursor.x;
           end;
      #10: NewLine;
      #8:
          if FCursor.X > FWindMin.X then begin
            Dec(FCursor.X);
            if SaveChar(FCursor.X, FCursor.Y, ' ') then
               ShowNewText:= true;
            if FCursor.X < L then L:= FCursor.X;
          end;
      #9: begin { Tabulator }
          i:= TabWidth - (FCursor.x - FWindMin.x) mod TabWidth;
          while (i > 0) and (FCursor.X < FWindMax.X+1) do begin
            if SaveChar(FCursor.X, FCursor.Y, ' ') then
              ShowNewText:= true;
            Inc(FCursor.X);
            if FCursor.X > R then R := FCursor.X;
            Dec(i);
          end;
          if FCursor.X >= FWindMax.X+1 then NewLine;
        end;
      #7:
        Beep;
    end;
    Inc(Buffer);
    Dec(Count);
  end;
  if ShowNewText then
    ShowText(L, R);
  if FAutoTracking then TrackCursor;
end;

procedure TConsoleWnd.WriteChar(Ch: AnsiChar);
begin
  WriteBuf(@Ch, 1);
end;

function TConsoleWnd.KeyPressed : Boolean;
begin
  Application.ProcessMessages;
  if not Visible and UseHalt then  // Wenn das Fenster geschlossen
    Application.Terminate; // 30. 1. 2002
                                   // wurde ==> Programm-Abbruch
  KeyPressed:= FKeyCount > 0;
end;

function TConsoleWnd.ReadKey : AnsiChar;
begin
  Result:= #13; // Wenn Terminated: Readln soll ok sein.
  TrackCursor;
  if not KeyPressed then begin
    ShowCursor;
    repeat
      Sleep(10);
    until KeyPressed or Application.Terminated;
    HideCursor;
  end;
  if KeyPressed then begin
    ReadKey:= FKeyBuffer[0];
    Dec(FKeyCount);
    Move(FKeyBuffer[1], FKeyBuffer[0], FKeyCount);
  end;
end;

function TConsoleWnd.ReadBuf(Buffer: PAnsiChar; Count: Word): Word;
var Ch : AnsiChar;
    I : Word;
begin
  I:= 0;
  repeat
    Ch:= ReadKey;
    case Ch of
     #8:
        if I > 0 then begin
          Dec(I);
          WriteChar(#8);
        end;
     #32..#255:
        if I < Count - 2 then begin
          Buffer[I]:= Ch;
          Inc(I);
          WriteChar(Ch);
        end;
    end;
  until (Ch = #13) or (FCheckEOF and (Ch = #26)) or Application.Terminated;
  Buffer[I]:= Ch;
  Inc(I);
  if Ch = #13 then begin
    Buffer[I]:= #10;
    Inc(I);
    WriteChar(#13);
    WriteChar(#10);
  end;
  TrackCursor;
  ReadBuf:= I;
end;

procedure TConsoleWnd.GotoXY(x, y : integer);
begin
  CursorTo(x - 1, y - 1);
end;

function TConsoleWnd.WhereX : Integer;
begin
  WhereX:= FCursor.X + 1 - FWindMin.X;
end;

function TConsoleWnd.WhereY : Integer;
begin
  WhereY:= FCursor.Y + 1 - FWindMin.Y;
end;

procedure TConsoleWnd.ClrWindow;
var ind, w : integer;
    X, Y : Integer;
    Rect : TRect;
begin
  FCursor:= FWindMin;

  if (ScreenBuffer = nil) or
     (ScreenForeground = nil) or (ScreenBackground = nil) then exit;

  if (FWindMin.X = 0) and (FWindMin.Y = 0) and
     (FWindMax.X = FScreenSize.X-1) and (FWindMax.Y = FScreenSize.Y-1) then
    FOuterBackground:= FBackground;

  for Y:= FWindMin.Y to FWindMax.Y do begin
    ind:= ScreenIndex(0, Y);
    for X:= FWindMin.X to FWindMax.X do begin
      w:= ind + X;
      ScreenBuffer[w]:= ' ';
      w:= w*SizeOf(TColor);
      PColor(LongInt(ScreenForeground) + w)^:= FForeground;
      PColor(LongInt(ScreenBackground) + w)^:= FBackground;
    end;
  end;
  Rect.left:= FWindMin.X*FCharSize.X - HorzScrollBar.Position;
  Rect.top:= FWindMin.Y*FCharSize.Y - VertScrollBar.Position;
  Rect.right:= Rect.left + (FWindMax.X - FWindMin.X + 1)*FCharSize.X;
  Rect.bottom:= Rect.top + (FWindMax.Y - FWindMin.Y + 1)*FCharSize.Y;
{$ifdef VCL}
  InvalidateRect(Rect, true);
  UpdateWindow(Handle);
{$else}
  InvalidateRect(ClientRect, true);
{$endif}
end;

procedure TConsoleWnd.ClrScr;
begin
  // maximized window
  FWindMin.X:= 0;
  FWindMin.Y:= 0;
  FWindMax.X:= FScreenSize.X-1;
  FWindMax.Y:= FScreenSize.Y-1;
  ClrWindow;
end;

procedure TConsoleWnd.ClrEol;
var i : Integer;
    ShowNewText : boolean;
begin
  ShowNewText:= false;
  for i:= FCursor.X to FWindMax.X do begin
    if SaveChar(i, FCursor.Y, ' ') then
      ShowNewText:= true;
  end;
  if ShowNewText then
    ShowText(FCursor.X, FWindMax.X+1);
end;

procedure TConsoleWnd.FormResize(Sender: TObject);
begin
  HorzScrollBar.Increment:= FCharSize.x;
  VertScrollBar.Increment:= FCharSize.y;
  HorzScrollBar.Range:= Max(0, FScreenSize.X*FCharSize.x);
  VertScrollBar.Range:= Max(0, FScreenSize.Y*FCharSize.y);
  HorzScrollBar.Position:= Min(HorzScrollBar.Position, HorzScrollBar.Range);
  VertScrollBar.Position:= Min(VertScrollBar.Position, VertScrollBar.Range);
{$ifndef VCL}
  Canvas.SetClipRect(ClientRect);
{$endif}
end;

procedure TConsoleWnd.CopyTextToClipboard;
// Die gesamte Information des Fensters wird zum Clipboard übertragen.
var block_size, i, y : integer;
    block, dest : PAnsiChar;
    Last : integer;
    Gefunden : boolean;
begin
  if (ScreenBuffer = nil) then exit;

  // Leere Zeilen am Ende des Fensters werden abgeschnitten.
  Last:= FScreenSize.y;
  Gefunden:= false;
  while not Gefunden and (Last > 0) do begin
    for i:= 0 to FScreenSize.x-1 do begin
      if ScreenPtr(i, Last-1)^ <> ' ' then
        Gefunden:= true;
    end;
    if not Gefunden then
      dec(Last);
  end;

  block_size:= (FScreenSize.x + 2) * Last + 1;
  GetMem(block, block_size);
  if block <> nil then begin
    dest:= block;

    for y:= 0 to Last - 1 do begin
      for i:= 0 to FScreenSize.x-1 do
        dest[i]:= ScreenPtr(i, y)^;
      inc(dest, FScreenSize.x);
      dest^:= #13; inc(dest);
      dest^:= #10; inc(dest);
    end;
    dest^:= #0;

    Clipboard.Clear;
    Clipboard.AsText:= char(block);
    FreeMem(block);
  end;
end;

procedure TConsoleWnd.WindowScroll(Horz : boolean; Action : integer);
var
  X, Y: Integer;

  function GetNewPos(Pos, Page, FontSize, Range: Integer): Integer;
  begin
    case Action of
      Vk_Up:       Result := Pos - FontSize;
      Vk_Down:     Result := Pos + FontSize;
      Vk_Prior:    Result := Pos - FontSize*(Page div FontSize);
      Vk_Next:     Result := Pos + FontSize*(Page div FontSize);
      Vk_Home:     Result := 0;
      Vk_End:      Result := Range;
    else
      Result := Pos;
    end;
    if Result < 0 then Result:= 0;
    if Result > Range-Page then Result:= Range-Page;
  end;

begin
  X := HorzScrollBar.Position;
  Y := VertScrollBar.Position;
  if Horz then
    X := GetNewPos(X, ClientWidth div 2, FCharSize.X, HorzScrollBar.Range)
  else
    Y := GetNewPos(Y, ClientHeight, FCharSize.y, VertScrollBar.Range);
  ScrollTo(X, Y);
end;

procedure TConsoleWnd.FormKeyPress(Sender: TObject; var Key: Char);
begin
  if FKeyCount < SizeOf(FKeyBuffer) then begin
    FKeyBuffer[FKeyCount]:= AnsiChar(Key);
    Inc(FKeyCount);
  end;
end;

procedure TConsoleWnd.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
var ch : AnsiChar;
    SpecialKey : AnsiChar;
    FCtrKeyDown : boolean;
    temp : char;
begin
  ch:= #0;
  SpecialKey:= #0;
  FCtrKeyDown:= ssCtrl in Shift;
  if (Key = ord ('C')) and FCtrKeyDown then begin
    Key:= 0;
    if FCheckBreak then
      Application.Terminate
    else
      CopyTextToClipBoard;
  end;

  if FCheckBreak and (Key = Vk_Escape) then Application.Terminate;
  
  if FAutoScroll then begin
    ch:= #1;
    case Key of
      Vk_LEFT:   if FCtrKeyDown then
                   WindowScroll(true, Vk_Prior)
                 else
                   WindowScroll(true, Vk_Up);
      Vk_RIGHT:  if FCtrKeyDown then
                   WindowScroll(true, Vk_Next)
                 else
                   WindowScroll(true, Vk_Down);
      Vk_HOME:   WindowScroll(not FCtrKeyDown, Key);
      Vk_END:    WindowScroll(not FCtrKeyDown, Key);
      Vk_UP:     WindowScroll(false, Key);
      Vk_DOWN:   WindowScroll(false, Key);
      Vk_PRIOR:  WindowScroll(false, Vk_Prior);
      Vk_NEXT:   WindowScroll(false, Vk_Next);
      else ch:= #0;
    end;
    if ch <> #0 then
      exit;
  end;

  case Key of
{$ifdef VCL}
//    Vk_Escape:    SpecialKey:= #27;
    Vk_Tab:       SpecialKey:= #9;
//    Vk_Return:    SpecialKey:= #13;
{$else}
    Vk_Tab:       SpecialKey:= #9;
{$endif}

    VK_UP:    ch:= #72;
    VK_Down:  ch:= #80;
    VK_LEFT:  ch:= #75;
    VK_RIGHT: ch:= #77;
    VK_PRIOR: if FCtrKeyDown then ch:= 'v'
                             else ch:= #73;
    VK_NEXT:  if FCtrKeyDown then ch:= #132
                             else ch:= #81;
    VK_HOME:  ch:= #71;
    VK_End:   ch:= #79;
    VK_Insert: ch:= #82;
    VK_Delete: ch:= #83;

    VK_F1: if ssAlt in Shift then ch:= #129
                             else ch:= #59;
    VK_F2: if ssAlt in Shift then ch:= #130
                             else ch:= #60;
    VK_F3: ch:= #61;
    VK_F4: ch:= #62;
    VK_F5: ch:= #63;
    VK_F6: ch:= #64;
    VK_F7: ch:= #65;
    VK_F8: ch:= #66;
    VK_F9: ch:= #67;
    VK_F10: ch:= #68;
    VK_F11: ch:= #69;
    VK_F12: ch:= #70;

    ord('A')..ord('Z'):
         if ssAlt in Shift then
           ch:= AnsiChar(ord(Key) + 128)
{$ifndef VCL}
         else
         if FCtrKeyDown then
           ch:= AnsiChar(ord(Key) and $1f)
{$endif} ;
    else   begin end;
  end;
  if SpecialKey <> #0 then begin
    temp := Char(SpecialKey);
    FormKeyPress(Sender, temp);
  end else
  if (ch <> #0) and (FKeyCount+1 < SizeOF(FKeyBuffer)) then begin
    FKeyBuffer[FKeyCount]:= #0;
    FKeyBuffer[FKeyCount+1]:= Ch;
    Inc(FKeyCount, 2);
  end;
end;
{$ifdef VCL}
procedure TConsoleWnd.WindowPaint(var _Message : TWMPaint);
var X1, X2, Y1, Y2: Integer;
    PS : TPaintStruct;
begin
  DC:= _Message.DC;
  if DC = 0 then DC:= BeginPaint(Handle, PS);
  try
    if (FCharSize.x > 0) and (FCharSize.y > 0) then begin
      InitDeviceContext(true);
      X1:= Max(0, (PS.rcPaint.left + HorzScrollBar.Position) div FCharSize.X);
      X2:= (PS.rcPaint.right + FCharSize.X - 1 + HorzScrollBar.Position) div
           FCharSize.X;
      Y1:= Max(0, (PS.rcPaint.top  + VertScrollBar.Position) div FCharSize.Y);
      Y2:= (PS.rcPaint.bottom + FCharSize.Y - 1 + VertScrollBar.Position) div
           FCharSize.Y;
      while Y1 < Y2 do begin
        TextOutput(X1, Y1, X2 - X1);
        Inc(Y1);
      end;
      DoneDeviceContext(true);
    end;
  finally
    if _Message.DC = 0 then EndPaint(Handle, PS);
  end;
end;
{$else}
procedure TConsoleWnd.FormPaint(Sender: TObject);
var X1, X2, Y1, Y2: Integer;
    Rect : TRect;
begin
  if (FCharSize.x > 0) and (FCharSize.y > 0) then begin
    InitDeviceContext(true);
    Rect:= Canvas.ClipRect;
    if Rect.Left < 0 then
      Rect.Left:= 0;
    if Rect.Top < 0 then
      Rect.Top:= 0;
    if Rect.Right > Width then
      Rect.Right:= Width;
    if Rect.Bottom > Height then
      Rect.Bottom:= Height;
    X1:= Max(0, (Rect.Left + HorzScrollBar.Position) div FCharSize.X);
    X2:= (Rect.Right + FCharSize.X - 1 + HorzScrollBar.Position) div FCharSize.X;
    Y1:= Max(0, (Rect.Top + VertScrollBar.Position) div FCharSize.Y);
    Y2:= (Rect.Bottom + FCharSize.Y - 1 + VertScrollBar.Position) div FCharSize.Y;
    while Y1 < Y2 do begin
      TextOutput(X1, Y1, X2 - X1);
      Inc(Y1);
    end;
    DoneDeviceContext(true);
  end;
end;
{$endif}

procedure TConsoleWnd.CharCoordinates(var Coord : TPoint; x, y : integer);
begin
  Coord.y:= (VertScrollBar.Position + Y) div FCharSize.y + 1;
  Coord.x:= (HorzScrollBar.Position + X) div FCharSize.x + 1;
end;

procedure TConsoleWnd.FormMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if (Button = mbLeft) and (FSingleClick or (ssDouble in Shift)) then begin
    CharCoordinates(FDblClickPoint, x, y);
  end;
end;

procedure TConsoleWnd.write(const t : string);
var
  s : AnsiString;
begin
  s := AnsiString(t);
  WriteBuf(PAnsiChar(s), Length(s));
end;

procedure TConsoleWnd.write(d : extended);
begin
  write('%g', [d]);
end;

procedure TConsoleWnd.write(i : integer);
begin
  write('%d', [i]);
end;

procedure TConsoleWnd.write(p : pointer);
begin
  write('%8.8p', [p]);
end;

procedure TConsoleWnd.write(const FormatStr : string; const Args: array of const);
begin
  write(Format(FormatStr, Args));
end;

procedure TConsoleWnd.write(const Args: array of const);
var i : integer;
    s : string;
    e : extended;
begin
  for i:= Low(Args) to High(Args) do
    with Args[i] do begin
      case VType of
        vtInteger:     write(VInteger);
        vtBoolean:     if VBoolean then write('true')
                                   else write('false');
        vtChar:        begin
                         s:= char(VChar); write(s);
                       end;
        vtExtended:    write(VExtended^);
        vtString:      begin
                         s:= string(VString^); write(s);
                       end;
        vtPointer:     write(VPointer);
        vtPChar:       begin
                         s:= char(VPChar^); write(s);
                       end;
        vtObject:      begin
                         s:= VObject.ClassName; write(s);
                       end;
        vtClass:       begin
                         s:= VClass.ClassName; write(s);
                       end;
        vtWideChar:    begin
                         s:= VWideChar; write(s);
                       end;
        vtPWideChar:   begin
                         s:= VPWideChar^; write(s);
                       end;
        vtAnsiString:  begin
                         s:= string(VAnsiString^); write(s);
                       end;
        vtCurrency:    write('%m', [VCurrency^]);
      {$ifdef VER140}     // Delphi 6
        vtVariant:     write(VVariant^);
      {$endif}
        vtInterface:   write(VInterface);
        vtWideString:  begin
                         s:= WideString(VWideString^); write(s);
                       end;
        vtInt64:       begin
                         e:= VInt64^; write(e);
                       end;
        else           write('???');
      end;
    end;
end;

procedure TConsoleWnd.writeln;
begin
  write(#13#10);
end;

procedure TConsoleWnd.writeln(const t : string);
begin
  write(t);
  writeln;
end;

procedure TConsoleWnd.writeln(const FormatStr : string; const Args: array of const);
begin
  write(FormatStr, Args);
  writeln;
end;

procedure TConsoleWnd.writeln(const Args: array of const);
begin
  write(Args);
  writeln;
end;

procedure TConsoleWnd.WriteHexByte(b : integer);
begin
  write('%2.2x', [b and $ff]);
end;

procedure TConsoleWnd.WriteHexWord(w : integer);
begin
  write('%4.4x', [w and $ffff]);
end;

procedure TConsoleWnd.TimeStamp;
begin
  write('<Time: %s>', [TimeToMs(SysUtils.Time)]);
end;

procedure TConsoleWnd.FormMouseWheel(Sender: TObject; Shift: TShiftState;
  WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
begin
  if WheelDelta > 0 then
    WindowScroll(false, VK_Up)
  else
  if WheelDelta < 0 then
    WindowScroll(false, VK_Down);
end;

procedure TConsoleWnd.WM_ConsMsg(var _Message : TMessage);
var Point : TPoint;
begin
  with _Message do begin
    if (lParam = WM_LBUTTONUP) or (lParam = WM_RBUTTONUP) then begin
      if GetCursorPos(Point) and (@ConsoleCallBack <> nil) then
        ConsoleCallBack(Point.x, Point.y, FCallingC);
    end;
  end;
end;

{$ifdef VCL}
procedure TConsoleWnd.WM_MonMsg(var _Message : TMessage);
var Point : TPoint;
begin
  with _Message do begin
    if (lParam = WM_LBUTTONUP) or (lParam = WM_RBUTTONUP) then begin
      if GetCursorPos(Point) then
        CommCallBack(Point.x, Point.y, FCallingDelphi);
    end;
  end;
end;
{$endif}


///////////////////////////////////////////////////////
//                                                   //
// TMenuClass: Methoden                              //
//                                                   //
///////////////////////////////////////////////////////

constructor TMainMenuClass.Create(AOwner : TCustomForm);
begin
  AOwner.Menu:= TMainMenu.Create(AOwner);
  inherited Create(AOwner.Menu.Items);
end;

constructor TPopUpMenuClass.Create(AOwner : TCustomForm);
begin
  PopUpMenu:= TPopUpMenu.Create(AOwner);
  inherited Create(PopUpMenu.Items);
end;

destructor  TPopUpMenuClass.Destroy;
begin
  PopUpMenu.free;
end;

constructor TBasicMenuClass.Create(AOwner : TMenuItem);
begin
  FOwner:= AOwner;
end;

procedure TBasicMenuClass.SetHandlerProc(Proc : TMenuHandlerProc);
begin
  FHandlerProc:= Proc;
end;

function TBasicMenuClass.AddToItem(const Title : string; AddTo : TMenuItem) : TMenuItem;
begin
  result:= TMenuItem.Create(FOwner);
  result.Caption:= Title;
  result.OnClick:= MenuHandler;
  AddTo.Add(result);
end;

function TBasicMenuClass.AddItem(const Title : string; NotSubItem : boolean) : TMenuItem;
begin
  if NotSubItem then
    result:= AddToItem(Title, FOwner)
  else
    result:= AddToItem(Title, FOwner[FOwner.Count-1]);
end;

function TBasicMenuClass.Delete(Index : integer) : boolean;
begin
  result:= false;
  if (Index >= 0) and (Index < FOwner.Count) then begin
    FOwner.Delete(Index);
    result:= true;
  end;
end;

procedure TBasicMenuClass.MenuHandler(Sender : TObject);
var i, j : integer;
begin
  if @FHandlerProc = nil then
    exit;
  for j:= 0 to FOwner.Count-1 do begin
    if Sender = FOwner[j] then begin
      FHandlerProc(Sender as TMenuItem, j, -1);
      exit;
    end;
    for i:= 0 to FOwner[j].Count-1 do begin
      if Sender = FOwner[j][i] then begin
        FHandlerProc(Sender as TMenuItem, j, i);
        exit;
      end;
    end;
  end;
  FHandlerProc(Sender as TMenuItem, -1, -1);
end;

end.
