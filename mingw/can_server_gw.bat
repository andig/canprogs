set MinGW=C:\MinGW
set mysql_flag=

%MinGW%\bin\g++ %1 %2 -Wall -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__CAN_SERVER__ ^
  -I%MinGW%\include ^
  -L%MinGW%\lib ^
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
  ../KTcpCanClient.cpp ^
  ../KTcpServer.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KTcpClient.cpp ^
  ../KTcpElsterServer.cpp ^
  ../KScanTable.cpp ^
  ../special/Kusb2can.cpp ^
  ../KSniffedFrame.cpp ^
  -o can_server -lws2_32

%MinGW%\bin\g++ %1 %2 -Wall -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__CAN_CLIENT__ ^
  -I%MinGW%\include ^
  -L%MinGW%\lib ^
  ../KIpSocket.cpp ^
  ../KStream.cpp ^
  ../NUtils.cpp ^
  ../NCanUtils.cpp ^
  ../KElsterTable.cpp ^
  ../KCanDriver.cpp ^
  ../KTcpClient.cpp ^
  ../KTcpCanClient.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KTcpElsterServer.cpp ^
  ../special/Kusb2can.cpp ^
  -o can_client -lws2_32


