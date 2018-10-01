set MinGW=C:\MinGW

%MinGW%\bin\g++ -Wall -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__NO_VCL__ -D__DEBUG__ -m32 ^
  -I%MinGW%\include ^
  -L%MinGW%\lib ^
  ../KIpSocket.cpp ^
  ../KThread.cpp ^
  ../KCriticalSection.cpp ^
  ../KTcpClient.cpp ^
  ../KTcpCanClient.cpp ^
  ../KCanDriver.cpp ^
  ../KComm.cpp ^
  ../KCanCommDriver.cpp ^
  ../KRPiCanDriver.cpp ^
  ../KCanServer.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KStream.cpp ^
  ../NUtils.cpp ^
  ../NCanUtils.cpp ^
  ../KCanElster.cpp ^
  ../KElsterTable.cpp ^
  ../UBruecke.cpp ^
  ../KScanTable.cpp ^
  ../special/Kusb2can.cpp ^
  ../KSniffedFrame.cpp ^
  -o cs_bruecke_dos -lws2_32