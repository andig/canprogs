rem ================ ist fehlehaft!

del *.obj
del CS_Bruecke.exe

bcc32 -D__WINDOWS__ -D__CONSOLE__ -DVLC -D__CAN__ -DUSE_KCOMM -I.. -I. -LC:\Programme\Borland\CBuilder6\Lib\Release -LC:\Programme\Borland\CBuilder6\Lib ^
  vcl.lib ^
  rtl.lib ^
  vclx.lib ^
  bcbsmp.lib ^
  dclocx.lib ^
  obj/Console.obj ^
  obj/Comm_Dlg.obj ^
  obj/Comm.obj ^
  ../KIpSocket.cpp  ^
  ../KThread.cpp ^
  ../KCriticalSection.cpp ^
  ../KCanDriver.cpp ^
  ../KComm.cpp ^
  ../KRPiCanDriver.cpp ^
  ../KCanServer.cpp ^
  ../KStream.cpp ^
  ../NUtils.cpp ^
  ../NCanUtils.cpp ^
  ../KElsterTable.cpp ^
  ../KCanElster.cpp  ^
  ../KScanTable.cpp ^
  ../KTcpCanClient.cpp ^
  ../KTcpClient.cpp ^
  ../KHttpClient.cpp ^
  ../UBruecke.cpp ^
  ../KCanCommDriver.cpp ^
  ../special/Kusb2can.cpp ^
  UCS_Dlg.cpp ^
  UMain.cpp
  

del *.obj
del *.tds
ren console.exe CS_Bruecke.exe


