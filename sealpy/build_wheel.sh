#!/bin/bash

#Build wheel from source code.
python3 -m build --wheel ./

#Copy the wheel package to current folder.
cp dist/*.whl ./
