del *.obj
del can_server.exe
del can_client.exe

bcc32 %1 %2 -D__WINDOWS__ -D__CONSOLE__ -D__CAN__ -D__CAN_SERVER__ ^
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
  ../KCanElster.cpp ^
  ../KTcpClient.cpp ^
  ../KTcpCanClient.cpp ^
  ../KTcpServer.cpp ^
  ../KTcpElsterServer.cpp ^
  ../KScanTable.cpp ^
  ../KSniffedFrame.cpp ^
  ../KCanTcpDriver.cpp ^
  ../special/Kusb2can.cpp 
  
del *.obj
ren kipsocket.exe can_server.exe

bcc32 %1 %2 -D__WINDOWS__ -D__CONSOLE__ -D__CAN__ -D__CAN_CLIENT__ ^
  ../KIpSocket.cpp  ^
  ../KStream.cpp ^
  ../NUtils.cpp ^
  ../NCanUtils.cpp ^
  ../KElsterTable.cpp ^
  ../KCanElster.cpp ^
  ../KTcpClient.cpp ^
  ../KTcpCanClient.cpp ^
  ../KTcpElsterServer.cpp ^
  ../special/Kusb2can.cpp  


del *.obj
del *.tds
ren kipsocket.exe can_client.exe


