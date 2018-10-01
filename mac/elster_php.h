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


#if !defined(elster_php_H)

  #define elster_php_H

  class elster
  {
    public:
      static const char * geterrormsg(void);
      static const char * getvaluemsg();
      static const char * getvalue(const char *); // get elster value
      static bool setvalue(const char *); // setvalue, setbits und clrbits
      static bool setbits(const char *); // set bits
      static bool clrbits(const char *); // clear bits
      static const char * getstring(const char *); // get elster string
      static bool setstring(const char * params); // set elster string
      static const char * getname(const char *); // get elster name
      static const char * gettype(const char *); // get elster type
      static const char * toggle_trace(void); // set tracing
      static bool setdev(const char *); // set can device (Windows COMx)
      static bool setcs(void); // set optical interface (ComfortSoft)
      static bool set_can232(void); // set CAN232 / USBtin as interface
      static bool initcan();
      static void undef(); // delete instance
      static bool setsniffedframe(const char * params);
      static const char * getsniffedvalue(void);
  };

#endif

