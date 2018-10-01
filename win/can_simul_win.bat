del *.obj
del can_simul.exe

bcc32 -D__WINDOWS__ -D__CONSOLE__ -D__CAN__ -D__SIMULATION__ ^
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
  ../KElsterTable.cpp ^
  ../KCanElster.cpp ^
  ../special/Kusb2can.cpp ^
  ../KSniffedFrame.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KTcpClient.cpp ^
  ../KScanTable.cpp

del *.obj
del *.tds
ren kipsocket.exe can_simul.exe

