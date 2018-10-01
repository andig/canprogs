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

Das beiliegende Programm can_server ist auf einem Raspberry Pi lauffähig. Es kann mit 

  ./can_server.arm

erzeugt werden. 

Mit dem Aufruf

  ./can_server

sehen Sie:

Elster-Kromschröder TCP/IP server
copyright (c) 2014 Jürg Müller, CH-5524

server established: 192.168.1.53:5524
device: can0
Kennwort: 0xa51b  Version: 165-27


allgemeiner Aufruf: ( <can device> | "simulation" ) [ <scan table name> ] [ "binary_protocol" ] [ "trace" ] 

also z.B.           ./can_server can0 trace


+==============================================+
|                                              |
| Bitte Nachtrag vom 25. Januar 2017 beachten! |
|                                              |
+==============================================+
_______________________________________________________________

can_server setzt auf dem Port 5524 ein einfaches Server-TCP-Protokoll auf.

Als Gegenstelle kann zu Testzwecken can_client (Raspi), can_client_mac (MAC) oder can_client.exe (Win) verwendet werden. Diese Programme verlangen eine Tastatur-Eingabe in Form des seriellen Kromschröder- bzw. ComfortSoft-Formats (10 oder 12 hexadezimale Bytes):

  0D.00.03.01.00.FA.01.D4.80.08.02.68

oder 

  0D.00.03.01.00.FA.01.D4.80.08

_______________________________________________________________

Zweck des Programms can_server:

ComfortSoft.exe (Windows) mittels der Daten des Raspberry Pi via TCP/IP-Schnittstelle zu bedienen.

Damit ComfortSoft lauffähig ist, werden "com0com" und "CS_Bruecke.exe" benötigt. "com0com" (oder ein anderes virtuelles Null-Modem-Paar) muss vom Internet heruntergeladen werden; "CS_Bruecke.exe" liegt bei.  

ComfortSoft (Windows) <--> com0com (Windows / Open Source) <--> CS_Bruecke.exe (Windows / Eigenbau) <== ethernet ==> can_server (raspi / Eigenbau)

_______________________________________________________________

Software-Stand:

Die Tests der Software beschränkt sich auf ein WPM2 (WPL33).
Auch das Abspeichern geänderter Parameter ist implementiert. 

Abfragen auf der CAN-ID 000 sind eingeschränkt.

Die für die Kommunikationseröffnung benötigte Kennung wird mit 

  ./can_scan can0 680 301.000b

abgefragt. Also auf dem CAN-Device 301 mit dem Elster-Index 000b.

_______________________________________________________________

Zur Entwicklung beigetragen haben:

Der einzigen Mittschnitt der optischen Schnittstelle, den ich auf dem Internet fand:

  http://www.ip-symcon.de/forum/threads/15888-Wolf-CSZ-mit-ISM1

Klaus vom messpunkt.org

Marc2012 und andere vom www.haustechnikforum.de

chriss1980 und andere vom knx-user-forum.de

_______________________________________________________________


Stichwörter: 
  Raspberry Pi CAN-BUS 
  Stiebel-Eltron Elster-Kromschröder Wärmepumpe WPL33 WPMII Wärmepumpen Manager WPM2
  ComfortSoft

_______________________________________________________________

Erweiterungen vom 21.Sept.2014

Das Programm CS_Bruecke kann eine WP simulieren. Dabei wird der can_server (Raspi) nicht gebraucht. Die WP, die simuliert wird, ist meine eigene. Die Daten dazu sind in scan_data.inc. Ihre Daten (vgl. scan_data.inc) können Sie mit

  ./can_scan can0 680 total

auf dem Raspi erzeugen.

- Die Simulation kann auch mit 

  ./can_simul tty.usbserial-FTK1S17H trace

auf dem MAC oder
 
  can_simul.exe com3 trace

auf Windows durchgeführt werden.  


In Planung:

- CS_Bruecke mit CAN232 von Lawicel verwenden.

_______________________________________________________________

Erweiterung vom 1.Okt.2014

can_simul kann beim Start eine scan-Tabelle einlesen (vgl. scan_data.inc)

  ./can_simul tty.usbserial-FTK1S17H ../scan_data.inc trace

bzw:
 
  can_simul.exe com3 .\scan_dta.inc trace

_______________________________________________________________

Erweiterung vom 15. Jan.2015

Die "Kennung" für den WPMiw wird nicht beim Elster-Index 0x000b sondern bei 0x0199 gelesen. 

Korrektur: Erweiterung der Tabelle "KComfortFrame::scan_geraete_id" (NCanUtils.cpp). 

_______________________________________________________________

To do (22. Januar 2015):

Der can_server bedient mit einem einzigen Thread die TCP-Anfragen. Wenn es ein Problem gibt, stirbt dieser Thread und damit auch das Programm. Für kurzzeitige Verbindungen (um z.B. ComfortSoft verwenden zu können)  genügt die jetzige Lösung von can_server. Will man aber eine andauernde Verbindung, ist die Lösung nicht brauchbar. Es braucht dann einen Mechanismus, der zu neuen Verbindungen neue Threads generiert und die Threads zu den geschlossenen Verbindungen entfernt. Im Projekt miniDLNA wird ein solcher Mechanismus verwendet.

Dazu der Kommentar von robots in

http://www.haustechnikdialog.de/Forum/t/168379/Stiebel-Eltron-WPF-per-Konsole-auslesen-und-Parameter-aendern


***************

Problem with my scanning tool is, that my server crashes after longer sessions :D But it should work against your tcp server as well (protocol is the same)

**************

_______________________________________________________________

Erweiterung vom 16. Februar 2015

Die Information von olima (siehe http://www.ip-symcon.de/forum/threads/15888-Wolf-CSZ-mit-ISM1) wurde von mir in scan_data_olima.inc verpackt. Damit kann mit der ComfortSoft eine "Wolf GasTherme CSZ11" simuliert werden. Olima verwendet die ComfortSoft mit einem "Wolf ISM1 Interface (RS232)".

In scan_data_olima.inc sind nur die Initialisierungsdaten vorhanden. Alle anderen Daten fehlen - leider! Dementsprechend sind keine Messwerte usw. vorhanden.

Tipp zu allen Simulationen:
Durch verändern/einfügen eines Elster-Wertes in einer "scan_date.inc"-Datei kann man nach dem Importieren (Einlesen) der Simulations-Daten in die CS_Bruecke in der ComfortSoft nach zirka 10 Sekunden die Veränderung sehen. Damit konnte ich zu einigen Werten, wie z.B.

  { "BETRIEBS_STATUS"                                  , 0x0176, 0},

die entsprechenden Flags

  // Verdichter 1:        0x0001
  // Verdichter 2:        0x0002
  // Pufferladepumpe 1:   0x0040
  // Pufferladepumpe 2:   0x0080
  // DHC 1:               0x1000
  // DHC 2:               0x2000
  // Warmwasserladepumpe: 0x8000
  // EVU Sperre:          0x8000

bestimmen. Eigentlich lässt sich die ganze Elster-Tabelle mit Simulationen bestimmen!

_______________________________________________________________

ComfortSoft unter wine (OSX) führt zum Fehler: 


fixme:actctx:parse_depend_manifests Could not find dependent assembly L"Microsoft.VC80.MFCLOC" (8.0.50608.0)
fixme:mscoree:_CorDllMain (0x7c4c0000, 1, 0x1): stub
fixme:mscoree:_CorDllMain (0x7c550000, 1, 0x1): stub
Method '<Module>:_WinMainCRTStartup ()' in assembly 'C:\Program Files\ComfortSoft\ComfortSoft.EXE' contains native code that cannot be executed by Mono in modules loaded from byte arrays. The assembly was probably created using C++/CLI.

...


run vcrun2005
_______________________________________________________________

Erweiterung vom 26. Juli 2015:

can_server kann neu simulierte Daten liefern. Der Aufruf ist

  ./can_server simulation ../scan_data.inc

Die Simulation funktioniert auch auf den Betriebssystemen OSX und Windows. 

Mit der Simulation erhoffe ich mir, den TCP/IP-Server verbessern zu können. (Bessere Voraussetzungen zum Debuggen.)  

_______________________________________________________________

Erweiterung vom 30. Juli 2015:

Das Programm CS_Bruecke können die meisten nicht selbst kompilieren. Deshalb gibt es jetzt eine einfache MinGW-Version (siehe cs_bruecke_gw.bat). Diese bietet nur eine Simulation der WP-Daten, aber damit lassen sich die Elster-Indizes mit den zugehörigen CAN-IDs ermitteln. Vorgehen:

  - von der eigenen WP einen Scan herstellen

    ./can_scan ttyABC 680 total > scan_data.txt

  - auf einem Windows-Rechner com0com installieren
    ebenfalls ComfortSoft installieren

  - cs_bruecke_dos starten
 
    cs_bruecke_dos COM3 ..\scan_data.txt

  - jetzt ComfortSoft starten

  - im DOS-Fenster werden die Elster-Indizes aufgelistet, die von ComfortSoft verwendet werden

Der Verbindungsaufbau:

  ComfortSoft <==> com0com <==> cs_bruecke_dos  

_______________________________________________________________

Erweiterung 8. August 2015:

Parameter "binary_protocol": Die Daten werden binär übertragen (wie zwischen optischem Interface und ComfortSoft). Der einfachste Weg, um mit der ComfortSoft arbeiten zu können, ist:

 - zuerst com0com installieren (z.B. com3 und com4 (oder CNCB0)
   eventuell mit setupg.exe (ist Teil von com0com) ein weiteres Paar generieren

 - Daten mit com0tcp umleiten:
   
   com2tcp --baud 2400 --parity e --ignore-dsr --stop 2 com4 192.168.1.67 5524   

 - can_server starten

   ./can_server can0 binary_protocol

   oder

   ./can_server ttyXX binary_protocol

   oder

   can_server com1 binary_protocol        (am com1-Port ist der USBtin angeschlossen)

   oder

   can_server simulation binary_protocol

_______________________________________________________________
_______________________________________________________________

Konfiguration mit höherer Server-Stabilität (4. April 2016):

Als Server auf dem Linux-System dient "ser2net" und auf Windows "com2tcp".


Null-Modem mit socat auf Linux installieren
-------------------------------------------

socat -d -d pty,raw,echo=0 pty,raw,echo=0 &

Die Ausgabe von socat:

  2016/04/03 16:33:21 socat[28883] N PTY is /dev/ttys002
  2016/04/03 16:33:21 socat[28883] N PTY is /dev/ttys003
  2016/04/03 16:33:21 socat[28883] N starting data transfer loop with FDs [5,5] and [7,7]


ser2net auf Linux installieren
------------------------------

./ser2net -c ser2net.conf -u

Das File ser2net.conf sollte eine Zeile wie die folgende enthalten:

  2004:raw:0:/dev/ttys002:2400 EVEN 2STOPBIT 8DATABITS -XOFFXON LOCAL -RTSCTS

Wobei "/dev/ttys002" einem von socat generierten Device entsprechen sollte.


com2tcp auf Windows installieren
--------------------------------

com2tcp --baud 2400 --parity e --ignore-dsr --stop 2 com4 192.168.0.55 2004

Wobei "com4" ein Port ist, welches von com0com erzeugt wurde.


Simulation des optischen Interfaces (auf Linux)
-----------------------------------------------

./can_simul ttys003

ttys003 vgl. socat.


Danach kann ComfortSoft auf Windows verwendet werden.

_______________________________________________________________

25. Januar 2017:

- Projekt CS_Bruecke.cbproj an RAD Studio 10.1 Berlin, das gratis zur persönlichen Nutzung heruntergeladen werden kann.

- Die gesamte Software kann auch mit einem USB2CAN von 8devices.com unter Windows verwendet werden. D.h. dann ist kein 
  "can_server" notwendig und damit ist die Instabilität behoben! 



