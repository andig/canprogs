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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "NTypes.h"

#include "NCgi.h"

#include "KStream.h"
#include "NUtils.h"
#include "KElsterTable.h"
#include "KCanElster.h"

const char * KEnv::null_str = "";

KEnv::KEnv()
{
  content_length = null_str;
  content_type = null_str;
  query_string = NULL;
}

KEnv::~KEnv()
{
  if (query_string)
    delete [] query_string;
}

const char * KEnv::GetEnv(const char * name) const
{
  const char * res = getenv(name);
  return res ? res : null_str;
}

void KEnv::Init()
{
  content_length = GetEnv("CONTENT_LENGTH");
  content_type = GetEnv("CONTENT_TYPE");
  if (query_string)
    delete [] query_string;

  const char * query = GetEnv("QUERY_STRING");
  long len = query ? strlen(query) : 0;

  query_string = new char [len + 1];
  if (query)
    strcpy((char *) query_string, query);
  ((char *)query_string)[len+1] = 0;
  for (long i = 0; i < len; i++)
    if (query_string[i] == '&')
      ((char *)query_string)[i] = 0;
}

const char * KEnv::get_query(const char * name) const
{
  if (!query_string || !name || !*name)
    return NULL;

  const char * ptr = query_string;
  long str_len = strlen(name);
  while (*ptr)
  {
    if (!strncmp(ptr, name, str_len))
    {
      if (ptr[str_len] == '=')
        return ptr + str_len + 1;

      if (ptr[str_len] == 0)
        return ptr + str_len;
    }
    ptr += strlen(ptr) + 1;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

static KEnv env;
static KStream content;

static void output(const char * msg, const char * text_type = "plain")
{
  // nano /var/log/apache2/error.log
  // put cgi_template.html ../cgi-bin
  printf("Query-String: %s\r\n", env.query_string);
  printf("Content-Language: de\r\n");
  printf("Content-Type: text/%s\r\n", text_type);
  printf("Content-Length: %d\r\n", (int) strlen(msg));
  printf("\r\n");
  printf("%s", msg);
}


#if defined(__CGI__)

int main(int argc, char* argv[])
{
  KStream html;
  const char html_file[] = "cgi_template.html";
  char buffer[1024];

  env.Init();
  if (env.content_length &&
      strcmp(env.content_length, "0"))
  {
    TInt64 Len;
    const char * p = env.content_length;
    if (NUtils::GetInt(p, Len) &&
        Len > 0)
    {
      unsigned len = (unsigned) Len;
      content.SetSize(len+1);
      if (content.GetMemory() &&
          (int) fread((char *) content.GetMemory(), 1, len, stdin) == (int) len)
      {
        ((char *) content.GetMemory())[len] = 0;
      } else
        content.SetSize(0);
    }
  }

  if (content.Size())
  {
    content.ReplaceBytes((const unsigned char *) "content:<br>", 0, 12);
    output((const char *)content.GetMemory());
    return -1;
  }

  char path[1024];
  int p = 0;
  strcpy(path, argv[0]);
  for (int i = (int) strlen(path)-1; i >= 0; i--)
    if (path[i] == '/' || path[i] == '\\' || path[i] == ':')
    {
      p = i+1;
      break;
    }
  path[p] = 0;
  strcat(path, html_file);

  if (!html.ReadFile(path))
  {
    sprintf(buffer, "\"%s\" not found\n", path);
    output(buffer);

    return -1;
  }

  KCanElster Can;
#if defined(__WINDOWS__)
  Can.driver_type = KCanServer::dt_cs; // optisches Interface
  if (!Can.Init("COM3", 0x680))
#else
  if (!Can.Init("can0", 0x780))
#endif
  {
    sprintf(buffer, "device \"%s\" not connected\n", Can.GetDev());
    output(buffer);

    Can.Halt();

    return -1;
  }
  Can.Resume();

  long scan_pos = 0;
  do
  {
    const char * param = strstr((const char *) html.GetMemory() + scan_pos, "${");
    if (!param)
      break;

    scan_pos = param - (const char *) html.GetMemory();
    const char * ende = strstr((const char *) param, "}");
    if (!ende)
      break;
    long old_len = ende - param + 1;
    long offset = param - (const char *) html.GetMemory();

    char str[256];
    if (old_len >= (long) High(str))
      break;

    strncpy(str, param+2, old_len-3);
    str[old_len-3] = 0;
    const char * ptr = str;
    unsigned short recv_id = NUtils::GetHex(ptr);

    const ElsterIndex * elster_index = NULL;
    int elster_idx = -1;
    unsigned char elster_type = 0;
    if (NUtils::IsHexDigit(str[0]) &&
        NUtils::IsHexDigit(str[1]) &&
        NUtils::IsHexDigit(str[2]) &&
        (str[3] == '.' || str[3] == ':'))
    {
      if (str[3] == '.')
      {
        elster_index = GetElsterIndex(str + 4);
        if (elster_index)
          elster_idx = elster_index->Index;
      } else {
        ptr++;
        elster_idx = NUtils::GetHex(ptr);
        elster_index = GetElsterIndex((unsigned short) elster_idx);
      }
      if (elster_index)
        elster_type = elster_index->Type;
    }
    if (elster_idx < 0)
    {
      strcpy(str, "not found");
    } else {
      unsigned short value;

      NUtils::SleepMs(100);
      bool Ok = Can.GetValue(recv_id, (unsigned short) elster_idx, value);
      if (Ok)
      {
        if (elster_type == et_double_val || elster_type == et_triple_val)
        {
          double d;
          Ok = Can.GetDoubleValue(value, recv_id, (unsigned short) elster_idx, elster_type, d);
          if (Ok)
            SetDoubleType(str, elster_type, d);
        } else
          SetValueType(str, elster_type, value);
      }
      if (!Ok)
        strcpy(str, "---");
    }
    html.ReplaceBytes((const unsigned char *)str, (unsigned long) offset, (unsigned) strlen(str), (unsigned) old_len);

    scan_pos += (int) strlen(str);
  } while(true);

  output((const char *) html.GetMemory(), "html");

  Can.Halt();

  return 0;
}

#endif