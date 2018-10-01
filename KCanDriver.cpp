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

#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

#include "NTypes.h"

#include "KCanDriver.h"
#include "KIpSocket.h"

bool KCanDriver::Init(const char * CanDev)
{
  dev[0] = 0;
  if (!CanDev || !*CanDev ||
      strlen(CanDev) > High(dev))
    return false;

  strcpy(dev, CanDev);
  if (dev[0] == 'C')
    dev[0] = 'c';
  if (dev[1] == 'O')
    dev[1] = 'o';
  if (dev[2] == 'M')
    dev[2] = 'm';

  bool Ok;
#if defined(__LINUX__)
  Ok = true;
  // Raspi: /dev/can0 ist nicht definiert (aber unter ifconfig zu sehen)
  #if !defined(__ARM__)
  struct stat Stat;
  if (strlen(dev) < 100)
  {
    char d[128] = {0};
    if (strncmp(dev, "/dev/", 5))
      strcpy(d, "/dev/");
    strcat(d, dev);
    Ok = !stat(d, &Stat) && (Stat.st_mode & S_IFMT);
  }
  #endif
#else
  struct stat Stat;
  Ok = !stat(dev, &Stat) && (Stat.st_mode & S_IFMT);
#endif

 // if (!Ok)
 //   printf("device \"%s\" does not exist\n", dev);

  return Ok;
}

bool KCanDriver::SendData(const KComfortFrame & Frame)
{
  KCanFrame CanFrame;

  if (!Frame.SetCanFrame(CanFrame))
    return false;

  return SendData(CanFrame);
}

bool KCanDriver::IsValidDevice(const char * Dev)
{
  if (NCanUtils::GetDriverType(Dev) != NCanUtils::dt_unknown)
    return true;

  return KIpSocket::IsHostAddr(Dev);
}

