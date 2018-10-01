set CORE_PATH=C:\strawberry

#copy ..\elster_perl.cpp 
#copy ..\elster_perl.h 
#copy ..\elster_perl.i 
#copy ..\elster_perl_wrap.cxx
#copy ..\elster_perl.pm

%CORE_PATH%\perl\bin\swig -perl5 -module elster_perl -c++ elster_perl.i

bcc32 -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__DSWIG__ ^
  -I%CORE_PATH%\perl\lib\CORE -I.. -L%CORE_PATH%\perl\lib\CORE ^
  elster_perl.cpp ^
  elster_perl_wrap.cxx ^
  ../elster_instance.cpp ^
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
  ../KCanElster.cpp ^
  ../KElsterTable.cpp ^
  ../KTcpCanClient.cpp ^
  ../KTcpClient.cpp ^
  ../KTcpServer.cpp ^
  ../KTcpElsterServer.cpp ^
  ../KScanTable.cpp ^
  ../special/Kusb2can.cpp ^
  ../KSniffedFrame.cpp ^
  -n.\obj -o.\elster_perl.dll -lws2_32 -lperl520

del *.obj
