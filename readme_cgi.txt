/*
 *
 *  Copyright (C) 2014 Jürg Müller, CH-5524
 *
 *  This program is free software: you can redistribute it and/or 
 *  modify it under the terms of the GNU Lesser General Public 
 *  License as published by the Free Software Foundation version 3
 *  of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program. If not, 
 *  see http://www.gnu.org/licenses/ .
 */


Das beiliegende Programm can_cgi ist auf einem Raspberry Pi lauffähig. Es kann mit dem Skript
  
  can_cgi.arm

neu generiert werden.

can_cgi benötigt cgi_template.html. Beide Files müssen sich im cgi-bin-Verzeichnis des Web-Servers befinden.

__________________________________________________________________________________

can_cgi verwendet als Device "can0" und zum Werte anfordern die CAN-ID 0x780. Beides kann im Bedarfsfall in NCgi.cpp geändert werden.

Codierung von cgi_template.html:

  - normale html-Codierung
  - Vorgaben für WP-Werte:
 
      ${<device>.<elster-parameter>}   oder
      ${<device>:<parameter>}

    wobei für das <device> der hexadezimale Wert für das Gerät eingesetzt werden muss.
    <elster-parameter>: Name aus der Elster-Tabelle
    <parameter> hex. Wert.  

Als Beispiel gibt es ein cgi_template.html im Verzeichnis can_progs.








