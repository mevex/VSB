@echo off

set compilerFlags=-MT -Oi -Od -Zi -nologo -FC -WX -W4 -wd4100 -wd4189 -wd4201
set libraries=user32.lib gdi32.lib

pushd build

cl %compilerFlags% ..\code\win32_vsb.cpp ..\code\vsb.cpp -link -opt:ref %libraries%

popd
