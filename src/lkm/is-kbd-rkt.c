//////////////////////////////////////////////////////////////
//: Filename    : is-kbd-rkt.c
//: Date        : 2021-10-18
//: Author      : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
//: Version     : 1.2
//: Description : A Linux Kernel Module that detects an SMM keyboard rootkit.
//
// #include
//
#include "../include/is-kbd-rkt.h"
#include "../include/is-kbd-rkt-data.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <asm/msr.h>

//
// MODULE metadata
//
MODULE_AUTHOR("Kjetil Kristoffer Solberg <post@ikjetil.no>");
MODULE_DESCRIPTION("Linux Kernel Module for detecting SMM keyboard rootkit");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.2");

//
// #define
//
#define INTEL_LPC_RCBA_REG 0xF0

//
// function prototypes
//
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static u32 get_rcba(void);
static u64 get_apicba(void);
static void gather_data(IS_KEYBOARD_RKT_DATA* p);
static char *iskbdrkt_devnode(struct device *dev, umode_t *mode);

//
// static variables
//
static int major_num;
static struct class* char_class = NULL;
static struct device* char_device = NULL;

//
// device read
//
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset)
{
	if ( len == sizeof(IS_KEYBOARD_RKT_DATA) ) {
		struct IS_KEYBOARD_RKT_DATA data;
		const int cbSize = sizeof(IS_KEYBOARD_RKT_DATA);
		char* ptr = (char*)&data;
		int i = 0;

		gather_data(&data);

		for ( i = 0; i < cbSize; i++ )
		{
			put_user(*ptr, (char*)(buffer+i));
			ptr++;
		}

		return cbSize;
	}

    return 0;
}

//
// Function: get_rcba
//
// (i): Gets root complex base address. 
//
static u32 get_rcba(void)
{
	struct pci_dev *dev = NULL;
	u32 rcba;
	
	rcba = 0;	
	dev = pci_get_device(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, NULL);
_retry:
	if ( dev != NULL ) {
		pci_bus_read_config_dword(dev->bus, PCI_DEVFN(31, 0), INTEL_LPC_RCBA_REG, &rcba);
		if ( rcba == 0 ) {
			dev = pci_get_device(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, dev);
			goto _retry;
		}
	}
	
	rcba &= RCBA_MASK;

	return rcba;
}

//
// Function: get_apicba
//
// (i): get apic base address
//
static u64 get_apicba(void)
{
	u64 apicba = __rdmsr(0x1B);
	return apicba;
}

//
// Function: gather_data
//
// (i): Gathers and fills IS_KEYBOARD_RKT_DATA structure.
//
static void gather_data(IS_KEYBOARD_RKT_DATA* p)
{
	u32 __iomem *pIOTR0;
	u32 __iomem *pIOTR1;
	u32 __iomem *pIOTR2;
	u32 __iomem *pIOTR3;
	int i = 0;
	int irq = 0;
	u32 __iomem *pApicIoRegSel;
	u32 __iomem *pApicIoWin;

	memset((void*)p, 0, sizeof(IS_KEYBOARD_RKT_DATA));
	p->cbSize = sizeof(IS_KEYBOARD_RKT_DATA);

	//
	// RCBA
	//
	p->dwRootComplexBaseAddress = get_rcba();
	if ( p->dwRootComplexBaseAddress == 0 ) {
		strcpy(p->szErrorMessage, "Invalid Root Complex Base Address");
		return;
	}

	//
	// IRQTn
	//
	pIOTR0 = ioremap(p->dwRootComplexBaseAddress + 0x1E80, 8);
	pIOTR1 = ioremap(p->dwRootComplexBaseAddress + 0x1E88, 8);
	pIOTR2 = ioremap(p->dwRootComplexBaseAddress + 0x1E90, 8);
	pIOTR3 = ioremap(p->dwRootComplexBaseAddress + 0x1E98, 8);

	p->qwIOTRn[0] = readq(pIOTR0);
	p->qwIOTRn[1] = readq(pIOTR1);
	p->qwIOTRn[2] = readq(pIOTR2);
	p->qwIOTRn[3] = readq(pIOTR3);
	
	//
	// IOAPIC
	//
	p->dwApicBaseAddress = get_apicba();
	p->dwIoApicBaseAddress = IO_APIC_BASE_ADDRESS;

	pApicIoRegSel = ioremap(p->dwIoApicBaseAddress,4);
    pApicIoWin = ioremap(p->dwIoApicBaseAddress + 0x10, 4);

	for (i = 0, irq = 0x10; i < IO_APIC_IRQ_COUNT && irq <= 0x3E; i++, irq += 2)
    {
		writel(irq, pApicIoRegSel);
		p->qwIOAPIC_REDTBL[i] = readl(pApicIoWin);

		writel(irq+1, pApicIoRegSel);
		p->qwIOAPIC_REDTBL[i] |= ((u64)readl(pApicIoWin) << 32);
   }
}

//
// file_operations
//
static struct file_operations file_ops = {
	.owner = THIS_MODULE,
    .read = device_read,
};

//
// iskbdrkt_devnode
//
static char *iskbdrkt_devnode(struct device *dev, umode_t *mode)
{
	if (!mode)
		return NULL;
	
	*mode = 0444;
	
	return NULL;
}

//
// LKM init method
//
static int __init is_kbd_rtk_init(void)
{
    //
	// register char device
	//
    major_num = register_chrdev(0, DEVICE_NAME, &file_ops);
    if ( major_num < 0 ) {
        printk(KERN_ALERT DEVICE_NAME ": error registering device: %d\n", major_num );
        return major_num;
    } 
    printk(KERN_INFO DEVICE_NAME ": lkm loaded with device major number %d\n", major_num);

    //
	// register device class
	//
    char_class = class_create(THIS_MODULE,CLASS_NAME);
    if ( IS_ERR(char_class) ) {
        unregister_chrdev(major_num, DEVICE_NAME);
		printk(KERN_ALERT DEVICE_NAME ": error registering device class\n");
		return PTR_ERR(char_class);
    }
    printk(KERN_INFO DEVICE_NAME ": class registered successfully\n");

	char_class->devnode = iskbdrkt_devnode;

    //
	// register device deriver
	//
    char_device = device_create(char_class, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME);
    if (IS_ERR(char_device)){
        class_destroy(char_class);
		unregister_chrdev(major_num, DEVICE_NAME);
		printk(KERN_ALERT DEVICE_NAME "error creating device\n");
		return PTR_ERR(char_device);
    }
    printk(KERN_INFO DEVICE_NAME ": device class created correctly\n");
    
	//
	// return success
	//
	return 0;
}

//
// LKM exit method
//
static void __exit is_kbd_rtk_exit(void)
{
	device_destroy(char_class, MKDEV(major_num, 0));
    class_unregister(char_class);
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO DEVICE_NAME ": module unloaded\n");
}

//
// LKM init and exit methods
//
module_init(is_kbd_rtk_init);
module_exit(is_kbd_rtk_exit);
