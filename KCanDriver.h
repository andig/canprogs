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

#if !defined(KCanDriver_H)

  #define KCanDriver_H

  #include "NCanUtils.h"

  class KCanDriver
  {
    public: 
      char dev[128];
      bool AsSimulation;
      bool Trace;

      KCanDriver() { AsSimulation = false; Trace = false; }
      virtual ~KCanDriver() { };

      virtual bool Init(const char * CanDev);
      virtual void Close() = 0;
      virtual bool Connect() = 0;
      virtual unsigned GetPort() const { return 0; }; // TCP/IP port

      virtual bool ReceiveData(KCanFrame & Frame) = 0;
      virtual bool ReadData(KCanFrame & Frame) = 0;
      virtual bool SendData(const KCanFrame & Frame) { return false; }
      virtual bool SendData(const KComfortFrame & Frame);
      const char * GetDev() const { return dev; }
    
      static bool IsValidDevice(const char * Dev);
  };

#endif

