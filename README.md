# is-kbd-rkt
A Linux Kernel Module with app for detecting SMM keyboard rootkit. 
 
## Example Output 
```bash
################################################################################
## Is Keyboard Rootkitted App ##################################################
## Author  : Kjetil Kristoffer Solberg <post@ikjetil.no>
## Version : 1.41
##
## BASE ADDRESS ####################
APIC            : 0xfee00800
IO APIC         : 0xfec00000
Root Complex    : 0xfed1c000
## IOTRn ###########################
IOTR0           : 0x000200f000fc2001 TRSE-bit SET
IOTR1           : 0x000200f000002101 TRSE-bit SET
IOTR2           : 0x0000000000000000 TRSE-bit NOT SET
IOTR3           : 0x0000000000000000 TRSE-bit NOT SET
## IOAPIC_IRQn #####################
IO APIC IRQ1    : 0x0000000000010000 Interrupt Mask-bit SET
## CONCLUSION ######################
No SMI Handler trapping the keyboard on IOTR0-IOTR3 or IRQ1
```

## Details 
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

Build the application in the app directory with build-debug.sh.
```bash
./build-debug.sh
```

Run the application,
```bash
./iskbdrkt
```

You need g++ to build the application. 
You need linux-headers to build the kernel object. 
You might need to disable secure boot in order for you to insert the kernel object into the kernel. 
You might want to sign the kernel object to bypass disabling secure boot.
