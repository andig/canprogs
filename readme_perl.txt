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

Es gibt ein Perl5-Modul "elster_perl" (Datei elster_perl.so). Dieses Modul befindet sich in der Testphase!


Hier eine kleine Demonstration:

$perl -e 'use elster_perl; $a = elster_perl::getvalue("680 601 000a"); print "$a\n";'

1034

$perl -e 'use elster_perl; $a = elster_perl::getstring("680 601 000a"); print "$a\n";'

04.10

>perl -e 'use elster_perl; $a = elster_perl::gettype("000a"); print "$a\n";'

9

$perl -e 'use elster_perl; $a = elster_perl::getname("000a"); print "$a\n";'

DATUM

$perl -e 'use elster_perl; $a = elster_perl::getstring("680 500 930"); print "$a\n";'

90.076

$perl -e 'use elster_perl; $a = elster_perl::getname("930"); print "$a\n";'

WAERMEERTRAG_HEIZ_SUM_KWH

$perl -e 'use elster_perl; $a = elster_perl::gettype("930"); print "$a\n";'

5


Erzeugt wird elster_perl.so mit 

$./can_elster_perl.arm

Die Software "swig" wird benötigt. Sie generiert Perl-"Wrap"-Dateien.


toggle_trace(""): toggelt das Trace-Flag
setdev("can1"): setzt das Device 
setcs(""): das optische Interface wird erwartet (ist noch nicht getestet) 

getname("000b"): gibt den Namen zum Elster-Index zurück (siehe ElsterTable.inc)
gettype("000b"): gibt den "enum ElsterType" zum Elster-Index zurück (siehe KElsterTable.h)
getvalue("680 601 000b"): liest den Wert zum Elster-Index (hier 0x000b) aus und gibt ihn als positive ganze Zahl zurück
getstring("680 601 000b"): gleich wie getvalue, zurückgegeben wird aber ein formatierter String
 

Nach dem Aufruf von "getvalue" oder "getstring" kann die Schnittstelle nicht mehr verändert werden.

____________________________________________________

Unter MAC habe ich es geschafft, die elster_perl.dylib zu bilden. Beim Laden des Moduls gibt es Linker-Fehler (Segmentation fault: 11).
Es könnte sein, dass der Stack zu klein ist. Stack vergrössern:

ulimit -s 65000

funktioniert nicht. Mit 

ulimit -a

kann man die Settings anschauen.

==> Ist gelöst: Richtiges Perl verwenden:
  
  1. Mit "perl -v" aktuelle Version ansehen.
  2. Mit "which -a perl" alle Versionen auflisten. Das richtige perl bestimmen und die Pfade in can_elster_perl.mac definieren. 

----------------------------------------------------

Optisches Interface verwenden:

perl -e 'use elster_perl; elster_perl::setcs(); elster_perl::setdev("COM3"); $a = elster_perl::getstring("680 180 000c"); print "$a\n";'

----------------------------------------------------

4.3.2017: getestet unter Windows XP mit 

  usbtin_simul com8 trace

FHEM-Befehle:

reload 50_Elster
define stiebel elster com9

Fehlermeldung im Log:

Can't locate Win32/SerialPort.pm in @INC (you may need to install the Win32::SerialPort module) (@INC contains: . c:/strawberry-perl-5/perl/site/lib c:/strawberry-perl-5/perl/vendor/lib c:/strawberry-perl-5/perl/lib ./FHEM) at ./FHEM/DevIo.pm line 385.

Dieses Modul wird aber gar nicht gebraucht.

Weitere Meldung:

FHEMWEB WEB CSRF error: fhem_25164687156556.1 ne fhem_114298635482787. For detals see the csrfToken FHEMWEB attribute

----------------------------------------------------

Fhem auf Raspi ausgetestet (5.3.2017):

stiebel_simul <==> CAN-Bus <==> fhem (50_Elster.pm)

Achtung: in 50_Elster.pm "elster_perl::set_can232();" auskommentieren!


