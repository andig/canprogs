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

#if !defined(KCanUVR_H) && defined(__UVR__)

  #define KCanUVR_H

  #include "KCanServer.h"

  class KCanUVR : public KCanServer
  {
    private:
      KCanFrame sniffed_frame[5][64];

      void InitSendFrame(unsigned short receiver_id, unsigned short elster_idx);

    public:
      bool Trace;

      KCanUVR();

      bool Init(const char * CanDev);
      bool AddToSniffed(const KCanFrame & Frame);
      void EmptyServer();
      const KCanFrame * GetSniffFrame(unsigned can_id);

      static int sniff_index(unsigned can_id);
      static bool GetIdAndChan(const char * can_val, unsigned & can_id, int & chan);
  };


#endif

