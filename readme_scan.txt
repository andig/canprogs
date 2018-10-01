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

Das beiliegende Programm can_scan ist auf einem Raspberry Pi lauffähig. Es kann mit dem Skript
  
  can_scan.arm

neu generiert werden.

Ohne Parameter liefert can_scan folgendes:

  elster-kromschröder can-bus address scanner and test utility
  copyright (c) 2014 Jürg Müller, CH-5524

  use scanner
    can_scan <can dev> <sender can id> [ "total" ] [ "cs_com" ] [ "trace" ]
  or read/change value
    can_scan <can dev> <sender can id> <receiver can id> [ "." <elster index> [  "." <new value> ] ] [ "cs_com" ] [ "trace" ]

  example: can_scan can0 780 180           (scan all elster indices from 0000 to 1fff)
  or       can_scan can0 780 180.0126      (read minutes at elster index 0126)
  or       can_scan can0 780 180.0126.0f00 (set minutes to 15)

  allowed sender can id: 680 or 700 or 780

  BE CAREFUL WITH THIS TOOL !!!


Die Warnung bezieht sich auf die Möglichkeit, Werte zu überschreiben.


+==============================+
| Alle Werte sind hexadezimal! |
+==============================+ 

_______________________________________________________________

Zweck des Programms can_scan:

a) Es wird verwendet, um die "ElsterTabelle" zu verbessern.

b) Es können damit alle Werte zu einer can-id ausgelesen werden. Z.B. können für <scan can-id> die folgenden Werte eingesetzt werden: 

   180, 300, 301, 302, 303, 480, 500, 600, 601, 602, 603

c) Es kann ein einzelner Wert zu einem Index aus der Elster-Tabelle ausgelesen werden. Im can-Telegramm wird dann folgendes eingesetzt:

   n1 0m fa xx xx

   wobei n = <receiver can id> / 80 ; m = <receiver can id> modulo 8; xxxx der Index aus der Elster-Tabelle

   entspricht 

   cansend  <sender can id>#n1.0m.fa.xx.xx

d) Es kann ein Parameter verändert werden. Dabei ist höchste Vorsicht geboten! Das can-Telegramm ist wie folgt zusammengesetzt

   n2 0m fa xx xx vv vv

   wobei hier noch der neue Wert vvvv dazugesetzt wird.

   entspricht 

   cansend  <sender can id>#n2.0m.fa.xx.xx.vv.vv

siehe auch: https://knx-user-forum.de/forum/öffentlicher-bereich/knx-eib-forum/code-schnipsel/26505-anbindung-tecalor-ttw13/page4  #56


Beispiele:
----------

1. Wert auslesen:

./can_scan can0 680 180.0126 trace

setzt das folgende Telegramm ab:

 680: 31 00 fa 01 26

2. Wert ändern

./can_scan can0 680 180.0126.0f00 trace

setzt das folgende Telegramm ab:
  
 680: 32 00 fa 01 26 0f 00


Bemerkung: cansend ist ein Programm aus dem Internet.
 
_______________________________________________________________

Stichwörter: 
  Raspberry Pi CAN-BUS 
  Stiebel-Eltron Elster-Kromschröder Wärmepumpe WPL33 WPMII Wärmepumpen Manager WPM2
  mysql logging

_______________________________________________________________

Erweiterungen vom 20. Sept. 2014

./can_scan can0 680 total

listet alle gültigen CAN-IDs auf. Danach wird eine Liste aller verfügbaren Werte aufgelistet. Diese Liste kann in CS_Bruecke eingebunden werden. Die WP-Anlage kann dann mit CS_Bruecke für die ComfortSoft simuliert werden.

------

Werte, wie für das Tagestotal der Aufnahmeenergie, werden zusammengesetzt. Ausschnitt aus der Elster-Tabelle:   

  { "EL_AUFNAHMELEISTUNG_WW_TAG_WH"                    , 0x091a, et_double_val},
  { "EL_AUFNAHMELEISTUNG_WW_TAG_KWH"                   , 0x091b, 0},

./can_scan can0 680 500.091a

value: 0220  (EL_AUFNAHMELEISTUNG_WW_TAG_KWH  0.544)

oder 

./can_scan can0 680 500.091c

liefert

value: 0069  (EL_AUFNAHMELEISTUNG_WW_SUM_MWH  3.105)
 
_______________________________________________________________

Erweiterung vom 18. Februar 2015

Parameter cs_com: Damit kann das optische Interface verwendet werden. Geprüft wurde es allerdings erst mit "can_simul". 

_______________________________________________________________

Feststellung: Das Resultat eines scans ist von der CAN-Id abhängig:

./can_scan can0 680 480.025c

liefert

value: 8000  (VORLAUFSOLL_GEBLAESE  not available)

während 

./can_scan can0 780 480.025c

value: 0096  (VORLAUFSOLL_GEBLAESE  15.0)

liefert.

27. Juli 2015

_______________________________________________________________