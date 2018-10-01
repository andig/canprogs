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
#include <string.h>
#include <stdlib.h>

#include "elster_perl.h"

#include "KCanElster.h"
#include "KElsterTable.h"
#include "elster_instance.h"

const char * elster_perl::getname(const char * params) // "000b"
{
  unsigned short elster_index;
  const char * res;

  res = elster_instance::GetOneParam(params, elster_index);
  if (res)
    return res;

  const ElsterIndex * elst_ind = GetElsterIndex(elster_index);
  if (elst_ind)
    return elst_ind->Name;

  return "";
}

const char * elster_perl::gettype(const char * params) // "000b"
{
  static char result[32];
  unsigned short elster_index;
  const char * res;

  res = elster_instance::GetOneParam(params, elster_index);
  if (res)
    return res;

  const ElsterIndex * elst_ind = GetElsterIndex(elster_index);
  if (elst_ind)
  {
    sprintf(result, "%d", elst_ind->Type);
    return result;
  }

  return "";
}

const char * elster_perl::getvalue(const char * params) // "680 601 000a"
{
  static char result[32];
  unsigned short send_id;
  unsigned short recv_id;
  unsigned short elster_index;
  unsigned short value;
  const char * res;

  res = elster_instance::GetThreeParams(params, send_id, recv_id, elster_index);
  if (res)
    return res;

  if (!elster_instance::elsterInit(send_id))
    return elster_instance::GetErrMsg();

  if (elster_instance::GetValue(recv_id, elster_index, value))
  {
    sprintf(result, "%d", value);
    return result;
  }

  return "";
}

const char * elster_perl::setvalue(const char * params) // "680 601 000a 0000"
{
  unsigned short send_id;
  unsigned short recv_id;
  unsigned short elster_index;
  unsigned short value;
  const char * res;

  res = elster_instance::GetFourParams(params, send_id, recv_id, elster_index, value);
  if (res)
    return res;

  if (!elster_instance::elsterInit(send_id))
    return elster_instance::GetErrMsg();

  if (elster_instance::SetValue(recv_id, elster_index, value))
    return "ok";

  return "";
}

const char * elster_perl::setbits(const char * params) // "680 601 000a 0000"
{
  static char result[32];
  unsigned short send_id;
  unsigned short recv_id;
  unsigned short elster_index;
  unsigned short bits;
  unsigned short value;
  const char * res;

  res = elster_instance::GetFourParams(params, send_id, recv_id, elster_index, bits);
  if (res)
    return res;

  if (!elster_instance::elsterInit(send_id))
    return elster_instance::GetErrMsg();

  if (!elster_instance::GetValue(recv_id, elster_index, value))
    return "";

  value |= bits;

  if (!elster_instance::SetValue(recv_id, elster_index, value))
  {
    return "";
  }

  sprintf(result, "%4.4x", value);

  return result;
}

const char * elster_perl::clrbits(const char * params) // "680 601 000a 0000"
{
  static char result[32];
  unsigned short send_id;
  unsigned short recv_id;
  unsigned short elster_index;
  unsigned short bits;
  unsigned short value;
  const char * res;

  res = elster_instance::GetFourParams(params, send_id, recv_id, elster_index, bits);
  if (res)
    return res;

  if (!elster_instance::elsterInit(send_id))
    return elster_instance::GetErrMsg();

  if (!elster_instance::GetValue(recv_id, elster_index, value))
    return "";

  value &= ~bits;

  if (!elster_instance::SetValue(recv_id, elster_index, value))
  {
    return "";
  }

  sprintf(result, "%4.4x", value);

  return result;
}

const char * elster_perl::getstring(const char * params) // "680 601 000a"
{
  static char result[64];
  unsigned short send_id;
  unsigned short recv_id;
  unsigned short elster_index;
  unsigned short value;
  const char * res;

  res = elster_instance::GetThreeParams(params, send_id, recv_id, elster_index);
  if (res)
    return res;

  if (!elster_instance::elsterInit(send_id))
    return elster_instance::GetErrMsg();

  if (!elster_instance::GetValue(recv_id, elster_index, value))
    return "";

  const ElsterIndex * elst_ind = GetElsterIndex(elster_index);
  if (elst_ind)
  {
    double d;

    if (elster_instance::GetDoubleValue(value, recv_id, elster_index, elst_ind->Type, d))
    {
      SetDoubleType(result, elst_ind->Type, d);
    } else
      SetValueType(result, elst_ind->Type, value);
  } else
    sprintf(result, "%d", value);

  return result;
}

const char * elster_perl::toggle_trace(void)
{
  return elster_instance::toggle_trace();
}

const char * elster_perl::setdev(const char * params)
{
  return elster_instance::setdev(params);
}

const char * elster_perl::setcs(void)
{
  return elster_instance::setcs();
}



