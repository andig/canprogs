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

#ifndef KXmlNodeList_H

  #define KXmlNodeList_H

  typedef char TTagName[64];

  class KXmlNode;

  class KXmlAttrList
  {
    public:
      KXmlAttrList * mNext;
      const char * mName;
      const char * mValue;

     KXmlAttrList() { mNext = 0; mName = 0; mValue = 0; }
     ~KXmlAttrList();

  };

  // linked list
  class KXmlNodeList
  {
    private:
      KXmlNode * mFirstNode;       // first node in list (or NULL)
      KXmlAttrList * mAttrList;
      const char * mNodeValue;

    public:
      TTagName mName;

      KXmlNodeList();
      virtual ~KXmlNodeList();

      void Init();
      void Append(KXmlNode * Node);
      KXmlNode * AppendNode(const char * Name, const char * Text = NULL);

      KXmlNode * GetFirstNode() const { return mFirstNode; }
      KXmlNode * GetLastNode() const;
      KXmlAttrList * GetAttrList() const { return mAttrList; }
      const KXmlAttrList * GetPrevAttr(const KXmlAttrList * Attr) const;
      const KXmlAttrList * GetLastAttr() const { return GetPrevAttr(0); }

      void SetNodeValue(const char * Value, int ValueLen);
      const char * GetNodeValue() const { return mNodeValue; };

      // start with Node == NULL
      bool GetNextNode(const KXmlNode * & Node) const;
      bool GetPrevNode(const KXmlNode * & Node) const;

      bool IsEmpty() const { return mFirstNode == 0; }

      void AppendAttr(const char * Name, const char * ValuePos, int ValueLen);
      void AppendAttr(const char * Name, const char * Value);

      KXmlNode * FindNext(const char * Name, KXmlNode * NextNode) const;
      KXmlNode * Find(const char * Name) const;

      const char * FindAttr(const char * Name) const;
      const char * FindNodeAttr(const char * NodeName, const char * Name);

      bool channel_check(bool use_msql, const char * template_, class KStream * Stream) const;

      static unsigned ResolveEntities(char * str);

  };


  class KXmlNode : public KXmlNodeList
  {
    public:
      KXmlNode * mNext;  // pointer to next node
      const char * mText;
      unsigned short can_value;
      double double_value;
      int time_days;
      int time_ms;

      KXmlNode() { mNext = 0; mText = NULL; }
      ~KXmlNode();
      const KXmlNode * GetNext() const { return mNext; }
      void BuildStream(KStream & Stream) const;
      void BuildRootStream(KStream & Stream) const;

      bool set_channel_value(class KStream * stream, const char * template_, bool trace);
  };

  bool test_template(const char * template_);


#endif
