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

#if !defined(KMySql_H) && defined(__MYSQL__)

  #define KMySql_H

  #include "KStream.h"
  #include "NCanUtils.h"

  class KMySql
  {
    private:
      struct st_mysql * connection;
      bool IsOpen;

      void PrintError(const char * Msg);

    public:
      char Host[256];
      char User[256];
      char Password[256];
      char Database[256];

      KMySql();
      ~KMySql();

      bool Open();
      bool Opened();
      void Close();
      bool Query(const char * sql);
      bool Query(const KStream & stream, bool trace);
      struct st_mysql_res * GetMySqlRes();
      bool LogFrame(const KCanFrame & Frame);
      bool GetXmlParams(class KXmlNode * Root);

  };

  bool MySqlTest(KMySql & MySql);

#endif

