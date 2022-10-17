#!/bin/bash

cd dots

csplit --digits=3 --quiet --prefix=outfile raw "/digraph/" "{*}"

for file in outfile*; do 
  sem -j+0 dot $file -T png -o img-$file.png 
done
sem --wait
