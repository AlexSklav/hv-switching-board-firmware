@echo off
REM Store the version of the firmware in a variable.
REM See https://stackoverflow.com/a/2340018/345236
for /f %%i in ('python version.py') do set VERSION=%%i

REM Use define build flag to set firmware software version during build.
set FLAGS="-D___SOFTWARE_VERSION___=\"%VERSION%\""
echo %FLAGS%
