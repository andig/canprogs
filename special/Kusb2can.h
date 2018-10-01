/*
 *
 *  Copyright (C) 2017 Jürg Müller, CH-5524
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

//
// Dynamische Einbindung der usb2can.dll (8devices.com).
//
#if !defined(Kusb2can_H) && defined(__WINDOWS__)

  #define Kusb2can_H

  #include "../NCanUtils.h"
  #include "../KCanDriver.h"

  class Kusb2canDriver : public KCanDriver
  {
    private:
      unsigned long CanalHandle;
    public:
      Kusb2canDriver();
      virtual ~Kusb2canDriver();

      virtual bool Init(const char * CanDev); // "E12345678"
      virtual void Close();
      virtual bool Connect();

      virtual bool ReceiveData(KCanFrame & Frame);
      virtual bool ReadData(KCanFrame & Frame);
      virtual bool SendData(const KCanFrame & Frame);
      virtual bool SendData(const KComfortFrame & Frame);
  };

  bool LoadCanalLibrary();
  
#endif

