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

#ifndef XML_PARSER_H

  #define XML_PARSER_H

  #include "KXmlNodeList.h"
  #include "KStream.h"
  #include "NCanUtils.h"

  // xml Grammar:
  //
  // body ::= [comment] "<" Name (attribute)* ( "/>" | ">" (body)* "</" Name ">")
  // attribute ::= AttributeId "="  """"  Value  """"
  // alternativ "comment":   <!-- comment -->
  class KParser : public KStream
  {
    private:
      int mStreamLen;      // strlen(mStream)
      const char * mStream;// XML stream (0-terminated)

      int mActualPos;      // actual parser position

      int mErrorPos;       // error position in mStream

    public:
      int mRecursionDepth; // recursion depth within xml stream
      bool mStatusOk;      // parser status

      // GetAttribute results
      // GetAttribute parses the xml stream
      //   Attr="Value"
      //   ^     ^- mValueLen
      //   +------- mAttribute
      int  mAttributePos;
      int  mValuePos;
      int  mValueLen;
      TTagName mAttribute;
 //     KStringStream mErrorMsg;

      KParser();

      void GetAttribute();

      void GetTagName(int & TagNamePos, TTagName & TagName);

      void SetError(int ErrPos);

      // init. parser
      void Initialize();
      // parses header of the form:
      //   <?xml version="1.0" encoding="UTF-8"?> analysieren.
      bool ParseXmlHeader();

      // the character at the parser position might not be:
      //   0, '/', '>', or '<'
      bool IsActualPositionValid();

      int GetIdentifier(char * Id, int MaxIdLen);

      // skips blanks, line feeds and tabulator characters
      char SkipSpaces();

      // skips comments
      // if the result is true: the next parsing character is an '<'
      bool StartNewBody(KXmlNode * NewNode = NULL);

      // EinBody sollte mit "</BodyName>" abgeschlossen sein.
      void TestBodyEnd(TTagName & TagName);

      // it expects Char at the parser position (all other characters are an
      // error)
      void NeedChar(char Char);
      // true, if Char is at the parser position
      bool TestChar(char Char);
      // "/>": there is not a subbody
      // ">": there is a subbody
      bool HasSubBody();

      bool GenerateFromXml(KXmlNode * Parent);

      bool Parse(KXmlNode * Root);

      bool ParseFile_(const char * FileName, KXmlNode * Root);

      static bool ParseFile(const char * FileName, KXmlNode * Root);
  };



#endif
