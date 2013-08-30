@echo OFF
call "%VS90COMNTOOLS%\vsvars32.bat"

devenv WiretapViewer-VS08.sln /Rebuild Debug
if "%errorlevel%"=="1" goto failure

devenv WiretapViewer-VS08.sln /Rebuild Release
if "%errorlevel%"=="1" goto failure

echo All good
goto end

:failure
echo Failure
pause

:end