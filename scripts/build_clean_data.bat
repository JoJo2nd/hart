@echo off
pushd %~dp0\..\data\builder
python builder.py -d ..\assets -o ..\..\bin --clean
popd