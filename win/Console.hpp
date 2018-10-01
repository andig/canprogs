// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'Console.pas' rev: 6.00

#ifndef ConsoleHPP
#define ConsoleHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <ExtCtrls.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Console
{
//-- type declarations -------------------------------------------------------
typedef Graphics::TColor *PColor;

typedef Types::TPoint TMinMaxInfo[5];

typedef Types::TPoint *PMinMaxInfo;

typedef void __cdecl (*TConsoleCallBack)(int x, int y, void * ptr);

typedef void __fastcall (*TCommCallBack)(int x, int y, void * ptr);

class DELPHICLASS TConsoleWnd;
class PASCALIMPLEMENTATION TConsoleWnd : public Forms::TForm 
{
	typedef Forms::TForm inherited;
	
__published:
	void __fastcall FormResize(System::TObject* Sender);
	void __fastcall FormShow(System::TObject* Sender);
	void __fastcall FormKeyDown(System::TObject* Sender, Word &Key, Classes::TShiftState Shift);
	void __fastcall FormKeyPress(System::TObject* Sender, char &Key);
	void __fastcall FormMouseDown(System::TObject* Sender, Controls::TMouseButton Button, Classes::TShiftState Shift, int X, int Y);
	void __fastcall FormMouseWheel(System::TObject* Sender, Classes::TShiftState Shift, int WheelDelta, const Types::TPoint &MousePos, bool &Handled);
	
private:
	#pragma pack(push, 1)
	Types::TPoint FScreenSize;
	#pragma pack(pop)
	
	#pragma pack(push, 1)
	Types::TPoint FCursor;
	#pragma pack(pop)
	
	#pragma pack(push, 1)
	Types::TPoint FWindMin;
	#pragma pack(pop)
	
	#pragma pack(push, 1)
	Types::TPoint FWindMax;
	#pragma pack(pop)
	
	int FScrollY;
	Word FTabWidth;
	bool FChangeOemToAnsi;
	bool FNewResize;
	Graphics::TColor FForeground;
	Graphics::TColor FBackground;
	Graphics::TColor FOuterBackground;
	#pragma pack(push, 1)
	Types::TPoint FCharSize;
	#pragma pack(pop)
	
	Graphics::TColor OldBackground;
	Graphics::TColor OldForeground;
	bool FAutoTracking;
	bool FCheckEOF;
	bool FCheckBreak;
	bool FReading;
	bool FUseHalt;
	HDC DC;
	HFONT SaveFont;
	int FKeyCount;
	char FKeyBuffer[64];
	char *ScreenBuffer;
	Graphics::TColor *ScreenForeground;
	Graphics::TColor *ScreenBackground;
	void __fastcall ScrollTo(int X, int Y);
	void __fastcall CursorTo(int x, int y);
	void __fastcall ShowText(int L, int R);
	void __fastcall TextOutput(int X, int Y, int Laenge);
	void __fastcall InitDeviceContext(bool FPainting);
	void __fastcall DoneDeviceContext(bool FPainting);
	void __fastcall TrackCursor(void);
	void __fastcall UpdateCursor(void);
	char * __fastcall ScreenPtr(int X, int Y);
	bool __fastcall SaveChar(int X, int Y, char c);
	int __fastcall ColorLength(PColor PColorRef, int Len);
	void __fastcall SetScreenSize(const Types::TPoint &NewSize);
	int __fastcall ScreenIndex(int X, int Y);
	int __fastcall GetScreenSizeX(void);
	int __fastcall GetScreenSizeY(void);
	void __fastcall WindowScroll(bool Horz, int Action);
	void __fastcall InvalidateRect(const Types::TRect &Rect, bool EraseBackground);
	MESSAGE void __fastcall WindowPaint(Messages::TWMPaint &_Message);
	MESSAGE void __fastcall WM_ConsMsg(Messages::TMessage &_Message);
	MESSAGE void __fastcall WM_MonMsg(Messages::TMessage &_Message);
	
__published:
	__property bool AutoTracking = {read=FAutoTracking, write=FAutoTracking, nodefault};
	__property bool CheckBreak = {read=FCheckBreak, write=FCheckBreak, nodefault};
	__property bool CheckEOF = {read=FCheckEOF, write=FCheckEOF, nodefault};
	__property Graphics::TColor Foreground = {read=FForeground, write=FForeground, nodefault};
	__property Graphics::TColor Background = {read=FBackground, write=FBackground, nodefault};
	__property int ScreenSizeX = {read=GetScreenSizeX, nodefault};
	__property int ScreenSizeY = {read=GetScreenSizeY, nodefault};
	__property Word TabWidth = {read=FTabWidth, write=FTabWidth, nodefault};
	__property bool ChangeOemToAnsi = {read=FChangeOemToAnsi, write=FChangeOemToAnsi, nodefault};
	__property bool UseHalt = {read=FUseHalt, write=FUseHalt, nodefault};
	
public:
	bool FAutoScroll;
	bool FCursorTracking;
	#pragma pack(push, 1)
	Types::TPoint FDblClickPoint;
	#pragma pack(pop)
	
	bool FSingleClick;
	void *FCallingC;
	void *FCallingDelphi;
	void __fastcall MyDestroy(void);
	__fastcall virtual TConsoleWnd(Classes::TComponent* AOwner);
	__fastcall virtual ~TConsoleWnd(void);
	void __fastcall WriteBuf(char * Buffer, Word Count);
	Word __fastcall ReadBuf(char * Buffer, Word Count);
	void __fastcall ClrScr(void);
	void __fastcall NewScreenSize(int x, int y);
	void __fastcall WriteChar(char Ch);
	void __fastcall GotoXY(int x, int y);
	bool __fastcall KeyPressed(void);
	char __fastcall ReadKey(void);
	void __fastcall TextBackground(Graphics::TColor Color);
	void __fastcall TextColor(Graphics::TColor Color);
	void __fastcall LowVideo(void);
	void __fastcall HighVideo(void);
	void __fastcall ShowCursor(void);
	void __fastcall HideCursor(void);
	void __fastcall Window(int XMin, int YMin, int XMax, int YMax);
	void __fastcall SetNewColor(Graphics::TColor ForeGrnd, Graphics::TColor BackGrnd);
	int __fastcall WhereX(void);
	int __fastcall WhereY(void);
	void __fastcall ClrWindow(void);
	void __fastcall ClrEol(void);
	void __fastcall write(const AnsiString t)/* overload */;
	void __fastcall write(void * p)/* overload */;
	void __fastcall write(Extended d)/* overload */;
	void __fastcall write(int i)/* overload */;
	void __fastcall write(const AnsiString FormatStr, const System::TVarRec * Args, const int Args_Size)/* overload */;
	void __fastcall write(const System::TVarRec * Args, const int Args_Size)/* overload */;
	void __fastcall writeln(void)/* overload */;
	void __fastcall writeln(const AnsiString t)/* overload */;
	void __fastcall writeln(const AnsiString FormatStr, const System::TVarRec * Args, const int Args_Size)/* overload */;
	void __fastcall writeln(const System::TVarRec * Args, const int Args_Size)/* overload */;
	void __fastcall WriteHexByte(int b);
	void __fastcall WriteHexWord(int w);
	void __fastcall TimeStamp(void);
	void __fastcall CopyTextToClipboard(void);
	void __fastcall CharCoordinates(Types::TPoint &Coord, int x, int y);
	__property Types::TPoint Cursor = {read=FCursor};
public:
	#pragma option push -w-inl
	/* TCustomForm.CreateNew */ inline __fastcall virtual TConsoleWnd(Classes::TComponent* AOwner, int Dummy) : Forms::TForm(AOwner, Dummy) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TConsoleWnd(HWND ParentWindow) : Forms::TForm(ParentWindow) { }
	#pragma option pop
	
};


typedef void __fastcall (*TMenuHandlerProc)(Menus::TMenuItem* Sender, int IndexItem, int IndexSubItem);

class DELPHICLASS TBasicMenuClass;
class PASCALIMPLEMENTATION TBasicMenuClass : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	Menus::TMenuItem* FOwner;
	TMenuHandlerProc FHandlerProc;
	void __fastcall MenuHandler(System::TObject* Sender);
	
public:
	__fastcall TBasicMenuClass(Menus::TMenuItem* AOwner);
	void __fastcall SetHandlerProc(TMenuHandlerProc Proc);
	Menus::TMenuItem* __fastcall AddItem(const AnsiString Title, bool NotSubItem);
	bool __fastcall Delete(int Index);
	Menus::TMenuItem* __fastcall AddToItem(const AnsiString Title, Menus::TMenuItem* AddTo);
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TBasicMenuClass(void) { }
	#pragma option pop
	
};


class DELPHICLASS TMainMenuClass;
class PASCALIMPLEMENTATION TMainMenuClass : public TBasicMenuClass 
{
	typedef TBasicMenuClass inherited;
	
public:
	__fastcall TMainMenuClass(Forms::TCustomForm* AOwner);
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TMainMenuClass(void) { }
	#pragma option pop
	
};


class DELPHICLASS TPopUpMenuClass;
class PASCALIMPLEMENTATION TPopUpMenuClass : public TBasicMenuClass 
{
	typedef TBasicMenuClass inherited;
	
public:
	Menus::TPopupMenu* PopUpMenu;
	__fastcall TPopUpMenuClass(Forms::TCustomForm* AOwner);
	__fastcall virtual ~TPopUpMenuClass(void);
};


//-- var, const, procedure ---------------------------------------------------
static const Shortint Black = 0x0;
static const int Blue = 0x7f0000;
static const Word Green = 0x7f00;
static const int Cyan = 0x7f7f00;
static const Shortint Red = 0x7f;
static const int Magenta = 0x7f00ff;
static const Word Brown = 0x7f7f;
static const int LightGray = 0xc0c0c0;
static const int DarkGray = 0x3f3f3f;
static const int LightBlue = 0xff0000;
static const Word LightGreen = 0xff00;
static const int LightCyan = 0xffff3f;
static const Byte LightRed = 0xff;
static const int LightMagenta = 0xff00ff;
static const Word Yellow = 0xffff;
static const int White = 0xffffff;
static const Word MonWm_NOTIFY = 0x8064;
extern PACKAGE TConsoleCallBack ConsoleCallBack;
extern PACKAGE TCommCallBack CommCallBack;
extern PACKAGE AnsiString __fastcall TimeToMs(double t);

}	/* namespace Console */
using namespace Console;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// Console
