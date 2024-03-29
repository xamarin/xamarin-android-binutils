call build-common.cmd

set SOURCE_DIR=%MY_DIR%src
set HOST_BUILD_DIR=%BUILD_DIR%\%HOST%\xa-utils
set HOST_BIN_DIR=%HOST_BUILD_DIR%\bin\Release

if exist %HOST_BUILD_DIR% (rmdir /S /Q %HOST_BUILD_DIR%)
mkdir %HOST_BUILD_DIR%

cd %HOST_BUILD_DIR%

cmake --version

cmake -G "%CMAKE_VS_GENERATOR%" -A x64 ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DCMAKE_MSVC_RUNTIME_LIBRARY="%MSVC_RUNTIME_LIBRARY%" ^
 -DBINUTILS_VERSION="2.38" ^
 %SOURCE_DIR%
IF %ERRORLEVEL% GEQ 1 EXIT /B 1

msbuild /p:Configuration=Release /m xa-utilities.sln
IF %ERRORLEVEL% GEQ 1 EXIT /B 2

copy %HOST_BIN_DIR%\as.exe  %HOST_ARTIFACTS_BIN_DIR%\as.exe
IF %ERRORLEVEL% GEQ 1 EXIT /B 3

copy %HOST_BIN_DIR%\as.pdb  %HOST_ARTIFACTS_BIN_DIR%\as.pdb
IF %ERRORLEVEL% GEQ 1 EXIT /B 4

cd %MY_DIR%
