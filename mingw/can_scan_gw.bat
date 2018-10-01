set MinGW=C:\MinGW

%MinGW%\bin\g++ %1 %2 -Wall -D__WINDOWS__ -D__CONSOLE__ -D__CAN__ -D__SCAN__ ^
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
  ../KCanTcpDriver.cpp ^
  ../KTcpClient.cpp ^
  ../KStream.cpp ^
  ../NUtils.cpp ^
  ../NCanUtils.cpp ^
  ../KElsterTable.cpp ^
  ../KCanElster.cpp ^
  ../KScanTable.cpp ^
  ../special/Kusb2can.cpp ^
  ../KSniffedFrame.cpp ^
  -o can_scan -lws2_32