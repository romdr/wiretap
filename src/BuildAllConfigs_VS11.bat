@echo OFF
call "%VS110COMNTOOLS%\vsvars32.bat"

devenv WiretapViewer-VS11.sln /Rebuild Debug
if "%errorlevel%"=="1" goto failure

devenv WiretapViewer-VS11.sln /Rebuild Release
if "%errorlevel%"=="1" goto failure

devenv WiretapProfiler-VS11.sln /Rebuild Debug
if "%errorlevel%"=="1" goto failure

devenv WiretapProfiler-VS11.sln /Rebuild Release
if "%errorlevel%"=="1" goto failure

echo All good
goto end

:failure
echo Failure
pause

:end