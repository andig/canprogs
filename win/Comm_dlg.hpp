// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'Comm_dlg.pas' rev: 6.00

#ifndef Comm_dlgHPP
#define Comm_dlgHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <StdCtrls.hpp>	// Pascal unit
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

namespace Comm_dlg
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TdlgCOMM;
class PASCALIMPLEMENTATION TdlgCOMM : public Forms::TForm 
{
	typedef Forms::TForm inherited;
	
__published:
	Stdctrls::TLabel* lbPort;
	Stdctrls::TComboBox* cbPort;
	Stdctrls::TLabel* lpBaudrate;
	Stdctrls::TComboBox* cbBaudrate;
	Stdctrls::TButton* btnOk;
	Stdctrls::TLabel* lbParity;
	Stdctrls::TComboBox* cbParity;
	Stdctrls::TComboBox* cbStopBits;
	Stdctrls::TComboBox* cbBits;
	Stdctrls::TLabel* lbStopbits;
	Stdctrls::TLabel* lbBits;
	Stdctrls::TComboBox* cbDTR;
	Stdctrls::TComboBox* cbRTS;
	Stdctrls::TLabel* lbDTR;
	Stdctrls::TLabel* lbRTS;
	Stdctrls::TButton* btnCancel;
	
public:
	__fastcall virtual TdlgCOMM(Classes::TComponent* AOwner);
public:
	#pragma option push -w-inl
	/* TCustomForm.CreateNew */ inline __fastcall virtual TdlgCOMM(Classes::TComponent* AOwner, int Dummy) : Forms::TForm(AOwner, Dummy) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomForm.Destroy */ inline __fastcall virtual ~TdlgCOMM(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TdlgCOMM(HWND ParentWindow) : Forms::TForm(ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Comm_dlg */
using namespace Comm_dlg;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// Comm_dlg
