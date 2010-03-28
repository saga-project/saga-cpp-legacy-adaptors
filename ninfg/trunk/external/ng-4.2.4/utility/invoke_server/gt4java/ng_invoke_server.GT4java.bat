@echo off

REM
REM batch file for invoking Invoke Server GT4java
REM %JAVA_HOME%, %COG_INSTALL_PATH%, %NG_DIR% is required.
REM
set JAVA_HOME=\path\to\java_home
set COG_INSTALL_PATH=\path\to\cog
set NG_DIR=\path\to\ninfg

REM
REM setup options for Invoke Server
REM
set IS_OPTIONS=

:startSetupArgs
if %1a==a goto endSetupArgs
set IS_OPTIONS=%IS_OPTIONS% %1
shift
goto startSetupArgs
:endSetupArgs

REM
REM setup CLASSPATH for Invoke Server
REM
set LOCALCLASSPATH=
for %%i in ("%COG_INSTALL_PATH%\lib\*.jar") do call "%NG_DIR%\bin\nglcp.bat" %%i
for %%i in ("%COG_INSTALL_PATH%\lib\gt4_0_0\*.jar") do call "%NG_DIR%\bin\nglcp.bat" %%i
set LOCALCLASSPATH=%NG_DIR%\lib\ngisgt4.jar;%LOCALCLASSPATH%

REM
REM launch Invoke Server
REM
%JAVA_HOME%\bin\java.exe -Djava.endorsed.dirs=%COG_INSTALL_PATH%\lib\endorsed -cp "%LOCALCLASSPATH%" org.apgrid.grpc.tools.invokeServer.gt4.NgInvokeServerGT4 %IS_OPTIONS%
