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

#if !defined(VStream_H)
  #define VStream_H

  #include "KStream.h"
  #include "NTypes.h"

  class VStream : public KStream
  {
    protected:
      bool mOwnsMemory;

    public:

      VStream();
      ~VStream();

      bool SetNotOwnedMemory(unsigned char * Memory, unsigned long Length);
      virtual bool OwnsMemory() const { return mOwnsMemory; }

      bool GetFour(unsigned long Offset, FOURCC_ & FourCC);

      bool SetWord(unsigned long Offset, unsigned short w);
      bool SetUInt(unsigned long Offset, unsigned int l);
      bool SetFour(unsigned long Offset, const FOURCC_ & FourCC);
      bool SetFour(unsigned long Offset, const char * FourCC);

      bool AddWord(unsigned short w);
      bool AddLong(unsigned int l);
      bool AddString(const char * Str);
      bool AddAsciiInt(int l);
      bool AddUtf8(unsigned short w);
      bool CompareFour(unsigned long Offset, const char * FourCC);
      FOURCC_ MakeFour(const char * FourCC);
      virtual bool InsertBytes(unsigned long Offset, long Count);
      bool AddZero();

      void PrintFour(unsigned long Offset) const;
      void PrintRiff() const;

      void operator =(const VStream & A);      // copy
      void operator <<(::VStream & A);         // memory transfer
      void operator +=(const VStream & A);     // append
      void operator +=(const char * A);        // append
      virtual bool Append(const unsigned char * Stream, unsigned long StreamLen);
      virtual bool AppendString(const char * String);
      void AppendHtml(const unsigned char * utf8, unsigned long StreamLen);
      void AppendHtmlString(const char * utf8);

      // Wenn NewSize = 0: der Speicher mMemory wird freigegeben
      // Wenn NewSize < mMemorySize: mSize wird gleich NewSize gesetzt
      // sonst: es wird ein neuer Speicher alloziert. Vorhandene Daten
      //        werden in den neuen Speicher kopiert
      virtual bool SetSize(unsigned long NewSize);

      bool LoadFromFile(const char * FileName, unsigned long MaxLen = 0);
      bool SaveToFile(const char * FileName) const;
      bool SaveBinToFile(long Pos, long Length, const char * Filename, bool Append = false) const;

      bool Utf8_ToWStr(const unsigned char * Utf8, unsigned long Len);
      bool WstrToUtf8(const unsigned short * Wstr, unsigned long Len);
      bool htmlTo_utf8(const char * Html);
      static bool htmlToUtf8(char * Html);
      bool utf8_To_html(const unsigned char * Utf8, unsigned long Len);
      bool MacToUtf8(const unsigned char * Mac, unsigned long Len);
      bool WStrToMac(const unsigned short * Wstr, unsigned long Len);
      bool CP1252_ToUtf8(const unsigned char * cp1252, unsigned long Len);

      unsigned PatternIndex(unsigned PatternLen, unsigned Index) const;
      long SearchPattern(unsigned long StartAddress, unsigned long EndAddress,
                         const unsigned char * Pattern, unsigned PatternLen) const;
      long SearchHex(unsigned long StartAddress, const char * String);
      long SearchNext(unsigned long StartAddress, const char * String);

      bool FileTrim();
      bool FileExtractCR();
      bool FileInsertCR();

      static TInt64 SwapInt64(TInt64 Int);

  };

#endif
