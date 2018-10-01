/*
 *
 *  Copyright (C) 2015 Jürg Müller, CH-5524
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

#if defined(__WINDOWS__)
  // __MINGW__
  #if defined(__CYGWIN__)
    #include <winsock.h>
  #endif
  #include <windows.h>
#endif

#if defined(__LINUX__)
  // Definition von sockaddr_in (Socket address, internet style)
  #include <netinet/in.h>
  #include <sys/socket.h>
#endif

#include "NTypes.h"

#include "KTcpClient.h"


/////////////////////////////////////////////////////////////////////////
//
// Klasse KTcpClient
//
/////////////////////////////////////////////////////////////////////////

#if !defined(SD_BOTH)
  #define SD_BOTH        2
#endif

KTcpClient::KTcpClient() //: KIpSocket()
{
  mConnected = false;
  mReadTimeout = 2000;
  mSendTimeout = 4000; // 1 Sek. Schreiben auf Flash (ARM-Board) braucht lange
  mTrace = false;
  mRemotePath[0] = 0;
}

KTcpClient::~KTcpClient()
{
  Disconnect();
}

void KTcpClient::Disconnect()
{
  if (mConnected)
  {
    // unter Windows SD_BOTH == 2
    // unter Linux SHUT_RDWR == 2
    if (mActive)
      ErrorCheck(shutdown(mSocket, SD_BOTH), "shutdown");

    mConnected = false;
  }

  Close();
}

bool KTcpClient::CheckConnection(int aTimeout)
{
  bool Result;
  TSocketBlockModeEnum SaveBlockMode;

  SaveBlockMode = GetBlockMode();
  SetBlockMode(bmNonBlocking);
  Result = Connect(aTimeout);
  Disconnect();
  SetBlockMode(SaveBlockMode);

  return Result;
}

bool KTcpClient::Connect(int aTimeout)
{
  sockaddr_in addr;

  KIpSocket::Open();

  if (mActive && !mConnected)
  {
    GetSocketAddr(addr, mRemoteHost, mRemotePort);
    mConnected = ErrorCheck(connect(mSocket, (sockaddr *)(&addr), sizeof(addr)),
                            "connect") == 0;

    if (!mConnected && mBlockMode == bmNonBlocking && aTimeout > 0)
      Select(NULL, &mConnected, NULL, aTimeout);

    if (!mConnected)
    {
    #if defined(__CONSOLE__)
      printf("connect error 0x%x\n", mLastError);
    #endif
      Close();
    }
  }

  return mConnected;
}

bool KTcpClient::ReceiveBuffer(char * Buffer, int BufferLen)
{
  bool Ok = true;

  while (Ok && BufferLen > 0)
  {
    int getl = ReceiveBuf(Buffer, BufferLen, 0, 2000);
    Ok = getl > 0;
    if (Ok && mTrace)
      for (int i = 0; i < getl; i++)
      {
        char c = Buffer[i];
        if (c != '\n' && (c < ' ' || c >= 127))
          c = '.';
        printf("%c", c);
      }
    Buffer += getl;
    BufferLen -= getl;
  }

  return Ok;
}

bool KTcpClient::ReadBuffer(char* aBuffer, int aToRead)
{
  int Read, ToRead;
  bool Ok;

  ToRead = aToRead;
  do
  {
    Read = ReceiveBuf(aBuffer, ToRead, 0, mReadTimeout);
    Ok = Read > 0;
    if (Ok)
    {
      aBuffer += Read;
      ToRead -= Read;
    }
  } while (Ok && mActive && ToRead > 0);
  #if defined(__CONSOLE__) && defined(__WINDOWS__)
    if (!Ok || !mActive)
    {
      printf("ReadBuffer error 0x%x (bytes to read %d of %d; timeout: %d)\n",
             mLastError, ToRead, aToRead, mReadTimeout);
    }
    #if defined(__DEBUG__) && (0)
    else
      printf("ReadBuffer bytes read %d\n", aToRead);
    #endif
  #endif

  aBuffer[0] = 0; // aBuffer muss länger als aToRead sein!

  // closed by peer
  if (!mActive)
    mConnected = false;

  return Ok;
}

bool KTcpClient::SendBuffer(const char * aBuffer, int aToSend, int TimeOut)
{
  int Sent, ToSend;
  bool Ok;

  if (!aBuffer || !aToSend)
    return true;

  ToSend = aToSend;
  do
  {
    Sent = SendBuf(aBuffer, ToSend > 10240 ? 10240 : ToSend, 0, TimeOut);
    Ok = Sent > 0;
    if (Ok)
    {
      aBuffer += Sent;
      ToSend -= Sent;
    }
  } while (Ok && mActive && ToSend > 0);
  #if defined(__CONSOLE__)
    if (!Ok || !mActive)
    {
      printf("SendBuffer error 0x%x (bytes to send %d of %d; timeout: %d)\n",
             mLastError, ToSend, aToSend, TimeOut);
    }
  #endif

  // closed by peer
  if (!mActive)
    mConnected = false;

  return Ok;
}

bool KTcpClient::ReadLine(char * aString, int aLength)
{
  const char * First = aString;
  aString[0] = 0;

  int Len = 0;
  char Temp[4];

  bool Ok = false;
  bool CR = false;

  while (!Ok &&
         ReceiveBuf(Temp, 1, 0, Len ? 100 : 2000) == 1)
  {
    if (Temp[0] == 13)
    {
      CR = true;
    } else
    if (Temp[0] == 10 && CR)
    {
      Ok = true;
    } else
    if (aLength <= 1)
    {
      CR = false;
      Ok = false;
    } else {
      *aString = Temp[0];
      aString++;
      *aString = 0;
      aLength--;
      Len++;
      CR = false;
    }
  }

  if (Ok && mTrace)
    printf("Tcp recv: %s\n", First);
  return Ok;
}

bool KTcpClient::WriteString(const char * aBuffer)
{
  if (!aBuffer || !*aBuffer)
    return true;

  return SendBuffer(aBuffer, (int)strlen(aBuffer));
}

bool KTcpClient::WriteLine(const char * aBuffer)
{
  if (mTrace && aBuffer)
  {
    const char * Ptr = aBuffer;
    do
    {
      const char * Next = strstr(Ptr, "\r\n");
      printf("Tcp send: ");
      if (Next)
      {
        for (int i = 0; i < Next-Ptr; i++)
          printf("%c", Ptr[i]);
        Ptr = Next + 2;
      } else {
        printf("%s", Ptr);
        Ptr = NULL;
      }
      printf("\n");
    } while(Ptr);
  }
  int len =  aBuffer ? (int) strlen(aBuffer) : 0;

  if (len)
    if (!SendBuffer(aBuffer, len, mSendTimeout))
      return false;

  return SendBuffer("\r\n", 2, len ? mSendTimeout : 100);
}


//////////////////////////////////////////////////////////////////////////////
//
// KHttp
//
//////////////////////////////////////////////////////////////////////////////

const char * KHttp::Methods[htUnknown] =
{
  "OPTIONS",
  "GET",
  "HEAD",
  "POST",
  "PUT",
  "DELETE",
  "TRACE",
  "CONNECT"
};

KHttp::KHttp()
{
  mIpError = false;
  mShowLevel = 0;
}

const char * KHttp::StatusCode(int aCode)
{
  // Hypertext Transfer Protocol -- HTTP/1.1
  // rfc2616
  switch (aCode)
  {
    case 100: return "Continue";
    case 101: return "Switching Protocols";

    // 2XX: Success
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";

    // 3XX: Redirections
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";

    // 4XX Client Errors
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Time-out";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Required";
    case 412: return "Precondition Failed";
    case 413: return "Request Entity Too Large";
    case 414: return "Request-URI Too Large";
    case 415: return "Unsupported Media Type";
    case 416: return "Requested range not satisfiable";
    case 417: return "Expectation Failed";

    // 5XX Server errors
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Time-out";
    case 505: return "HTTP Version not supported";

    default:  return "unknown";
  }
}

bool KHttp::ReadLine(KTcpClient & aClientSocket, char * aString, int aLength, bool aShow)
{
  aString[0] = 0;
  if (mIpError)
    return false;

  mIpError = !aClientSocket.ReadLine(aString, aLength);
#if defined(__CONSOLE__)
  if ((aShow || mShowLevel >= 3) && !mIpError)
    printf("%s\n", aString);
#endif
  return !mIpError;
}

bool KHttp::WriteLine(KTcpClient & aClientSocket, const char * aString, bool aShow)
{
  if (mIpError)
    return false;

#if defined(__CONSOLE__)
  if (aShow || mShowLevel >= 3)
    printf("%s\n", aString);
#endif
  mIpError = !aClientSocket.WriteLine(aString);

  return !mIpError;
}

