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

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
//#include <system.classes.hpp>
//#include <RTLConsts.hpp>

#include "UCS_Dlg.h"
#include "NUtils.h"

#include "KScanTable.h"
#include "../special/Kusb2can.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

#include "UBruecke.h"

const char * copy_tstr(const my_char * from)
{
#if !defined(__BORLANDC__) || (__BORLANDC__ < 0x640) // XE2
  return from;
#else
  static char copy_str_char[1024];

  int i = 0;
  while (from[i])
  {
    copy_str_char[i] = (char) from[i];
    i++;
  }
  copy_str_char[i] = 0;

  return copy_str_char;
#endif
}

static void tstrcpy(char * dest, const my_char * src)
{
#if !defined(__BORLANDC__) || (__BORLANDC__ < 0x640) // XE2
  strcpy(dest, src);
#else
  while (*src)
    *dest++ = (char) *src++;
  *dest = 0;
#endif
}

__fastcall TForm1::TForm1(TComponent* Owner)
  : TForm(Owner)
{
}

void __fastcall TForm1::FormCreate(TObject *Sender)
{
  cbDevice->ItemIndex = 3;
  dev = dev_can_server;
  pcMain->ActivePage = tsStart;
  edLog->Text = ExtractFilePath(Application->ExeName) + "cs_log.txt";

  int h = cbDevice->Top - cbComm->Top;
  lbRaspi->Top = lbDevice->Top + h;
  lbSecondCom->Top = lbDevice->Top + h;
  edRaspi->Top = cbDevice->Top + h;
  cbSecondCom->Top = cbDevice->Top + h;
  cbxBinaryProtocol->Top = cbDevice->Top + 2*h - 2;
  edUSB2CAN->Top = lbDevice->Top + h - 4;
  lblUSB2CAN->Top = lbDevice->Top + h;

  cbDeviceChange(NULL);

  Bruecke::Init_CS_Com();
  flgUseElsterTable = cbxUseElsterTable->Checked;
  flgCS_Telegram = cbxCS_Telegram->Checked;
  flgCanTelegram = cbxCanTelegram->Checked;
  flgRecvTelegram = cbxRecvTelegram->Checked;
  flgRecvCS = cbxRecvCS->Checked;
  flgNotChange = cbxNotChange->Checked;
}

void __fastcall TForm1::FormShow(TObject *Sender)
{
  btnStart->SetFocus();
}

void TForm1::EnableControls(bool Ok)
{
  if (cbComm->Visible)
    cbComm->Enabled = Ok;
  if (cbDevice->Visible)
    cbDevice->Enabled = Ok;
  if (edRaspi->Visible)
    edRaspi->Enabled = Ok;
  if (cbSecondCom->Visible)
    cbSecondCom->Enabled = Ok;
  if (btnFileOpen->Visible)
    btnFileOpen->Enabled = Ok;
  if (edLog->Visible)
    edLog->Enabled = Ok;
  if (cbxBinaryProtocol->Visible)
    cbxBinaryProtocol->Enabled = Ok;
  if (edUSB2CAN->Visible)
    edUSB2CAN->Enabled = Ok;
}

void __fastcall TForm1::cbDeviceChange(TObject *Sender)
{
  if (0 <= cbDevice->ItemIndex && cbDevice->ItemIndex < 6)
  {
    dev = (t_dev) cbDevice->ItemIndex;
    bool UseCom = dev == dev_nullmodem ||
                  dev == dev_can232;
    lbRaspi->Visible = dev == dev_can_server || dev == dev_can232_remote;
    edRaspi->Visible = lbRaspi->Visible;
    cbxBinaryProtocol->Visible = dev == dev_can_server;
    lbSecondCom->Visible = UseCom;
    cbSecondCom->Visible = UseCom;
    edUSB2CAN->Visible = dev == dev_usb2can;
    lblUSB2CAN->Visible = dev == dev_usb2can;
    if (UseCom)
    {
      if (dev == dev_nullmodem)
        lbSecondCom->Caption = "2. Schnittstelle";
      else
        lbSecondCom->Caption = "can232";
    }
  }

  tsScan->TabVisible = (dev_simulation == dev);

  lbExplanation->Font->Color = clRed;
  switch (dev)
  {
    case dev_nullmodem:
      lbExplanation->Caption = "opt. interface <= COMx => CS_Bruecke <= COMx => com0com <= COMx => Com'Soft";
      break;

    case dev_can_server:
      lbExplanation->Caption = "can_server <= TCP/IP => CS_Bruecke <= COMx => com0com <= COMx => Com'Soft";
      break;

    case dev_can232:
      lbRaspi->Caption = "IP-Adresse Raspi";
      lbExplanation->Caption = "CAN232 <= com0com => CS_Bruecke <= COMx => com0com <= COMx => Com'Soft";
      break;

    case dev_simulation:
      lbExplanation->Caption = "CS_Bruecke <= COMx => com0com <= COMx => Com'Soft";
      break;

    case dev_usb2can:
      lbExplanation->Caption = "USB2CAN <= usb2can.dll => CS_Bruecke <= COMx => com0com <= COMx => Com'Soft";
      break;

    case dev_can232_remote:
      lbRaspi->Caption = "Remote IP-Adresse";
      lbExplanation->Caption = "CAN232 <= TCP/IP => CS_Bruecke <= COMx => com0com <= COMx => Com'Soft";
      break;

    default:
      lbExplanation->Caption = "";
      break;
  }
}

bool TForm1::InitKennwort()
{
  char Kenn[256];

  tstrcpy(Kenn, edVersion->Text.c_str());
  return Bruecke::InitKennwort(Kenn);
}

void __fastcall TForm1::btnStartClick(TObject *Sender)
{
  if (btnStart->Caption == "Stopp")
  {
    Stop = true;
    return;
  }

  if (dev == dev_usb2can && !LoadCanalLibrary())
  {
    ShowMessage("usb2can.dll not loaded!");
    return;
  }

  if (!InitCom())
  {
    CloseCom();
    return;
  }

  if (dev != dev_nullmodem &&
      !InitKennwort())
  {
    CloseCom();
    return;
  }
  if (!Bruecke::Start(cbComm->ItemIndex + 1))
    return;

#if defined(__USBTIN__)
  if (cbxUSBtin->ItemIndex > 0 && !USBtin_Simul)
  {
	USBtin_Simul = new KCanElster;
	char USBtin[256];

	tstrcpy(USBtin, cbxUSBtin->Text.c_str());
	if (!USBtin_Simul->InitUSBtin(USBtin, true))
    {
      CloseCom();
      return;
    }
  }
#endif

  btnStart->Caption = "Stopp";
  EnableControls(false);
  cbxLogClick(NULL);

  Bruecke::Execute(flgUseElsterTable,
                   flgRecvTelegram,
                   flgCanTelegram,
                   flgNotChange,
                   flgCS_Telegram,
                   flgRecvCS);

  btnStart->Caption = "Start";
  Bruecke::set_log_file(false, "");
  EnableControls(true);
}


void TForm1::CloseCom()
{
  Bruecke::CloseCom();
}

bool TForm1::InitCom()
{
  t_dev new_dev = (t_dev) cbDevice->ItemIndex;
  TInt64 timeout_ms = 500;

  if (new_dev == dev_can_server)
  {
    char time[256];
    tstrcpy(time, edTimeout->Text.c_str());
    const char * timeout = time;
    while (timeout && *timeout == ' ')
      timeout++;
    if (timeout &&
        NUtils::GetInt(timeout, timeout_ms))
    {
      while (*timeout  && (unsigned char) *timeout <= ' ')
        timeout++;
      if (*timeout)
        timeout_ms = 500;
    } else
      timeout_ms = 500;
    if (timeout_ms < 10)
      timeout_ms = 10;
    if (timeout_ms > 2000)
      timeout_ms = 2000;
    char str[32];
    sprintf(str, "%d", (int)timeout_ms);
    edTimeout->Text = str;
  }
  bool Ok;
  try
  {
    char Raspi[256];
    char usb2can[256];

    tstrcpy(Raspi, edRaspi->Text.c_str());
    tstrcpy(usb2can, edUSB2CAN->Text.c_str());
    Ok = Bruecke::InitCom(new_dev,
                          cbSecondCom->ItemIndex + 1,
                          timeout_ms,
                          Raspi,
                          usb2can,
                          cbxBinaryProtocol->Checked,
                          this);
  }
  catch (Exception &exception)
  {
    Ok = false;
    Application->ShowException(&exception);
  }
  return Ok;
}

void __fastcall TForm1::FormCloseQuery(TObject *Sender, bool &CanClose)
{
  CanClose = cbComm->Enabled;
}

void __fastcall TForm1::btnFileOpenClick(TObject *Sender)
{
  OpenDialog->FileName = edLog->Text;
  if (OpenDialog->Execute())
  {
    edLog->Text = OpenDialog->FileName;
  }
}

void __fastcall TForm1::btnLoadScanClick(TObject *Sender)
{
  OpenDialog->FileName = edScanFile->Text;
  if (OpenDialog->Execute())
  {
    edScanFile->Text = OpenDialog->FileName;
  }
}

void __fastcall TForm1::btnLoadClick(TObject *Sender)
{
  char ScanFile[256];

  tstrcpy(ScanFile, edScanFile->Text.c_str());
  if (Bruecke::LoadScanTable(ScanFile))
  {
    InitKennwort();
  }
}

void __fastcall TForm1::btnSaveClick(TObject *Sender)
{
  char Filename[1024];
  char name[32];
  int len;
  int nr = 1;

  strcpy(Filename, copy_tstr(ExtractFilePath(Application->ExeName).c_str()));
  len = (int) strlen(Filename);

  do
  {
    Filename[len] = 0;
    sprintf(name, "scan_data_%d.inc", nr);
    strcat(Filename, name);
    nr++;
  } while (FileExists(Filename));

  if (KScanTable::SaveScanTable(Filename))
    printf("Tabelle in \"%s\" gespeichert\n", Filename);
  else
    printf("Tabelle nicht gespeichert\n");
}

void __fastcall TForm1::cbxLogClick(TObject *Sender)
{
  Bruecke::set_log_file(cbxLog->Checked, copy_tstr(edLog->Text.c_str()));
}

void __fastcall TForm1::FormDestroy(TObject *Sender)
{
  Bruecke::ExitProg();
}

void __fastcall TForm1::cbxUseElsterTableClick(TObject *Sender)
{
  flgUseElsterTable = cbxUseElsterTable->Checked;
} 

void __fastcall TForm1::cbxCS_TelegramClick(TObject *Sender)
{
  flgCS_Telegram = cbxCS_Telegram->Checked;
}

void __fastcall TForm1::cbxRecvCSClick(TObject *Sender)
{
  flgRecvCS = cbxRecvCS->Checked;
}

void __fastcall TForm1::cbxCanTelegramClick(TObject *Sender)
{
  flgCanTelegram = cbxCanTelegram->Checked;
}

void __fastcall TForm1::cbxRecvTelegramClick(TObject *Sender)
{
  flgRecvTelegram = cbxRecvTelegram->Checked;
}

void __fastcall TForm1::cbxNotChangeClick(TObject *Sender)
{
  flgNotChange = cbxNotChange->Checked;
}

void __fastcall TForm1::btnSendClick(TObject *Sender)
{
  KCanFrame Frame;
  unsigned short recv_id;
  unsigned short elster_idx;
  unsigned short value;
  bool Ok;
  const char * str = copy_tstr(edSend->Text.c_str());

  recv_id = NUtils::GetHex(str);
  Ok = *str == '.';
  if (Ok)
  {
    str++;
    elster_idx = NUtils::GetHex(str);
    Ok = *str == '.';
  }
  if (Ok)
  {
    str++;
    value = NUtils::GetHex(str);
  }
  if (!Ok)
    return;

  Frame.InitElsterFrame(recv_id, 0x680, elster_idx);
  Frame.SetValue(value);
  Frame.Data[0] &= 0xf0;
  Frame.Data[0] += 2;

  Bruecke::Send(Frame);
}


