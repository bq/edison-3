@echo off

echo #########################################################################
echo WARNING: this script has been generated to support Visual Studio version 11.0
echo #########################################################################
ping -n 3 127.0.0.1 >nul 

set COMP_PATH_ROOT=%~dp0\..
set COMP_PATH_MobiCoreDriverLib=%COMP_PATH_ROOT%\t-sdk\TlcSdk
set COMP_PATH_TlSdk=%COMP_PATH_ROOT%\t-sdk\TlSdk
set COMP_PATH_Logwrapper=%COMP_PATH_ROOT%\t-sdk\TlcSdk\Logwrapper
set COMP_PATH_TlAes=%COMP_PATH_ROOT%\Samples\Aes\TlAes\Out
set COMP_PATH_TlRsa=%COMP_PATH_ROOT%\Samples\Rsa\TlRsa\Out
set COMP_PATH_TlSampleRot13=%COMP_PATH_ROOT%\Samples\Rot13\TlSampleRot13\Out
set COMP_PATH_TlSampleSha256=%COMP_PATH_ROOT%\Samples\Sha256\TlSampleSha256\Out

call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"

echo #########################################################################

msbuild "Aes\TlcAes\Locals\Code\Windows\TlcSampleAes.vcxproj" ^
/t:rebuild /p:Configuration=Release /p:Platform=Win32
@if ERRORLEVEL 1 goto error

msbuild "Aes\TlcAes\Locals\Code\Windows\TlcSampleAes.vcxproj" ^
/t:rebuild /p:Configuration=Debug /p:Platform=Win32
@if ERRORLEVEL 1 goto error

msbuild "Aes\TlcAes\Locals\Code\Windows\TlcSampleAes.vcxproj" ^
/t:rebuild /p:Configuration=Release /p:Platform=x64
@if ERRORLEVEL 1 goto error

msbuild "Aes\TlcAes\Locals\Code\Windows\TlcSampleAes.vcxproj" ^
/t:rebuild /p:Configuration=Debug /p:Platform=x64
@if ERRORLEVEL 1 goto error

echo #########################################################################

msbuild "Rot13\TlcSampleRot13\Locals\Code\Windows\TlcSampleRot13.vcxproj" ^
/t:rebuild /p:Configuration=Release /p:Platform=Win32
@if ERRORLEVEL 1 goto error

msbuild "Rot13\TlcSampleRot13\Locals\Code\Windows\TlcSampleRot13.vcxproj" ^
/t:rebuild /p:Configuration=Debug /p:Platform=Win32
@if ERRORLEVEL 1 goto error

msbuild "Rot13\TlcSampleRot13\Locals\Code\Windows\TlcSampleRot13.vcxproj" ^
/t:rebuild /p:Configuration=Release /p:Platform=x64
@if ERRORLEVEL 1 goto error

msbuild "Rot13\TlcSampleRot13\Locals\Code\Windows\TlcSampleRot13.vcxproj" ^
/t:rebuild /p:Configuration=Debug /p:Platform=x64
@if ERRORLEVEL 1 goto error

echo #########################################################################

msbuild "Sha256\TlcSampleSha256\Locals\Code\Windows\TlcSampleSha256.vcxproj" ^
/t:rebuild /p:Configuration=Release /p:Platform=Win32
@if ERRORLEVEL 1 goto error

msbuild "Sha256\TlcSampleSha256\Locals\Code\Windows\TlcSampleSha256.vcxproj" ^
/t:rebuild /p:Configuration=Debug /p:Platform=Win32
@if ERRORLEVEL 1 goto error

msbuild "Sha256\TlcSampleSha256\Locals\Code\Windows\TlcSampleSha256.vcxproj" ^
/t:rebuild /p:Configuration=Release /p:Platform=x64
@if ERRORLEVEL 1 goto error

msbuild "Sha256\TlcSampleSha256\Locals\Code\Windows\TlcSampleSha256.vcxproj" ^
/t:rebuild /p:Configuration=Debug /p:Platform=x64
@if ERRORLEVEL 1 goto error

echo #########################################################################

msbuild "Rsa\TlcRsa\Locals\Code\Windows\TlcSampleRsa.vcxproj" ^
/t:rebuild /p:Configuration=Release /p:Platform=Win32
@if ERRORLEVEL 1 goto error

msbuild "Rsa\TlcRsa\Locals\Code\Windows\TlcSampleRsa.vcxproj" ^
/t:rebuild /p:Configuration=Debug /p:Platform=Win32
@if ERRORLEVEL 1 goto error

msbuild "Rsa\TlcRsa\Locals\Code\Windows\TlcSampleRsa.vcxproj" ^
/t:rebuild /p:Configuration=Release /p:Platform=x64
@if ERRORLEVEL 1 goto error

msbuild "Rsa\TlcRsa\Locals\Code\Windows\TlcSampleRsa.vcxproj" ^
/t:rebuild /p:Configuration=Debug /p:Platform=x64
@if ERRORLEVEL 1 goto error

echo #########################################################################
echo Done
goto :EOF

:error
echo #########################################################################
echo An error is detected

exit /B 1