@echo off
rem download and extract current binary data

pushd %~dp0\..\

python scripts\binary_asset_unpackage.py 

popd