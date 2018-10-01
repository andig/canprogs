del *.obj
del cs_bruecke_dos.exe

bcc32 -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__DEBUG__ -D__NO_VCL__ ^
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
  ../KStream.cpp ^
  ../NUtils.cpp ^
  ../NCanUtils.cpp ^
  ../KCanElster.cpp ^
  ../KElsterTable.cpp ^
  ../UBruecke.cpp ^
  ../special/Kusb2can.cpp ^
  ../KScanTable.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KSniffedFrame.cpp

del *.obj
del *.tds
ren KIpSocket.exe cs_bruecke_dos.exe