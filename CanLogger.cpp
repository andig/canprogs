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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__LINUX__)
  #include <unistd.h>
#endif
#include <time.h>

#include "NTypes.h"

#include "NUtils.h"
#include "KCanServer.h"
#include "KHttpClient.h"
#include "XmlParser.h"
#include "KElsterTable.h"

using namespace NUtils;


void SetLogFilename(const char * Filename, int LogTime, char * LogFile)
{
  int Day, Month, Year;
  GetDateFromDays(LogTime, Day, Month, Year);

  sprintf(LogFile, "%s_%4.4d%2.2d%2.2d", Filename, Year, Month, Day);

  printf("log file: %s\n", LogFile);
}

#if defined(__MYSQL__)
  #include "KMySql.h"
#endif

#include "KTcpCanClient.h"


unsigned HttpLen;
char HttpPut[500000];

#if defined(__CAN__)

int main(int argc, char* argv[])
{
  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;
#if defined(__TEST_MICROCHIP__) || defined(__UVR__)
  trace = true;
#endif
  
  bool elster_format = argc >= 4 &&
                       !strcmp(argv[argc-1], "elster_format"); // Testmodus
  if (elster_format)
    argc--;
#if defined(__TEST_MICROCHIP__) || defined(__UVR__)
  elster_format = false;
#endif
  
  const char * av1 = argc > 1 ? argv[1] : NULL;
  const char * av2 = argc > 2 ? argv[2] : NULL;
  const char * av3 = argc > 3 ? argv[elster_format ? 4 : 3] : NULL;
  const char * av4 = argc > 4 ? argv[elster_format ? 5 : 4] : NULL;
  const char * can_device = av1;
  bool use_xml = false;

  if (argc >= 2)
  {
    use_xml = strlen(av1) > 4 &&
              !strcmp(av1 + strlen(av1) - 4, ".xml");
  }

  char Line[256];
  char LogFile[1024];
  int  LogTime;
  bool csv   = av4 && !strcmp(av4, "csv") && !use_xml; // comma separated values
  bool use_mysql = false;
#if defined(__MYSQL__)
  use_mysql = av4 && !strcmp(av4, "mysql") && !use_xml;
#endif
  bool DoLog = av3 && !use_mysql && !use_xml;
  int  SampleTime = 10;
#if defined(__WINDOWS__)
  printf("can-bus data logger\n"
         "copyright (c) 2014 Juerg Mueller, CH-5524\n\n");
#else
  #if defined(__UVR__)
    printf("can-bus data logger for UVR1611\n"
  #else
    printf("can-bus data logger\n"
           "copyright (c) 2014 Jürg Müller, CH-5524\n\n");
  #endif
#endif
  KXmlNode Root;
  KStream http_read_str;

  const char * template_ = NULL;
  const char * mysql_put = NULL;
  const char * ip_address = NULL;
  if (argc >= 2)
  {
    if (use_xml)
    {
      if (!KParser::ParseFile(av1, &Root))
        return -1;
      if (argc > 2)
      {
        printf("too much parameters\n\n");
        csv = false;
        use_mysql = false;
        DoLog = false;
        argc = 1;
      }
    } else
    if (!KCanDriver::IsValidDevice(av1))
    {
      printf("first parameter must begin with \"can\", \"tty\" or end with \".xml\"\n\n");
      argc = 1;
    }
  }
  if (argc <= 1)
  {
    printf("usage:\n"
#if defined(__MYSQL__)
           "  can_logger <can dev> <sample time in sec> [ (<filename> | <mysql host>) [ csv | mysql ] ]\nor\n"
           "  can_logger <file.xml>\n\n"
           "example: ./can_logger can0 10 192.168.1.137 mysql\n"
           "or       ./can_logger can1 3600 canlog csv\n\n");
#else
           "can_logger <can dev> <sample time in sec> [ <filename> [ csv ] ] ]\n\n"
           "example: ./can_logger can1 3600 canlog csv\n\n");
#endif
  }
  
#if defined(__MYSQL__)
  KMySql MySql;

  if (use_xml)
  {
    use_mysql = Root.Find("mysql") != NULL;
    if (use_mysql &&
        !MySql.GetXmlParams(&Root))
      return -1;
  } else
  if (use_mysql)
    strcpy(MySql.Host, av3);
#endif

  if (!NUtils::set_term_signal())
  {
    return -1;
  }

  KHttpClient * TcpClient = NULL;

  if (use_xml)
  {
    can_device = Root.FindNodeAttr("device", "Name");
    if (!use_mysql)
    {
      mysql_put = Root.FindNodeAttr("mysql_put", "Name");
      if (!mysql_put)
        template_ = Root.FindNodeAttr("template", "Name");
    }
    if (template_ || mysql_put)
    {
      DoLog = false;
      ip_address = Root.FindNodeAttr("ip_address", "Name");
      if (!ip_address)
      {
        printf("ip_address is missing in xml\n");
        return -1;
      }
      TcpClient = new KHttpClient;

      TcpClient->mTrace = trace;
      TcpClient->SetBlockMode(bmBlocking);
      TcpClient->SetRemoteHost(ip_address);
      TcpClient->SetRemotePort(80);
    }
    av2 = Root.FindNodeAttr("sleep", "Name");
    if (!use_mysql && !template_ && !mysql_put)
      av3 = Root.FindNodeAttr("log_file", "Name");
    SampleTime = 104;
  }
#if defined(__WINDOWS__)
  if (!can_device)
    return -1;
#endif

  KCanServer Can;
  KCanFrame Frame;

  if (av2)
  {
    TInt64 sec;
    const char * t = av2;

    if (NUtils::GetInt(t, sec) &&
        !*t &&
        0 <= sec && sec <= 24*3600)
    {
      SampleTime = (int) sec;
    } else {
      printf("not valid sample time: %s\n\n", av2);
      return -1;
    }
  }

  if (DoLog)
  {
    Time(Frame.TimeStampDay, Frame.TimeStampMs);
    LogTime = Frame.TimeStampDay;
    SetLogFilename(av3, LogTime, LogFile);
  }

  if (!Can.Init(can_device))
  {
    printf("Could not initilize device \"%s\"\n\n", can_device);
    return -1;
  }

  printf("device: %s\n", Can.GetDev());
  if (Can.DriverType == NCanUtils::dt_can232_remote)
    printf("port: %d\n", Can.GetPort());
#if defined(__MYSQL__)
  if (argc > 2)
  {
    if (!DoLog)
      printf("mysql host: %s\n", MySql.Host);
  }
#endif
  printf("sample time: %d sec\n", SampleTime);
  if (csv)
    printf("use comma separated values\n");

  printf("\n");

  Can.Resume();

#if defined(__MYSQL__)
  if (use_mysql)
  {
    strcpy(MySql.Host, av3);

    // test connection
    if (!MySql.Open())
    {
      Can.Halt();

      return -1;
    }
    MySql.Close();
  }
#endif

  while (!Can.Terminated() && !NUtils::term_signal_is_set())
  {
    FILE * log = NULL;
    bool read = false;
    time_t timer;

    time(&timer);
    HttpLen = 0;
    while (Can.GetFrame(Frame))
    {
#if defined(__MYSQL__)
      if (use_mysql)
      {
        if (!MySql.Opened())
          MySql.Open();
        if (!MySql.Opened())
        {
          Can.Terminate();
          break;
        }
        MySql.LogFrame(Frame);
      }
#endif
      if (elster_format)
      {
        int elster_idx = Frame.GetElsterIdx();
        if (elster_idx >= 0 && Frame.HasValue())
        {
          char value_msg[128];
          char time_str[128];
          const ElsterIndex * elst_ind = GetElsterIndex(elster_idx);
          unsigned short val = Frame.GetValue();
            
          Frame.PrintTime(time_str, false);
            
          if (elst_ind)
            SetValueType(value_msg, elst_ind->Type, val);
          else
            sprintf(value_msg, "%d", val);
          
          printf("%s  %3.3x %4.4x %s \"%s\"\n", time_str,
                Frame.Id, elster_idx, value_msg, elst_ind ? elst_ind->Name : "?");
        }
      }
      if (mysql_put)
      {
        if (HttpLen == 0)
        {
          strcpy(HttpPut, mysql_put);
          strcat(HttpPut, ";\r\n");
          HttpLen = (unsigned) strlen(HttpPut);
        }
        char insert[256];

        Frame.FormMySqlInsert(insert);
        strcat(insert, "\r\n");
        strcpy(HttpPut + HttpLen, insert);
        HttpLen = (unsigned) strlen(HttpPut);

        if (HttpLen > High(HttpPut) - 1000)
        {
          if (!TcpClient->PutHttpStream(HttpPut))
          {
            printf("HTTP server not connected\n");
          }
          HttpLen = 0;
        }
      } else
      if (template_)
      {
        char buffer[512];
        char insert[256];

        Frame.FormHttpVariables(insert);
        sprintf(buffer, template_, insert);


        if (!TcpClient->ReadHttpStream(&http_read_str, buffer))
        {
          printf("HTTP server not connected\n");
          if (!trace)
            printf("==> %s\n", buffer);
        }
      }
      read = true;
      Frame.FormatFrame(csv, Line);
      if (DoLog)
      {
        // new day -> use new file
        if (Frame.TimeStampDay != LogTime)
        {
          LogTime = Frame.TimeStampDay;
          SetLogFilename(av3, LogTime, LogFile);

          if (log)
            fclose(log);
          log = NULL;
        }
        if (!log)
        {
          log = fopen(LogFile, "ab+");
          if (!log)
          {
            printf("log file \"%s\" not opened\n", LogFile);
            Can.Terminate();
            break;
          }
        }
        if (log)
          fwrite(Line, 1, strlen(Line), log);
      }
      // in DoLog or mysql mode: show only last frame
      if (Frame.Id & 0x60000000 || // RTR/ERR flags
          !(DoLog || use_mysql || template_ || mysql_put) || trace)
      {
        if (!elster_format)
          printf("%s", Line);
        read = false;
      }
    }
    if (log)
      fclose(log);
    if (read && !elster_format)
      printf("%s", Line);
#if defined(__MYSQL__)
    if (use_mysql)
    {
      if (MySql.Opened())
        MySql.Close();
    }
#endif
    if (mysql_put && HttpLen > 0 &&
        !TcpClient->PutHttpStream(HttpPut))
    {
      printf("HTTP server not connected\n");
    }

    time_t new_timer;
    time(&new_timer);
    int del = SampleTime - (int)(new_timer - timer);
    if (del > SampleTime)
      del = SampleTime;
    del *= 10;
    if (del <= 0)
      del = 1;
    while (del > 0 && !Can.RingBufferFull() && !NUtils::term_signal_is_set())
    {
      NUtils::SleepMs(100);
      del--;
    }
  }
  char stream[64];

  Can.Halt();

  if (TcpClient)
    delete TcpClient;

  Time(Frame.TimeStampDay, Frame.TimeStampMs);
  Frame.PrintTime(stream, false);
  printf("%s terminated\n", stream);

  return 0;
}
          

#endif


