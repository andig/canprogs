rem usbtin_simul COM3 ..\scan_data.inc ..\mac\can_log_20140724.log trace

del *.obj
del usbtin_simul.exe

bcc32 -D__WINDOWS__ -D__CONSOLE__ -D__CAN__ -D__USBTIN_SIMULATION__ -D__USBTIN__ ^
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
  ../special/Kusb2can.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KTcpClient.cpp ^
  ../KSniffedFrame.cpp

del *.obj
del *.tds
ren kipsocket.exe usbtin_simul.exe

