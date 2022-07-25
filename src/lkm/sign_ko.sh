#!/bin/bash

echo "##"
echo "## Sign Kernel Object"
echo "##"
echo "## Usage: sign_ko.sh <./filename.ko>"
echo "##"

if [[ $# -eq 1 ]] 
then
    echo "> signing $1 <"
    /usr/src/linux-headers-$(uname -r)/scripts/sign-file sha256 /root/module-signing/new.MOK.priv /root/module-signing/MOK.der "$1"
    if [[ $? -eq 0 ]]
    then
        echo "> signing complete <"
    else
        echo "> signing error <"
    fi
else 
    echo "> wrong number of arguments <"
fi