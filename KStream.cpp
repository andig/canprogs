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

#if defined(__PYTHON__)
  #include <Python.h>
#endif

#include <stdio.h>
#include <string.h>

#include "NTypes.h"

#include "KStream.h"

#include "NUtils.h"

KStream::KStream()
{
  mMemory = NULL;
  mMemorySize = 0;           // size of mMemory
  mSize = 0;
  mBigEndian = false;
}

KStream::~KStream()
{
  SetSize(0);
}

bool KStream::CheckSize(unsigned long NewSize)
{
  if (NewSize <= mMemorySize)
    return true;

  NewSize *= 2;
  if (NewSize < 2000)
    NewSize = 2000;

  SetSize(NewSize);

  return mMemory != NULL;
}

bool KStream::SetSize(unsigned long NewSize)
{
  mSize = NewSize;

  if (NewSize == 0)                  // Puffer löschen
  {
    if (mMemory)
      delete [] mMemory;

    mMemory = NULL;
    mMemorySize = 0;
    mSize = 0;

    return true;
  }

  if (NewSize <= mMemorySize)        // Puffer ist ok
    return true;

  unsigned char * NewMemory;

#if defined(__ARM__)
  if (NewSize < 0x4000000) // 67 MB
#else
  if (NewSize < 0x20000000) // 0.5 GB
#endif
  {
    NewSize = 8*((NewSize + 7) / 8);
    NewMemory = new unsigned char[NewSize];
    memset(NewMemory, 0, NewSize);
  } else {
#if defined(__CONSOLE__)
    printf("KStream::SetSize: new size of %u\n", (unsigned)NewSize);
#endif
    NewSize = 0;
    NewMemory = NULL;
    mSize = 0;
  }

  // bestehendes Memory kopieren (vorhandene Daten übernehmen)
  if (mMemory)
  {
    if (NewMemory)
      memcpy(NewMemory, mMemory, mMemorySize);
    delete [] mMemory;
  }

  mMemory = NewMemory;
  mMemorySize = NewSize;

  return NewSize != 0;
}

unsigned long KStream::SetStream(unsigned char * Stream, unsigned long StreamLen)
{
  if (!Stream || !StreamLen)
    return false;

  SetSize(0);

  mSize = StreamLen;
  mMemory = Stream;
  mMemorySize = StreamLen;

  return mSize;
}

void KStream::operator <<(KStream & A)
// Instanz transferieren (Memory und mBigEndian)
{
  SetSize(0);

  mMemory = A.mMemory;
  mMemorySize = A.mMemorySize;
  mSize = A.mSize;
  mBigEndian = A.mBigEndian;

  A.mMemory = NULL;
  A.mSize = A.mMemorySize = 0;
}

void KStream::SetLength(unsigned long Length)
{
  if (Length < mMemorySize)
    mSize = Length;
  else
    mSize = mMemorySize;
}

bool KStream::InitString(const char * str)
{
  mSize = 0;
  if (!str)
    return false;

  if (!SetSize(strlen(str) + 1))
    return false;

  mSize = 0;
  if (!mMemory)
    return false;

  strcpy((char *)mMemory, str);
  mSize = strlen(str);

  return true;
}

bool KStream::UseMoreMemory(unsigned MoreBytes)
{
  bool Ok = true;
  if (mSize + MoreBytes > mMemorySize)
  {
    unsigned long Size = mSize;

    Ok = SetSize(Size + 2000);
    mSize = Size;
  }
  return Ok;
}

bool KStream::AddByte(unsigned char b)
{
  if (!UseMoreMemory(1))
    return false;

  return SetByte(mSize, b);
}

bool KStream::AppendString(const char * str)
{
  if (str && *str)
    return Append((const unsigned char *) str, (unsigned) strlen(str));

  return false;
}

bool KStream::Append(const unsigned char * Stream, unsigned long StreamLen)
{
  return ReplaceBytes(Stream, mSize, (unsigned) StreamLen, 0);
}

bool KStream::InsertBytes(unsigned long Offset, long Count)
{
  if (!Count)
    return true;

  if (Count < 0 && -Count > (long) mSize)
    return false;

  if (Offset > mSize)
    return false;

  long src = Offset;
  long dst = Offset;
  unsigned long mov_len = mSize - Offset;
  unsigned long NewSize = mSize + Count;
  if (Count > 0)
  {
    if (!CheckSize(mSize + Count + 1))
      return false;

    dst += Count;
  } else {
    Count = -Count;
    if ((unsigned long) Count > mov_len)
      return false;

    src += Count;
    mov_len -= Count;
  }
  if (mov_len > 0)
  {
    memmove((unsigned char *)(mMemory + dst), mMemory + src, mov_len);
  }
  mSize = NewSize;
  mMemory[mSize] = 0;

  return true;
}

bool KStream::ReplaceBytes(const unsigned char * new_bytes, unsigned long Offset,
                           unsigned new_count, unsigned old_count)
{
  if (!new_bytes && new_count)
    return false;

  if (!InsertBytes(Offset, (int)(new_count) - (int)(old_count)))
    return false;

  if (new_count)
  {
    memcpy(mMemory + Offset, new_bytes, new_count);
  }

  return true;
}

bool KStream::Replace(const char * old_str, const char * new_str)
{
  if (!old_str || strlen(old_str) > mSize)
    return false;

  const unsigned char * ptr = (const unsigned char *) strstr((const char *) mMemory, old_str);
  if (!ptr)
    return false;

  if (!new_str)
    new_str = "";

  return ReplaceBytes((const unsigned char *) new_str, ptr-mMemory,
                      (unsigned) strlen(new_str), (unsigned) strlen(old_str));
}

bool KStream::ReadFile(const char * FileName, unsigned long MaxLen)
{
  SetSize(0);

  FILE * InpFile = NUtils::OpenExistingFile(FileName);
  if (!InpFile)
    return false;

  TFileSize size = NUtils::FileSize(InpFile);
  if (MaxLen && MaxLen < size)
    size = MaxLen;

  if (size > 0)
  {
    unsigned char * Stream = new unsigned char[size+3];

    if (NUtils::ReadFileBuffer(InpFile, Stream, size))
    {
      Stream[size] = 0;
      Stream[size+1] = 0;  // unicode stop character
      Stream[size+2] = 0;
      mMemorySize = size+3;
      mSize = size;
      mMemory = Stream;
    } 
  }
  fclose(InpFile);

  return mMemory != NULL;
}

bool KStream::SaveFile(const char * Filename) const
{
  return NUtils::SaveFile(Filename, (const char *)mMemory, mSize);
}

unsigned char KStream::GetByte(unsigned long Offset) const
{
  if (Offset + 1 <= mSize && mMemory)
    return mMemory[Offset];

  return 0;
}

char KStream::GetLower(unsigned long Offset) const
{
  return LowerCase(GetByte(Offset));
}

char KStream::GetUpper(unsigned long Offset) const
{
  return UpperCase(GetByte(Offset));
}

char KStream::LowerCase(char c)
{
  if ('A' <= c && c <= 'Z')
    c += (char)('a' - 'A');

  return c;
}

char KStream::UpperCase(char c)
{
  if ('a' <= c && c <= 'z')
    c -= (char)('a' - 'A');

  return c;
}

unsigned short KStream::GetWord(unsigned long Offset) const
{
  unsigned short Result = 0;

  if (Offset + 2 <= mSize && mMemory)
    Result = *(unsigned short *)(mMemory + Offset);

  if (mBigEndian != cBigEndianMachine)
    Result = SwapWord(Result);

  return Result;
}

unsigned int KStream::GetUInt(unsigned long Offset) const
{
  unsigned int Result = 0;

  if (Offset + 4 <= mSize && mMemory)
    Result = *(unsigned int *)(mMemory + Offset);

  if (mBigEndian != cBigEndianMachine)
    Result = SwapUInt(Result);

  return Result;
}

bool KStream::SetByte(unsigned long Offset, unsigned char b)
{
  if (Offset + 1 > mMemorySize)
    return false;

  *(unsigned char *)(mMemory + Offset) = b;

  if (Offset + 1 > mSize)
    mSize = Offset + 1;

  return true;
}

int KStream::SwapUInt(unsigned Long)
{
  return (int)((Long >> 24) & 0xff) +
              ((Long >> 8)  & 0xff00) +
              ((Long << 8)  & 0xff0000) +
              (Long << 24);
}

static const unsigned char Base64CodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char Base64DecodeTable[256];
static bool Base64_Ok = false;

bool KStream::DecodeBase64()
{
  if (Size() < 4)
    return false;

  if (!Base64_Ok)
  {
    memset(Base64DecodeTable, 0xff, sizeof(Base64DecodeTable));
    for (long i = 0; i < 64; i++)
      Base64DecodeTable[Base64CodeTable[i]] = i;
    Base64_Ok = true;
  }
  long lenstr = Size();
  unsigned char * s = mMemory;

  long p = 0;
  for (long i = 0; i < lenstr; i++)
  {
    if (s[i] > ' ' && s[i] < 0x7f)
      s[p++] = s[i];
  }
  s[p] = 0;
  SetLength(p);

  long lens = p;

  if (lens % 4)
    return false;

  long len = 3*(lens / 4);
  if (s[lens-1] == '=')
  {
    len--;
    lens--;
  }
  if (s[lens-1] == '=')
  {
    len--;
    lens--;
  }

  for (long i = 0; i < lens; i++)
  {
    if (s[i] == 0xff)
      return false;
  }
  long j = 0;
  p = 0;
  while (p < len)
  {
    unsigned c = (Base64DecodeTable[s[j]] << 18) +
                 (Base64DecodeTable[s[j+1]] << 12) +
                 (Base64DecodeTable[s[j+2]] <<  6) +
                 Base64DecodeTable[s[j+3]];
    s[p] = (unsigned char)(c >> 16);
    if (p + 1 < len)
      s[p+1] = (unsigned char)(c >> 8);
    if (p + 2 < len)
      s[p+2] = (unsigned char)c;
    j += 4;
    p += 3;
  }
  s[len] = 0;
  SetSize(len+1);
  SetLength(len);

  return true;
}

bool KStream::EncodeBase64()
{
  if (Size())
    return true;

  long oldlen = Size();
  long len = 4*((oldlen + 2) / 3);
  long fuell = (3 - (oldlen % 3)) % 3;
  SetSize(len+3);
  unsigned char * s = mMemory;
  s[len] = 0;
  SetLength(len);

  long p = 3*((oldlen + 2) / 3) - 3;
  long i = len - 4;
  while (p >= 0)
  {
    unsigned c = s[p] << 16;
    if (p + 1 < oldlen)
      c |= s[p+1] << 8;
    if (p + 2 < oldlen)
      c |= s[p+2];

    s[i]   = Base64CodeTable[(c >> 18) & 0x3f];
    s[i+1] = Base64CodeTable[(c >> 12) & 0x3f];
    s[i+2] = Base64CodeTable[(c >> 6) & 0x3f];
    s[i+3] = Base64CodeTable[c & 0x3f];
    p -= 3;
    i -= 4;
  }
  while (fuell > 0)
  {
    s[len - fuell] = '=';
    fuell--;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

FOURCC_ MakeFour(const char * FourCC)
{
  return *(FOURCC_ *)FourCC;
}

void PrintFour(FOURCC_ FourCC)
{
#if defined(__CONSOLE__)
  printf("%c%c%c%c",
         ((char *)(&FourCC))[0] ? ((char *)(&FourCC))[0] : '_',
         ((char *)(&FourCC))[1] ? ((char *)(&FourCC))[1] : '_',
         ((char *)(&FourCC))[2] ? ((char *)(&FourCC))[2] : '_',
         ((char *)(&FourCC))[3] ? ((char *)(&FourCC))[3] : '_') ;
#endif
}

