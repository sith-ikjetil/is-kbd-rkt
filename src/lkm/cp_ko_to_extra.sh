#!/bin/bash

echo "##"
echo "## Copy Kernel Object to /lib/modules/<uname -r>/extra"
echo "##"
echo "## Usage: cp_ko_to_extra.sh <./filename.ko>"
echo "##"

if [[ $# -eq 1 ]] 
then
    echo "> copying kernel object <"

    DIR="/lib/modules/`uname -r`/extra"
    [ ! -d "$DIR" ] && mkdir $DIR

    cp $1 $DIR
else
    echo "> invalid number of arguments <"
fi
