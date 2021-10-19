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
#include <asm-generic/iomap.h>
#include <linux/pci.h>

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
static u32 GetRootComplexBaseAddress(void);
static void GatherData(IS_KEYBOARD_RKT_DATA* p);

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

		GatherData(&data);

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
// GetRootComplexBaseAddress
//
static u32 GetRootComplexBaseAddress(void)
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
	
	return rcba;
}

//
// GatherData
//
static void GatherData(IS_KEYBOARD_RKT_DATA* p)
{
	memset((void*)p, 0, sizeof(IS_KEYBOARD_RKT_DATA));
	p->cbSize = sizeof(IS_KEYBOARD_RKT_DATA);

	//unsigned short PCI_CONFIGURATION_SPACE_ADDRESS_PORT	= 0x0cf8;// Configuration Space Address 
	//unsigned short PCI_CONFIGURATION_SPACE_DATA_PORT	= 0x0cfc;// Configuration Space Data 

	//
	// RCBA
	//
	p->dwRootComplexBaseAddress = GetRootComplexBaseAddress();
	if ( p->dwRootComplexBaseAddress == 0 ) {
		strcpy(p->szErrorMessage, "Invalid Root Complex Base Address");
	}

	//
	// IRQTn
	//
	void* pmm = p->dwRootComplexBaseAddress + 0x1E80;

	/*volatile u32* pIOTR0 = (u32*)((u64)p->dwRootComplexBaseAddress + 0x1E80);
    volatile u32* pIOTR1 = (u32*)((u64)p->dwRootComplexBaseAddress + 0x1E88);
    volatile u32* pIOTR2 = (u32*)((u64)p->dwRootComplexBaseAddress + 0x1E90);
    volatile u32* pIOTR3 = (u32*)((u64)p->dwRootComplexBaseAddress + 0x1E98);
*/
    p->qwIOTRn[0] = *((u64*)pmm);//readl(pIOTR0);
    /*p->qwIOTRn[0] |= ((u64)(*(pIOTR0 + 1) << 32));
    p->qwIOTRn[1] = *pIOTR1;
    p->qwIOTRn[1] |= ((u64)(*(pIOTR1 + 1) << 32));
    p->qwIOTRn[2] = *pIOTR2;
    p->qwIOTRn[2] |= ((u64)(*(pIOTR2 + 1) << 32));
    p->qwIOTRn[3] = *pIOTR3;
    p->qwIOTRn[3] |= ((u64)(*(pIOTR3 + 1) << 32));
*/
}

//
// file_operations
//
static struct file_operations file_ops = {
	.owner = THIS_MODULE,
    .read = device_read,
};

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
