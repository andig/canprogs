set MinGW=C:\MinGW
set mysql_flag=

set Path=%Path%;%MinGW%\bin

%MinGW%\bin\g++ -Wall -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__VZ__ %mysql_flag%  ^
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
  ../KStream.cpp ^
  ../NUtils.cpp ^
  ../NCanUtils.cpp ^
  ../XmlParser.cpp ^
  ../KXmlNodeList.cpp ^
  ../KMySql.cpp ^
  ../KElsterTable.cpp ^
  ../KTcpCanClient.cpp ^
  ../KTcpClient.cpp ^
  ../KHttpClient.cpp ^
  ../KScanTable.cpp ^
  ../KCanElster.cpp ^
  ../special/Kusb2can.cpp ^
  ../KSniffedFrame.cpp ^
 -o can_vz -lws2_32

