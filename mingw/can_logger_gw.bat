
set MinGW=C:\MinGW
set mysql_flag=

%MinGW%\bin\g++ %1 %2 -Wall -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ %mysql_flag% ^
  -I%MinGW%\include ^
  -L%MinGW%\lib ^
  ../CanLogger.cpp ^
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
  ../KTcpClient.cpp ^
  ../KHttpClient.cpp ^
  ../special/Kusb2can.cpp ^
  -o can_logger -lws2_32

