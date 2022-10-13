#!/bin/bash

cd build/dots

csplit --digits=3 --quiet --prefix=outfile raw "/digraph/" "{*}"

for file in outfile*; do dot $file -T png -o $file.png; done
