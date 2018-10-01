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

#if !defined(UBruecke_H)

  #define UBruecke_H

  #include "NCanUtils.h"
  #include "KCanElster.h"
  #if defined(VCL)
    #include "Comm.hpp"
  #else
    #include "KComm.h"
  #endif

  extern enum t_dev {dev_nullmodem, dev_can_server, dev_can232, dev_simulation,
                     dev_usb2can, dev_can232_remote} dev;
  extern volatile bool Stop;
  extern KCanElster * USBtin_Simul;

  #if defined(__BORLANDC__) && (__BORLANDC__ >= 0x640)
    #define my_char wchar_t
  #else
    #define my_char char
  #endif
  const char * copy_tstr(const my_char * from);


  namespace Bruecke
  {
    bool LoadScanTable(const char * Filename);

    void set_log_file(bool set, const char * Filename);
    void log_frame(bool send, const KComfortFrame & frame);

    unsigned short GetKennwort();
    bool ElsterComm(const KComfortFrame & send, KComfortFrame & recv);

    bool ConnectedIndex(const KComfortFrame & cs, unsigned char & Index);
    bool NotConnected(const KComfortFrame & cs);
  #if defined(VCL)
    void Init_Simple_Com(TComm * & Com);
  #else
    void Init_Simple_Com(KComm * & Com);
  #endif
    void Init_CS_Com();
    void Init_Null_Com();
    bool InitKennwort(const char * Text);
    void WriteToCS(KComfortFrame * cs_recv, bool log);
    bool InitCom(t_dev new_dev, int sec_com_nr, int timeout_ms,
				 const char * remote_ip,
                 const char * usb2can_name,
                 bool binary_protocol,
                 void * Form = NULL);

    void CloseCom();
    bool Start(int com_nr);
    void Execute(bool & cbxUseElsterTable, bool & cbxRecvTelegram,
                 bool & cbxCanTelegram, bool & cbNotChange,
                 bool & cbxCS_Telegram, bool & cbxRecvCS);
    void Send(const KCanFrame & Frame);

    void ExitProg();
  }


#endif
