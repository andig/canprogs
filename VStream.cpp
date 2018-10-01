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

#include <string.h>
#include <stdio.h>

#include "NTypes.h"

#include "VStream.h"
#include "NUtils.h"

using namespace NUtils;


/////////////////////////////////////////////////////////
//                                                     //
// class VStream       (universal stream buffer)       //
//                                                     //
/////////////////////////////////////////////////////////

VStream::VStream()
{
  mBigEndian = cBigEndianMachine;
  mOwnsMemory = true;
}

VStream::~VStream()
{
  SetSize(0);
}

bool VStream::SetNotOwnedMemory(unsigned char * Memory, unsigned long Length)
{
  if (!Memory)
    return false;

  if (mOwnsMemory)
  {
    if (mMemory)
      delete [] mMemory;
  }
  mOwnsMemory = false;
  mMemory = Memory;
  mSize = mMemorySize = Length;

  return true;
}

bool VStream::SetSize(unsigned long NewSize)
{
  if (NewSize == 0)                  // Puffer löschen
  {
    if (mMemory && mOwnsMemory)
      delete [] mMemory;

    mMemory = NULL;
    mMemorySize = 0;
    mOwnsMemory = true;
    mSize = 0;

    return true;
  }
  if (!mOwnsMemory)
  {
    if (NewSize <= mMemorySize)
    {
      mSize = NewSize;
      return true;
    }
    return false;
  }
  return KStream::SetSize(NewSize);
}

void VStream::operator =(const VStream & A)
// Memory und mBigEndian kopieren
{
  SetSize(A.mSize);
  if (A.mMemory &&
      0 < A.mSize && A.mSize <= A.mMemorySize)
  {
    memcpy(mMemory, A.mMemory, A.mSize);
  }
  mBigEndian = A.mBigEndian;
}

void VStream::operator <<(VStream & A)
// Instanz transferieren (Memory und mBigEndian)
{
  *(KStream *) (this) << *(KStream *)(&A);

  mOwnsMemory = A.mOwnsMemory;
}

void VStream::operator +=(const VStream & A)
{
  if (A.mSize)
    Append(A.mMemory, A.mSize);
}

void VStream::operator +=(const char * A)
{
  if (A)
    AppendString(A);
}

bool VStream::Append(const unsigned char * Stream, unsigned long StreamLen)
{
  if (!StreamLen)
    return false;

  if (mMemorySize < StreamLen + mSize)
  {
    unsigned long Size = mSize;

    if (!SetSize(Size + StreamLen + 2000))
    {
      mSize = Size;
      return false;
    }
    mSize = Size;
  }
  memmove(mMemory + mSize, Stream, StreamLen);
  mSize += StreamLen;

  AddZero();
  return true;
}

bool VStream::AppendString(const char * String)
{
  if (String)
    return Append((const unsigned char *)String, strlen(String));

  return false;
}

bool VStream::AddWord(unsigned short w)
{
  if (!UseMoreMemory(2))
    return false;

  return SetWord(mSize, w);
}

bool VStream::AddLong(unsigned int l)
{
  if (!UseMoreMemory(4))
    return false;

  return SetUInt(mSize, l);
}

bool VStream::AddString(const char * Str)
{
  if (Str && *Str)
  {
    Append((const unsigned char *) Str, strlen(Str));
  }
  return false;
}

bool VStream::AddUtf8(unsigned short w)
{
  bool Ok;
  if (w <= 0x7f)
    Ok = AddByte((unsigned char) w);
  else
  if (w <= 0x7ff)
  {
    Ok = AddByte((unsigned char)(0xc0 | ((w >> 6) & 0x1f))) &&
         AddByte((unsigned char)(0x80 | (w & 0x3f)));
  }	else {
    Ok = AddByte((unsigned char)(0xe0 | ((w >> 12) & 0x0f))) &&
         AddByte((unsigned char)(0x80 | ((w >>  6) & 0x3f))) &&
         AddByte((unsigned char)(0x80 | (w & 0x3f)));
  }
  return Ok;
}

bool VStream::InsertBytes(unsigned long Offset, long Count)
{
  if (!mOwnsMemory)
    return false;

  return KStream::InsertBytes(Offset, Count);
}

bool VStream::AddZero()
{
  if (!mMemory)
    return false;

  if (mSize < mMemorySize && !mMemory[mSize])
    return true;

  if (!mOwnsMemory)
    return false;

  unsigned long Size = mSize;
  SetSize(Size+1);
  mSize = Size; // kann 0 sein!

  mMemory[Size] = 0;

  return true;
}

bool VStream::SetWord(unsigned long Offset, unsigned short w)
{
  if (Offset + 2 > mMemorySize)
    return false;

  if (mBigEndian != cBigEndianMachine)
    w = SwapWord(w);

  *(unsigned short *)(mMemory + Offset) = w;

  if (Offset + 2 > mSize)
    mSize = Offset + 2;

  return true;
}

bool VStream::SetUInt(unsigned long Offset, unsigned int l)
{
  if (Offset + 4 > mMemorySize)
    return false;

  if (mBigEndian != cBigEndianMachine)
    l = SwapUInt(l);

  *(unsigned int *)(mMemory + Offset) = l;

  if (Offset + 4 > mSize)
    mSize = Offset + 4;

  return true;
}

bool VStream::AddAsciiInt(int l)
// Lokale Methode zu "SaveRawDataToFile" um einen Integer im Stream abzuspeichern.
{
  char Buffer[16];
  int len, i;

  if (Size() > 0)
    AddByte(' ');

  sprintf(Buffer, "%d", l);
  len = (int)strlen(Buffer);

  for (i = 0; i < len; i++)
    AddByte((unsigned char)(Buffer[i]));

  return true;
}

bool VStream::LoadFromFile(const char * FileName, unsigned long MaxLen)
{
  if (!mOwnsMemory)
    return false;

  return ReadFile(FileName, MaxLen);
}

bool VStream::SaveToFile(const char * FileName) const
{
  return SaveFile(FileName);
}

bool VStream::SaveBinToFile(long Pos, long Length, const char * Filename, bool Append) const
{
  if (Length + Pos > (long) mSize)
    Length = mSize - Pos;

  if (Pos < 0 || Length < 0)
    return false;

  return NUtils::SaveFile(Filename, (const char *)(mMemory + Pos), Length, Append);
}

FOURCC_ VStream::MakeFour(const char * FourCC)
{
  FOURCC_ res = ::MakeFour(FourCC);
  if (mBigEndian != cBigEndianMachine)
    res = SwapUInt(res);

  return res;
}

bool VStream::CompareFour(unsigned long Offset, const char * FourCC)
{
  if (Offset + 4 > mMemorySize)
    return false;

  return GetUInt(Offset) == MakeFour(FourCC);
}

bool VStream::GetFour(unsigned long Offset, FOURCC_ & FourCC)
{
  if (Offset + 4 > mMemorySize)
    return false;

  FourCC = GetUInt(Offset);

  return true;
}

bool VStream::SetFour(unsigned long Offset, const FOURCC_ & FourCC)
{
  if (Offset + 4 > mMemorySize)
    return false;

  SetUInt(Offset, FourCC);

  return true;
}

bool VStream::SetFour(unsigned long Offset, const char * FourCC)
{
  return SetFour(Offset, MakeFour(FourCC));
}

void VStream::PrintFour(unsigned long Offset) const
{
  unsigned Four = GetUInt(Offset);
  ::PrintFour(Four);
}


//////////////////////////////////////////////////////////////////////////
//
// allg. Konvertierungsmethoden
//
//////////////////////////////////////////////////////////////////////////


bool VStream::Utf8_ToWStr(const unsigned char * Utf8, unsigned long Len)
{
  bool Ok = true;
  SetSize(2*(Len+1));
  SetLength(0);

  unsigned short w = 0;
  unsigned char c;
  int bytes = 0;
  const unsigned short err = (unsigned char) '?';
  for (unsigned long i = 0; i < Len; i++)
  {
    c = Utf8[i];
    if (c <= 0x7f)
    {
    if (bytes)
    {
      AddWord(err);
      bytes = 0;
      Ok = false;
    } else
    AddWord(c);
  } else
  if (c <= 0xbf)
  {
    if (bytes)
    {
      w = (unsigned short)((w << 6) | (c & 0x3f));
      bytes--;
      if (bytes == 0)
        AddWord(w);
    } else {
      AddWord((unsigned char)c);//err);
      Ok = false;
    }
  } else
  if (c <= 0xdf)
  {
    bytes = 1;
    w = (unsigned short)(c & 0x1f);
  } else
  if (c <= 0xef)
  {
    bytes = 2;
    w = (unsigned short)(c & 0x0f);
  // c <= 0xf7 : 3 bytes    (c & 0x7)
  } else {
    AddWord(err);
    bytes = 0;
    Ok = false;
    }
  }
  if (bytes)
  {
    AddWord(err);
    Ok = false;
  }
  AddZero();

  return Ok;
}

bool VStream::WstrToUtf8(const unsigned short * Wstr, unsigned long Len)
{
  bool Ok = true;
  long len = 0;
  unsigned short w = 0;

  for (unsigned long i = 0; i < Len; i++)
  {
    w = Wstr[i];
    if (w <= 0x7f)
      len++;
    else
    if (w <= 0x7ff)
      len += 2;
    else
      len +=3;
  }
  if (w || !Len)
    len++;

  SetSize(len);
  SetLength(0);

  for (unsigned long j = 0; j < Len; j++)
  {
    w = Wstr[j];
    AddUtf8(w);
  }
  AddZero();

  return Ok;
}

bool VStream::htmlTo_utf8(const char * Html)
{
  bool Ok = true;
  SetLength(0);

  while (*Html)
  {
    if (*Html == '%')
    {
      unsigned char res = '?';

      char Digit1 = *(++Html);
      char Digit2 = 0;
      if (Digit1)
        Digit2 = *(++Html);
      if (!Digit2)
        return false;

      Html++;
      if (IsHexDigit(Digit1) && IsHexDigit(Digit2))
      {
        res = (unsigned char)(16*GetHexDigit(Digit1) + GetHexDigit(Digit2));
      } else
        Ok = false;
      AddByte(res);
    } else
      AddByte((unsigned char) *Html++);
  }
  return Ok;
}

bool VStream::htmlToUtf8(char * Html)
{
  bool Ok = true;
  int utf_len = 0;
  unsigned len = (unsigned) strlen(Html);

  for (unsigned i = 0; i < len; i++)
  {
    char c = Html[i];
    if (c == '%')
    {
      unsigned char res = '?';

      char Digit1 = Html[i+1];
      char Digit2 = 0;
      if (Digit1)
        Digit2 = Html[i+2];

      if (IsHexDigit(Digit1) && IsHexDigit(Digit2))
      {
        res = (unsigned char)(16*GetHexDigit(Digit1) + GetHexDigit(Digit2));
        i += 2;
      } else
        Ok = false;
      c = (char) res;
    }
    Html[utf_len++] = c;
  }
  Html[utf_len] = 0;
  return Ok;
}

static unsigned char Digits[17] = "0123456789ABCDEF";

void VStream::AppendHtml(const unsigned char * utf8, unsigned long StreamLen)
{
  for (unsigned long i = 0; i < StreamLen; i++)
  {
    unsigned char c = utf8[i];
    if (c == ' ' || c == '%' || c > 0x7f)
    {
      AddByte('%');
      AddByte(Digits[c >> 4]);
      AddByte(Digits[c & 0xf]);
    } else
      AddByte(c);
  }
  AddZero();
}

void VStream::AppendHtmlString(const char * utf8)
{
  if (utf8)
    AppendHtml((const unsigned char *)utf8, strlen(utf8));
}

bool VStream::utf8_To_html(const unsigned char * Utf8, unsigned long Len)
{
  SetLength(0);

  AppendHtml(Utf8, Len);

  return true;
}

bool VStream::MacToUtf8(const unsigned char * Mac, unsigned long Len)
{
  SetLength(0);
  for (unsigned long i = 0; i < Len; i++)
  {
    unsigned short w = MacToUnicode(Mac[i]);
    AddUtf8(w);
  }
  AddZero();

  return true;
}

bool VStream::WStrToMac(const unsigned short * Wstr, unsigned long Len)
{
  SetLength(0);
  for (unsigned long i = 0; i < Len; i++)
  {
    unsigned short c = Wstr[i];
    AddByte(UnicodeToMac(c));
  }
  AddZero();

  return true;
}

bool VStream::CP1252_ToUtf8(const unsigned char * cp1252, unsigned long Len)
{
  SetLength(0);
  for (unsigned long i = 0; i < Len; i++)
  {
    unsigned short w = CP1252_ToUnicode(cp1252[i]);
    AddUtf8(w);
  }
  AddZero();

  return true;
}

void VStream::PrintRiff() const
{
#if defined(__CONSOLE__)
  unsigned p = 3*4;

  printf("%8.8x: ", 0);
  PrintFour(0);
  printf(" %8.8x ", GetUInt(4));
  PrintFour(8);
  printf("\n");

  while (p < (unsigned) Size())
  {
    printf("%8.8x: ", p);
    PrintFour(p);
    unsigned l = GetUInt(p+4);
    printf(" %8.8x\n", l);
    if (l & 1)
      l++;
    p += l + 8;
  }

  printf("%8.8x: size %8.8x\n", p, (unsigned) Size());
#endif
}

unsigned VStream::PatternIndex(unsigned PatternLen, unsigned Index) const
{
  return mBigEndian ? Index : PatternLen - 1 - Index;
}

long VStream::SearchPattern(unsigned long StartAddress, unsigned long EndAddress,
                            const unsigned char * Pattern, unsigned PatternLen) const
{
  if (!PatternLen || !Pattern || PatternLen > 256)
    return -1;

  char Pat[256];
  for (int j = 0; j < (int) PatternLen; j++)
    Pat[j] = LowerCase(Pattern[j]);

  if (EndAddress > mSize - PatternLen)
    EndAddress = mSize - PatternLen;

  char start = Pat[PatternIndex(PatternLen, 0)];
  for (unsigned long i = StartAddress; i <= EndAddress; i++)
    if (start == GetLower(i))
    {
      unsigned l = 1;
      while (l < PatternLen &&
             Pat[PatternIndex(PatternLen, l)] == GetLower(i + l))
        l++;
      if (l == PatternLen)
        return i;
    }

  return -1;
}

static long UseMin(long V1, long V2)
{
  if (V1 < 0)
    return V2;

  if (V2 < 0)
    return V1;

  if (V1 < V2)
    return V1;
  else
    return V2;
}

long VStream::SearchHex(unsigned long StartAddress, const char * String)
{
  long res1, res2;
  unsigned PatternLen = 0;
  unsigned char Pattern[256];

  int Pos = 0;
  while (String[Pos] == ' ' || IsHexDigit(String[Pos]))
  {
    while (String[Pos] == ' ')
      Pos++;

    if (IsHexDigit(String[Pos]))
    {
      unsigned char c = (unsigned char) GetHexDigit(String[Pos++]);
      if (IsHexDigit(String[Pos]))
        c = (unsigned char)(16*c + GetHexDigit(String[Pos++]));
      if (PatternLen <= High(Pattern))
        Pattern[PatternLen++] = c;
    }
  }

  if (String[Pos])
    return -1;

  bool SaveEndian = mBigEndian;

  mBigEndian = true;
  res1 = SearchPattern(StartAddress, mSize, Pattern, PatternLen);
  mBigEndian = false;
  res2 = SearchPattern(StartAddress, UseMin(res1, mSize), Pattern, PatternLen);

  mBigEndian = SaveEndian;

  return UseMin(res1, res2);
}

long VStream::SearchNext(unsigned long StartAddress, const char * String)
{
  long Pos1, Pos2;
  bool SaveEndian = mBigEndian;
  unsigned len = (unsigned) strlen(String);

  Pos1 = SearchHex(StartAddress, String);

  mBigEndian = true;
  Pos2 = SearchPattern(StartAddress, UseMin(Pos1, mSize),
                       (const unsigned char *)String, len);
  Pos1 = UseMin(Pos1, Pos2);
  mBigEndian = false;
  Pos2 = SearchPattern(StartAddress, UseMin(Pos1, mSize),
                       (const unsigned char *)String, len);

  mBigEndian = SaveEndian;

  return UseMin(Pos1, Pos2);
}

bool VStream::FileTrim()
{
  unsigned long pos, new_pos = 0;
  unsigned char * mem = (unsigned char *) GetMemory();

  if (!mem || !Size())
    return false;

  for (pos = 0; pos < Size(); )
  {
    unsigned long p = pos;

    while (p < Size() && mem[p] != '\n')
      p++;
    if (p && mem[p-1] == '\r')
      p--;

    unsigned long b_pos = p;
    while (b_pos && (mem[b_pos-1] == ' ' || mem[b_pos-1] == '\t'))
      b_pos--;

    while (pos < b_pos)
    {
      mem[new_pos++] = mem[pos++];
    }
    pos = p;
    while (pos < Size() && (mem[pos] == '\r' || mem[pos] == '\n'))
    {
      mem[new_pos++] = mem[pos++];
    }
  }
  if (pos > new_pos)
  {
    mem[new_pos] = 0;
    SetLength(new_pos);
    return true;
  }

  return false;
}

bool VStream::FileExtractCR()
{
  long pos, new_pos = 0;
  unsigned char * mem = (unsigned char *) GetMemory();

  if (!mem || !Size())
    return false;

  for (pos = 0; pos < (long) Size(); )
  {
    if (mem[pos] != '\r')
      mem[new_pos++] = mem[pos];
    pos++;
  }
  if (pos > new_pos)
  {
    mem[new_pos] = 0;
    SetLength(new_pos);
    return true;
  }

  return false;
}

bool VStream::FileInsertCR()
{
  FileExtractCR();

  unsigned long pos, new_pos = 0;
  unsigned char * mem = (unsigned char *) GetMemory();

  if (!mem || !Size())
    return false;

  int count = 0;
  for (pos = 0; pos < Size(); pos++)
  {
    if (mem[pos] == '\n')
      count++;
  }
  if (!count)
    return false;

  if (!SetSize(Size() + count))
    return false;

  mem = (unsigned char *) GetMemory();
  memmove(mem + count, mem, Size()-count);

  for (pos = count; pos < Size(); )
  {
    if (mem[pos] == '\n')
      mem[new_pos++] = '\r';

    mem[new_pos++] = mem[pos++];
  }

  return true;
}

TInt64 VStream::SwapInt64(TInt64 Int)
{
  TInt64 Result;

  ((unsigned char *)(&Result))[0] = ((unsigned char *)(&Int))[7];
  ((unsigned char *)(&Result))[1] = ((unsigned char *)(&Int))[6];
  ((unsigned char *)(&Result))[2] = ((unsigned char *)(&Int))[5];
  ((unsigned char *)(&Result))[3] = ((unsigned char *)(&Int))[4];
  ((unsigned char *)(&Result))[4] = ((unsigned char *)(&Int))[3];
  ((unsigned char *)(&Result))[5] = ((unsigned char *)(&Int))[2];
  ((unsigned char *)(&Result))[6] = ((unsigned char *)(&Int))[1];
  ((unsigned char *)(&Result))[7] = ((unsigned char *)(&Int))[0];

  return Result;
}
