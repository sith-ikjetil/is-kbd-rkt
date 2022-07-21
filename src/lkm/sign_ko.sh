#!/bin/bash
if [[ $# -eq 1 ]] 
then
    echo "Signing $1"
    /usr/src/linux-headers-$(uname -r)/scripts/sign-file sha256 /root/module-signing/new.MOK.priv /root/module-signing/MOK.der "$1"
else
    echo "##"
    echo "## Sign Kernel Object"
    echo "##"
    echo "## Usage: sign_ko.sh <./filename.ko>"
    echo "##"
    echo "## Error #############################################"
    echo "Wrong number of arguments."
fi