del *.obj
del can_vz.exe

bcc32 -D__WINDOWS__ -D__CONSOLE__ -D__CAN__ -D__VZ__ ^
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
  ../KElsterTable.cpp ^
  ../KXmlNodeList.cpp ^
  ../XmlParser.cpp ^
  ../KTcpClient.cpp ^
  ../KHttpClient.cpp ^
  ../KScanTable.cpp ^
  ../KCanElster.cpp ^
  ../KSniffedFrame.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KTcpClient.cpp ^
  ../special/Kusb2can.cpp  
  

del *.obj
del *.tds
ren kipsocket.exe can_vz.exe


