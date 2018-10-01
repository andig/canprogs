/*
 *
 *  Copyright (C) 2016 Jürg Müller, CH-5524
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

#if !defined(KCanLogFile_H)

  #define KCanLogFile_H

  #include "NCanUtils.h"
  #include "KStream.h"

  class KCanLogFile
  {
    private:
      unsigned count;
      unsigned pos;
      KCanFrame * Frames;
      int last_day;
      unsigned LogFileToCanFrames(const KStream & stream, KCanFrame * Frames);
    public:
    
      KCanLogFile();
      ~KCanLogFile();
      void Init();
    
      KCanFrame * BuildCanFrames(const KStream & stream);
      bool ReadFile(const char * Filename);
      unsigned GetCount() const { return count; }
      KCanFrame * GetFrames() const { return Frames; }
    
      unsigned SetPos();
      bool GetNextFrame(KCanFrame & Frame);
    
      void PrintScanTable() const;
  };
 
#endif

