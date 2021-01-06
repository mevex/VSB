@echo off

set compilerFlags=-MT -Oi -Od -Zi -nologo -FC -WX -W4 -wd4100 -wd4189 -wd4201 -wd4505
set libraries=user32.lib gdi32.lib Winmm.lib

pushd build
del *.pdb > NUL 2>&1
cl %compilerFlags% ..\code\vsb.cpp -LD -link -opt:ref -incremental:no -PDB:vsb_%time:~0,2%_%time:~3,2%_%time:~6,2%.pdb %libraries% -EXPORT:GameUpdateAndRender
cl %compilerFlags% ..\code\win32_vsb.cpp -link -opt:ref -incremental:no %libraries%

popd
