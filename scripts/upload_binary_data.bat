@echo off
rem upload current binary data

pushd %~dp0

python binary_asset_package.py %* -r ./../ -d ./../data/assets -d ./../data/builder -d ./../data/reference -d ./../external/SDL2 -d ./../tools -d ./../external/LLVM

popd
