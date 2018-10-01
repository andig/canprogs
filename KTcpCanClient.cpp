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

#include "KTcpCanClient.h"

KTcpCanClient::KTcpCanClient()
{
  UseBinaryProtocol = false;
  IsServer = true;
}

bool KTcpCanClient::ReadElster(char * Buffer)
{
  int Read = ReceiveBuf(Buffer, 1, 0, mReadTimeout);
  if (Read == 1 && *Buffer != '\n')  // auf LF synchronisieren
  {
    for (int Tot = Read ; Tot < 3*12; Tot++)
    {
      Read = ReceiveBuf(Buffer + Tot, 1, 0, 5);
      if (Read != 1)
      {
        Buffer[Tot] = 0;
        if (mTrace)
          printf("broken %s\n", Buffer);
        return false;
      }
      if (Buffer[Tot] == '\n')
      {
        Buffer[Tot + 1] = 0;
        return true;
      }
    }
    Buffer[3*12] = 0;
    if (mTrace)
      printf("no LF %s\n", Buffer);
  }
  return false;
}

bool KTcpCanClient::ReadElster(KComfortFrame & recv)
{
  char Buffer[128];

  recv.SetOk(false);
  int Read = ReceiveBuf(Buffer, 1, 0, mReadTimeout);
  if (Read == 1 && (!IsServer || *Buffer == 0x0d))
  {
    for (int Tot = Read ; Tot < 12; Tot++)
    {
      Read = ReceiveBuf(Buffer + Tot, 1, 0, 5);
      if (Read != 1)
      {
        return false;
      }
    }
    memcpy(recv.Data, Buffer, sizeof(recv.Data));
    bool Ok = recv.CheckSum();
    if (!Ok && mTrace)
      printf("recv: checksum error\n");
    return Ok;
  }
  return false;
}

bool KTcpCanClient::SendElster(char * Buffer, int timeout)
{
  while (strlen(Buffer) && Buffer[strlen(Buffer)-1] == ' ')
    Buffer[strlen(Buffer)-1] = 0;

  strcat(Buffer, "\n");

  return SendBuffer(Buffer, (int) strlen(Buffer), timeout);
}

bool KTcpCanClient::SendElster(const KComfortFrame & send, int timeout)
{
  return SendBuffer((const char *) send.Data, sizeof(send.Data), timeout);
}

bool KTcpCanClient::SendComfortFrame(const KComfortFrame & send, KComfortFrame & recv)
{
  bool Ok = false;
  char str[128];

  if (!UseBinaryProtocol)
  {
    send.SetToString(str);
    strcat(str, "\n");
  }

  Connect(1000);
  if (UseBinaryProtocol)
  {
    if (SendElster(send, 1000))
      Ok = ReadElster(recv);
  } else {
    if (SendElster(str, 1000)) // 1000 ms timeout
      Ok = ReadElster(str);

    if (Ok)
      Ok = recv.SetFromString(str, true);
  }
  Disconnect();

  return Ok;
}



