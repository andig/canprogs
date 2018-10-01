//---------------------------------------------------------------------------

#ifndef UCS_DlgH
#define UCS_DlgH
#if (__BORLANDC__ < 0x640)
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <StdCtrls.hpp>
#else
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Dialogs.hpp>
#endif
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// Von der IDE verwaltete Komponenten
  TPageControl *pcMain;
  TTabSheet *tsStart;
  TLabel *lbComm;
  TComboBox *cbComm;
  TComboBox *cbDevice;
  TLabel *lbDevice;
  TLabel *lbRaspi;
  TLabel *lbSecondCom;
  TEdit *edRaspi;
  TComboBox *cbSecondCom;
  TButton *btnStart;
  TTabSheet *tsFilter;
  TCheckBox *cbxUseElsterTable;
  TCheckBox *cbxCS_Telegram;
  TCheckBox *cbxCanTelegram;
  TCheckBox *cbxRecvTelegram;
  TCheckBox *cbxLog;
  TEdit *edLog;
  TOpenDialog *OpenDialog;
  TButton *btnFileOpen;
  TCheckBox *cbxNotChange;
  TTabSheet *tsScan;
  TLabel *lbScanFile;
  TEdit *edScanFile;
  TButton *btnLoadScan;
  TButton *btnLoad;
  TButton *btnSave;
  TTabSheet *tsExtras;
  TLabel *lbTimeout;
  TEdit *edTimeout;
  TEdit *edVersion;
  TLabel *lbVersion;
  TLabel *lbExplanation;
  TCheckBox *cbxRecvCS;
  TCheckBox *cbxBinaryProtocol;
  TComboBox *cbxUSBtin;
  TLabel *lbUSBtin;
  TEdit *edSend;
  TButton *btnSend;
  TEdit *edUSB2CAN;
  TLabel *lblUSB2CAN;
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall cbDeviceChange(TObject *Sender);
  void __fastcall btnStartClick(TObject *Sender);
  void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
  void __fastcall btnFileOpenClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall btnLoadScanClick(TObject *Sender);
  void __fastcall btnLoadClick(TObject *Sender);
  void __fastcall btnSaveClick(TObject *Sender);
  void __fastcall cbxLogClick(TObject *Sender);
  void __fastcall FormDestroy(TObject *Sender);
  void __fastcall cbxUseElsterTableClick(TObject *Sender);
  void __fastcall cbxCS_TelegramClick(TObject *Sender);
  void __fastcall cbxRecvCSClick(TObject *Sender);
  void __fastcall cbxCanTelegramClick(TObject *Sender);
  void __fastcall cbxRecvTelegramClick(TObject *Sender);
  void __fastcall cbxNotChangeClick(TObject *Sender);
  void __fastcall btnSendClick(TObject *Sender);
private:	// Anwender-Deklarationen
  unsigned short Kennwort;
  bool InitKennwort();
  void WriteToCS(struct KComfortFrame * cs_recv);
public:		// Anwender-Deklarationen
  bool flgUseElsterTable;
  bool flgCS_Telegram;
  bool flgCanTelegram;
  bool flgRecvTelegram;
  bool flgRecvCS;
  bool flgBinaryProtocol;
  bool flgNotChange;
  __fastcall TForm1(TComponent* Owner);
  void EnableControls(bool Ok);
  bool InitCom();
  void CloseCom();
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------

#endif
