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

#if !defined(NCanUtils_H)

  #define NCanUtils_H

  #include <stdio.h>

  namespace NCanUtils
  {

    enum driver_type
    {
      dt_unknown,
      dt_can,    // SocketCAN: für PICAN CAN-Bus Board oder USB2CAN von 8devices.com
      dt_can232, // RS232: CAN232, CANUSB von Lawicel oder USBtin von Fischl.de
      dt_can232_remote, // CAN232 via TCP-Client
      dt_cs      // optisches Interface: ComfortSoft-Protokoll
  #if defined(__WINDOWS__)
      , dt_8dev    // Interface von 8devices.com - verwendet usb2com.dll (only for windows)
  #endif
    };
    
    enum driver_type GetDriverType(const char * dev);
  };


  struct KCanFrame
  {
    unsigned Counter;
    int      TimeStampDay;
    int      TimeStampMs;
    unsigned Id;
    int      Len;
    unsigned Flags;
    unsigned char Data[8];

    KCanFrame();
    void Init();
    bool InitElsterFrame(unsigned short sender_id,
                         unsigned short receiver_id,
                         unsigned short elster_idx);

    bool EqualData(const KCanFrame & Frame) const;
    int GetReceiverId() const;
    unsigned short GetIdToValue() const;
    bool SetReceiverId(unsigned short id);
    bool IsAnswer(const KCanFrame & Frame) const;
    bool IsAnswerToElsterIndex(const KCanFrame & Frame) const;
    bool Supported() const;
    int GetValue() const;
    bool SetValue(unsigned short value);
    bool HasValue() const;
    int GetElsterIdx() const;
    bool SetElsterIdx(unsigned short idx);
    bool Changed(const KCanFrame & SendFrame) const;
    bool GetDataFromStream(const char * str);
    void FormMySqlInsert(char * statement) const;
    void FormHttpVariables(char * stream) const;
    // format: 302.0016
    bool SetElsterFrame(const char * str);
    // format: { 0x180, 0x0001, 0x0000},
    bool SetElsterFrame2(const char * str);

    void PrintTime(char * stream, bool csv) const;
    void FormatFrame(bool csv, char * OutStr, bool UseElster = false) const;
    void PrintFrame() const;
    static void PrintDateTime(int days, int ms, char * stream);
    static void PrintMs(int days, int ms, char * stream);
    
    bool InitFromLog(const char * log);
  };

  struct KComfortFrame
  {
    typedef enum
    {
      mt_init,
      mt_get,
      mt_change,
      mt_short_change,
      mt_invalid,
    } msg_type;

    typedef struct
    {
      unsigned recv_id;
      unsigned elster_idx;
    } t_scan_geraete_id;

    unsigned char Data[12];
    static unsigned short InitId;
    static const t_scan_geraete_id scan_geraete_id[];
    static KComfortFrame InitFrame;

    unsigned GetCheckSum() const;
    unsigned GetSum() const;
    bool CheckSum() const;
    void SetCheckSum();
    bool SetCanFrame(KCanFrame & Frame) const;
    bool GetCanFrame(const KCanFrame & Frame);
    void SetOk(bool Ok = true);
    void SetOkValue(unsigned Value);
    bool EqualData(const KComfortFrame & Frame) const;
    void SetToString(char * Str) const;
    bool SetFromString(const char * Str, bool AppendCheck = true);
    bool IsInitFrame() const;
    int OkFrameValue() const;
    void SetInitOk();
    msg_type GetMsgType() const;
  };

  struct KCan232Frame
  {
    // T14611234 4 00010203             1 + 8 + 1 + 2*8 = 26 Byte max.
    // t550      8 aabbccddeeff0a0b
    char msg[32];

    KCan232Frame(const char * str = 0);
    bool SetCanFrame(KCanFrame & Frame) const;
    bool GetCanFrame(const KCanFrame & Frame);
    bool IsOk() const;
    KCan232Frame & operator =(const char * str);
  };

#endif

