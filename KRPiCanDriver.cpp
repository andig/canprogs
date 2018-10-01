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

#if defined(__LINUX__)

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__ARM__)
  #include <net/if.h>
  #include <linux/can.h>
  #include <linux/can/raw.h>
#else

  struct sockaddr_can
  {
    unsigned can_family;
    unsigned can_ifindex;
    union {
      // transport protocol class address information (e.g. ISOTP)
      struct { unsigned rx_id, tx_id; } tp;

      // reserved for future CAN protocols address information
    } can_addr;
  };

  struct can_frame
  {
    unsigned      can_id;  // 32 bit CAN_ID + EFF/RTR/ERR flags
    unsigned char can_dlc; // frame payload length in byte (0 .. 8)
    unsigned char data[64] __attribute__((aligned(8)));
  };

  struct	ifreq
  {
	  char	ifr_name[32];
    unsigned ifr_ifindex;
  };


#ifndef PF_CAN
  #define PF_CAN 29
  #define AF_CAN PF_CAN
#endif

#ifndef CAN_RAW
  #define CAN_RAW 1
#endif

#ifndef SIOCGIFINDEX
  #define SIOCGIFINDEX	0x8933
#endif

typedef unsigned __u32;


#endif  // __ARM__

#if !defined(CANFD_MTU)
  /*
   * Controller Area Network Identifier structure (can_id)
   *
   * bit 0-28	: CAN identifier (11/29 bit)
   * bit 29	: error message frame flag (0 = data frame, 1 = error message)
   * bit 30	: remote transmission request flag (1 = rtr frame)
   * bit 31	: frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
   */
  struct canfd_frame
  {
    unsigned      can_id;  // 32 bit CAN_ID + EFF/RTR/ERR flags
    unsigned char len;     // frame payload length in byte (0 .. 64)
    unsigned char flags;   // additional flags for CAN FD
    unsigned char __res0;  // reserved / padding
    unsigned char __res1;  // reserved / padding
    unsigned char data[8] __attribute__((aligned(8)));
  };
#endif

struct timehdr
{
  struct cmsghdr hdr;  // 12 bytes
  struct timeval time; // 8 bytes : 20 bytes total (arm)
};

#include "NTypes.h"

#include "NUtils.h"
#include "KRPiCanDriver.h"

using namespace NUtils;

static unsigned RecvCounter = 0;


KRPiCanDriver::KRPiCanDriver()
{
  addr = new sockaddr_can;
  memset(addr, 0, sizeof(*addr));
}

KRPiCanDriver::~KRPiCanDriver()
{
  Close();

  delete addr;
}

bool KRPiCanDriver::Init(const char * CanDev)
{
  mDomain = PF_CAN;
  mSocketType = stRaw;
  mProtocol = CAN_RAW;
  WantTimeStamp = 1;

  if (!KCanDriver::Init(CanDev))
    strcpy(dev, "can0");

  SetBlockMode(bmBlocking);
  KIpSocket::Open();

  return mActive;
}

bool KRPiCanDriver::Bind()
{
  int Result = -1;

  if (mActive)
  {
    struct ifreq ifr;

    Result = ErrorCheck(setsockopt(mSocket, SOL_SOCKET, SO_TIMESTAMP,
                                   &WantTimeStamp, sizeof(WantTimeStamp)), "setsockopt");
    if (Result < 0)
      WantTimeStamp = 0;

    strcpy(ifr.ifr_name, dev);
    ioctl(mSocket, SIOCGIFINDEX, &ifr);

    addr->can_family = AF_CAN;
    addr->can_ifindex = ifr.ifr_ifindex;

    Result = ErrorCheck(bind(mSocket, (sockaddr *)(addr), sizeof(*addr)), "bind");
  }
  return Result == 0;
}

bool KRPiCanDriver::Connect()
{
  return Bind();
}

bool KRPiCanDriver::ReceiveData(KCanFrame & Frame)
{
  bool ReadReady;

  if (Select(&ReadReady, NULL, NULL, 1))
  {
    if (ReadReady)
    {
      struct canfd_frame frame;
      struct iovec iov;
      struct msghdr msg;
      // 36 bytes
      unsigned ctrl_len = CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32));
      char ctrlmsg[ctrl_len];

      iov.iov_base = &frame;
      iov.iov_len = sizeof(frame);
      msg.msg_namelen = sizeof(*addr);
      msg.msg_control = ctrlmsg;
      msg.msg_controllen = (unsigned) sizeof(ctrlmsg);
      msg.msg_flags = 0;
      msg.msg_name = addr;
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;

      if (ErrorCheck(recvmsg(mSocket, &msg, 0), "recvmsg") < 0)
      {
        return false;
      }
      Frame.Init();
      if (!WantTimeStamp)
      {
        Time(Frame.TimeStampDay, Frame.TimeStampMs);
      } else {
        struct timehdr * hdr = (struct timehdr *)ctrlmsg;
        struct timeval * Time = &hdr->time;
        NUtils::Time(*Time, Frame.TimeStampDay, Frame.TimeStampMs);
      }
      Frame.Len = frame.len > sizeof(Frame.Data) ? sizeof(Frame.Data) : frame.len;
      Frame.Id = frame.can_id;
      Frame.Flags = frame.flags;
      for (int i = 0; i < Frame.Len; i++)
        Frame.Data[i] = frame.data[i];
      Frame.Counter = RecvCounter++;

      return true;
    }
  }
  return false;
}

bool KRPiCanDriver::ReadData(KCanFrame & Frame)
{
  bool ReadReady;

  if (Select(&ReadReady, NULL, NULL, 1))
  {
    if (ReadReady)
    {
      can_frame frame;

      if (ErrorCheck(read(mSocket, &frame, sizeof(struct can_frame)), "read") < 0)
      {
        return false;
      }
      Frame.Init();
      NUtils::Time(Frame.TimeStampDay, Frame.TimeStampMs);
      Frame.Len = frame.can_dlc > sizeof(Frame.Data) ? sizeof(Frame.Data) : frame.can_dlc;
      Frame.Id = frame.can_id;
      Frame.Flags = 0;
      for (int i = 0; i < Frame.Len; i++)
        Frame.Data[i] = frame.data[i];
      Frame.Counter = RecvCounter++;

      return true;
    }
  }
  return false;
}

bool KRPiCanDriver::SendData(const KCanFrame & Frame)
{
  bool WriteReady;

  if (Select(NULL, &WriteReady, NULL, 1))
  {
    if (WriteReady)
    {
      can_frame frame;

      memset(&frame, 0, sizeof(frame));
      frame.can_dlc = Frame.Len;
      frame.can_id = Frame.Id;
      for (int i = 0; i < Frame.Len; i++)
        frame.data[i] = Frame.Data[i];

      return (ErrorCheck(write(mSocket, &frame, sizeof(frame)), "write") >= 0);
    }
  }
  return false;
}


#endif

