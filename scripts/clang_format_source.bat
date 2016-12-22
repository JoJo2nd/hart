@echo off
rem upload current binary data

pushd %~dp0

python clang_format_files.py -d ..\hart -d ..\game

popd