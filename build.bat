@echo off

where emcc > nul
if %errorlevel% neq 0 (
    call emsdk activate
)

set linkerFlags=-msimd128 -sOFFSCREEN_FRAMEBUFFER -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sALLOW_MEMORY_GROWTH
set debugFlags=-g -sASSERTIONS=2 -sSAFE_HEAP=1 -sDEMANGLE_SUPPORT=1
set releaseFlags=-O3

if not exist .\build mkdir .\build
pushd .\build
@REM call emcc --check
@REM call emcc ..\main.c %debugFlags% %linkerFlags%
call emcc ..\main.c %releaseFlags% %linkerFlags%
popd