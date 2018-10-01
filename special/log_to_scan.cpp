/*
 *
 *  Copyright (C) 2017 Jürg Müller, CH-5524
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

#include <stdio.h>
#include <string.h>

#include "../KCanLogFile.h"

int main(int argc, char* argv[])
{
  printf("Reads log file - prints scan format\n"
#if defined(__WINDOWS__)
         "copyright (c) 2017 Juerg Mueller, CH-5524\n\n");
#else
         "copyright (c) 2017 Jürg Müller, CH-5524\n");
#endif
  
  bool trace = !strcmp(argv[argc-1], "trace");
  if (trace)
    argc--;
 
  if (argc != 2)
  {
    printf("\nusage:\n"
#if defined(__WINDOWS__)
             "  log_to_scan <log file name>\n\n"
             "example: log_to_scan ..\\mac\\can_log_20140724.log \n\n");
#else
             "  ./log_to_scan <log file name>\n\n"
             "example: ./log_to_scan ../mac/can_log_20140724.log\n\n");
#endif
    return -1;
  }
  
  KCanLogFile logfile;
  if (logfile.ReadFile(argv[1]))
    logfile.PrintScanTable();
  
  return 0;
}