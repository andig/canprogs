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
#include <string.h>

#include "NTypes.h"

#include "KXmlNodeList.h"

#include "NUtils.h"

#include "KStream.h"
#include "KElsterTable.h"

#if defined(__UVR__)
  #include "KCanUVR.h"
#endif


void KXmlNodeList::SetNodeValue(const char * Value, int ValueLen)
{
  if (mNodeValue)
    delete [] mNodeValue;
  mNodeValue = NULL;

  if (Value && ValueLen > 0)
  {
    mNodeValue = new char [ValueLen+1];
    memcpy(const_cast<char *> (mNodeValue), Value, ValueLen);
    const_cast<char *> (mNodeValue)[ValueLen] = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// KXmlNodeList
//
////////////////////////////////////////////////////////////////////////////////

unsigned KXmlNodeList::ResolveEntities(char * str)
{
  unsigned len_out = 0;
  unsigned len = (unsigned) strlen(str);
  for (int i = 0; i < (int) len; i++)
  {
    char c = str[i];
    if (c == '&')
    {
      if (!strncmp(str + i, "&amp;", 5))
      {
        c = '&';
        i += 4;
      } else
        if (!strncmp(str + i, "&apos;", 6))
        {
          c = '\'';
          i += 5;
        } else
          if (!strncmp(str + i, "&quot;", 6))
          {
            c = '"';
            i += 5;
          }
    }
    str[len_out++] = c;
  }
  str[len_out] = 0;

  return len_out;
}

void KXmlNodeList::AppendAttr(const char * Name, const char * ValuePos, int ValueLen)
{
  KXmlAttrList * Attr = new KXmlAttrList;

  if (Name && Name[0])
  {
    Attr->mName = new char[strlen(Name)+1];
    strcpy(const_cast<char *>(Attr->mName), Name);
  }
  if (ValuePos && ValueLen > 0)
  {
    char buffer[1024];

    if (ValueLen >= 1023)
      ValueLen = 1023;

    strncpy(buffer, ValuePos, ValueLen);
    buffer[ValueLen] = 0;

    unsigned len_out = ResolveEntities(buffer);

    Attr->mValue = new char[len_out + 1];
    strcpy(const_cast<char *>(Attr->mValue), buffer);
  }
  if (!mAttrList)
    mAttrList = Attr;
  else {
    KXmlAttrList * AttrList = mAttrList;
    while (AttrList->mNext)
      AttrList = AttrList->mNext;
    AttrList->mNext = Attr;
  }
}

void KXmlNodeList::AppendAttr(const char * Name, const char * Value)
{
  int len = Value ? (int) strlen(Value) : 0;

  AppendAttr(Name, len ? Value : "", len);
}

const char * KXmlNodeList::FindAttr(const char * Name) const
{
  const KXmlAttrList * AttrList = mAttrList;
  while (AttrList)
  {
    if (AttrList->mName &&
        !strcmp(AttrList->mName, Name))
      return AttrList->mValue;

    AttrList = AttrList->mNext;
  }
  return NULL;
}

const char * KXmlNodeList::FindNodeAttr(const char * NodeName, const char * Name)
{
  const KXmlNode * Node = Find(NodeName);

  return Node ? Node->FindAttr(Name) : NULL;
}

KXmlNodeList::KXmlNodeList()
{
  mName[0] = 0;
  mFirstNode = NULL;
  mAttrList = NULL;
  mNodeValue = NULL;
}

KXmlNodeList::~KXmlNodeList()
{
  Init();
}

void KXmlNodeList::Init()
{
  mName[0] = 0;

  while (mFirstNode)
  {
    KXmlNode * Node = mFirstNode->mNext;

    delete mFirstNode;

    mFirstNode = Node;
  }

  if (mAttrList)
    delete mAttrList;
  mAttrList = NULL;

  SetNodeValue(NULL, 0);
}

void KXmlNodeList::Append(KXmlNode * Node)
{
  Node->mNext = NULL;
  if (mFirstNode)
  {
    KXmlNode * node = mFirstNode;

    while (node->mNext)
      node = node->mNext;

    node->mNext = Node;
  } else
    mFirstNode = Node;
}

KXmlNode * KXmlNodeList::AppendNode(const char * Name, const char * Text)
{
  KXmlNode * res = new KXmlNode;

  if (Text)
  {
    res->mText = new char[strlen(Text)+1];
    strcpy((char *) res->mText, Text);
  }
  int len = Name ? (int) strlen(Name) : 0;
  if (len >= (int) High(res->mName))
    len = (int) High(res->mName) - 1;
  if (len > 0)
    strncpy(res->mName, Name, len);

  res->mName[len] = 0;

  Append(res);

  return res;
}

const KXmlAttrList * KXmlNodeList::GetPrevAttr(const KXmlAttrList * Attr) const
{
  if (Attr == mAttrList)
    return NULL;

  const KXmlAttrList * ResAttr = mAttrList;

  while (ResAttr && ResAttr->mNext != Attr)
    ResAttr = ResAttr->mNext;

  return ResAttr;
}

bool KXmlNodeList::GetNextNode(const KXmlNode * & Node) const
{
  if (!Node)
    Node = mFirstNode;
  else
    Node = Node->mNext;

  return Node != NULL;
}

bool KXmlNodeList::GetPrevNode(const KXmlNode * & Node) const
{
  if (Node == mFirstNode)
  {
    Node = NULL;
    return false;
  }
  const KXmlNode * ResNode = mFirstNode;
  while (ResNode && ResNode->mNext != Node)
    ResNode = ResNode->mNext;

  Node = ResNode;

  return Node != NULL;
}

KXmlNode * KXmlNodeList::GetLastNode() const
{
  const KXmlNode * ResNode = NULL;

  GetPrevNode(ResNode);

  return const_cast<KXmlNode *> (ResNode);
}

KXmlNode * KXmlNodeList::FindNext(const char * Name, KXmlNode * NextNode) const
{
  if (!Name || !Name[0])
    return NULL;

  while (NextNode)
  {
    if (NextNode->mName &&
        !strcmp(NextNode->mName, Name))
      return NextNode;

    KXmlNode * res = NextNode->Find(Name);
    if (res)
      return res;

    NextNode = NextNode->mNext;
  }
  return NULL;
}

KXmlNode * KXmlNodeList::Find(const char * Name) const
{
  return FindNext(Name, mFirstNode);
}

bool KXmlNodeList::channel_check(bool use_mysql, const char * template_, KStream * Stream) const
{
  if (!Stream)
    return false;

  char param_value[256];
  bool Ok = true;
  int cnt = 1;
  const KXmlNode * Node = NULL;
  while (GetNextNode(Node))
  {
    if (strlen(Node->mName) == 0)
    {
      printf("invalid xml file\n");
      return false;
    }

    if (!strcmp(Node->mName, "channel"))
    {
    #if !defined(__UVR__)
      const char * can_msg = Node->FindAttr("can_msg");
      // elster: <can id>.<elster index> (4-stellig hex.)
      const char * can_inst = Node->FindAttr("can_inst");
    #else
       // uvr:    <can_id>.<1..10>        (hex.)
      const char * can_val = Node->FindAttr("can_val");
    #endif

      const char * uuid = Node->FindAttr("uuid");
      const char * query = Node->FindAttr("query");
      const char * name = Node->FindAttr("Name");
      const char * format = Node->FindAttr("format");

      if (!name || strlen(name) == 0)
      {
        printf("\"Name\" is missing in channel %d\n", cnt);
        return false;
      }
      Stream->SetLength(0);
      if (query || !uuid)
      {
        if (template_)
        {
          Ok = Stream->InitString(template_);
          if (query)
            Ok = Stream->Replace("%s", query);
          if (!Ok)
          {
            printf("\"%cs\" needed within \"template\" (%s)!\n", '%', name);
            return false;
          }
        } else
          Ok = Stream->InitString(query);

        if (Ok &&
            strstr((const char *) Stream->GetMemory(), "${value}"))
          Ok = Stream->Replace("${value}", "0.0");

        if (Ok &&
            strstr((const char *) Stream->GetMemory(), "${ts}"))
          Ok = Stream->Replace("${ts}", "0000");

        if (Ok &&
            strstr((const char *) Stream->GetMemory(), "${timestamp}"))
          Ok = Stream->Replace("${timestamp}", "0000");

        // search value in xml node
       long pos = 0;
        while (Ok)
        {
          const char * param = strstr((const char *) Stream->GetMemory() + pos, "${");
          if (!param)
          {
            break;
          }
          pos = param - (const char *) Stream->GetMemory();
          const char * param_end = param ? strstr(param, "}") : NULL;
          if (!param_end)
          {
            Ok = false;
            break;
          }
          long param_len = param_end - param + 1;
          if (param_len <= 3 || param_len >= (long) High(param_value))
          {
            Ok = false;
            break;
          }
          strncpy(param_value, param + 2, param_len-3);
          param_value[param_len-3] = 0;

          const char * par = Node->FindAttr(param_value);
          if (!par)
          {
            Ok = false;
            break;
          }
          strncpy(param_value, param, param_len);
          param_value[param_len] = 0;

          Ok = Stream->Replace(param_value, par);
        }
        if (!Ok)
        {
          printf("error in query \"%s\" (%s)\n", Stream->GetMemory(), name);
          return false;
        }
      } else
      if (use_mysql)
      {
        printf("query is missing in channel %d (%s)\n", cnt, name);
        return false;
      } else {
        if (!uuid || strlen(uuid) == 0)
        {
          printf("uuid is missing in channel %d (%s)\n", cnt, name);
          return false;
        }
      }

      if (format)
      {
        ElsterType type = GetElsterType(format);
        if (type == et_default)
        {
          printf("unknown format \"%s\" (%s)\n", format, name);
          return false;
        }
      }

    #if defined(__UVR__)
      if (!can_val)
      {
        printf("can_val is missing in channel %d (%s)\n", cnt, name);
        return false;
      }
      unsigned can_id;
      int chan;
      if (!KCanUVR::GetIdAndChan(can_val, can_id, chan))
      {
        printf("invalid can_val \"%s\" (%s)\n", can_val, name);
        return  false;
      }
    #else
      // Elster-Index
      if (can_inst)
      {
        KCanFrame Frame;

        if (!Frame.SetElsterFrame(can_inst))
        {
          printf("invalid can_inst \"%s\" (%s)\n", can_inst, name);
          return false;
        }
      } else
      if (!can_msg || !*can_msg)
      {
        printf("can_inst/can_msg is missing in channel %d (%s)\n", cnt, name);
        return false;
      } else {
        KCanFrame Frame;

        if (!Frame.GetDataFromStream(can_msg))
        {
          printf("invalid can_msg(%s): \"%s\"\n", name, can_msg);
          return false;
        }
      }
    #endif
      cnt++;
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////
//
// KXmlNode
//
////////////////////////////////////////////////////////////////////////////////

KXmlNode::~KXmlNode()
{
  if (mText)
    delete [] mText;
}

void KXmlNode::BuildStream(KStream & Stream) const
{
  Stream.AppendString("<");
  Stream.AppendString(mName);

  const KXmlAttrList * List = GetAttrList();
  while (List)
  {
    Stream.AppendString(" ");
    Stream.AppendString(List->mName);
    Stream.AppendString("=\"");
    Stream.AppendString(List->mValue);
    Stream.AppendString("\"");
    List = List->mNext;
  }

  if (GetFirstNode() || mText)
  {
    Stream.AppendString(">");
    if (mText)
    {
      Stream.AppendString(mText);
    } else
      Stream.AppendString("\n");

    const KXmlNode * Node = NULL;
    while (GetNextNode(Node))
    {
      Node->BuildStream(Stream);
    }
    Stream.AppendString("</");
    Stream.AppendString(mName);
    Stream.AppendString(">\n");
  } else
    Stream.AppendString(" />\n");
}

void KXmlNode::BuildRootStream(KStream & Stream) const
{
  Stream.SetLength(0);
  Stream.AppendString("<?xml version=\"1.0\"?>\n");

  const KXmlNode * Node = GetFirstNode();
  if (Node)
    Node->BuildStream(Stream);
}

bool KXmlNode::set_channel_value(KStream * stream, const char * template_, bool trace)
{
   if (strcmp(mName, "channel"))
     return false;

  char param_value[256];
  const char * format = FindAttr("format");
  const char * query = FindAttr("query");
  const char * uuid = FindAttr("uuid");
  ElsterType type = GetElsterType(format);
  char Value[64];
  bool Ok = true;

  if (type == et_double_val ||
      type == et_triple_val)
  {
    SetDoubleType(Value, type, double_value);
  } else
    SetValueType(Value, type, can_value);

  stream->SetLength(0);
  if (query || !uuid)
  {
    // Platzhalter ${...} durch den entsprecheden Wert ersetzen
    //
    // intern verwendete Platzhalter: ${value}, ${ts} und ${timestamp}
    //
    // alle anderen Platzhalter werden in der Parameterliste zum "channel" gesucht
    // z.B. für ${uuid}
    //
    // <channel uuid="xxx..."    ... />

    if (template_)
    {
      Ok = stream->InitString(template_);
      if (Ok && query)
        Ok = stream->Replace("%s", query);
    } else
      Ok = stream->InitString(query);

    if (Ok &&
        strstr((const char *) stream->GetMemory(), "${value}"))
      Ok = stream->Replace("${value}", Value);

    if (Ok &&
        strstr((const char *) stream->GetMemory(), "${ts}"))
    {
      NUtils::PrintDateTime(time_days, time_ms, param_value);
      Ok = stream->Replace("${ts}", param_value);
    }

    if (Ok &&
        strstr((const char *) stream->GetMemory(), "${timestamp}"))
    {
      NUtils::PrintMs(time_days, time_ms, param_value);
      Ok = stream->Replace("${timestamp}", param_value);
    }

    // search value in xml
    long pos = 0;
    while (Ok)
    {
      const char * param = strstr((const char *) stream->GetMemory() + pos, "${");
      if (!param)
        break;

      pos = param - (const char *) stream->GetMemory();
      const char * param_end = param ? strstr(param, "}") : NULL;
      if (!param_end)
        break;

      long param_len = param_end - param + 1;
      if (param_len <= 3 || param_len >= (long) High(param_value))
        break;

      strncpy(param_value, param + 2, param_len-3);
      param_value[param_len-3] = 0;

      const char * par = FindAttr(param_value);
      if (!par)
        break;

      strncpy(param_value, param, param_len);
      param_value[param_len] = 0;

      Ok = stream->Replace(param_value, par);
    }
  } else {
    Ok = stream->InitString(template_) &&
         stream->Replace("%s", uuid) &&
         stream->Replace("%s", Value);
  }
  if (!Ok && trace)
  {
    const char * name = FindAttr("Name");
    printf("\"%s\" has non value\n", name);
  }
  return Ok;
}


////////////////////////////////////////////////////////////////////////////////
//
// KXmlAttrList
//
////////////////////////////////////////////////////////////////////////////////

KXmlAttrList::~KXmlAttrList()
{
  if (mName)
    delete [] mName;
  if (mValue)
    delete [] mValue;
  if (mNext)
    delete mNext;
}


////////////////////////////////////////////////////////////////////////////////

bool test_template(const char * template_)
{
  if (!template_ || strlen(template_) == 0)
  {
    printf("template is missing\n");
    return false;
  }
  const char * ptr = strstr(template_, "%s");
  if (!ptr ||
      !strstr(ptr + 2, "%s"))
  {
    printf("two \"%c\" are needed within \"template\"!\n", '%');
    return false;
  }
  return true;
}

