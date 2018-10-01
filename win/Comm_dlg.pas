unit Comm_dlg;

interface

{$ifdef LINUX}
  {$undef VCL}
{$endif}

{$define VCL}

uses
{$ifdef VCL}
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, StdCtrls;
{$else}
  // Achtung: StdCtrls darf hier nicht aufgelistet sein!
  SysUtils, Types, Classes, Qt, QGraphics, QControls, QForms, QTypes,
  QStdCtrls, StdCtrls, Controls;
{$endif}


type
  TdlgCOMM = class(TForm)
    lbPort: TLabel;
    cbPort: TComboBox;
    lpBaudrate: TLabel;
    cbBaudrate: TComboBox;
    btnOk: TButton;
    lbParity: TLabel;
    cbParity: TComboBox;
    cbStopBits: TComboBox;
    cbBits: TComboBox;
    lbStopbits: TLabel;
    lbBits: TLabel;
    cbDTR: TComboBox;
    cbRTS: TComboBox;
    lbDTR: TLabel;
    lbRTS: TLabel;
    btnCancel: TButton;
  private
    { Private-Deklarationen }
  public
    { Public-Deklarationen }
    constructor Create (AOwner: TComponent); override;
  end;

implementation

{$ifdef VCL}
  {$R *.DFM}
{$else}
  {$R *.xfm}
{$endif}

constructor TdlgCOMM.Create (AOwner: TComponent);
begin
  inherited Create (AOwner);
{$ifdef LINUX}
  Font.Name:= 'helvetica';
  Scaled:= false;
{$endif}

  if AOwner is TForm then
    Icon:= (AOwner as TForm).Icon
  else
{$ifdef VCL}
  if (Application.Icon <> nil) and (Application.Icon.Handle <> 0) then
{$else}
  if (Application.Icon <> nil) and (Application.Icon.Handle <> nil) then
{$endif}
    Icon:= Application.Icon;
end;

end.
