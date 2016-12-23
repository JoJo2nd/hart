@echo off
rem upload current binary data

pushd %~dp0

python clang_format_files.py ^
-r ..\hart\include\base ^
-r ..\hart\include\core ^
-r ..\hart\include\render ^
-d ..\hart\include ^
-d ..\hart\win32\hart\include ^
-d ..\hart\src\common\base ^
-d ..\hart\src\common\core ^
-d ..\hart\src\common\render ^
-d ..\hart\src\win32\base ^
-d ..\game

del /S ..\hart\*.TMP
del /S ..\game\*.TMP

popd