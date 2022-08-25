#!/bin/bash
echo "Compiling iskbdrkt..."
echo "> using debug build <"
g++ -ggdb -o iskbdrkt is-kbd-rkt.cpp --std=c++17
if [[ $? -eq 0 ]] 
then
    echo "> iskbdrkt build ok <"
else
    echo "> iskbdrkt build error <"
fi
echo "> build process complete <"