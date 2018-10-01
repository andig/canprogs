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

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__WINDOWS__)
  #include <windows.h>
#endif
#if defined(__LINUX__)
  #include <sys/socket.h>
  #include <netinet/in.h>
#endif

#include "NTypes.h"

#include "KHttpClient.h"

#include "NUtils.h"

KHttpClient::KHttpClient()
{
  mReadTimeout = 500; // 500 ms tiemout
  mSendTimeout = 1000;
  mJpeg = false;
  ContentLength = 0;
  strcpy(mRemotePath, "/");
  SetRemotePort(80);
}

bool KHttpClient::SetRemotePath(const char * Path)
{
  int len = Path ? (int) strlen(Path) : 0;
  bool Ok = len < (int) High(mRemotePath);
  if (!Ok)
    len = (int) High(mRemotePath) - 1;
  if (len > 0)
    strncpy(mRemotePath, Path, len);
  mRemotePath[len] = 0;

  return Ok;
}

bool KHttpClient::TraceReceive(int Length)
{
  bool Ok = true;
  char Buffer[1024];

  printf("Tcp recv: ");
  while (Ok && Length > 0)
  {
    int l = Length > (int) High(Buffer) ? High(Buffer) : Length;
    Ok = ReceiveBuffer(Buffer, l);
    Length -= l;
  }
  printf("\n");
  return Ok;
}

// Anfrage vom Internet Explorer:
//
// GET /path HTTP/1.1              << Unterverzeichnis hier einfügen
// Accept: */*
// Accept-Language: de-ch
// Accept-Encoding: gzip, deflate
// User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; ...
// Host: localhost:8080
// Connection: Keep-Alive
// leere Zeile

bool KHttpClient::ReadHttpStream(KStream * aAnswer, const char * aGetString)
{
  bool Ok = false;
  bool connected = mConnected;

  if (aAnswer != NULL)
    aAnswer->SetLength(0);

  ContentLength = 0;
  if (connected || Connect(1000))
  {
    char Get[512];
    char remote[128];

    strcpy(Get, "GET ");
    strcat(Get, mRemotePath);
    strcat(Get, " HTTP/1.1");
    sprintf(remote, "HOST: %s:%d", mRemoteHost, mRemotePort);
    Ok = WriteLine(aGetString ? aGetString : Get);
    if (Ok)
      Ok = WriteLine(remote);
    if (Ok)
      Ok = WriteLine("");

    if (Ok)
      Ok = HttpAnswer(aAnswer, connected);
  }

  if (!connected || !Ok)
    Disconnect();

  if (mTrace)
    printf("\n");

  return Ok;
}

bool KHttpClient::ReadHttpStream(KStream * aAnswer, const char * aGetString,
                                 KStream * aContent)
{
  char str[256];
  bool Ok = false;
  bool connected = mConnected;

  if (aAnswer != NULL)
    aAnswer->SetLength(0);

  ContentLength = 0;
  if (connected || Connect(1000))
  {
    Ok = WriteLine(aGetString);

    if (Ok)
    {
      sprintf(str, "HOST: %s:%d", mRemoteHost, mRemotePort);
      Ok = WriteLine(str);
    }
    if (Ok && aContent && aContent->Size() > 0)
    {
      ContentLength = (int) aContent->Size();
      sprintf(str, "CONTENT-LENGTH: %d\r\n", ContentLength);
      Ok = WriteLine(str);
    }

    if (Ok && ContentLength > 0)
    {
      for (int i = 0; mTrace && i < ContentLength; i++)
      {
        char c = aContent->GetCharMemory()[i];
        if (c != '\n' && (c < ' ' || c > 126)) c = '.';
        printf("%c", c);
      }
      if (mTrace)
        printf("\n\n");

      Ok = SendBuffer(aContent->GetCharMemory(), ContentLength, mSendTimeout);
    }
    if (Ok)
      Ok = HttpAnswer(aAnswer, connected);
  }

  if (!connected || !Ok)
    Disconnect();

  if (mTrace)
    printf("\n");

  return Ok;
}


bool KHttpClient::HttpAnswer(KStream * aAnswer, bool & connected)
{
  const char * ParsePtr;
  bool Ok;
  bool HeadOk = true;
  bool Chunked = false;
  char buffer[1024];
  char * stream = buffer;
  int stream_len = High(buffer);
  bool genResult = aAnswer != NULL;
  char * res = NULL;

  mJpeg = false;

  if (genResult)
    aAnswer->SetLength(0);

  Ok = ReadLine(stream, stream_len);
  if (strncmp("HTTP/1.1 200 OK", stream, 14))
    HeadOk = false;

  if (Ok)
  {
    // lesen bis zur ersten Leerzeile
    // (Header wird mit einer Leerzeile abgeschlossen)
    do
    {
      Ok = ReadLine(stream, stream_len);
      // "Content-Length: %d"
      if (Ok)
      {
        char * ptr = stream; // lower case
        while (*ptr)
        {
          if ('A' <= *ptr && *ptr <= 'Z')
            *ptr -= (char)('A' - 'a');
          ptr++;
        }
        const char * cont_len = "content-length";
        ParsePtr = strstr(stream, cont_len);
        if (ParsePtr)
        {
          ParsePtr += strlen(cont_len);
          while (*ParsePtr && *ParsePtr <= ' ')
            ParsePtr++;
          if (*ParsePtr == ':')
          {
            ParsePtr++;
            ContentLength = (int)strtol(ParsePtr, NULL, 10);
          }
        } else
        if (strstr(stream, "transfer-encoding: chunked"))
          Chunked = true;
        else
        if (strstr(stream, "content-type: image/jpeg"))
          mJpeg = true;
        else
        if (strstr(stream, "connection: close"))
          connected = false;
      }
    } while (Ok && strlen(stream) != 0);

    if (0 < ContentLength)
    {
      if (genResult)
      {
        aAnswer->SetSize(ContentLength+1);
        aAnswer->SetLength(ContentLength);
        stream = res = (char *) aAnswer->GetMemory();
        stream_len = ContentLength;
      } else
      if (ContentLength < stream_len)
        stream_len = ContentLength;

      if (!genResult && mTrace)
        Ok = TraceReceive(stream_len);
      else
        Ok = ReceiveBuffer(stream, stream_len);
      if (genResult && Ok)
        res[ContentLength] = 0;
    } else
    if (Chunked)
    {
      bool Ends = false;
      while (Ok && !Ends)
      {
        Ok = ReadLine(buffer, High(buffer)); // hex. Länge

        const char * Ptr = buffer;
        int Len = NUtils::GetHex(Ptr);
        Ends = !Len || *Ptr;

        if (Ok)
        {
          Len += 2;
          while (Ok && Len > 0)
          {
            int l = Len > (int) High(buffer) ? (int) High(buffer) : Len;
            Ok = ReceiveBuffer(buffer, l);
            Len -= l;
            if (Ok && genResult)
              aAnswer->Append((const unsigned char *)buffer, l);
          }
        }
      }
    }
  }

  if (genResult && !Ok)
      aAnswer->SetLength(0);

  return Ok && HeadOk;
}

bool KHttpClient::PutHttpStream(const char * aStream)
{
  bool Ok = false;

  if (Connect(1000))
  {
    char Put[512];

    sprintf(Put, "PUT /put_can_log.php HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Content-Length: %d\r\n",
            mRemoteHost, (int) strlen(aStream) + 2);
    Ok = WriteLine(Put);
    if (Ok)
      Ok = WriteLine(aStream);

    bool connected = false;
    if (Ok)
      Ok = HttpAnswer(NULL, connected);

    Disconnect();
  }

  return Ok;
}

bool KHttpClient::ServeStream(KStream & Stream)
{
  if (mTrace)
  {
    Stream.AppendString("\r\nAccept: text/plain, text/html; charset=UTF-8"
                        "\r\nAccept-Charset: ISO-8859-1"
                        "\r\nContent-Language: de");
  }
  bool Ok = ReadHttpStream(NULL, (const char *) Stream.GetMemory());
  if (!Ok)
  {
    printf("TCP/IP server not connected\n");
    if (!mTrace)
      printf("==> %s\n", Stream.GetMemory());
  }

  return Ok;
}

