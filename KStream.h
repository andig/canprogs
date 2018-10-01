/*
 *
 *  Copyright (C) 2015 Jürg Müller, CH-5524
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

#if !defined(KStream_H)

  #define KStream_H

  typedef unsigned int FOURCC_;

  class KStream
  {
    protected:
      unsigned char * mMemory;
      unsigned long mMemorySize;           // size of mMemory
      unsigned long mSize;                 // used size

      bool UseMoreMemory(unsigned MoreBytes);

    public:
      bool mBigEndian;

      KStream();
      ~KStream();

      const unsigned char * GetMemory() const { return mMemory; };
      const char * GetCharMemory() const { return (const char *) mMemory; };
      unsigned long Size() const { return mSize; };
      unsigned long MemorySize() const { return mMemorySize; };
      virtual bool OwnsMemory() const { return true; }

      // mMemory wird gleich Stream gesetzt (die Verwaltung des Streams wird
      // von der Instanz übernommen)
      unsigned long SetStream(unsigned char * Stream, unsigned long StreamLen);

      bool InitString(const char * str);
      virtual bool AddByte(unsigned char b);
      virtual bool AppendString(const char * str);
      virtual bool Append(const unsigned char * Stream, unsigned long StreamLen);
      virtual bool InsertBytes(unsigned long Offset, long Count);
      bool ReplaceBytes(const unsigned char * new_bytes, unsigned long Offset,
                        unsigned new_count, unsigned old_count = 0);
      bool Replace(const char * old_str, const char * new_str);

      // Wenn NewSize = 0: der Speicher mMemory wird freigegeben
      // Wenn NewSize < mMemorySize: mSize wird gleich NewSize gesetzt
      // sonst: es wird ein neuer Speicher alloziert. Vorhandene Daten
      //        werden in den neuen Speicher kopiert
      virtual bool SetSize(unsigned long NewSize);
      bool CheckSize(unsigned long NewSize);
      // mSize wird gleich Length gesetzt, aber höchstens gleich mMemorySize
      // (d.h. mMemory bleibt unverändert)
      void SetLength(unsigned long Length);

      void operator <<(::KStream & A);         // memory transfer

      virtual bool ReadFile(const char * Filename, unsigned long Max = 0);
      virtual bool SaveFile(const char * Filename) const;

      unsigned char GetByte(unsigned long Offset) const;
      char GetLower(unsigned long Offset) const;
      char GetUpper(unsigned long Offset) const;
      static char LowerCase(char c);
      static char UpperCase(char c);
      unsigned short GetWord(unsigned long Offset) const;
      unsigned int   GetUInt(unsigned long Offset) const;

      bool SetByte(unsigned long Offset, unsigned char b);

      static int SwapUInt(unsigned Long);
      static inline unsigned short SwapWord(unsigned short Word)
      {
        return (unsigned short)((Word >> 8) + (Word << 8));
      }

      bool EncodeBase64();
      bool DecodeBase64();
  };

  void PrintFour(FOURCC_ FourCC);
  FOURCC_ MakeFour(const char * FourCC);

#endif

