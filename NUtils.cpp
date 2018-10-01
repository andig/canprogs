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

#if defined(__LINUX__)
  #include <sys/types.h>
  #include <sys/time.h>
  #include <unistd.h> 
  #include <signal.h>
#endif
#if defined(__WINDOWS__)
  #include <windows.h>
#endif
#if !defined(__VC__) && !defined(_MSC_VER)
  #include <dirent.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__VC__)
  #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
  #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#include "NTypes.h"

#include "NUtils.h"

namespace NUtils
{

void SleepMs(int aMs)
{
#if defined(__WINDOWS__)
  Sleep(aMs);
#endif
#if defined(__LINUX__)
  usleep(1000*aMs);
#endif
}

double TimeSecs()
// Anzahl Sekunden seit dem 31.12.1999 0:00
{
  int Days, Ms;

  Time(Days, Ms);
  return Days*86400.0 + ((double) Ms) / 1000.0;
}

void Time(const struct timeval & Time, int & aDays, int & aMs)
{
  aMs = 1000*(Time.tv_sec % 86400) + Time.tv_usec/1000;
  aDays = (int) (Time.tv_sec / 86400);
  // 72 76 80 84 88 92 96 : 7 Schaltjahre
  aDays -= (2000-1970)*365 + (2000-1970)/4;
  if (aMs < 0)
  {
    aMs += 1000*86400;
    aDays--;
  }
}

const unsigned cMonthDays[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
// jan feb mar  apr  mai  jun  jul  aug  sep  okt  nov  dez
//  31  28  31   30   31   30   31   31   30   31   30   31

#if defined(__WINDOWS__)
void Time(int & aDays, int & aMs)
{
  SYSTEMTIME SysTime;

  GetLocalTime(&SysTime);

  // Tage und ms seit 31. Dezember 1999 00:00
  // aDays == 1 ist der 1.1.2000
  aDays = SysTime.wYear;
  aDays = aDays*365 +
          (aDays + 3) / 4 + // Zusatztag für die Schaltjahre
          cMonthDays[SysTime.wMonth-1] +
          SysTime.wDay - 1;

  // im Schaltjahr nach dem 1. März einen Tag addieren
  if (!(SysTime.wYear & 3) && SysTime.wMonth >= 3)
    aDays++;

  aDays -= 2000*365 + (2000 + 3) / 4;
  aMs = 3600 * ((int) SysTime.wHour) +
        60 * SysTime.wMinute +
        SysTime.wSecond;
  aMs = 1000 * aMs + SysTime.wMilliseconds;
}
#else

void Time(int & aDays, int & aMs)
{
  struct timeval _Time;
  struct timezone _tz;

  if (!gettimeofday(&_Time, &_tz))  // seit 1. Jan. 1970
  {
    _Time.tv_sec -= 60*_tz.tz_minuteswest;
    Time(_Time, aDays, aMs);
  } else {
    aMs = 0;
    aDays = 0;
  }
}

#endif

void GetDateFromDays(int aDays, int & aDay, int & aMonth, int & aYear)
// aDays == 0 entspricht dem 1.1.2000 (aDay = 1, aMonth = 1, aYear = 2000)
{
  aMonth = 1;
  aDay = aDays;

  // immer 4 Jahre abzählen: 4*365 Tage + 1 Tag vom Schaltjahr
  aYear = aDay / (4*365 + 1);
  aDay -= aYear*(4*365 + 1);
  aYear = 4*aYear + 2000;
  if (aDay <= 0)
  {
    aDay += 4*365 + 1;
    aYear -= 4;
  }
  // das 1. Jahr (z.B. 2012) ist ein Schaltjahr, danach bis zu 3 normale Jahre
  if (aDay >= 366)
  {
    aYear++;
    aDay -= 366;

    while (aDay >= 365)
    {
      aYear++;
      aDay -= 365;
    }
  }
  aDay++; // der erste Tag im Monat ist der 1.
  while ((int) cMonthDays[aMonth] < aDay)
  {
    if (aMonth == 2 && !(aYear & 3))
    {
      if ((int) cMonthDays[2]+1 >= aDay)  // 28. Feb. in einem Schaltjahr
        break;
      aDay--;    // für den 29. Feb. (ist nicht in cMonthDays enthalten)
    }
    aMonth++;
  }
  aDay -= cMonthDays[aMonth-1];
}

void GetDateAndTime(double aTime, int & aDay, int & aMonth, int & aYear,
                    int & aHour, int & aMin, int & aSec, int & aMs)
{
  double Days = aTime / 86400.0;

  GetDateFromDays(int (Days), aDay, aMonth, aYear);

  int Ms = int (1000.0*(aTime - 86400.0*int(Days)));
  aHour = Ms / 3600000;
  aMin = (Ms / 60000) % 60;
  aSec = (Ms / 1000) % 60;
  aMs = Ms % 1000;
}

bool GetDays(int Year, int Month, int Day, int & Days)
{
  if (Year < 2000 || Year > 2100 ||
      Month < 1 || Month > 12 ||
      Day < 1 || Day > 31)
    return false;

  if (Month == 2)
  {
    if ((Year % 4) != 0 && Day >= 29)
      return false;

    if (Day > 29)
      return false;
  } else
  if ((unsigned) Day > cMonthDays[Month] - cMonthDays[Month-1])
    return false;

  Year -= 2000;
  Days = 365*Year + (Year - 1) / 4 + cMonthDays[Month-1] + Day;
  if ((Year % 4) == 0 && Month > 2)
    Days++;

  return true;
}

void PrintDayTime(int time_ms, char * OutStr)
{
  int Sec = time_ms / 1000;
  int Min = Sec / 60;
  int Hour = Min / 60;

  sprintf(OutStr, "%2.2d:%2.2d:%2.2d.%3.3d",
          Hour, Min % 60, Sec % 60, time_ms % 1000);
}

void PrintTime(int time_days, int time_ms, char * OutStr, bool csv)
{
  int Day, Month, Year;

  GetDateFromDays(time_days, Day, Month, Year);
  if (csv)
  {
    sprintf(OutStr, "'%4.4d-%2.2d-%2.2d',%d", Year, Month, Day, time_ms);
  } else {
    char day_str[32];

    PrintDayTime(time_ms, day_str);
    sprintf(OutStr, "%d.%d.%d %s",
            Day, Month, Year, day_str);
  }
}

void PrintMs(int days, int ms, char * stream)
{
  int time_stamp_1 = days;
  int time_stamp_2;

  time_stamp_1 += 30*365 + 30 / 4; // Tage zwischen 1970 und 2000
  time_stamp_1 *= 24*36; // 86'4 * 10'957 = 9'466'848
  time_stamp_1 += ms / 100000;
  if (ms < 0)
  {
    time_stamp_2 = 100000 + ms % 100000;
    time_stamp_1--;
  } else
    time_stamp_2 = ms % 100000;

  // GMT time_stamp_1 -= 2*36;

  // ./ms2time.sh 1'399'346'245'967  ==> 2014-05-06 05:17:25
  // date -d "2014-05-06 05:17:25" ==>  1'399'346'245  (03:17:25 einsetzen GMT!)
  sprintf(stream, "%d%5.5d", time_stamp_1, time_stamp_2);
}

void PrintDateTime(int days, int ms, char * stream)
{
  // format: YYYY-MM-DD HH:MM:SS.mmm
  int Day, Month, Year;
  char day_str[32];

  GetDateFromDays(days, Day, Month, Year);
  PrintDayTime(ms, day_str);

  sprintf(stream, "%4.4d-%2.2d-%2.2d %s", Year, Month, Day, day_str);
}

bool IsDigit(char Digit)
{
  return ('0' <= Digit && Digit <= '9');
}

int GetDigit(char Digit)
{
  if ('0' <= Digit && Digit <= '9')
    return Digit - '0';

  return -1;
}

bool IsHexDigit(char aDigit)
{
  if (('0' <= aDigit && aDigit <= '9') ||
      ('a' <= aDigit && aDigit <= 'f') ||
			('A' <= aDigit && aDigit <= 'F'))
    return true;

  return false;
}

int GetHexDigit(char aDigit)
{
  if ('0' <= aDigit && aDigit <= '9')
    return (int) aDigit - '0';

  if ('A' <= aDigit && aDigit <= 'F')
    return (int) aDigit - ('A' - 10);

  if ('a' <= aDigit && aDigit <= 'f')
    return (int) aDigit - ('a' - 10);

  return -1;
}

unsigned GetHex(const char * & Line)
{
  if (!Line)
    return 0;

  unsigned Hex = 0;

  while (IsHexDigit(*Line))
  {
    Hex = 16*Hex + GetHexDigit(*Line++);
  }
  return Hex;
}

bool GetHexByte(const char * & Line, unsigned char & byte)
{
  if (!IsHexDigit(Line[0]) ||
      !IsHexDigit(Line[1]))
    return false;

  byte = 16*GetHexDigit(Line[0]) +
            GetHexDigit(Line[1]);
  Line += 2;

  return true;
}

bool GetUnsigned(const char * & Line, unsigned & res)
{
  TInt64 r;
  bool Ok = GetInt(Line, r);
  if (Ok)
    Ok = (r >= 0 && r <= 0xffffffff);
  if (Ok)
    res = (unsigned) r;
  
  return Ok;
}
  
unsigned GetInt(TInt64 & res, const char * Line)
{
  if (!Line)
    return 0;

  const char * ptr = Line;
  bool Ok = GetInt(ptr, res);
  if (!Ok)
    return 0;

  return (unsigned)(ptr - Line);
}

bool GetInt(const char * & Line, TInt64 & res)
{
  if (!Line)
    return false;

  bool IsHex = false;
  if (Line[0] == '0' && Line[1] == 'x')
  {
    Line += 2;
    IsHex = true;
  } else
  if (Line[0] == '$')
  {
    Line++;
    IsHex = true;
  }
  if (IsHex)
  {
    if (!IsHexDigit(Line[0]))
      return false;

    unsigned u = GetHex(Line);
    res = (int) u;
    return true;
  }

  bool neg = *Line == '-';
  if (neg)
    Line++;

  bool Ok = ('0' <= *Line && *Line <= '9');

  res = 0;
  while ('0' <= *Line && *Line <= '9')
  {
    res = 10*res + (unsigned char)(*Line) - '0';
    Line++;
  }
  if (neg)
    res = -res;

  return Ok;
}

bool GetDouble(const char * & Ptr, double & res)
{
  bool neg = *Ptr == '-';
  if (neg)
    Ptr++;

  res = 0;
  if (*Ptr < '0' || *Ptr > '9')
    return false;

  while ('0' <= *Ptr && *Ptr <= '9')
  {
    res = 10.0*res + (unsigned char)(*Ptr - '0');
    Ptr++;
  }
  if (*Ptr == '.')
  {
    double Res2 = 0;
    double quot = 1.0;

    Ptr++;
    if (*Ptr < '0' || *Ptr > '9')
      return false;

    while ('0' <= *Ptr && *Ptr <= '9')
    {
      Res2 = 10.0*Res2 + (unsigned char)(*Ptr - '0');
      quot *= 10.0;
      Ptr++;
    }
    Res2 /= quot;
    res += Res2;
  }
  if (neg)
    res = -res;

  return true;
}

bool IsNumber(const char * aPtr)
{
  TInt64 res;

  if (!GetInt(aPtr, res))
    return false;

  return *aPtr == 0;
}
  
bool SkipBlanks(const char * & Line)
{
  bool Ok = *Line == ' ' || *Line == 9; // horizontal tab
  
  while (*Line == ' ' || *Line == 9)
    Line++;
  
  return Ok;
}

char LowerCase(char c)
{
  if ('A' <= c && c <= 'Z')
    c += (char)('a' - 'A');

  return c;
}

char UpperCase(char c)
{
  if ('a' <= c && c <= 'z')
    c -= (char)('a' - 'A');

  return c;
}
  
void ToLowerCase(char * Str)
{
  if (!Str)
    return;

  while (*Str)
  {
    if ('A' <= *Str && *Str <= 'Z')
      *Str = (char)(*Str + ('a' -'A'));
    Str++;
  }
}
  
const char * ExtractFilename(const char * FilePath)
{
  if (!FilePath)
    return "";

  for (long Pos = strlen(FilePath)-1; Pos >= 0; Pos--)
  {
    char c = FilePath[Pos];
    if (c == '/' || c == '\\')
      return FilePath + Pos + 1;
  }
  return FilePath;
}

const char * ExtractExtension(const char * FilePath)
{
  if (!FilePath)
    return "";

  long len = strlen(FilePath);
  for (long Pos = len-1; Pos > 0; Pos--)
  {
    char c = FilePath[Pos];
    if (c == '.')
      return FilePath + Pos;
    if (c == '/' || c == '\\')
      break;
  }
  return FilePath + len;
}

void MakeStandardPath(char * Path, bool UseHomePath)
{
  if (UseHomePath)
    SetHomePath(Path);

  long len = strlen(Path);
  for (long i = 0; i < len; i++)
    if (Path[i] == '\\')
      Path[i] = '/';

  if (len <= 0 || Path[len-1] != '/')
    strcat(Path, "/");

  char * Ptr;
  do
  {
    Ptr = strstr(Path, "//");
    if (Ptr)
      strcpy(Ptr, Ptr+1);
  } while (Ptr);

#if defined(__WINDOWS__)
  for (unsigned j = 0; j < strlen(Path); j++)
    if (Path[j] == '/')
      Path[j] = '\\';
#endif
}

void SetHomePath(char * Path)
{
  char * home;
#if defined(__LINUX__)
  home = getenv("HOME");
#elif defined(__WINDOWS__)
  home = getenv("USERPROFILE");
#else
  home = NULL;
#endif
  if (home && Path[0] == '~')
  {
    long len = strlen(home);

    memmove(Path + len - 1, Path, strlen(Path) + 1);
    memmove(Path, home, len);
  }
}

bool FileExists(const char * aFileName)
{
  bool Ok = true;
  struct stat Stat;

#if defined(__MAC__)
  int res = stat(aFileName, &Stat);
  Ok = !res;
#else
  stat(aFileName, &Stat);
#endif
  if (Ok)
    Ok =  S_ISREG(Stat.st_mode);

  return Ok;
}
  
#include <errno.h>

bool DirExists(const char * aDir)
{
  struct stat Stat;

#if defined(__WINDOWS__)
  int len = aDir ? strlen(aDir) : 0;
  bool cut = len && (aDir[len-1] == '\\' || aDir[len-1] == '/');
  if (cut)
  {
    while (len && (aDir[len-1] == '\\' || aDir[len-1] == '/'))
      len--;
    ((char *)(aDir))[len] = 0;
  }
#endif

  bool Ok = !stat(aDir, &Stat);
 
  if (!Ok)
    Ok = !errno;
#if defined(__WINDOWS__)
  if (cut)
    strcat((char *) aDir, "\\");
#endif

  return Ok && (Stat.st_mode & S_IFDIR);
}

TFileSize FileSize(FILE * InpFile)
{
  long Result = 0;

  if (InpFile &&
      !fseek(InpFile, 0, SEEK_END))
  {
    Result = ftell(InpFile);
    if (fseek(InpFile, 0, SEEK_SET))
      Result = 0;
  }
  return Result;
}

bool ReadFileBuffer(FILE * InpFile, unsigned char * & Stream, TFileSize & FileLen)
{
  bool Ok = true;
  TFileSize Len;
  TFileSize Offset = 0;

  while (Ok && Offset < FileLen)
  {
    Len = FileLen - Offset;
    if (Len > 0x800000)
      Len = 0x800000;
    Ok = fread(Stream + Offset, 1, Len, InpFile) == Len;
    Offset += Len;
  }
  if (!Ok)
  {
    delete[] Stream;
    Stream = NULL;
    FileLen = 0;
  }
  return Ok;
}

unsigned char * ReadFileStream(const char * FileName)
{
  unsigned char * Result;
  FILE * InpFile;
  TFileSize FileLen;
  TFileSize Len;
  char Temp[512];

  if (!FileName)
    return NULL;

  Result = NULL;

  Len = Min(High(Temp), strlen(FileName));
  strncpy(Temp, FileName, Len);
  Temp[Len] = 0;
  if (Temp[0] == '"')
  {
    memmove(Temp, Temp + 1, Len); // Len--;
    if (strchr(Temp, '"'))
    {
      *strchr(Temp, '"') = 0;
    }
  }
  if (strchr(Temp, '\n'))
    *strchr(Temp, '\n') = 0;
  if (strchr(Temp, '\r'))
    *strchr(Temp, '\r') = 0;
  if (!strncmp(Temp, "file:", 5))
    memmove(Temp, Temp+5, strlen(Temp) + 4);

  InpFile = OpenExistingFile(Temp);
  if(InpFile)
  {
    FileLen = FileSize(InpFile);
    if(FileLen)
    {
      Result = new unsigned char[FileLen+3];
      ReadFileBuffer(InpFile, Result, FileLen);
      if (Result)
      {
        Result[FileLen] = 0;
        Result[FileLen+1] = 0;  // für Unicode-File
        Result[FileLen+2] = 0;
      }
    }
    fclose(InpFile);
  }
#if defined(__CONSOLE__) && defined(__DEBUG__)
  else
    printf("file %s not read\n", Temp);
#endif
  return (unsigned char *) Result;
}

FILE * OpenExistingFile(const char * aName)
{
  char Name[500];
  if (aName && strlen(aName) < 200)
  {
    strcpy(Name, aName);
    aName = Name;
    SetHomePath(Name);
  }
  if (FileExists(aName))
    return fopen(aName, "rb");
  return NULL;
}

bool CanOpenDir(const char * aDir)
{
#if defined(__VC__) || defined(_MSC_VER) || defined(__DSWIG__)
  return DirExists(aDir);
#else
  DIR * Dir = opendir(aDir);
  if (Dir)
  {
    closedir(Dir);
    return true;
  }
  return false;
#endif
}

TFileSize FileSize(const char * Filename)
{
  FILE * InputFile;
  long Result = 0;

  InputFile = fopen(Filename, "rb");
  if (InputFile)
  {
    Result = FileSize(InputFile);
    fclose(InputFile);
  }
  return Result;
}

bool ReadFileBuffer(FILE * InpFile, char * & Stream, long & FileLen)
{
  bool Ok = true;
  long Len;
  long Offset = 0;

  while (Ok && Offset < FileLen)
  {
    Len = FileLen - Offset;
    if (Len > 0x800000)
      Len = 0x800000;
    Ok = (long) fread(Stream + Offset, 1, Len, InpFile) == Len;
    Offset += Len;
  }
  if (!Ok)
  {
    delete[] Stream;
    Stream = NULL;
    FileLen = 0;
  }
  return Ok;
}

bool ReadFile(const char * Filename, char * & stream, long & FileLen)
{
  FILE * file = fopen(Filename, "rb");
  if (!file)
  {
#if defined(__CONSOLE__)
    printf("file \"%s\" not found\n", Filename);
#endif
    return false;
  }

  FileLen = FileSize(Filename);
  stream = new char[FileLen+1];
  if (!ReadFileBuffer(file, stream, FileLen))
  {
    fclose(file);

    delete [] stream;
    stream = NULL;
    FileLen = 0;

#if defined(__CONSOLE__)
    printf("file \"%s\" not read\n", Filename);
#endif
    return false;
  }
  stream[FileLen] = 0;

  return true;
}

bool SaveFile(const char * Filename, const char * Stream, long StreamLen, bool Append)
{
  if (!Stream)
    return false;

  FILE * file = fopen(Filename, Append ? "ab" : "wb");
  if (!file)
    return false;

  long Pos = 0;
  bool Ok = true;
  while (Ok && Pos < StreamLen)
  {
    long len = StreamLen - Pos;
    if (len > 0x800000)
      len = 0x800000;
    Ok = (long) fwrite(Stream + Pos, 1, len, file) == len;
    Pos += len;
  }
  fclose(file);

  return Ok;
}

bool DecodeUnixTime(const char * aDate, tm & aTm)
{
  bool Ok = true;

  memset(&aTm, 0, sizeof(aTm));
  // Valid range for 32 bit Unix time_t:  1970 through 2038
  while ('0' <= *aDate && *aDate <= '9')
    aTm.tm_year = 10*aTm.tm_year + *aDate++ - '0';
  if (*aDate == '/')
    aDate++;
  aTm.tm_year -= 1900; // calendar year minus 1900

  aTm.tm_wday = aTm.tm_year + ((aTm.tm_year+3) / 4); // Weekday (0..6; Sunday = 0)

  while ('0' <= *aDate && *aDate <= '9')
    aTm.tm_mon = 10*aTm.tm_mon + *aDate++ - '0';
  if (*aDate == '/')
    aDate++;
  if (aTm.tm_mon == 0 || aTm.tm_mon > 12) // 0..11
  {
    aTm.tm_mon = 1;
    Ok = false;
  }
  aTm.tm_mon--;

  while ('0' <= *aDate && *aDate <= '9')
    aTm.tm_mday = 10*aTm.tm_mday + *aDate++ - '0';
  while (*aDate == ' ')
    aDate++;
  if (aTm.tm_mday == 0 || aTm.tm_mday > 31) // 1..31
  {
    aTm.tm_mday = 1;
    Ok = false;
  }

  aTm.tm_yday = aTm.tm_mday + cMonthDays[aTm.tm_mon]; // day of year 0..365
  if (aTm.tm_mon <= 1 || (aTm.tm_year & 3))
    aTm.tm_yday--;
  aTm.tm_wday = (aTm.tm_wday + aTm.tm_yday) % 7;

  while ('0' <= *aDate && *aDate <= '9')
    aTm.tm_hour = 10*aTm.tm_hour + *aDate++ - '0';
  if (*aDate == ':')
    aDate++;
  if (aTm.tm_hour >= 24)          // 0..23
  {
    aTm.tm_hour = 0;
    Ok = false;
  }

  while ('0' <= *aDate && *aDate <= '9')
    aTm.tm_min = 10*aTm.tm_min + *aDate++ - '0';
  if (*aDate == ':')
    aDate++;
  if (aTm.tm_min >= 60)           // 0..59
  {
    aTm.tm_min = 0;
    Ok = false;
  }

  while ('0' <= *aDate && *aDate <= '9')
    aTm.tm_sec = 10*aTm.tm_sec + *aDate++ - '0';
  if (aTm.tm_sec >= 60)           // 0..59
  {
    aTm.tm_sec = 0;
    Ok = false;
  }

  aTm.tm_isdst = -1;  // Information über Sommerzeit steht nicht zur Verfügung.

  return Ok;
}

static bool quitting = false;

bool term_signal_is_set()
{
  return quitting;
}

#if defined(__LINUX__)
static void sigterm(int sig)
{
  quitting = true;
	signal(sig, SIG_IGN);
}

static struct sigaction sa;

bool set_term_signal()
{
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigterm;
  if (sigaction(SIGTERM, &sa, NULL) ||
      sigaction(SIGQUIT, &sa, NULL) ||
      sigaction(SIGINT, &sa, NULL))
  {
    printf("Failed to set SIGTERM handler. EXITING.\n");
    return false;
  }
  return true;
}
#else
bool set_term_signal()
{
  return true;
}
#endif


////////////////////////////////////////////////////////////////////////////////

static const unsigned short MacLatinToUnicode[0x80] =
{
  /* 0x80: */ 0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1,
  /* 0x88: */ 0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
  /* 0x90: */ 0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3,
  /* 0x98: */ 0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
  /* 0xA0: */ 0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
  /* 0xA8: */ 0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
  /* 0xB0: */ 0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211,
  /* 0xB8: */ 0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x2126, 0x00E6, 0x00F8,
  /* 0xC0: */ 0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB,
  /* 0xC8: */ 0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
  /* 0xD0: */ 0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA,
  /* 0xD8: */ 0x00FF, 0x0178, 0x2044, 0x00A4, 0x2039, 0x203A, 0xFB01, 0xFB02,
  /* 0xE0: */ 0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1,
  /* 0xE8: */ 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
  /* 0xF0: */ 0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC,
  /* 0xF8: */ 0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7
};

// windows code page 1252
static const unsigned short CodePage1252ToUnicode[0x80] =
{
  /* 0x80: */ 0x20AC,    '?', 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
  /* 0x88: */ 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152,    '?', 0x017D,    '?',
  /* 0x90: */    '?', 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
  /* 0x98: */ 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153,    '?', 0x017E, 0x0178,
  /* 0xA0: */ 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
  /* 0xA8: */ 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  /* 0xB0: */ 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
  /* 0xB8: */ 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
  /* 0xC0: */ 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
  /* 0xC8: */ 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  /* 0xD0: */ 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
  /* 0xD8: */ 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
  /* 0xE0: */ 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
  /* 0xE8: */ 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  /* 0xF0: */ 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
  /* 0xF8: */ 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};


unsigned short MacToUnicode(unsigned char c)
{
  if (c <= 0x7f)
    return c;

  return MacLatinToUnicode[c - 0x80];
}

unsigned char UnicodeToMac(unsigned short u)
{
  if (u <= 0x7f)
    return (unsigned char) u;

  for (unsigned i = 0x80; i < 0x100; i++)
    if (u == MacLatinToUnicode[i])
      return (unsigned char) i;

  return '?';
}

unsigned short CP1252_ToUnicode(unsigned char c)
{
  if (c <= 0x7f)
    return c;

  return CodePage1252ToUnicode[c - 0x80];
}

unsigned char UnicodeToCP1252(unsigned short u)
{
  if (u <= 0x7f)
    return (unsigned char) u;

  for (unsigned i = 0x80; i < 0x100; i++)
    if (u == CodePage1252ToUnicode[i])
      return (unsigned char) i;

  return '?';
}

}
////////////////////////////////////////////////////////////////////////////////


#if (0)

// Test zu "timestamp" des Volkszählers
int main(int argc, char* argv[])
{
  int Days;

  // 1334416684000.  14. April 2012  17:19
  //
  // ./ms2time.sh 1399346245967  ==> 2014-05-06 03:17:25
  //              1399349845000
  //                   -36
  //              1399353445000
  //              1399334400000
  //                   11800
  // date -d "2014-05-06 05:17:25" ==>  1399346245

  if (NCanUtils::GetArtDays(2012, 4, 14, Days))
  {
    char str[64];
    char mstr[64];

    int sec = 60*(60*15 + 18) + 4;

    NCanUtils::PrintTime((int) Days, 1000*sec, str, false);
    NCanUtils::PrintMs((int) Days, 1000*sec, mstr);
    printf("%d   %s   %s\n", sec, mstr, str);
  }

  return 0;
}

#endif



