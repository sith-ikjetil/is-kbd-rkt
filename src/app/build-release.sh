#!/bin/bash
echo "Compiling iskbdrkt..."
echo "> Using release build <"

g++ -o iskbdrkt is-kbd-rkt.cpp --std=c++17

if [[ $? -eq 0 ]] 
then
    echo "> Compilation Succeeded <"
else
    echo "> Compilation Failed <"
fi