# is-kbd-rkt
License: **GPL-3.0-or-later**  

A Linux Kernel Module with "/proc/is-kbd-rkt" file and "iskbdrkt" app for detecting SMM keyboard rootkit. 
 
## Example Output iskbdrkt app
```bash
##
## Is Keyboard Rootkitted App 
## Author  : Kjetil Kristoffer Solberg <post@ikjetil.no>
## Version : 1.7
## Usage   : 
##           -n 50 = Number of times to run the test.
##              (default = 50, max = 1000, min = 1)
##           --no-color    = no colored output
##           --result-only = only output the conclusion result
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

## Example Output /proc/is-kbd-rkt file
```bash
##
## Is Keyboard Rootkitted
## Version : 1.6
## Author  : Kjetil Kristoffer Solberg <post@ikjetil.no>
##
## BASE ADDRESS ####################
APIC           : 0xfee00c00
IO APIC        : 0xfec00000
Root Complex   : 0xffffc000
## IOTRn ###########################
IOTR0          : 0x80b9ffca83000012
IOTR1          : 0x00001261e8000003
IOTR2          : 0xe8ce8b00000320be
IOTR3          : 0x0200000d00001222
## IOAPIC IRQn #####################
IOAPIC IRQ 1   : 0x0000000000010000
## CONCLUSION ##########################
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
