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


#if !defined(elster_perl_H)

  #define elster_perl_H

  namespace elster
  {
    const char * geterrormsg(void);
    const char * getvaluemsg();
    const char * getvalue(const char *); // get elster value
    bool setvalue(const char *); // setvalue, setbits und clrbits
    bool setbits(const char *); // set bits
    bool clrbits(const char *); // clear bits
    const char * getstring(const char *); // get elster string
    bool setstring(const char * params); // set elster string
    const char * getname(const char *); // get elster name
    const char * gettype(const char *); // get elster type
    const char * toggle_trace(void); // set tracing
    bool setdev(const char *); // set can device (Windows COMx)
    bool setcs(void); // set optical interface (ComfortSoft)
    bool set_can232(void); // set CAN232 / USBtin as interface
    bool initcan();
    void undef(); // delete instance
    bool setsniffedframe(const char * params);
    const char * getsniffedvalue(const char * params);
    const char * getfirstsniffedvalue(const char * params);
  };


#endif

