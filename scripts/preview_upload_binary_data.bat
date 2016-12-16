@echo off
rem upload current binary data

pushd %~dp0

call upload_binary_data.bat --preview

popd
