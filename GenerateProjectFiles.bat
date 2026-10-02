@echo off
setlocal
for /f "tokens=2 delims=:" %%a in ('chcp') do set "SE_OLD_CODEPAGE=%%a"
chcp 65001 >nul

rem cmd는 배치 파일을 한 줄씩 현재 코드페이지로 읽는다. 위에서 UTF-8(65001)로 바꿨으므로 이 줄부터 한국어를 쓸 수 있다.
rem 소스 파일을 추가, 삭제, 이동한 뒤 실행한다. 각 .vcxproj의 파일 목록과 .vcxproj.filters를 폴더 구조대로 다시 만든다.
rem 스크립트 본체: Engine\Build\BatchFiles\GenerateProjectFiles.cs (.NET 10 SDK 필요)

set "SE_ROOT=%~dp0"
set "SE_EXIT_CODE=0"

rem dotnet run 파일.cs(단일 파일 실행)는 .NET 10 SDK부터 지원한다.
rem SDK 없이 런타임만 있으면 버전 대신 다른 문장이 나올 수 있으므로 숫자인지도 확인한다.
set "SE_DOTNET_MAJOR="
for /f "tokens=1 delims=." %%v in ('dotnet --version 2^>nul') do set "SE_DOTNET_MAJOR=%%v"
if not defined SE_DOTNET_MAJOR goto :NoDotnet
echo %SE_DOTNET_MAJOR%| findstr /r "^[0-9][0-9]*$" >nul || goto :NoDotnet
if %SE_DOTNET_MAJOR% LSS 10 goto :NoDotnet

rem 빌드 결과는 .NET 기본 위치(%TEMP%) 대신 저장소의 Intermediate에 둔다.
dotnet run "%SE_ROOT%Engine\Build\BatchFiles\GenerateProjectFiles.cs" --artifacts-path "%SE_ROOT%Intermediate\DotNET\GenerateProjectFiles"
set "SE_EXIT_CODE=%ERRORLEVEL%"
goto :End

:NoDotnet
echo 오류: .NET 10 이상의 SDK를 찾지 못했다.
echo Visual Studio 설치 관리자에서 ".NET 데스크톱 개발" 워크로드를 추가하거나 https://dotnet.microsoft.com/download 에서 SDK를 설치한다.
set "SE_EXIT_CODE=1"

:End
rem 탐색기에서 더블클릭으로 실행하면 창이 바로 닫히므로 결과를 볼 수 있게 멈춘다. 명령 프롬프트에서 실행하면 멈추지 않는다.
rem PATH에 Git의 Unix find가 먼저 있을 수 있으므로 Windows find.exe를 경로로 지정한다.
echo %CMDCMDLINE% | "%SystemRoot%\System32\find.exe" /i "%~f0" >nul && pause
chcp %SE_OLD_CODEPAGE% >nul
exit /b %SE_EXIT_CODE%
