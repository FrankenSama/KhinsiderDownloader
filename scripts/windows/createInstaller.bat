@echo off
set "APP_VERSION="
for /f "usebackq delims=" %%v in ("..\..\APP_VERSION") do set "APP_VERSION=%%v"
for /f "tokens=1 delims=-" %%v in ("%APP_VERSION%") do set "APP_VERSION=%%v"
if not defined APP_VERSION set "APP_VERSION=3.0"
ISCC.exe /DMyAppVersion=%APP_VERSION% installerinfo.iss
