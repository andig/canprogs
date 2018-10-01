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

#if !defined(KScanTable_H)

  #define KScanTable_H

  #include "NCanUtils.h"

  struct scan_data
  {
    unsigned short id;
    unsigned short index;
    unsigned short value;
    int Compare(const scan_data & data) const;
    void set_scan_data(const KCanFrame & Frame);
  };

  class KScanTable
  {
    public:
      static bool Init_scan_table();
      static unsigned Init_Kennwort();
      static bool Get_scan_value(const KComfortFrame & send, KComfortFrame & recv);
      static bool GetValue(unsigned short receiver_id, unsigned short elster_idx,
                           unsigned short & Value);
      static bool SetValue(const KCanFrame & frame);
      static bool LoadScanTable(const char * Filename);
      static bool SaveScanTable(const char * Filename);

      static unsigned char ShortCanId(unsigned short id);
      static unsigned short LongCanId(unsigned char id);
  };


#endif

