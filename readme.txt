    
Allgemeine Hinweise

- Das Programm can_logger wird beim Start mit "Speicherzugriffsfehler" abgebrochen:
  Dann passt vermutlich Deine C++-Umgebung nicht mit meiner überein.
  Abhilfe schafft eine Neukompilierung. Dazu braucht es den g++-Kompiler. Also 
  prüfen, ob der Befehl

  g++

  gefunden wird. Wenn nein, installieren:

  sudo apt-get update
  sudo apt-get install g++

  oder

  sudo apt-get update
  sudo apt-get install build-essential
  
  Danach das Programm neu übersetzen. In das Verzeichnis "can_progs" wechseln und 
  das Skript mit ".arm" ausführen.

  cd can_progs
  ./can_logger.arm

  (In Linux ist "./" notwendig. Es steht für "dieses Verzeichnis"; "can_logger.arm" 
  alleine genügt nicht!)

  Mit
 
  ./make_all.arm

  werden alle Programme neu generiert.


- mysql kann nicht verwendet werden:
  Am besten alle notwendigen Dateien installieren.

  sudo apt-get update
  sudo apt-get install libmysqlclient-dev
  sudo apt-get install mysql-client 
 

- Das Programm CS_Bruecke.exe verlangt beim Start die beiden Bibliotheken 
  "vcl60.bpl" und "rtl60.bpl". Im Verzeichnis "win/release" gibt es aber 
  eine vollständige "CS_Bruecke.exe". Im Verzeichnis mingw gibt es das Programm 
  cs_bruecke_dos.exe. Damit können simulierte Daten (vom Typ "scan_data.inc")
  mit der ComfortSoft analysiert werden. Das dient zur genauen Bestimmung von
  Elster-Index und zugehöriger CAN-ID.

- CAN Id (von robots, haustechnikdialog):

  000 - direkt
  180 - Kessel
  280 - atez
  300, 301 ... - Bedienmodule (bei mir 301, 302 und 303)
  400 - Raumfernfühler
  480 - Manager
  500 - Heizmodul
  580 - Buskoppler
  600, 601 ... -  Mischermodule (bei mir 601, 602, 603)
  680 - PC (ComfortSoft)
  700 - Fremdgerät
  780 - DCF-Modul

- ModulType auf der Basis des ComfortSoft-Protokolls, 2. Byte (siehe robots, haustechnikdialog):

  0 - write
  1 - read
  2 - response
  3 - ack
  4 - write ack
  5 - write respond
  6 - system
  7 - system respond
  20/21 (hex.) - write/read large telegram

- CAN-Bus-Telegramme beim Einschalten der Steuerung (von chriss1980, knx-user-forum):

  0480  7  96 00 FD 09 00 00 00 
  0180  7  36 00 FD 09 00 00 00 
  0601  7  C6 01 FD 09 00 00 00 
  0301  7  66 01 FD 08 00 00 00 
  0680  7  D6 00 FD 08 00 00 00 
  0602  7  C6 02 FD 09 00 00 00 
  0302  7  66 02 FD 08 00 00 00

  danach alle 7 Minuten diese Telegramme:

  0602  7  66 02 FE 01 00 00 00 
  0180  7  66 79 FE 01 00 00 00 
  0480  7  A6 79 FE 01 00 00 00

  can-Bus: 20kBit/s, 11Bit-IDs

- ComfortSoft-Telegramme beim Einschalten (von st0ne, knx-user-forum):

  401 7 86 01 FD 01 00 00 00
  69E 7 D6 1E FD 01 00 00 00  ... Vom Bedienteil

--------------------------------------------------------------------------------------

Software-Versionen WPM (die dezimale Versionsangabe am Display entspricht der hexadezimalen Zahl im "Little-Endian"-Format):

- 7077/165.27 (0xa51b): meine WP
- 17665/325.01 (0x0145): WPMiw, xanatos, haustechnikdialog
- 17667/325.03 (0x0345): WPMiw, chriss1980, knx-user-forum 
- 9011/51.35 (0x3323): WPMI (tecalor TTF16), Gelbaerchen, knx-user-forum
- 6502/102.25 (0x6619): winki, knx-user-forum
- 6526/126.25 (0x7e19): WPMII, berti, knx-user-forum
- 1470/190.05 (0xbe05): WPMi, robots, haustechnikdialog
- 2204/156.8 (0x9c08): olima, ip-symcon
- 5541/165.21 (0xa515): WPMII, hajo23
- nicht vorhanden (0x8000): WPM 3, heckmannju, 
- nicht vorhanden (0x8000):  WPMme, radiator (die Versionsnummer steht beim Elster-Index 0x0199 und 0x019a) 


Bemerkung: 

Die erste Anfrage (der ComfortSoft) via optischer Schnittstelle benötigt die oben genannte Version. Sie läuft nicht über den CAN-Bus. Der WP-Manager gibt die Antwort direkt. Nach dieser Abfrage stellt die ComfortSoft eine Auswahl "Max. Buskennung im System" bereit.  

Erste Abfrage der ComfortSoft an der seriellen Schnittstelle ist immer wie folgt:

  0d 00 0d 01 00 0b 00 00 00 00 00 26 (bedeutet Anfrage zu Elster-Index 0x000b an Gerät 680)

Antwort

  55 55 55 55 55 55 55 55 a5 1b 03 68  (0xa51b entspricht meiner Version)


Voraussetzung ISG (Stiebel Doku):

WPMiW mindestens 2508 (204.9/0xcc09) 
wenn ein FEK installiert ist, mindestens 9506 (37.34/0x2225)

WPM II mindestens 6529 oder 8447 (je nach WP) 
wenn ein FEK installiert ist, mindestens 9506 (37.34/0x2225)

Meine WPM II hat die Version 6527.

---------------------------------------------------------------------------------------

Nicht gelöste Probleme:

- Reset für WP: siehe chriss1980, http://knx-user-forum.de/code-schnipsel/25862-anbindung-tecalor-ttw13-7.html/page5

  Botschaft 3000FA00FBxxxx (mit opt. Interface 0d00090000fb800000000191 und 
  Antwort 555555555555...0352)

  Ich habe die Vermutung, dass ältere WPMs das nicht über den CAN-Bus handeln können, 
  sondern nur mit dem optischen Interface. Deshalb braucht es für das ISG eine neuere 
  Software-Version für den Manager. 


- heckmannju hat Adresse 100: http://knx-user-forum.de/code-schnipsel/25862-anbindung-tecalor-ttw13-8.html
  ebenfalls Adresse 700
  Sind die Adressen 100 und 700 für des FEK? Nein, das FEK hat die Adresse 301. Es belegt für Abfragen eventuell die Adresse 700.

---------------------------------------------------------------------------------------

24. Juli 2015: Umstrukturierung der Software und Vorbereitung zur Verbesserung von can_server  

---------------------------------------------------------------------------------------

9. Juli 2016: Software für das optische Original-Interface angepasst. DTR muss auf false (-12V) stehen. 

---------------------------------------------------------------------------------------
  
25. Januar 2017:

- Projekt CS_Bruecke.cbproj an RAD Studio 10.1 Berlin angepasst, das gratis zur persönlichen Nutzung heruntergeladen werden kann.

- Die gesamte Software kann auch mit einem USB2CAN von 8devices.com unter Windows verwendet werden.

