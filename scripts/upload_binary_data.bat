@echo off
rem upload current binary data

pushd %~dp0

python binary_asset_package.py -r ./../ -d ./../data/assets -d ./../data/builder -d ./../external/SDL2 -d ./../tools

popd
