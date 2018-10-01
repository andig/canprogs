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


#if !defined(elster_instance_H)

  #define elster_instance_H

  #include "KCanElster.h"

  namespace elster_instance
  {
    const char * GetErrMsg();
    const char * GetValueMsg();
    
    const char * GetType(const char * params);
    const char * GetName(const char * params);
    
    const char * GetValue(const char * params);
    bool SetValue(const char * params);
    
    const char * GetString(const char * params);
    bool SetString(const char * params);
    
    bool ClrBits(const char * params);
    bool SetBits(const char * params);    
    
    bool SetCS(void);
    bool SetCAN232(void);
    // USB2CAN von 8devices sollte verwendet werden, wenn das Device mit "ED" beginnt. (nur für Windows)
    bool SetDev(const char * dev);
    const char * ToggleTrace(void);
 
    bool InitCan();
    void Undef();
    
    bool SetSniffedFrame(const char * params);
    const char * GetSniffedValue(const char * params);
    const char * GetSniffedValue();
  };


#endif

