#!/bin/bash
echo "Compiling iskbdrkt..."
echo "> Using debug build <"

g++ -g -o iskbdrkt is-kbd-rkt.cpp --std=c++17

if [[ $? -eq 0 ]] 
then
    echo "> Compilation Succeeded <"
else
    echo "> Compilation Failed <"
fi