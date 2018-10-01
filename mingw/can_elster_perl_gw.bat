
set CORE_PATH=C:\strawberry-perl-5
set MinGW=%CORE_PATH%\c

set PATH=%MinGW%\bin

copy ..\elster_perl.h 
copy ..\elster_perl.i 

del elster_perl.dll

c:/swigwin-3.0.12/swig -perl5 -module elster_perl -c++ elster_perl.i

%MinGW%\bin\g++ -shared -Wall -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__DSWIG__ ^
  -I%CORE_PATH%\perl\lib\CORE -I.. -L%CORE_PATH%\perl\lib\CORE ^
  -L%MinGW%\lib -L%MinGW%\bin -L%MinGW%\i686-w64-mingw32/lib ^
  ../elster_perl.cpp ^
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
  ../special/Kusb2can.cpp ^
  ../KSniffedFrame.cpp ^
  ../KCanTcpDriver.cpp ^
  ../KScanTable.cpp -lperl520 -lpthread -lws2_32 ^
  -oelster_perl.dll

copy elster_perl.dll %CORE_PATH%\perl\vendor\lib
copy elster_perl.pm %CORE_PATH%\perl\vendor\lib
