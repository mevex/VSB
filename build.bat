@echo off

set compilerFlags=-Od -Zi -nologo -FC -WX -W4 -wd4100
set libraries=user32.lib gdi32.lib

pushd build

cl %compilerFlags% ..\code\win32_VSB.cpp %libraries%

popd
