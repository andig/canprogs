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

#include "KCanElster.h"
#include "KElsterTable.h"
#include "elster_instance.h"

const char * elster::geterrormsg(void)
{
  return elster_instance::GetErrMsg();
}

const char * elster::getvaluemsg()
{
  return elster_instance::GetValueMsg();
}

const char * elster::getname(const char * params) // "000b"
{
  return elster_instance::GetName(params);
}

const char * elster::gettype(const char * params) // "000b"
{
  return elster_instance::GetType(params);
}

const char * elster::getvalue(const char * params) // "680 601 000a"
{
  return elster_instance::GetValue(params);
}

bool elster::setvalue(const char * params) // "680 601 000a 0000"
{
  return elster_instance::SetValue(params);
}

bool elster::setbits(const char * params) // "680 601 000a 0000"
{
  return elster_instance::SetBits(params);
}

bool elster::clrbits(const char * params) // "680 601 000a 0000"
{
  return elster_instance::ClrBits(params);
}

const char * elster::getstring(const char * params) // "680 601 000a"
{
  return elster_instance::GetString(params);
}

bool elster::setstring(const char * params) // "680 601 000a 48.5"
{
  return elster_instance::SetString(params);
}

const char * elster::toggle_trace(void)
{
  return elster_instance::ToggleTrace();
}

bool elster::setdev(const char * params)
{
  return elster_instance::SetDev(params);
}

bool elster::setcs(void)
{
  return elster_instance::SetCS();
}

bool elster::set_can232(void)
{
  return elster_instance::SetCAN232();
}

bool elster::initcan()
{
  return elster_instance::InitCan();
}

void elster::undef()
{
  elster_instance::Undef();
}

bool elster::setsniffedframe(const char * params)
{
  return elster_instance::SetSniffedFrame(params);
}

const char * elster::getsniffedvalue()
{
  return elster_instance::GetSniffedValue();
}

