rem  #include <crtdbg.h>   ersetzen durch:
rem
rem  #ifdef _MSC_VER
rem    #include <crtdbg.h>
rem  #else
rem    #define _ASSERT(expr) ((void)0)
rem
rem    #define _ASSERTE(expr) ((void)0)
rem  #endif

set MINGW_PATH=C:\mingw
set PHP_PATH=D:\php-5.6.30-src
set Path=%MinGW_Path%\bin

copy ..\elster_php.h 
copy ..\elster_php.i 

del elster_php.dll

c:/swigwin-3.0.12/swig -php -module elster_php -c++ elster_php.i

%MINGW_PATH%\bin\g++ -shared -D__WINDOWS__ -D__CAN__ -D__CONSOLE__ -D__DSWIG__ -DZEND_WIN32 -w -D__cplusplus -DZEND_DEBUG ^
  -I%MINGW_PATH%\include -I.. -L%MINGW_PATH%\lib ^
  -I%PHP_PATH% -I%PHP_PATH%\main -I%PHP_PATH%\TSRM -I%PHP_PATH%\Zend -L%MINGW_PATH%\bin ^
  ../elster_php.cpp ^
  elster_php_wrap.cpp ^
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
  ../KScanTable.cpp -lpthread -lws2_32 ^
  -oelster_php.dll

