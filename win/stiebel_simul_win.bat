
del *.obj
del stiebel_simul.exe

bcc32 -D__WINDOWS__ -D__CONSOLE__ -D__CAN__ -D__STIEBEL_SIMULATION__ -D__USBTIN__ ^
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
  ../KCanLogFile.cpp ^
  ../KScanTable.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KTcpClient.cpp ^
  ../KSniffedFrame.cpp ^
  ../special/Kusb2can.cpp

del *.obj
del *.tds
ren kipsocket.exe stiebel_simul.exe

