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

// sudo apt-get install libmysqlclient-dev mysql-client
//
/*

 synology: /usr/syno/mysql/bin/mysql -uroot -p

 bind-address    = 192.168.1.37 in /etc/mysql/my.cnf mit "nano" setzen

DROP TABLE can_log;

CREATE TABLE can_log
(
  id          INT     UNSIGNED NOT NULL AUTO_INCREMENT,
  counter     INT     UNSIGNED NOT NULL,
  timestamp   BIGINT  NOT NULL,
  can_id      INT     UNSIGNED NOT NULL,
  d0          TINYINT UNSIGNED NULL,
  d1          TINYINT UNSIGNED NULL,
  d2          TINYINT UNSIGNED NULL,
  d3          TINYINT UNSIGNED NULL,
  d4          TINYINT UNSIGNED NULL,
  d5          TINYINT UNSIGNED NULL,
  d6          TINYINT UNSIGNED NULL,
  d7          TINYINT UNSIGNED NULL,
  PRIMARY KEY (id),
  UNIQUE KEY unique_properties (timestamp, can_id)
);

 */

#if defined(__MYSQL__)

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__WINDOWS__)
  #include <windows.h>
#endif

#include <mysql/mysql.h>

#include "NTypes.h"

#include "XmlParser.h"
#include "NUtils.h"

#include "KMySql.h"

using namespace NUtils;

#if !defined(__STATIC_MYSQL__)

#include <dlfcn.h>

extern "C"
{
  typedef MYSQL * STDCALL (* t_mysql_init)(MYSQL *mysql);

  typedef unsigned int STDCALL (* t_mysql_errno) (MYSQL *mysql);
  typedef const char * STDCALL (* t_mysql_error) (MYSQL *mysql);
  typedef MYSQL *		STDCALL (* t_mysql_real_connect) (MYSQL *mysql, const char *host,
                                       const char *user,
                                       const char *passwd,
                                       const char *db,
                                       unsigned int port,
                                       const char *unix_socket,
                                       unsigned long clientflag);
  typedef int		STDCALL (* t_mysql_select_db) (MYSQL *mysql, const char *db);
  typedef int		STDCALL (* t_mysql_query) (MYSQL *mysql, const char *q);
  typedef int		STDCALL (* t_mysql_send_query) (MYSQL *mysql, const char *q,
                                 unsigned long length);
  typedef int		STDCALL (* t_mysql_real_query) (MYSQL *mysql, const char *q,
                                 unsigned long length);
  typedef MYSQL_RES *     STDCALL (* t_mysql_store_result) (MYSQL *mysql);
  typedef MYSQL_RES *     STDCALL (* t_mysql_use_result)(MYSQL *mysql);
  typedef void STDCALL (* t_mysql_close) (MYSQL *sock);

  typedef my_ulonglong STDCALL (* t_mysql_num_rows) (MYSQL_RES *res);
  typedef unsigned int STDCALL (* t_mysql_num_fields) (MYSQL_RES *res);
  typedef my_bool STDCALL (* t_mysql_eof) (MYSQL_RES *res);
  typedef MYSQL_FIELD *STDCALL (* t_mysql_fetch_field_direct) (MYSQL_RES *res,
                                                unsigned int fieldnr);
  typedef MYSQL_FIELD * STDCALL (* t_mysql_fetch_fields) (MYSQL_RES *res);
  typedef MYSQL_ROW_OFFSET STDCALL (* t_mysql_row_tell) (MYSQL_RES *res);
  typedef MYSQL_FIELD_OFFSET STDCALL (* t_mysql_field_tell) (MYSQL_RES *res);
  typedef MYSQL_ROW	STDCALL (* t_mysql_fetch_row) (MYSQL_RES *result);

  t_mysql_init p_mysql_init = NULL;

  t_mysql_errno p_mysql_errno = NULL;
  t_mysql_error p_mysql_error = NULL;
  t_mysql_real_connect p_mysql_real_connect = NULL;
  t_mysql_select_db p_mysql_select_db = NULL;
  t_mysql_query p_mysql_query = NULL;
  t_mysql_send_query p_mysql_send_query = NULL;
  t_mysql_real_query p_mysql_real_query = NULL;
  t_mysql_store_result p_mysql_store_result = NULL;
  t_mysql_use_result p_mysql_use_result = NULL;
  t_mysql_close p_mysql_close = NULL;

  t_mysql_num_rows p_mysql_num_rows = NULL;
  t_mysql_num_fields p_mysql_num_fields = NULL;
  t_mysql_eof p_mysql_eof = NULL;
  t_mysql_fetch_field_direct p_mysql_fetch_field_direct = NULL;
  t_mysql_fetch_fields p_mysql_fetch_fields = NULL;
  t_mysql_row_tell p_mysql_row_tell = NULL;
  t_mysql_field_tell p_mysql_field_tell = NULL;
  t_mysql_fetch_row p_mysql_fetch_row = NULL;
};

void * LoadSoInstance = NULL;

#define LoadProc(name) \
  if (Ok)              \
  {                    \
    p_##name = (t_##name) dlsym(LoadSoInstance, #name); \
    Ok = p_##name != NULL;   \
    if (!Ok)                 \
      printf("mysql methode \"%s\" not loaded\n", #name);  \
  }


bool LoadSo()
{
  if (LoadSoInstance)
    return true;

  const char * libName = "libmysqlclient.so";

  LoadSoInstance = dlopen(libName, RTLD_NOW);
  if (!LoadSoInstance)
  {
    printf("%s not found\n", libName);
    return false;
  }

  bool Ok = true;

  LoadProc(mysql_init)

  LoadProc(mysql_errno)
  LoadProc(mysql_error)
  LoadProc(mysql_real_connect)
  LoadProc(mysql_select_db)
  LoadProc(mysql_query)
  LoadProc(mysql_send_query)
  LoadProc(mysql_real_query)
  LoadProc(mysql_store_result)
  LoadProc(mysql_use_result)
  LoadProc(mysql_close)

  LoadProc(mysql_num_rows)
  LoadProc(mysql_num_fields)
  LoadProc(mysql_eof)
  LoadProc(mysql_fetch_field_direct)
  LoadProc(mysql_fetch_fields)
  LoadProc(mysql_row_tell)
  LoadProc(mysql_field_tell)
  LoadProc(mysql_fetch_row)

  return Ok;
}

#else

#define p_mysql_init          mysql_init

#define p_mysql_errno         mysql_errno
#define p_mysql_error         mysql_error
#define p_mysql_real_connect  mysql_real_connect
#define p_mysql_select_db     mysql_select_db
#define p_mysql_query         mysql_query
#define p_mysql_send_query    mysql_send_query
#define p_mysql_real_query    mysql_real_query
#define p_mysql_store_result  mysql_store_result
#define p_mysql_use_result    mysql_use_result
#define p_mysql_close         mysql_close

#define p_mysql_num_rows      mysql_num_rows
#define p_mysql_num_fields    mysql_num_fields
#define p_mysql_eof           mysql_eof
#define p_mysql_fetch_field_direct mysql_fetch_field_direct
#define p_mysql_fetch_fields  mysql_fetch_fields
#define p_mysql_row_tell      mysql_row_tell
#define p_mysql_field_tell    mysql_field_tell
#define p_mysql_fetch_row     mysql_fetch_row

bool LoadSo()
{
  return true;
}

#endif

KMySql::KMySql()
{
  connection = NULL;
  IsOpen = false;

  strcpy(Host,     "localhost");
  strcpy(User,     "pi");
  strcpy(Password, "raspberry");
  strcpy(Database, "canlog");
}

KMySql::~KMySql()
{
  Close();
}

void KMySql::PrintError(const char * Msg)
{
  printf("%s: %s\n", Msg, p_mysql_error(connection));
}

bool KMySql::Open()
{
  if (!LoadSo())
    return false;

  if (!connection)
    connection = p_mysql_init(NULL);

  if (!IsOpen && connection)
  {
    if (!p_mysql_real_connect(connection,Host,User,Password,Database,0,NULL,0))
    {
      PrintError("mysql_real_connect");
      p_mysql_close(connection);
      connection = NULL;
    } else
      IsOpen = true;
  }

  return Opened();
}

bool KMySql::Opened()
{
  return IsOpen && connection;
}

void KMySql::Close()
{
  if (connection)
    p_mysql_close(connection);

  IsOpen = false;
  connection = NULL;
}

bool KMySql::Query(const char * Sql)
{
  if (!Opened())
    return false;

  int res = p_mysql_query(connection, Sql);
  if (res)
  {
    PrintError("mysql_query");
    return false;
  }
  return true;
}

struct st_mysql_res * KMySql::GetMySqlRes()
{
  if (!IsOpen)
    return NULL;

  return p_mysql_store_result(connection);
}

bool KMySql::LogFrame(const KCanFrame & Frame)
{
  if (!IsOpen)
    return false;

  char statement[1024];
  Frame.FormMySqlInsert(statement);

  return Query(statement);
}

#if (0)

bool MySqlTest(KMySql & MySql)
{
  if (!MySql.Open())
    return false;

  if (!MySql.Query("SELECT * FROM can_log"))
    return false;

  st_mysql_res * res_set = MySql.GetMySqlRes();

  int num_fields = p_mysql_num_fields(res_set);

  MYSQL_ROW row;
  while ((row = p_mysql_fetch_row(res_set)))
  {
    for (int i = 0; i < num_fields && row[i] != NULL; i++)
    {
      if (i)
        printf(",");
      if (!strcmp(res_set->fields[i].name, "timestamp"))
      {
        char time_str[64];
        long long Ms = atoll(row[i]);
        if (Ms < 0) Ms = 0;
        int days = (int)(Ms / 86400000);
        int ms = (int)(Ms % 86400000);

        days -= (2000-1970)*365 + (2000-1970)/4 + 1;

        NCanUtils::PrintTime(days, ms, time_str, false);
        printf("%s", time_str);
      } else
        printf("%s", row[i]);
    }
    printf("\n");
  }
  MySql.Close();

  return true;
}

#endif

bool KMySql::GetXmlParams(KXmlNode * Root)
{
  const char * ip_address = Root->FindNodeAttr("mysql", "Host");
  if (!ip_address)
    ip_address = Root->FindNodeAttr("ip_address", "Name");

  if (!ip_address)
  {
    printf("no ip address is specified\n");
    return false;
  }
  strcpy(Host, ip_address);

  const char * par = Root->FindNodeAttr("mysql", "User");
  if (par)
    strcpy(User, par);

  par = Root->FindNodeAttr("mysql", "Password");
  if (!par)
    par = Root->FindNodeAttr("mysql", "PW");
  if (par)
    strcpy(Password, par);

  par = Root->FindNodeAttr("mysql", "DB");
  if (!par)
    par = Root->FindNodeAttr("mysql", "Database");
  if (par)
    strcpy(Database, par);

  bool Ok = Open();
  if (!Ok)
  {
    printf("can not open database \"%s\" (Host: %s  User: %s,  Password: %s)\n",
           Database, Host, User, Password);
  } else
    Close();

  return Ok;
}

bool KMySql::Query(const KStream & stream, bool trace)
{
  bool Ok = Query((const char *)stream.GetMemory());
  if (!Ok)
    printf("error in query %s\n", stream.GetMemory());
  else
  if (trace)
    printf("query: %s\n", stream.GetMemory());

  return Ok;
}

#endif

