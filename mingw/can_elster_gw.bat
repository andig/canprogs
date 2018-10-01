
set CORE_PATH=C:\strawberry-perl-5
set MinGW=%CORE_PATH%\c

set PATH=%MinGW%\bin

set Python=C:\Python34
set mysql_flag=

%MinGW%\bin\g++ -shared -Wall -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__PYTHON__ ^
  -I%MinGW%\include ^
  -L%MinGW%\lib ^
  -I%Python%/include ^
  -L%Python%/libs ^
  ../KIpSocket.cpp ^
  ../KThread.cpp ^
  ../KCriticalSection.cpp ^
  ../KCanDriver.cpp ^
  ../KComm.cpp ^
  ../KCanCommDriver.cpp ^
  ../KRPiCanDriver.cpp ^
  ../KCanServer.cpp ^
  ../KStream.cpp ^
  ../NUtils.cpp ^
  ../NCanUtils.cpp ^
  ../KCanElster.cpp ^
  ../KElsterTable.cpp ^
  ../KScanTable.cpp ^
  ../elster_instance.cpp ^
  ../elster_py.cpp ^
  ../special/Kusb2can.cpp ^
  ../KSniffedFrame.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KTcpClient.cpp ^
  -o elster.pyd -Wl,--subsystem,windows,--out-implib,libelster.a -lpython34 -lws2_32

set PATH=C:\MinGW\bin
