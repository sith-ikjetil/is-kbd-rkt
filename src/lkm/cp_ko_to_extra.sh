#!/bin/bash
if [[ $# -eq 1 ]] 
then
    DIR="/lib/modules/`uname -r`/extra"
    [ ! -d "$DIR" ] && mkdir $DIR

    cp $1 $DIR
else
    echo "##"
    echo "## Copy Kernel Object to /lib/modules/<uname -r>/extra"
    echo "##"
    echo "## Usage: cp_ko_to_extra.sh <./filename.ko>"
    echo "##"
    echo "## Error #############################################"
    echo "Wrong number of arguments."
fi
