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

// Singleton für das Python- bzw. das Perl-Interface (elster.so bzw. elster_perl.so)

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#include <stdio.h>
#include <string.h>

#include "NTypes.h"

#include "elster_instance.h"

#include "NUtils.h"
#include "KElsterTable.h"
#include "KCanElster.h"

static KCanElster * Can = NULL;

bool CanIsInit = false;
bool trace = false;
#if defined(__WINDOWS__)
  enum NCanUtils::driver_type drv_type = NCanUtils::dt_can232;
  char can_dev[128] = "COM1";
#else
  enum NCanUtils::driver_type drv_type = NCanUtils::dt_can;
  char can_dev[128] = "can0";
#endif
char err_msg[128];
char value_msg[128];

static void PrintTrace()
{
  if (trace)
    printf("elster: %s\n", err_msg);
}

bool elster_instance::InitCan()
{
  if (CanIsInit)
    return true;
  
  if (!Can)
    Can = new KCanElster;

  Can->Trace = trace;
  Can->driver_type = drv_type;
  if (!Can->Init(can_dev, 0x680))
  {
    Can->Resume();
    NUtils::SleepMs(100);
    Undef();
    
    sprintf(err_msg, "device \"%s\" not connected\n", can_dev);
    PrintTrace();
        
    return false;
  }
  CanIsInit = true;
  drv_type = Can->driver_type;
  Can->Resume();

  if (trace)
    printf("elster: can connected\n");

  return true;
}

static bool elsterInit(unsigned short send_id)
{
  if (send_id < 0x680 && (send_id & 0x070) != 0)
  {
    sprintf(err_msg, "not allowed send_id \"%x\"", send_id);
    PrintTrace();
    return false;
  }

  if (!elster_instance::InitCan() || Can == NULL)
    return false;

  Can->SendCanId = send_id;
  
  return true;
}

static unsigned short get_hex(const char * & params)
{
  if (*err_msg)
    return 0;
  
  while (*params == ' ')
    params++;

  if (NUtils::IsHexDigit(*params))
  {
    return (unsigned short) NUtils::GetHex(params);
  } else {
    sprintf(err_msg, "parameter \"%s\" is not hex value", params);
    PrintTrace();
  }

  return 0;
}

static bool test_end(const char * & params)
{
  if (*err_msg)
    return true;
  
  while (*params == ' ')
    params++;

  if (*params)
  {
    strcpy(err_msg, "too much parameters");
    PrintTrace();
  }

  return *err_msg;
}

static bool GetParam(const char * params, unsigned short & value)
{
  value = get_hex(params);
  
  return test_end(params);
}

static bool GetTwoParams(const char * params, unsigned short & value1,
                         unsigned short & value2)
{
  value1 = get_hex(params);
  value2 = get_hex(params);
  
  return test_end(params);
}

static bool GetThreeParams(const char * params, unsigned short & value1,
                           unsigned short & value2)
{
  unsigned short send_id = get_hex(params);
  value1 = get_hex(params);
  value2 = get_hex(params);

  if (test_end(params))
    return false;

  return elsterInit(send_id);
}

static bool GetFourParams(const char * params, unsigned short & value1,
                          unsigned short & value2, unsigned short & value3)
{
  unsigned short send_id = get_hex(params);
  value1 = get_hex(params);
  value2 = get_hex(params);
  value3 = get_hex(params);
  
  if (test_end(params))
    return false;
  
  return elsterInit(send_id);
}

static bool GetValue(unsigned short recv_id, unsigned short elster_idx, unsigned short & value)
{
  if (!Can->GetValue(recv_id, elster_idx, value))
  {
    sprintf(err_msg, "value (%4.4x) not read", elster_idx);
    PrintTrace();
    return false;
  }
  return true;
}

static bool SetValue(unsigned short recv_id, unsigned short elster_idx, unsigned short new_val)
{
  if (!Can->SetValue(recv_id, elster_idx, new_val))
  {
    sprintf(err_msg, "value %4.4x (%4.4x) not saved", new_val, elster_idx);
    PrintTrace();
    return false;
  }
  return true;  
}

////////////////////////////////////////////////////////////////////////////////

const char * elster_instance::GetErrMsg()
{
  return err_msg;
}

const char * elster_instance::GetValueMsg()
{
  return value_msg;
}

const char * elster_instance::GetType(const char * params)
{
  err_msg[0] = 0;
  value_msg[0] = 0;
  unsigned short elster_idx;
  
  strcpy(value_msg, "?");
  if (GetParam(params, elster_idx))
    return value_msg;
  
  const ElsterIndex * Index = GetElsterIndex(elster_idx);
  if (Index)
  {
    int t = Index->Type;
    sprintf(value_msg, "%d", t);
  }
  return value_msg;
}

const char * elster_instance::GetName(const char * params)
{
  err_msg[0] = 0;
  value_msg[0] = 0;
  unsigned short elster_idx;
  
  strcpy(value_msg, "?");
  if (GetParam(params, elster_idx))
    return value_msg;

  const ElsterIndex * Index = GetElsterIndex(elster_idx);
  if (Index)
  {
    if (Index->Name)
      strcpy(value_msg, Index->Name);
  }
  return value_msg;
}

const char * elster_instance::GetValue(const char * params)
{
  err_msg[0] = 0;
  value_msg[0] = 0;
  unsigned short recv_id;
  unsigned short elster_idx;
  unsigned short val;
  
  if (!GetThreeParams(params, recv_id, elster_idx))
    return value_msg;
  
  if (!::GetValue(recv_id, elster_idx, val))
  {    
    return value_msg;
  }

  sprintf(value_msg, "%4.4x", val);
  
  return value_msg;
}

const char * elster_instance::GetString(const char * params)
{
  err_msg[0] = 0;
  value_msg[0] = 0;
  unsigned short recv_id;
  unsigned short elster_idx;
  unsigned short val;
  
  strcpy(value_msg, "?");
  if (!GetThreeParams(params, recv_id, elster_idx))
    return value_msg;
  
  if (!Can->GetValue(recv_id, elster_idx, val))
  {
    sprintf(err_msg, "value (%4.4x) not read", elster_idx);
    PrintTrace();
    return value_msg;
  }
  
  const ElsterIndex * elst_ind = GetElsterIndex(elster_idx);
  if (elst_ind)
  {
    double d;
    
    if (Can->GetDoubleValue(val, recv_id, elster_idx, elst_ind->Type, d))
    {
      SetDoubleType(value_msg, elst_ind->Type, d);
    } else
      SetValueType(value_msg, elst_ind->Type, val);
  } else
    sprintf(value_msg, "%d", val);
  
  return value_msg;
}

bool elster_instance::SetValue(const char * params)
{
  err_msg[0] = 0;
  unsigned short recv_id;
  unsigned short elster_idx;
  unsigned short new_val;
  
  if (!::GetFourParams(params, recv_id, elster_idx, new_val))
    return false;
  
  return ::SetValue(recv_id, elster_idx, new_val);
}

bool elster_instance::SetString(const char * params)
{
  err_msg[0] = 0;
  unsigned short send_id = get_hex(params);
  unsigned short recv_id = get_hex(params);
  unsigned short elster_idx = get_hex(params);
  
  if (*err_msg)
    return false;
  
  const ElsterIndex * Index = GetElsterIndex(elster_idx);
  if (!Index)
  {
    sprintf(err_msg, "index \"%x\" not in elster table", elster_idx);
    PrintTrace();
    return false;
  }
  int new_val = TranslateString(params, Index->Type);
  if (new_val < 0)
  {
    sprintf(err_msg, "elster type not supported");
    PrintTrace();
    return false;
  }

  if (test_end(params))
    return false;
  
  if (!elsterInit(send_id))
    return false;
  
  return ::SetValue(recv_id, elster_idx, new_val);
}

bool elster_instance::SetBits(const char * params)
{
  err_msg[0] = 0;
  value_msg[0] = 0;
  unsigned short recv_id;
  unsigned short elster_index;
  unsigned short bits;
  unsigned short value;
  
  if (!GetFourParams(params, recv_id, elster_index, bits))
    return false;
  
  if (!::GetValue(recv_id, elster_index, value))
    return false;
  
  value |= bits;
  sprintf(value_msg, "%4.4x", value);
  
  return ::SetValue(recv_id, elster_index, value);
}

bool elster_instance::ClrBits(const char * params)
{
  err_msg[0] = 0;
  value_msg[0] = 0;
  unsigned short recv_id;
  unsigned short elster_index;
  unsigned short bits;
  unsigned short value;
  
  if (!GetFourParams(params, recv_id, elster_index, bits))
    return false;
  
  if (!::GetValue(recv_id, elster_index, value))
    return false;
  
  value &= ~bits;
  sprintf(value_msg, "%4.4x", value);
  
  return ::SetValue(recv_id, elster_index, value);
}

const char * elster_instance::ToggleTrace(void)
{
  err_msg[0] = 0;

  trace = !trace;
  strcpy(value_msg, trace ? "true" : "false");
  
  if (trace)
    printf("elster build time: %s %s\n", __DATE__, __TIME__);
  
  return value_msg;
}

bool elster_instance::SetDev(const char * dev)
{
  err_msg[0] = 0;
  
  if (!dev)
    return false;
  
  if (strlen(dev) >= High(can_dev))
  {
    sprintf(err_msg, "invalid device \"%s\"", dev);
    PrintTrace();
    
    return false;
  }
  strcpy(can_dev, dev);
  return true;
}

bool elster_instance::SetCS(void)
{
  err_msg[0] = 0;
  if (CanIsInit && drv_type != NCanUtils::dt_cs)
  {
    strcpy(err_msg, "cannot change device");
    PrintTrace();
    return false;
  }
  drv_type = NCanUtils::dt_cs;
  return true;
}

bool elster_instance::SetCAN232(void)
{
  err_msg[0] = 0;
  if (CanIsInit && drv_type != NCanUtils::dt_can232)
  {
    strcpy(err_msg, "cannot change device");
    PrintTrace();
    return false;
  }
  drv_type = NCanUtils::dt_can232;

  return true;
}

void elster_instance::Undef()
{
  err_msg[0] = 0;
  if (Can)
  {
    Can->Halt();
    NUtils::SleepMs(1000);
    
    delete Can;
  }
  Can = NULL;
  CanIsInit = false;
}

bool elster_instance::SetSniffedFrame(const char * params)
{
  err_msg[0] = 0;
  value_msg[0] = 0;
  unsigned short recv_id;
  unsigned short elster_idx;
  KCanFrame Frame;
  
  if (GetTwoParams(params, recv_id, elster_idx))
    return false;
  
  Frame.InitElsterFrame(0x680, recv_id, elster_idx);
  
  Frame.Len = 5;
  Frame.Data[0] &= 0xf8; // clear flag
  
  bool Ok = false;
  if (Can)
    Ok = Can->SniffedData.AddToSniffed(Frame);

  if (trace)
  {
    if (Ok)
      printf("elster: set sniffed: %s\n", params);
    else {
      sprintf(err_msg, "elster: sniffed not set: %s", params);
      PrintTrace();
    }
  }
  return Ok;
}

static void SetValueMsg(const KSniffedFrame * SniffedFrame)
{
  if (SniffedFrame)
  {
    char res[64] = {0};
    const ElsterIndex * elst_ind = GetElsterIndex(SniffedFrame->elster_idx);
    unsigned short Value = SniffedFrame->value;
    if (SniffedFrame->elster_idx)
      SetValueType(res, elst_ind->Type, Value);
    else
      sprintf(res, "%4.4x", Value);
    
    sprintf(value_msg, "0x%3.3x 0x%4.4x %s", SniffedFrame->recv_id, SniffedFrame->elster_idx, res);
    if (trace)
      printf("elster: get sniffed: %s\n", value_msg);
  }
}

const char * elster_instance::GetSniffedValue(const char * params)
{
  err_msg[0] = 0;
  value_msg[0] = 0;
  unsigned short recv_id;
  unsigned short elster_idx;
  const KSniffedFrame * SniffedFrame = NULL;
  
  strcpy(value_msg, "?");
  if (!params ||
      GetTwoParams(params, recv_id, elster_idx))
    return value_msg;

  if (Can)
  {
    Can->EmptyServer();
    SniffedFrame = Can->SniffedData.SearchSniffedFrame(recv_id, elster_idx);
  }
  SetValueMsg(SniffedFrame);
 
  return value_msg;
}

const char * elster_instance::GetSniffedValue()
{
  err_msg[0] = 0;
  value_msg[0] = 0;
  
  strcpy(value_msg, "?");
  if (!Can)
    return value_msg;
  
  Can->EmptyServer();
  const KSniffedFrame * SniffedFrame = Can->GetFirstSniffedFrame();
  
  SetValueMsg(SniffedFrame);
  
  return value_msg;
}
