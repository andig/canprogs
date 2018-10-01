del *.obj
del can_logger.exe

bcc32 %1 %2 -D__WINDOWS__ -D__CONSOLE__ -D__CAN__ ^
  ../CanLogger.cpp ^
  ../KIpSocket.cpp  ^
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
  ../XmlParser.cpp ^
  ../KXmlNodeList.cpp ^
  ../KTcpClient.cpp ^
  ../KElsterTable.cpp ^
  ../KHttpClient.cpp ^
  ../KScanTable.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KTcpClient.cpp ^
  ../special/Kusb2can.cpp

del *.obj
del *.tds
ren canlogger.exe can_logger.exe