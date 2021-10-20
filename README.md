# is-kbd-rkt
A Linux Kernel Module for detecting SMM keyboard rootkit.

Run make in the lkm directory to build the kernel object.
```bash
make
```

Insert the kernel object into the kernel with: 
```bash
sudo insmod ./is-kbd-rkt.ko
```

When you want to, remove the kernel object from the kernel with:
```bash
sudo rmmod is-kbd-rkt
```

Build the application int the app directory with build-debug.sh.
```bash
./build-debug.sh
```

Run the application,
```bash
./iskbdrkt -v
```

You need g++ to build the application.
You need linux-headers to build the kernel object.
You might need to disable secure boot in order for you to insert the kernel object into the kernel.
You might want to sign the kernel object to bypass disabling secure boot.
