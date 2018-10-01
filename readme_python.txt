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

Es gibt ein Python3-Modul "elster" (Datei elster.so). Hier eine kleine Demonstration:

pi@raspberrypi ~/can_progs $ python3
Python 3.2.3 (default, Mar  1 2013, 11:53:50) 
[GCC 4.6.3] on linux2
Type "help", "copyright", "credits" or "license" for more information.
>>> import elster
>>> elster.setdev("ttys006")
b'ok'
>>> elster.getvalue("680 601 000a")
'1034'
>>> elster.getstring("680 601 000a")
"b'04.10.'"
>>> elster.gettype("000a")
'9'
>>> elster.getname("000a")
"b'DATUM'"
>>> elster.getstring("680 500 930")
"b'90.076'"
>>> elster.getname("930")
"b'WAERMEERTRAG_HEIZ_SUM_KWH'"
>>> elster.gettype("930")
'5'
>>> 


Erzeugt wird elster.so mit 

$./can_elster.arm


elster.trace(""): toggelt das Trace-Flag
elster.setdev("can1"): setzt das Device (muss nach "import elster" vorgenommen werden)
elster.setcs(""): das optische Interface wird erwartet (muss nach "import elster" vorgenommen werden; ist noch nicht getestet) 

Nach dem Aufruf von "getvalue" oder "getstring" kann die Schnittstelle nicht mehr verändert werden.

____________________________________________________

Das Windows-Modul mingw/elster.pyd ist mit Python 3.4.2 und MinGW generiert worden.

----------------------------------------------------

Mac-Test 16. Juli 2017:

socat gestartet: 

2017/07/11 15:09:31 socat[6095] N PTY is /dev/ttys002
2017/07/11 15:09:31 socat[6095] N PTY is /dev/ttys003
2017/07/11 15:09:31 socat[6095] N starting data transfer loop with FDs [5,5] and [7,7]

USBTin Simulation:

./usbtin_simul ttys003 trace

Python3:

>>> import elster
>>> elster.set_can232()
b'ok'
>>> elster.setdev("ttys002")
b'ok'
>>> elster.trace()
b'true'
>>> elster.getvalue("680 601 000a")
b''
>>> elster.geterrmsg()
b'not allowed send_id'

Für die Eingabe im Terminal:

import elster
elster.set_can232()
elster.setdev("ttys002")
elster.trace()
elster.getvalue("680 601 000a")
elster.geterrmsg()
elster.setvalue("680 601 000a 150b")

