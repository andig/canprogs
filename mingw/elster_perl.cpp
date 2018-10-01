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

const char * elster_perl::geterrormsg(void)
{
  return elster_instance::GetErrMsg();
}

const char * elster_perl::getvaluemsg()
{
  return elster_instance::GetValueMsg();
}

const char * elster_perl::getname(const char * params) // "000b"
{
  return elster_instance::GetName(params);
}

const char * elster_perl::gettype(const char * params) // "000b"
{
  return elster_instance::GetType(params);
}

const char * elster_perl::getvalue(const char * params) // "680 601 000a"
{
  return elster_instance::GetValue(params);
}

bool elster_perl::setvalue(const char * params) // "680 601 000a 0000"
{
  return elster_instance::SetValue(params);
}

bool elster_perl::setbits(const char * params) // "680 601 000a 0000"
{
  return elster_instance::SetBits(params);
}

bool elster_perl::clrbits(const char * params) // "680 601 000a 0000"
{
  return elster_instance::ClrBits(params);
}

const char * elster_perl::getstring(const char * params) // "680 601 000a"
{
  return elster_instance::GetString(params);
}

bool elster_perl::setstring(const char * params) // "680 601 000a 48.5"
{
  return elster_instance::SetString(params);
}

const char * elster_perl::toggle_trace(void)
{
  return elster_instance::ToggleTrace();
}

bool elster_perl::setdev(const char * params)
{
  return elster_instance::SetDev(params);
}

bool elster_perl::setcs(void)
{
  return elster_instance::SetCS();
}

bool elster_perl::set_can232(void)
{
  return elster_instance::SetCAN232();
}

bool elster_perl::initcan()
{
  return elster_instance::InitCan();
}

void elster_perl::undef()
{
  elster_instance::Undef();
}

bool elster_perl::setsniffedframe(const char * params)
{
  return elster_instance::SetSniffFrame(params);
}

const char * elster_perl::getsniffedvalue(void)
{
  return elster_instance::GetSniffedValue();
}

const char * elster_perl::getfirstsniffedvalue(const char * params)
{
  return elster_instance::GetFirstSniffedValue(params);
}


