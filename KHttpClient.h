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

#if !defined(KHttpClient_H)

  #define KHttpClient_H

  #include "KStream.h"
  #include "KTcpClient.h"

  class KHttpClient : public KTcpClient
  {
    private:
      int ContentLength;

      bool TraceReceive(int Length);
    public:

      bool mJpeg;

      KHttpClient();
      bool SetRemotePath(const char * Path);
      bool HttpAnswer(KStream * aAnswer, bool & connected);
      bool ReadHttpStream(KStream * aAnswer, const char * aGetString);
      bool ReadHttpStream(KStream * aAnswer, const char * aGetString,
                          KStream * aContent);
      bool PutHttpStream(const char * aStream);

      bool ServeStream(KStream & Stream);

  };



#endif

