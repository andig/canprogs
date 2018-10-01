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

#include "NTypes.h"

#include "NUtils.h"
#include "XmlParser.h"

using namespace NUtils;


static bool IsFirstIdentChar(char aCh)
{
  if (aCh == '_' ||
      ('a' <= aCh && aCh <= 'z') ||
      ('A' <= aCh && aCh <= 'Z'))
    return true;

  return false;
}

static bool IsIdentifierChar(char aCh)
{
  if ('0' <= aCh && aCh <= '9')
    return true;

  if (aCh == ':' || aCh == '-' || aCh == '_')
    return true;

  return IsFirstIdentChar(aCh);
}


////////////////////////////////////////////////////////////////////////////////
//
// KParser
//
////////////////////////////////////////////////////////////////////////////////

KParser::KParser()
{
  mStream = NULL;
  Initialize();
}

void KParser::Initialize()
{
  mStream = (const char *) GetMemory();
  mStreamLen = 0;
  if (mStream)
    mStreamLen = (int) strlen(mStream);

  // if there is no stream: it is an error
  mStatusOk = mStream != NULL;
  mActualPos = 0;
  mRecursionDepth = 0;

  mErrorPos = 0;
}


void KParser::GetAttribute()
{
  if (!mStatusOk)
  {
    mAttribute[0] = 0;
    mAttributePos = 0;
    mValuePos = 0;
    mValueLen = 0;

    return;
  }

  // attribute  ::= AttributeId "="  """"  Value  """"
  mAttributePos = mActualPos;

  GetIdentifier(mAttribute, High(mAttribute));

  NeedChar('=');
  NeedChar('"');
  mValuePos = mActualPos;
  mValueLen = 0;
  if (mStatusOk)
  {
    bool Loop = true;
    do
    {
      char Ch =  mStream[mActualPos];

      switch (Ch)
      {
        case '"':
          mActualPos++;
          Loop = false;
          break;    // alles i.o.

        case 0:
        case '>':
        case '<':
          SetError(mValuePos);
          Loop = false;
          break;

        default:
          mValueLen++;
          mActualPos++;
          break;
      }
    } while (Loop);
  }
}

void KParser::GetTagName(int & aTagNamePos, TTagName & aTagName)
{
  NeedChar('<');
  SkipSpaces();
  aTagNamePos = mActualPos;

  GetIdentifier(aTagName, High(aTagName));
}

bool KParser::IsActualPositionValid()
{
  if (mStatusOk)
  {
    char Ch = SkipSpaces();
    if (Ch != 0 &&
        Ch != '<' &&
        Ch != '/' &&
        Ch != '>')
      return true;
  }
  return false;
}

int KParser::GetIdentifier(char * Id, int MaxIdLen)
{
  int Len = 0;

  if (mStatusOk)
  {
    SkipSpaces();

    if (IsFirstIdentChar(mStream[mActualPos]))
    {
      while (IsIdentifierChar(mStream[mActualPos]))
      {
        if (Len < MaxIdLen-1)
          Id[Len] = mStream[mActualPos];
        Len++;
        mActualPos++;
      }
    }
    if (!Len)
      SetError(-1);
  }
  Id[Len < MaxIdLen ? Len : MaxIdLen-1] = 0;

  return Len;
}

bool KParser::ParseXmlHeader()
{
  if (!mStatusOk || !mStream[mActualPos])
    return false;

  // UTF-8 BOM (byte order mark)
  if (mStream[mActualPos] == (char) 0xEF &&
      mStream[mActualPos+1] == (char) 0xBB &&
      mStream[mActualPos+2] == (char) 0xBF)
    mActualPos += 3;

  if (mStream[mActualPos] == '<' &&
      mStream[mActualPos + 1] == '?')
  {
    NeedChar('<');
    NeedChar('?');
    if (mStatusOk)
    {
      SkipSpaces();
      if (!strncmp(mStream + mActualPos, "xml", 3))
        mActualPos += 3;
      else
        SetError(-1);
    }
    if (mStatusOk)
    {
      int Pos = mActualPos;

      while (mStream[mActualPos] && mStream[mActualPos] != '?')
        mActualPos++;
      if (mStream[mActualPos] == '?')
        mActualPos++;
      else
        SetError(Pos);
    }
    NeedChar('>');
  }
  return StartNewBody(NULL);
}

void KParser::SetError(int ErrPos)
{
  if (mStatusOk)
  {
    mStatusOk = false;
    if (ErrPos >= 0)
      mErrorPos = ErrPos;
    else
      mErrorPos = mActualPos;
  }
}

char KParser::SkipSpaces()
{
  char ch = 0;
  if (mStatusOk)
  {
    do
    {
      ch = mStream[mActualPos++];
    } while (0 < ch && ch <= ' ');
    mActualPos--;
  }
  return ch;
}

bool KParser::StartNewBody(KXmlNode * NewNode)
{
  bool Comment;
  char Ch;
  int  Pos;

  if (!mStatusOk)
    return false;

  do
  {
    // [comment]                            skip comment
    Pos = mActualPos;

    Ch = mStream[mActualPos];
    while (Ch != 0 && Ch != '<' && /* Ch != '/' && */ Ch != '>')
    {
      mActualPos++;
      Ch = mStream[mActualPos];
    }

    // "<" Name (attribute)* ...            a tag starts with "<"
    if (Ch != '<')
    {
      mActualPos = Pos;

      return false;    // '<' is missing (it is an error)
    }
                       // *aParsePos == '<'

    if (NewNode)
    {
      while (mStream[Pos] && mStream[Pos] <= ' ')
        Pos++;

      if (mActualPos > Pos+1)
      {
        int len = mActualPos-Pos;
        NewNode->mText = new char[len+1];
        strncpy((char *)NewNode->mText, (const char *)mStream + Pos, len);
        const_cast<char *>(NewNode->mText)[len] = 0;
      }
    }

    Comment = false;
    // <!-- Sample XML file -->
    if (!strncmp(mStream + mActualPos, "<!--", 4))
    {
      mActualPos += 4;
      SkipSpaces();
      do
      {
        if (mStream[mActualPos] == '-' &&
            !strncmp(mStream + mActualPos, "-->", 3))
        {
          mActualPos += 3;
          Comment = true;
          break;
        }
        mActualPos++;
      } while (true);
      if (!Comment)
        return false;  // end of comment is missing (it is an error)
    }
  } while (Comment);

  // test for body end:
  //   if there is "< /": jump back
  Pos = mActualPos;
  mActualPos++;
  Ch = SkipSpaces();
  mActualPos = Pos;
  if (Ch == '/')
  {
    return false; // body end found
  }
  // there is:
  //   itsXmlParser.mActualPos[0] = '<'

  return mStatusOk;
}

void KParser::TestBodyEnd(TTagName & TagName)
{
  // test for "Body" end: </TagName>
  NeedChar('<');
  NeedChar('/');
  if (mStatusOk)
  {
    TTagName EndTag;

    SkipSpaces();
    int ErrPos = mActualPos;
    GetIdentifier(EndTag, High(EndTag));
    if (strcmp(TagName, EndTag))
      SetError(ErrPos);
  }
  NeedChar('>');
}

void KParser::NeedChar(char Char)
{
  if (mStatusOk)
  {
    char ch = SkipSpaces();
    if (ch == Char)
      mActualPos++;
    else {
      SetError(-1);
    }
  }
}

bool KParser::TestChar(char Char)
{
  if (mStatusOk)
  {
    char ch = SkipSpaces();
    if (ch == Char)
    {
      mActualPos++;
      return true;
    }
  }
  return false;
}

bool KParser::HasSubBody()
{
  //                        v will be tested
  // "<" Name (attribute)* "/>"                   there is no subbody
  //
  // "<" Name (attribute)* ">" (body)* "</" Name ">"
  //                        ^ will be tested

  bool NoSubBody = TestChar('/');

  NeedChar('>');

  return mStatusOk && !NoSubBody;
}

bool KParser::GenerateFromXml(KXmlNode * Parent)
{
  int TagNamePos;
  TTagName TagName;
  KXmlNode * NewNode = NULL;

  if (!mStatusOk)
  {
    return false;
  }

  mRecursionDepth++;

  GetTagName(TagNamePos, TagName);
  if (mRecursionDepth == 1)
  {
    strcpy(Parent->mName, TagName);
    NewNode = Parent;
  } else
  if (Parent)
  {
    NewNode = new KXmlNode;
    strcpy(NewNode->mName, TagName);
    if (NewNode)
      Parent->Append(NewNode);
  }

  while (IsActualPositionValid())
  {
    GetAttribute();
    if (NewNode)
    {
      NewNode->AppendAttr(mAttribute, mStream + mValuePos, mValueLen);
    }
  }

  if (HasSubBody())
  {
    SkipSpaces();

    int StartPos = mActualPos;
    int Count = 0;
    while (StartNewBody(NewNode))
    {
      GenerateFromXml(mRecursionDepth == 1 ? Parent : NewNode);
      Count++;
    }
    if (Count == 0)
    {
      int EndPos = mActualPos-1;
      while (EndPos >= StartPos &&
             0 < (unsigned char) mStream[EndPos] && (unsigned char) mStream[EndPos] <= ' ')
        EndPos--;
      if (EndPos >= StartPos)
      {
        NewNode->SetNodeValue(mStream + StartPos, EndPos - StartPos + 1);
      }
    }

    TestBodyEnd(TagName);
  }

  mRecursionDepth--;

  return mStatusOk;
}

bool KParser::ParseFile_(const char * FileName, KXmlNode * Root)
{
  if (!ReadFile(FileName))
    return false;

  return Parse(Root);
}

bool KParser::Parse(KXmlNode * Root)
{
  Initialize();
  ParseXmlHeader();
  GenerateFromXml(Root);
  bool Result = mStatusOk;
  if (!Result &&
      0 <= mErrorPos && mErrorPos < mStreamLen)
  {
    printf("xml-parser error pos: ");
    for (int i = 0; i < 80 && mErrorPos + i < mStreamLen; i++)
      printf("%c", mStream[mErrorPos + i]);
    printf("\n");
  }

  return Result;
}

bool KParser::ParseFile(const char * FileName, KXmlNode * Root)
{
  KParser Parser;

  return Parser.ParseFile_(FileName, Root);
}



