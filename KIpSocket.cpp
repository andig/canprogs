/*
 *
 *  Copyright (C) 2007-2014 Jürg Müller, CH-5524
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
  // __MINGW__
  #if defined(__CYGWIN__)
    #include <winsock.h>
  #endif
  #if defined(__VC__)
    #include <winsock2.h>
  #endif
  #include <windows.h>
#endif

#if defined(__LINUX__)
  #include <sys/socket.h>
  #if defined(__ARM__)
    #if defined(__CAN__)
      #include <linux/can.h>
    #endif
    #if !defined(SO_REUSEPORT)
      #define SO_REUSEPORT 0x0200  /* allow local address & port reuse */
    #endif
  #endif
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <errno.h>
  #include <fcntl.h>
#endif

#include "NTypes.h"

#include "KIpSocket.h"
#include "NUtils.h"

#if !defined(EAGAIN)
  #define  EAGAIN  11
#endif



#if defined(__WINDOWS__)
  #if !defined(__CAN__)
    #include "NSystem.h"
  #endif

  static bool lIpInitialized = false;
  static void Initialization_IP_Socket();
  static void Finalization_IP_Socket();
#endif

/////////////////////////////////////////////////////////////////////////
//
// Klasse KIpSocket
//
/////////////////////////////////////////////////////////////////////////

const unsigned short KIpSocket::mDefaultTcpIpPort = 8071;

KIpSocket::KIpSocket()
{
#if defined(__WINDOWS__)
  if (!lIpInitialized)
    Initialization_IP_Socket();
#endif

  mActive = false;
  mLastError = 0;
  mBlockMode = bmNonBlocking;
  mSocket = INVALID_SOCKET;
  mSocketType = stStream;

  mDomain = PF_INET;
  mProtocol = IPPROTO_IP;
  mLocalHost[0] = 0;
  mLocalPort = 0;
  mRemoteHost[0] = 0;
  mRemotePort = 0;
}

KIpSocket::~KIpSocket()
{
  Close();
}

void KIpSocket::Close()
{
  if (mActive)
  {
    if (mSocket != INVALID_SOCKET)
    {
    #if defined(__LINUX__)
      ErrorCheck(close(mSocket), "close");
    #endif
    #if defined(__WINDOWS__)
      ErrorCheck(closesocket(mSocket), "closesocket"); // Juli 2004
    #endif
      mSocket = INVALID_SOCKET;
    }
    mActive = false;
  }
}

#if defined(__CONSOLE__)
  static const char * SocketError(int aError)
  {
    // Linux: errno.h
    // Windows: winsock.h
  #if defined(__WINDOWS__)
    aError -= 10000;
    if (aError <= 0)
      return 0;
  #endif

    switch (aError)
    {
    #if defined(__WINDOWS__)
      case 4:
        return "EINTR";
      case 9:
        return "EBADF";
      case 13:
        return "EACCES";
      case 14:
        return "EFAULT";
      case 22:
        return "EINVAL";
      case 24:
        return "EMFILE";

      // Windows Sockets definitions of regular Berkeley error constants
      case 35:
        return "EWOULDBLOCK";
      case 36:
        return "EINPROGRESS";
      case 37:
        return "EALREADY";
      case 38:
        return "ENOTSOCK";
      case 39:
        return "EDESTADDRREQ";
      case 40:
        return "EMSGSIZE";
      case 41:
        return "EPROTOTYPE";
      case 42:
        return "ENOPROTOOPT";
      case 43:
        return "EPROTONOSUPPORT";
      case 44:
        return "ESOCKTNOSUPPORT";
      case 45:
        return "EOPNOTSUPP";
      case 46:
        return "EPFNOSUPPORT";
      case 47:
        return "EAFNOSUPPORT";
      case 48:
        return "EADDRINUSE";
      case 49:
        return "EADDRNOTAVAIL";
      case 50:
        return "ENETDOWN";
      case 51:
        return "ENETUNREACH";
      case 52:
        return "ENETRESET";
      case 53:
        return "ECONNABORTED";
      case 54:
        return "ECONNRESET";
      case 55:
        return "ENOBUFS";
      case 56:
        return "EISCONN";
      case 57:
        return "ENOTCONN";
      case 58:
        return "ESHUTDOWN";
      case 59:
        return "ETOOMANYREFS";
      case 60:
        return "ETIMEDOUT";
      case 61:
        return "ECONNREFUSED";
      case 62:
        return "ELOOP";
      case 63:
        return "ENAMETOOLONG";
      case 64:
        return "EHOSTDOWN";
      case 65:
        return "EHOSTUNREACH";
      case 66:
        return "ENOTEMPTY";
      case 67:
        return "EPROCLIM";
      case 68:
        return "EUSERS";
      case 69:
        return "EDQUOT";
      case 70:
        return "ESTALE";
      case 71:
        return "EREMOTE";
      case 101:
        return "EDISCON";

      // Extended Windows Sockets error constant definitions
      case 91:
        return "WSASYSNOTREADY";
      case 92:
        return "WSAVERNOTSUPPORTED";
      case 93:
        return "WSANOTINITIALISED";
    #endif
    #if defined(__LINUX__)
      // file: sys/errno.h
      case EPERM:     // 1
        return "EPERM (Not super-user)";
      case ENOENT:   // 2
        return "ENOENT (No such file or directory)";
      case ESRCH:    // 3
        return "ESRCH (No such process)";

      case EINTR:   // 4
        return " (Interrupted system call)";
      case EIO:   // 5
        return " (I/O error)";
      case ENXIO:   // 6
        return " (No such device or address)";
      case E2BIG:   // 7
        return " (Arg list too long)";
      case ENOEXEC:   // 8
        return " (Exec format error)";
      case EBADF:   // 9
        return " (Bad file number)";
      case ECHILD:   // 10
        return " (No children)";
      case EAGAIN:   // 11
        return " (No more processes)";
      case ENOMEM:   // 12
        return " (Not enough core)";
      case EACCES:   // 13
        return " (Permission denied)";
      case EFAULT:   // 14
        return " (Bad address)";
      case ENOTBLK:   // 15
        return " (Block device required)";
      case EBUSY:   // 16
        return " (Mount device busy)";
      case EEXIST:   // 17
        return " (File exists)";
      case EXDEV:   // 18
        return " (Cross-device link)";
      case ENODEV:   // 19
        return " (No such device)";
      case ENOTDIR:   // 20
        return " (Not a directory)";
      case EISDIR:   // 21
        return " (Is a directory)";
      case EINVAL:   // 22
        return " (Invalid argument)";
      case ENFILE:   // 23
        return " (Too many open files in system)";
      case EMFILE:   // 24
        return " (Too many open files)";
      case ENOTTY:   // 25
        return " (Not a typewriter)";
      case ETXTBSY:   // 26
        return " (Text file busy)";
      case EFBIG:   // 27
        return " (File too large)";
      case ENOSPC:   // 28
        return " (No space left on device)";
      case ESPIPE:   // 29
        return " (Illegal seek)";
      case EROFS:   // 30
        return " (Read only file system)";
      case EMLINK:   // 31
        return " (Too many links)";
      case EPIPE:   // 32
        return " (Broken pipe)";
      case EDOM:   // 33
        return " (Math arg out of domain of func)";
      case ERANGE:   // 34
        return " (Math result not representable)";
      case ENOMSG:   // 35
        return " (No message of desired type)";
      case EIDRM:   // 36
        return " (Identifier removed)";
    #if !defined(__MAC__)
      case ECHRNG:   // 37
        return " (Channel number out of range)";
      case ECOMM:   // 70
        return "ECOMM (Communication error on send)";
    #endif
      case EPROTO:   // 71
        return "EPROTO (Protocol error)";
      case EMULTIHOP:   // 74
        return "EMULTIHOP (Multihop attempted)";
      case 75: //ELBIN:   // 75
        return "ELBIN (Inode is remote (not really error))";
    #if !defined(__MAC__)
      case EDOTDOT:   // 76
        return "EDOTDOT (Cross mount point (not really error))";
    #endif
      case EBADMSG:   // 77
        return "EBADMSG (Trying to read unreadable message)";
      case 79: //EFTYPE:   // 79
        return "EFTYPE (Inappropriate file type or format)";
    #if !defined(__MAC__)
      case ENOTUNIQ:   // 80
        return "ENOTUNIQ (Given log. name not unique)";
      case EBADFD:   // 81
        return "EBADFD (f.d. invalid for this operation)";
      case EREMCHG:   // 82
        return "EREMCHG (Remote address changed)";
    #endif
      case 83:// ELIBACC:   // 83
        return "ELIBACC (Can't access a needed shared lib)";
    #if !defined(__MAC__)
      case ELIBBAD:   // 84
        return "ELIBBAD (Accessing a corrupted shared lib)";
      case ELIBSCN:   // 85
        return "ELIBSCN (.lib section in a.out corrupted)";
      case ELIBMAX:   // 86
        return "ELIBMAX (Attempting to link in too many libs)";
    #endif
      case 87: //ELIBEXEC:   // 87
        return "ELIBEXEC Attempting to exec a shared library)";
      case ENOSYS:   // 88
        return "ENOSYS (Function not implemented)";
      case 89: // ENMFILE:   // 89
        return "ENMFILE (No more files)";
      case ENOTEMPTY:   // 90
        return "ENOTEMPTY (Directory not empty)";
      case ENAMETOOLONG:   // 91
        return "ENAMETOOLONG (File or path name too long)";
      case ELOOP:   // 92
        return "ELOOP (Too many symbolic links)";
      case EOPNOTSUPP:   // 95
        return "EOPNOTSUPP (Operation not supported on transport endpoint)";
      case EPFNOSUPPORT:   // 96
        return "EPFNOSUPPORT (Protocol family not supported)";
      case ECONNRESET:   // 104
        return "ECONNRESET (Connection reset by peer)";
      case ENOBUFS:   // 105
        return "ENOBUFS (No buffer space available)";
      case EAFNOSUPPORT:   // 106
        return "EAFNOSUPPORT (Address family not supported by protocol family)";
      case EPROTOTYPE:   // 107
        return "EPROTOTYPE (Protocol wrong type for socket)";
      case ENOTSOCK:   // 108
        return "ENOTSOCK (Socket operation on non-socket)";
      case ENOPROTOOPT:   // 109
        return "ENOPROTOOPT (Protocol not available)";
      case ESHUTDOWN:   // 110
        return "ESHUTDOWN (Can't send after socket shutdown)";
      case ECONNREFUSED:   // 111
        return "ECONNREFUSED (Connection refused)";
      case EADDRINUSE:   // 112
        return "EADDRINUSE (Address already in use)";
      case ECONNABORTED:   // 113
        return "ECONNABORTED (Connection aborted)";
      case ENETUNREACH:   // 114
        return "ENETUNREACH (Network is unreachable)";
      case ENETDOWN:   // 115
        return "ENETDOWN (Network interface is not configured)";
      case ETIMEDOUT:   // 116
        return "ETIMEDOUT (Connection timed out)";
      case EHOSTDOWN:   // 117
        return "EHOSTDOWN (Host is down)";
      case EHOSTUNREACH:   // 118
        return "EHOSTUNREACH (Host is unreachable)";
      case EINPROGRESS:   // 119
        return "EINPROGRESS (Connection already in progress)";
      case EALREADY:   // 120
        return "EALREADY (Socket already connected)";
      case EMSGSIZE:   // 122
        return "EMSGSIZE (Message too long)";
      case EPROTONOSUPPORT:   // 123
        return "EPROTONOSUPPORT (Unknown protocol)";
      case ESOCKTNOSUPPORT:   // 124
        return "ESOCKTNOSUPPORT (Socket type not supported)";
      case EADDRNOTAVAIL:   // 125
        return "EADDRNOTAVAIL (Address not available)";
      case ENETRESET:   // 126
        return "ENETRESET";
      case EISCONN:   // 127
        return "EISCONN (Socket is already connected)";
      case ENOTCONN:   // 128
        return "ENOTCONN (Socket is not connected)";
      case ETOOMANYREFS:   // 129
        return "ETOOMANYREFS";
      case 130: // EPROCLIM:   // 130
        return "EPROCLIM";
      case EDQUOT:   // 132
        return "EDQUOT";
      case ESTALE:   // 133
        return "ESTALE";
    #if !defined(__MAC__)
      case ENOMEDIUM:   // 135
        return "ENOMEDIUM (No medium (in tape drive))";
    #endif
      case 136: //ENOSHARE:   // 136
        return "ENOSHARE (No such host or network path)";
      case EILSEQ:   // 138
        return "EILSEQ";
    #endif
      default:
        return NULL;
    }
  }
#endif

int KIpSocket::ErrorCheck(long rc, const char * aPlace)
{
  mLastError = 0;
  if (rc == SOCKET_ERROR)
  {
  #if defined(__WINDOWS__)
    mLastError = WSAGetLastError();
    if (mLastError)
    {
      WSASetLastError(0);
      if (mLastError == WSAEWOULDBLOCK &&
          aPlace &&
          (!strcmp(aPlace, "accept") || !strcmp(aPlace, "connect")))
      {
        mLastError = 0;
        return rc;
      }
    }
  #else
    mLastError = errno;    // is thread safe with: -D _REENTRANT
    if (mLastError == EWOULDBLOCK &&
        aPlace && !strcmp(aPlace, "accept"))
    {
      mLastError = 0;
      return (int)rc;
    }
  #endif
  #if defined(__CONSOLE__)
    if (mLastError != EAGAIN)
    {
      const char * Msg = SocketError(mLastError);
      if (!aPlace)
        aPlace = "?";
      if (Msg)
        printf("socket error (%s) %d: %s\n", aPlace, (int)mLastError, Msg);
      else
        printf("socket error (%s) %d\n", aPlace, (int)mLastError);
    }
  #endif

  #if defined(WSAECONNRESET)
    if (mLastError == WSAECONNRESET) // windows
      mActive = false;
  #endif
  #if defined(ECONNRESET)
    if (mLastError == ECONNRESET)    // errno.h / linux
      mActive = false;
  #endif
  #if defined(ENOTCONN) && defined(__MAC__)
    if (mLastError == EPIPE || mLastError == ENOTCONN)
    {
      mActive = false;
    }
  #endif
  }
  return (int)rc;
}

bool KIpSocket::Open()
{
  static const int lSocketType[stSeqPacket+1] = {
    SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, SOCK_RDM, SOCK_SEQPACKET };

  if (!mActive)
  {
    mSocket = ErrorCheck(socket(mDomain, int(lSocketType[mSocketType]), mProtocol),
                         "socket");
    mActive = mSocket != INVALID_SOCKET;
    if (mActive)
    {
      TSocketBlockModeEnum BlockMode = mBlockMode;
    #if (0)
      {
        int Flag;

        Flag = -1;
        setsockopt(mSocket, SOL_SOCKET, SO_DONTROUTE, (char *)(&Flag), sizeof(Flag));
      }
    #endif
      mBlockMode = bmBlocking;
      SetBlockMode(BlockMode);
    }
  }
  return mActive && mSocket;
}

int KIpSocket::PeekBuf(char & Buf, int BufSize)
{
  return ErrorCheck(recv(mSocket, &Buf, BufSize, MSG_PEEK), "recv");
}

int KIpSocket::ReceiveBuf(char* aBuf, int aBufSize, int aFlags, int aTimeout)
{
  if (!mActive)
    return 0;

  bool Read;
  bool Except;
  if (mBlockMode == bmNonBlocking && aTimeout > 0)
  {
    Select(&Read, NULL, &Except, aTimeout);
    if (Except)
    {
      Close();
      return -1;
    }
    if (!Read)
      return 0;
  }
  return ErrorCheck(recv(mSocket, aBuf, aBufSize, aFlags), "recv");
}

bool KIpSocket::Select(bool * ReadReady, bool * WriteReady, bool * ExceptFlag, int TimeOut)
{
  fd_set ReadFds;
  fd_set * ReadFdsptr;
  fd_set WriteFds;
  fd_set * WriteFdsptr;
  fd_set ExceptFds;
  fd_set * ExceptFdsptr;
  timeval tv;
  timeval * Timeptr;
  bool Result;

  Result = false;
  if (mActive)
  {
    if (ReadReady)
    {
      ReadFdsptr = &ReadFds;
      FD_ZERO(&ReadFds);
      FD_SET(mSocket, &ReadFds);
    } else
      ReadFdsptr = NULL;

    if (WriteReady)
    {
      WriteFdsptr = &WriteFds;
      FD_ZERO(&WriteFds);
      FD_SET(mSocket, &WriteFds);
    } else
      WriteFdsptr = NULL;

    if (ExceptFlag)
    {
      ExceptFdsptr = &ExceptFds;
      FD_ZERO(&ExceptFds);
      FD_SET(mSocket, &ExceptFds);
    } else
      ExceptFdsptr = NULL;

    if (TimeOut >= 0)
    {
      tv.tv_sec = TimeOut / 1000;
      tv.tv_usec =  1000 * (TimeOut % 1000);
      Timeptr = &tv;
    } else
      Timeptr = NULL;

    Result = ErrorCheck(select(mSocket + 1, ReadFdsptr, WriteFdsptr, ExceptFdsptr, Timeptr),
                        "select") > 0;

    if (ReadReady)
      *ReadReady = FD_ISSET(mSocket, &ReadFds) != 0;
    if (WriteReady)
      *WriteReady = FD_ISSET(mSocket, &WriteFds) != 0;
    if (ExceptFlag)
      *ExceptFlag = FD_ISSET(mSocket, &ExceptFds) != 0;
  }
  return Result;
}

int KIpSocket::SendBuf(const char * aBuf, int aBufSize, int aFlags, int aTimeout)
{
  int Result;

  if (!mActive)
    return 0;

  bool Write = true;
  if (mBlockMode == bmNonBlocking && aTimeout > 0)
    Select(NULL, &Write, NULL, aTimeout);

  if (Write)
    Result = ErrorCheck(send(mSocket, (char *)aBuf, aBufSize, 0), "send");
  else
    Result = 0;

  return Result;
}

#if defined(__LINUX__) && !defined(__ARM__)
bool KIpSocket::SendFile(int file, TInt64 offset, TInt64 length)
{
  if (!mActive)
    return false;

  int res;
  bool Write;
  do
  {
    if (mBlockMode == bmNonBlocking)
      Select(NULL, &Write, NULL, 1000);

    if (!Write)
      return false;

    off_t o = offset;
    off_t l = length;
    res = ErrorCheck(sendfile(mSocket, file, o, &l, NULL, 0), "sendfile");
    offset += res;
    length -= res;
  } while (res >= 0 && length <= 0);
  return res >= 0;
}
#endif

void KIpSocket::SetBlockMode(TSocketBlockModeEnum aValue)
{
  if (aValue != mBlockMode)
  {
    if (mActive && aValue == bmNonBlocking)
    {
    #if defined(__WINDOWS__)
      unsigned long NonBlock;

      NonBlock = 1;
      ErrorCheck(ioctlsocket(mSocket, FIONBIO, &NonBlock), "ioctlsocket");
    #endif
    #if defined(__LINUX__)
      int flags = fcntl(mSocket, F_GETFL, 0);
      if (flags == -1)
        flags = 0;
      ErrorCheck(fcntl(mSocket, F_SETFL, O_NONBLOCK | flags), "fcntl");
    #endif
    }
    mBlockMode = aValue;
  }
}

bool KIpSocket::WaitForData(int TimeOut)
{
  bool ReadReady;
  bool ExceptFlag;
  char c;
  bool Result;

  Result = false;
  // Select also returns true when connection is broken.
  if (Select(&ReadReady, NULL, &ExceptFlag, TimeOut))
    Result = ReadReady && !ExceptFlag && (PeekBuf(c, sizeof(c)) == 1);

  return Result;
}

bool KIpSocket::Bind()
{
  sockaddr_in addr;
  bool Result;

  Result = false;
  if (mActive)
  {
//    if (!mLocalHost[0])
//      strcpy(mLocalHost, "127.0.0.1");
    GetSocketAddr(addr, mLocalHost, mLocalPort);
    Result = ErrorCheck(bind(mSocket, (sockaddr *)(& addr), sizeof(addr)), "bind") == 0;
#if defined(__LINUX__)
    if (!Result && mLastError == EADDRINUSE)
    {
      int optval = 1;
      if (ErrorCheck(setsockopt(mSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)), "setsockopt") == 0)
      {
        Result = ErrorCheck(bind(mSocket, (sockaddr *)(& addr), sizeof(addr)), "bind") == 0;
      }
    }
#endif
  }
  return Result;
}

unsigned short KIpSocket::GetLocalPort() const
{
  return mLocalPort;
}

void KIpSocket::GetSocketAddr(sockaddr_in & aResult, char * aHost, unsigned short aPort)
{
  char Q[256];
  memset(&aResult, 0, sizeof(aResult));
  aResult.sin_family = AF_INET;

  LookupHostAddr(aHost, Q, sizeof(Q));
  // Der String von Q muss eine TCP/IP-Adresse in der Form "0.0.0.0" enthalten.
  // Das Resultat s_addr ist in der Big-Endian-Reihenfolge.
  aResult.sin_addr.s_addr = inet_addr(Q);
  aResult.sin_port = htons(aPort); // SwapWord(aPort);
}

void KIpSocket::LocalDomainName(char * aResult, int aResultSize)
{
  if (aResultSize < 1)
    return;

  #if !defined(__LINUX__)
    char * p;
    char HostAddr[64];

    LocalHostAddr(HostAddr, sizeof(HostAddr));
    LookupHostName(HostAddr, aResult, aResultSize);
    p = strchr(aResult, '.');
    if (p)
      strcpy(aResult, p+1);
    else
      aResult[0] = 0;
  #else
    if (getdomainname(aResult, aResultSize))
      aResult[0] = 0;
  #endif
}

void KIpSocket::LocalHostAddr(char * aResult, int aResultSize)
{
  char HostName[256];

  LocalHostName(HostName, sizeof(HostName));
  LookupHostAddr(HostName, aResult, aResultSize);
}

void KIpSocket::LocalHostName(char * aResult, int aResultSize)
{
  if (aResultSize < 1)
    return;

  if (gethostname(aResult, aResultSize))
    aResult[0] = 0;
}

void KIpSocket::LookupHostAddr(const char * aHn, char * aResult, int aResultSize)
{
  hostent * H;
  bool Ok;

  if (aResultSize > 0)
    aResult[0] = 0;

  if (aResultSize < 32)
    return;

  Ok = false;
  if (aHn && aHn[0])
  {
    if ('0' <= aHn[0] && aHn[0] <= '9')
    {
      if (inet_addr((char *)(aHn)) != INADDR_NONE &&
          (int) strlen(aHn) < aResultSize)
      {
        strcpy(aResult, aHn);
        Ok = true;
      }
    } else {
      H = gethostbyname((char *)(aHn)); // Windows: reentrant
                                        // Linux use option: -pthread
                                        // or: -D_REENTRANT -lpthread
      if (H)
      {
        sprintf(aResult, "%d.%d.%d.%d",
                (unsigned char)(H->h_addr[0]),
                (unsigned char)(H->h_addr[1]),
                (unsigned char)(H->h_addr[2]),
                (unsigned char)(H->h_addr[3]));
        Ok = true;
      }
    }
  }
  if (!Ok)
    strcpy(aResult, "0.0.0.0");
}

bool KIpSocket::IsHostAddr(const char * aHn)
{
  char buffer[64];
  char addr[256];
  
  if (!aHn || strlen(aHn) > 200)
    return false;
  
  strcpy(addr, aHn);
  if (strchr(addr, ':'))
    *strchr(addr, ':') = 0;
  
  LookupHostAddr(addr, buffer, sizeof(buffer));
  
  return strlen(buffer) > 7;
}

void KIpSocket::LookupHostName(const char * ipaddr, char * aResult, int aResultSize)
{
  hostent * h;
  unsigned addr;
  bool Ok;

  if (aResultSize < 1)
    return;

  Ok = false;
  addr = inet_addr((char *)(ipaddr));
  if (addr != INADDR_NONE)
  {
    h = gethostbyaddr((char *)(&addr), sizeof(addr), AF_INET);
    if (h != NULL &&
        (int) strlen(h->h_name) < aResultSize)
    {
      strcpy(aResult, h->h_name);
      Ok = true;
    }
  }
  if (!Ok)
    aResult[0] = 0;
}

unsigned short KIpSocket::LookupPort(const char * aSn, char* aPn)
{
  servent * Se;
  unsigned short Result;

  Result = 0;
  if (aSn && aSn[0])
  {
    Se = getservbyname((char *)(aSn), aPn); // Windows: reentrant
    if (Se)
      Result = ntohs(Se->s_port);
    else
      Result = (unsigned short)(atoi(aSn));
  }
  return Result;
}

unsigned short KIpSocket::LookupProtocol(const char * pn)
{
  protoent * pe;
  unsigned short Result;

  Result = 0;
  pe = getprotobyname((char *)(pn));
  if (pe)
    Result = pe->p_proto;
  return Result;
}

int KIpSocket::ReceiveFrom(char* buf, int bufsize, sockaddr ToAddr, int & len, int flags, int aTimeout)
{
  int Result;
  bool Read;

  if (mBlockMode == bmNonBlocking && aTimeout > 0)
    Select(&Read, NULL, NULL, aTimeout);
  #if defined(__LINUX__) && !defined(__CYGWIN__)
  Result = ErrorCheck(recvfrom(mSocket, buf, bufsize, flags, &ToAddr, (unsigned int *) &len),
                      "recvfrom");
  #else
  Result = ErrorCheck(recvfrom(mSocket, buf, bufsize, flags, &ToAddr, &len),
                      "recvfrom");
  #endif

  return Result;
}

int KIpSocket::SendTo(char* buf, int bufsize, sockaddr ToAddr, int flags, int aTimeout)
{
  bool Write;

  if (mBlockMode == bmNonBlocking && aTimeout > 0)
    Select(NULL, &Write, NULL, aTimeout);

  return ErrorCheck(sendto(mSocket, buf, bufsize, flags, & ToAddr, sizeof(ToAddr)),
                    "sendto");
}

void KIpSocket::SetLocalHost(const char * Value)
{
  if (Value &&
      strcmp(Value, mLocalHost) &&       // nicht bereits gesetzt
      strlen(Value) < High(mLocalHost))
  {
    Close();
    strcpy(mLocalHost, Value);
  }
}

void KIpSocket::SetLocalPort(unsigned short Value)
{
  if (Value != mLocalPort)
  {
    Close();
    mLocalPort = Value;
  }
}

void KIpSocket::SetRemoteHost(const char * Value)
{
  if (Value &&
      strcmp(Value, mRemoteHost) &&
      strlen(Value) < High(mRemoteHost))
  {
    Close();
    strcpy(mRemoteHost, Value);
  }
}

void KIpSocket::SetRemotePort(unsigned short Value)
{
  if (Value != mRemotePort)
  {
    Close();
    mRemotePort = Value;
  }
}

bool KIpSocket::StoreRemote(const sockaddr_in & aAddr)
{
  const char * AddrStr;

  mRemotePort = ntohs(aAddr.sin_port);
  // TCP/IP-Adresse in den String "a.b.c.d" umwandeln.
  AddrStr = inet_ntoa(aAddr.sin_addr);
  mRemoteHost[0] = 0;
  if (strlen(AddrStr) < sizeof(mRemoteHost))
  {
    strcpy(mRemoteHost, AddrStr);
    return true;
  }
  return false;
}


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Für Windows muss die TCP/IP-Schnittstelle für jeden Prozess zuerst  //
// aktiviert werden.                                                   //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

#if defined(__WINDOWS__)

static void Initialization_IP_Socket()
{
  int ErrorCode;
  WSAData WSA_Data;

  if (!lIpInitialized)
  {
    ErrorCode = WSAStartup(0x0101, &WSA_Data);
    if (ErrorCode)
    {
      char Buffer[64];

      sprintf(Buffer, "WSAStartup error: %d", ErrorCode);
    #if defined(__CAN__)
      printf("%s\n", Buffer);
    #else
      NSystem::ShowMessage(Buffer);
    #endif
    }
    atexit(Finalization_IP_Socket);
  }
  lIpInitialized = true;
}

static void Finalization_IP_Socket()
{
  if (lIpInitialized)
  {
    int ErrorCode;

    ErrorCode = WSACleanup();
    if (ErrorCode)
    {
      char Buffer[64];

      sprintf(Buffer, "WSACleanup error: %d", ErrorCode);
    #if defined(__CAN__)
       printf("%s\n", Buffer);
    #else
      NSystem::ShowMessage(Buffer);
    #endif
    }
  }
}

#endif

