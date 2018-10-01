// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'Comm.pas' rev: 6.00

#ifndef CommHPP
#define CommHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Console.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <ShellAPI.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Comm
{
//-- type declarations -------------------------------------------------------
typedef Word MyBOOL;

#pragma option push -b-
enum TEvent { T_EV_BREAK, T_EV_CTS, T_EV_DSR, T_EV_ERR, T_EV_RING, T_EV_RLSD };
#pragma option pop

typedef Set<TEvent, T_EV_BREAK, T_EV_RLSD>  SEvent;

class DELPHICLASS TComm;
class PASCALIMPLEMENTATION TComm : public System::TObject 
{
	typedef System::TObject inherited;
	
public:
	void __fastcall FormKeyPress(System::TObject* Sender, char &Key);
	void __fastcall mnConfigClick(System::TObject* Sender);
	void __fastcall mnShowClick(System::TObject* Sender);
	
private:
	int FComPortNr;
	int FBaudrate;
	int FParity;
	int FByteSize;
	int FStopBits;
	bool FDTR;
	bool FRTS;
	bool FRtsFlowControl;
	SEvent FEventMask;
	unsigned FEvMask;
	bool FInDialog;
	int FInCommMonitor;
	System::TDateTime FLastStamp;
	_OVERLAPPED osWrite;
	_OVERLAPPED osRead;
	_OVERLAPPED osPostEvent;
	unsigned FCOM_Handle;
	#pragma pack(push, 1)
	_DCB FDCB;
	#pragma pack(pop)
	
	int FMonitorBuffer[1024];
	int FMonitorBufferIn;
	int FMonitorBufferOut;
	char FReadBuffer[100];
	int FReadBufferLen;
	int FReadBufferPtr;
	Extctrls::TTimer* FTimer;
	_NOTIFYICONDATAA FNotifyData;
	Console::TConsoleWnd* FConsoleWnd;
	void __fastcall SetFlags(unsigned Info, unsigned Mask, int Bit);
	void __fastcall SetEventMask(SEvent EventMask);
	
public:
	bool ErrorOccured;
	bool DezMonitor;
	bool HexMonitor;
	bool LFMonitor;
	Console::TPopUpMenuClass* pmLeiste;
	bool UseTimeStamp;
	__fastcall TComm(Classes::TComponent* AOwner);
	__fastcall virtual ~TComm(void);
	void __fastcall MyDestroy(void);
	void __fastcall ChangeCaption(const AnsiString Titel);
	void __fastcall ExitCOM(void);
	bool __fastcall COM_INITIALISIERT(void);
	Word __fastcall SetComParameters(void);
	void __fastcall SetComPortNr(int Nr);
	void __fastcall SetBaudrate(int Baud);
	Word __fastcall InitCOM(void);
	int __fastcall GetMonitorChar(void);
	void __fastcall PutMonitorBuffer(int ch);
	int __fastcall ReadFromCOM(void);
	void __fastcall PutBackToReadBuffer(void);
	int __fastcall ReadWithTimeOut(int TimeMs);
	void __fastcall ClearBuffer(void);
	Word __fastcall WriteToCOM(char ch);
	Word __fastcall WriteStringToCOM(const AnsiString str);
	Word __fastcall WriteBufferToCOM(char * str, int Len);
	Word __fastcall WriteCS(const AnsiString str);
	Word __fastcall WriteCS_OK(void);
	bool __fastcall ParameterDialog(void);
	int __fastcall GetTimerInterval(void);
	void __fastcall SetTimerInterval(int NewInterval);
	int __fastcall SendBreak(int Duration);
	int __fastcall GetCommModemStatus(void);
	unsigned __fastcall WaitCommEvent(void);
	unsigned __fastcall Event(void);
	bool __fastcall CTS(void);
	bool __fastcall DSR(void);
	bool __fastcall RING(void);
	bool __fastcall DCD(void);
	void __fastcall SetDTR(bool NewDTR);
	void __fastcall SetRTS(bool NewRTS);
	void __fastcall SetRtsFlowControl(bool NewRTS);
	void __fastcall SetBits(int NewBits);
	void __fastcall SetStopBits(int NewStopBits);
	void __fastcall SetParity(int NewParity);
	void __fastcall DelayMs(int TimeMs);
	void __fastcall CommMonitor(System::TObject* Sender);
	void __fastcall TimeStamp(void);
	__property int TimerInterval = {read=GetTimerInterval, write=SetTimerInterval, nodefault};
	void __fastcall NewScreenSize(int x, int y);
	void __fastcall Show(void);
	void __fastcall WriteChar(char ch);
	void __fastcall write(const AnsiString t)/* overload */;
	void __fastcall write(const char * t)/* overload */;
	void __fastcall writeln(void)/* overload */;
	void __fastcall writeln(const char * t)/* overload */;
	void __fastcall WriteTime(System::TDateTime Time);
	void __fastcall WriteNow(void);
	
__published:
	__property int Baudrate = {read=FBaudrate, write=SetBaudrate, nodefault};
	__property int Bits = {read=FByteSize, write=SetBits, nodefault};
	__property int ComPortNr = {read=FComPortNr, write=SetComPortNr, nodefault};
	__property int StopBits = {read=FStopBits, write=SetStopBits, nodefault};
	__property bool DTR = {read=FDTR, write=SetDTR, nodefault};
	__property int Parity = {read=FParity, write=SetParity, nodefault};
	__property bool RTS = {read=FRTS, write=SetRTS, nodefault};
	__property bool RtsFlowControl = {read=FRtsFlowControl, write=SetRtsFlowControl, nodefault};
	__property SEvent EventMask = {read=FEventMask, write=SetEventMask, nodefault};
};


//-- var, const, procedure ---------------------------------------------------
static const unsigned COM_HANDLE_CLOSE = 0xffffffff;
static const Word RXQUEUE = 0x200;
static const Word TXQUEUE = 0x200;
static const Word DefaultBaudRate = 0x12c0;
static const Word _MaxMonitorBuffer = 0x400;
static const Shortint _MaxReadBuffer = 0x64;
static const Word _NULL_CHAR = 0x1000;
extern PACKAGE bool HideError;
extern PACKAGE Word __fastcall MessageLoop(Word Save);
extern PACKAGE void __fastcall DelayMs(int TimeMs);
extern PACKAGE Word __fastcall CRC(char * Buffer, int Length);

}	/* namespace Comm */
using namespace Comm;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// Comm
