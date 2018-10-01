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


Das beiliegende Programm can_vz ist auf einem Raspberry Pi lauffähig. Es kann mit dem Skript
  
  can_vz.arm

neu generiert werden.

can_vz benötigt eine XML-Konfigurationsdatei, die als Parameter bezeichnet werden muss. 

Aufruf:
 
  ./can_vz VZ_config.xml

Die Datei VZ_config.xml steuert can_vz, dass die Werte der Wärmepumpe in eine Volkszaehler-MySQL-Datenbank eingetragen werden können.

Mit der Datei VZ_config2.xml können die Werte mit Zeitstempel in eine allgemeine MySQL-Datenbank eingefügt werden. Ist noch experimentell.

__________________________________________________________________________________

Ohne Parameter liefert can_vz folgendes:

  Volkszaehler can-bus logger
  copyright (c) 2014 Jürg Müller, CH-5524

  usage:
  can_vz <xml configuration> [ "trace" ]
  example: can_vz VZ_config.xml

__________________________________________________________________________________

Parameter "can_msg" (XML-Konfigurationsdatei):

Dieser Parameter soll ein can-Telegramm enthalten. Prinzipiell gibt es zwei Arten von Telegrammen, nämlich die aktive Anforderung und das passive Mithören. Beim aktiven Telegramm wird die Anforderung von can_msg am can-Bus abgesetzt und auf eine Antwort gewartet. Passive Telegramme werden aus den empfangenen Telegrammen herausgefiltert. Bei msg_can-Einträgen, bei denen das 2. Digit eine "1" ist gelten als aktive Anfragen, alle anderen werden passiv behandelt.

Beispiel für eine aktive Abfrage an der can-Adresse 180: "31 00 fa 00 16"

"61 02 fa 00 16" aktive Abfrage an der can-Adresse 302

"a0 79 fa 01 d7" passive Abfrage 

__________________________________________________________________________________

Zweck des Programms can_vz:

a) Messwerte via HTTP-Protokoll in eine Volkszaehler-MySQL-Datenbank zu schreiben.

b) Messwerte in eine beliebige MySQL-Datenbank zu schreiben.

__________________________________________________________________________________

Stichwörter: 

  Raspberry Pi CAN-BUS 
  Stiebel-Eltron Elster-Kromschröder Wärmepumpe WPL33 WPMII Wärmepumpen Manager WPM2
  Volkszähler.org mysql

__________________________________________________________________________________

Ergänzungen vom 19.Okt. 2014

Anstelle des Parameters "can_msg" kann für aktive Abfragen "can_inst" verwendet werden (siehe VC_config.xml).

can_inst="180.0016"   (Anfrage an can-Adresse 180 mit dem Elster-Index 0016 - alles hexadezimal)

Wenn zu einem "channel" das Attribut "format" fehlt, wird der "Type" aus der Elster-Tabelle eingesetzt.

Bei Elster-Type et_double_val werden 2 Werte gelesen, bei Elster-Type et_triple_val sind es 3 Werte. Die Werte werden zu einem "double" zusammengesetzt.

__________________________________________________________________________________

Wenn es einfach nicht funktioniert:

- Bitte zuerst mit can_scan die Anfragen testen. Z.B.

  ./can_scan can0 700 180.0016

  Damit ist sichergestellt, dass die Verbindung zur WP funktioniert.

- Die beiden "%s" in "template" sind Platzhalter. Sie dürfen nicht fehlen!

- Mit "trace" testen. 

  ./can_vz VZ_config.xml trace

  Bitte mir mitteilen, wo das Programm den Dienst verweigert.

Danke 

__________________________________________________________________________________

Erweiterungen vom 15. Nov. 2014:

Control-C wird abgefangen. Danach wird das Programm ordnungsgemäss beendet.

Das zeitliche Verhalten von KCanElster::Send wurde optimiert.

  Die Zeit zwischen Anfrage und Antwort dauert bis zu 160 ms.
  Die Zeit zwischen 2 Anfragen sollte mindestens 1000 ms betragen. Das stresst die Steuerung weniger. 


