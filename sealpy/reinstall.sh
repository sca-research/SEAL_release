#!/bin/bash

ver='1.0.0'

#Build wheel from source code.
python3 -m build --wheel ./

#Copy the wheel package to current folder.
cp dist/*.whl ./

#Force reinstall this package.
pip3 install --force-reinstall --break-system-packages --no-deps seal-${ver}-py3-none-any.whl
