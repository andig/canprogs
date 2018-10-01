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

#if !defined(KTcpCanClient_H)

  #define KTcpCanClient_H

  #include "KStream.h"
  #include "KTcpClient.h"
  #include "NCanUtils.h"

  class KTcpCanClient : public KTcpClient
  {
    public:
      bool UseBinaryProtocol; // wie das optische Interface
      bool IsServer;

      KTcpCanClient();
      bool ReadElster(char * Buffer);
      bool ReadElster(KComfortFrame & recv);
      bool SendElster(char * Buffer, int timeout);
      bool SendElster(const KComfortFrame & send, int timeout);
      bool SendComfortFrame(const KComfortFrame & send, KComfortFrame & recv);

  };



#endif

