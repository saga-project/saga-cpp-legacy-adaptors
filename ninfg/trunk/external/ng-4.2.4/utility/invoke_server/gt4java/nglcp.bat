@echo off
REM
REM Utility program of Invoke Server
REM this program append received element into LOCALCLASSPATH
REM
set _CLASSPATHCOMPONENT=%1
set LOCALCLASSPATH=%_CLASSPATHCOMPONENT%;%LOCALCLASSPATH%
