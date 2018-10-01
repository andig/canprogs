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

Das beiliegende Programm can_logger ist auf einem Raspberry Pi lauffähig. Es kann mit 

  ./can_logger.arm

erzeugt werden. 

Mit dem Aufruf

  ./can_logger

sehen Sie:

can-bus data logger
copyright (c) 2014 Jürg Müller, CH-5524

usage:
  can_logger <can dev> <sample time in sec> [ (<filename> | <mysql host>) [ csv | mysql ] ]
or
  can_logger <file.xml>

example: ./can_logger can0 10 192.168.1.137 mysql
or       ./can_logger can1 3600 canlog csv

can dev: can0
sample time: 10 sec

1. Beispiel (oben): mysql-host ist 192.168.1.137; ausgelesen wird im 10-Sekunden-Takt

2. Beispiel: Die Daten werden in der Datei canlog_DATUM abgelegt; ausgelesen wird im Stunden-Takt; zu jedem Tag gibt es eine Datei

csv (comma separated values): Die Werte werden durch ein Komma getrennt. (Wird z.B. von Excel verwendet.) 

mysql: can_logger schreibt in die MySQL-Tabelle "can_log" (vgl. create.sql). 

_______________________________________________________________

Zweck des Programms can_logger:

a) can-Datenfluss in einer Bash zu betrachten. Z.B. die Antwort zu einer "cansend" anzusehen.

b) can-Protokolle in einer Datei zu hinterlegen. Da der Raspberry Pi mit einer Flash-Karte arbeitet, sollte nicht allzu oft gespeichert werden (Flash hat nur 100k oder 200k Schreibzyklen), deshalb kann man eine "sample time" von 10 Minuten und mehr wählen.

c) can-Protokolle in einer entfernten MySQL-Datenbank speichern. Siehe dazu auch die Änderungen/Erweiterung vom 8.9.2014.
 
_______________________________________________________________

Änderungen:

- 8. 9. 2014: siehe unten

- 12. 9. 2014: <sample time in sec> ist nun der 2. Parameter (vorher war es der dritte).

_______________________________________________________________


can_logger arbeitet mit 2 Threads (unabhängige "Ausführungs-Objekte"). Der eine Thread liest die Can-Bus-Frames und speichert sie in einem Ring-Puffer (mit 10240 Frames) ab. Der andere, der "Main-Thread", entleert den Ring-Puffer. Die Zeit zwischen den Entleerungen kann vorgegeben werden (<sample time in sec>). Sie wird nicht abgewartet, wenn der Ring-Puffer zu 3/4 voll ist, dann wird sofort ausgelesen. Die Daten können wahlweise in einer lokalen Datei oder in einer MySQL-Datenbank gespeichert werden. 

Das Programm ist sehr performant! 
Ausgetestet mit MCP2515 Can-Bus Demo Board von Microchip als Sender und dem Raspberry Pi als Empfänger: 
  60 Can-Frames pro Sekunde bei einer Can-Bus-Bitrate von 125000 bit/s, 
  "sample time" von 10 Sekunden und 
  einer MySQL-Datenbank im Netz (Parameter "mysql")

_______________________________________________________________

Voraussetzungen für die Generierung und Verwendung von can_logger:

- Header: can.h, raw.h und mysql.h

- library: libmysqlclient (wird dynamisch geladen)

- geladene Module (vgl. candump): spi-bcm2708.ko, can.ko, can-dev.ko, can-raw.ko und mcp251x.ko 

________________________________________________________________

Beispiel zur xml-File-Struktur (ist nicht fertig entwickelt): 

<?xml version="1.0" encoding="UTF-8" ?>
<can>
  <device     Name="can0" />
  <mysql      User="pi" 
              Password="raspberry" 
              DB="log" 
              Host="192.168.1.115" />
  <sleep      Name="54" /> 
</can>

<?xml version="1.0" encoding="UTF-8" ?>
<can>
  <device     Name="can0" />
  <log_file   Name="logfile" />
  <sleep      Name="54" /> 
</can>

_______________________________________________________________

Stichwörter: 
  Raspberry Pi CAN-BUS 
  Stiebel-Eltron Elster-Kromschröder Wärmepumpe WPL33 WPMII Wärmepumpen Manager WPM2
  mysql logging

_______________________________________________________________

Änderungen vom 8.9.2014:

- Die MySQL-Tabelle log heisst neu "can_log" und die Struktur hat ebenfalls geändert (vgl. create.sql). (timestamp anstelle des Paars <date_stamp, ms_stamp>; flag wird nicht mehr mitgeführt)

- Ein Logging der gesamten anfallenden Can-Telegramme auf einer angemieteten Homepage mit MySQL-DB. Ich fand nicht heraus, wie ich direkt mittels einer MySQL-Client-Verbindung auf die DB zugreifen kann. Deshalb entschied ich mich, die Daten mit einem serverseitigen php-Skript in die DB zu schreiben.
  
  "can_logger" bereitet in diesem Fall einen Stream auf, in dem alle zu speichernden Telegramme in MySQL-"INSERTs" gepackt werden. Das Skript put_can_log.php nimmt den Stream auf dem HTTP-Server entgegen und speichert die "INSERTs" in der DB.

_______________________________________________________________

Vorgehen, um alle Telegramme in der MySQL-DB zu hinterlegen:

- konfiguration.php mit den richtigen Daten versehen. PASS und PASS_VALUE mit beliebigem Zugangscode versehen (keine HTTP-Spezialzeichen verwenden). 

- konfiguration.php und put_can_log.php auf den HTTP-Server kopieren.

- MySQL-Tabelle can_log auf dem Server generieren (vgl. create.sql).

- can_log.xml anpassen (wird auf dem Raspi verwendet). 

- zum Ausprobieren auf dem Raspi 

    $./can_logger can_log.xml trace 

  verwenden. Wenn das soweit i.O. ist: "trace" weglassen. Es wird dann nach jedem Übertragungsintervall die letzte CAN-Protokoll-Zeile ausgegeben. Die Zeile mit "result" wird vom php-Skript generiert.

    215  8.9.2014 14:22:25.340  14611234 00 [4] 00 01 02 03              ....
    result: Ok: 216   not Ok: 0

_______________________________________________________________

Erweiterung vom 22. Sept. 2014:

Der CAN232 von Lawicel kann verwendet werden. Der Aufruf auf dem MAC ist z.B.

  mac/can_logger tty.usbserial-FTK1S17H 0

Hinweis: das Device findet man im Verzeichnis /dev (ls /dev/tty*) 

Für Windows:

  win/can_logger COM3 0

_______________________________________________________________

Erweiterung vom 15. Nov. 2014:

Control-C wird abgefangen. Danach wird das Programm ordnungsgemäss beendet.



