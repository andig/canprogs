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

#if !defined(KElsterTable_H)

  #define KElsterTable_H

  #include "NCanUtils.h"

  typedef struct
  {
    const char * Name;
    unsigned short Index;
    unsigned char Type;
  } ElsterIndex;

  typedef enum
  {
    // Die Reihenfolge muss mit ElsterTypeStr übereinstimmen!
    et_default = 0,
    et_dec_val,       // Auflösung: xx.x / auch neg. Werte sind möglich
    et_cent_val,      // x.xx
    et_mil_val,       // x.xxx
    et_byte,
    et_bool,          // 0x0000 und 0x0001
    et_little_bool,   // 0x0000 und 0x0100
    et_double_val,
    et_triple_val,
    et_little_endian,
    et_betriebsart,
    et_zeit,
    et_datum,
    et_time_domain,
    et_dev_nr,
    et_err_nr,
    et_dev_id
  } ElsterType;

  typedef struct
  {
    unsigned short Index;
    const char * Name;
  } ErrorIndex;

  const ElsterIndex * GetElsterIndex(unsigned short Index);
  const ElsterIndex * GetElsterIndex(const char * str);
  ElsterType GetElsterType(const char * str);
  void SetValueType(char * Val, unsigned char Type, unsigned short Value);
  void SetDoubleType(char * Val, unsigned char Type, double Value);
  bool FormElsterTable(const KCanFrame & Frame, char * str);
  const char * ElsterTypeToName(unsigned Type);
  int TranslateString(const char * & str, unsigned char elster_type);


#endif

