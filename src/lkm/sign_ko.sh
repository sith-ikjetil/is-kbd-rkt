echo "Signing $1"
/usr/src/linux-headers-$(uname -r)/scripts/sign-file sha256 /root/module-signing/new.MOK.priv /root/module-signing/MOK.der "$1"
