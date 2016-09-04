@echo off
rem download and extract current binary data

pushd %~dp0

python binary_asset_unpackage.py -r ./../

popd