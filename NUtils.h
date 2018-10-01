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

#if !defined(NUtils_H)

  #define NUtils_H

  #include <stdio.h>
  #include <time.h>
  #include "NTypes.h"

  namespace NUtils
  {
    extern const unsigned cMonthDays[13];

    void GetDateFromDays(int aDays, int & aDay, int & aMonth, int & aYear);
    void GetDateAndTime(double aTime, int & aDay, int & aMonth, int & aYear,
                        int & aHour, int & aMin, int & aSec, int & aMs);
    void Time(const struct timeval & Time, int & aDays, int & aMs);
    void Time(int & aDays, int & aMs);
    double TimeSecs();
    void SleepMs(int aMs);

    unsigned DateTimeToFileDate(double aDateTime);
    bool DecodeUnixTime(const char * aDate, tm & aTm);

    // Das Resultat zeicht auf den '.'
    const char * ExtractExtension(const char * FilePath);
    // Nur der Filename wird zurückgegeben (ohne Pfad)
    const char * ExtractFilename(const char * FilePath);
    // Alle '/' werden zu '\' umgewandelt und der Pfad (nicht Filename) wird
    // mit '\' abgeschlossen
    void MakeStandardPath(char * Path, bool UseHomePath = true);
    void SetHomePath(char * Path);

    bool FileExists(const char * aFileName);
    bool DirExists(const char * aDir);
    bool CanOpenDir(const char * aDir);
    TFileSize FileSize(FILE * InpFile);
    TFileSize FileSize(const char * Filename);
    bool ReadFile(const char * Filename, char * & stream, long & FileLen);
    bool SaveFile(const char * Filename, const char * Stream, long StreamLen, bool Append = false);
    FILE * OpenExistingFile(const char * aName);

    // Dem Stream wird ein abschliessender 0-char angefügt.
    unsigned char * ReadFileStream(const char * FileName);
    bool ReadFileBuffer(FILE * InpFile, unsigned char * & Stream, TFileSize & FileLen);
    TFileSize ReadBinaryFileStream(const char * FileName, unsigned char * & Stream);

    bool GetDays(int Year, int Month, int Day, int & Days);
    void PrintDayTime(int time_ms, char * OutStr);
    void PrintTime(int time_days, int time_ms, char * OutStr, bool csv);
    void PrintMs(int days, int ms, char * stream);
    void PrintDateTime(int days, int ms, char * stream);

    bool IsDigit(char Digit);
    bool IsHexDigit(char aDigit);
    // dezimale oder hexadezimale Zahl als String erwartet
    // hex. Format: "0x.."  oder "$.."
    bool IsNumber(const char * aPtr);

    int GetHexDigit(char aDigit);
    unsigned GetHex(const char * & Line);
    int  GetDigit(char Digit);
    bool GetHexByte(const char * & Line, unsigned char & byte);
    bool GetInt(const char * & Line, TInt64 & res);
    bool GetUnsigned(const char * & Line, unsigned & res);
    unsigned GetInt(TInt64 & res, const char * Line);
    bool GetDouble(const char * & Ptr, double & res);
    bool SkipBlanks(const char * & Line);

    char LowerCase(char c);
    char UpperCase(char c);
    void ToLowerCase(char * Str);

    bool WriteUTF8_FileStream(const char * FileName, const unsigned char * Stream, TFileSize StreamLen);
    bool WriteUTF16_FileStream(const char * FileName, const unsigned char * Stream, TFileSize StreamLen);

    bool set_term_signal();
    bool term_signal_is_set();

    unsigned short MacToUnicode(unsigned char c);
    unsigned char UnicodeToMac(unsigned short u);
    unsigned short CP1252_ToUnicode(unsigned char c);
    unsigned char UnicodeToCP1252(unsigned short u);
  }

#endif

