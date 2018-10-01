set MinGW=C:\MinGW
set mysql_flag=

%MinGW%\bin\g++ %1 %2 -Wall -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__CGI__ ^
  -I%MinGW%\include ^
  -L%MinGW%\lib ^
  ../KIpSocket.cpp ^
  ../KThread.cpp ^
  ../KCriticalSection.cpp ^
  ../KCanDriver.cpp ^
  ../KComm.cpp ^
  ../KCanCommDriver.cpp ^
  ../KRPiCanDriver.cpp ^
  ../KStream.cpp ^
  ../NUtils.cpp ^
  ../NCanUtils.cpp ^
  ../KCanServer.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KTcpClient.cpp ^
  ../KElsterTable.cpp ^
  ../KScanTable.cpp ^
  ../KCanElster.cpp ^
  ../NCgi.cpp ^
  ../special/Kusb2can.cpp ^
  ../KSniffedFrame.cpp ^
  -o can_cgi -lws2_32