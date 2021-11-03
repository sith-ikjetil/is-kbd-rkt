/*
 *: Filename    : is-kbd-rkt.c
 *: Date        : 2021-10-18
 *: Author      : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
 *: Version     : 1.2
 *: Description : A Linux Kernel Module that produces output that detects an SMM keyboard rootkit.
*/
/*
 * #include
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/string.h>
#include <asm/msr.h>
#include <linux/proc_fs.h>
#include <linux/stddef.h>
#include "../include/is-kbd-rkt.h"
#include "../include/is-kbd-rkt-data.h"

/*
 * #define
 */
#define INTEL_LPC_RCBA_REG 	0xF0
#define PROC_FILENAME		"is_kbd_rkt"
#define PROC_MAX_SIZE		4096
#define MAX_BUFFER_SIZE	 	255
#define VERSION_NO			"1.6"

/*
 * module metadata
 */
MODULE_AUTHOR("Kjetil Kristoffer Solberg <post@ikjetil.no>");
MODULE_DESCRIPTION("Linux Kernel Module for detecting SMM keyboard rootkit");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION_NO);

/*
 * function prototypes
 */
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static u32 get_rcba(void);
static u64 get_apicba(void);
static void gather_data(IS_KEYBOARD_RKT_DATA* p);
static char *iskbdrkt_devnode(struct device *dev, umode_t *mode);
static ssize_t proc_read(struct file *, char __user *, size_t, loff_t *);
static void build_proc_info(char* source, const int max_size, IS_KEYBOARD_RKT_DATA* data);
static bool process_result(IS_KEYBOARD_RKT_DATA *p, IS_KEYBOARD_RKT_RESULT *r);

/*
 * static variables
 */
static int major_num;
static struct class* char_class = NULL;
static struct device* char_device = NULL;
static struct proc_dir_entry *proc_entry = NULL;
static bool has_rendered = false;

/*
 * device_read
 * device read function
 */
static ssize_t device_read(struct file *f, char *buffer, size_t len, loff_t *offset)
{	
	if (buffer == NULL) {
		return 0;
	}

	if (len == sizeof(IS_KEYBOARD_RKT_DATA)) {
		struct IS_KEYBOARD_RKT_DATA data;
		const int cbSize = sizeof(IS_KEYBOARD_RKT_DATA);
		char* ptr = (char*)&data;

		gather_data(&data);

		if (copy_to_user(buffer, ptr, cbSize) == 0 ) {
			return cbSize;
		}
	}

    return 0;
}

/*
 * read_proc_Info
 * proc file information read
 */
static ssize_t proc_read(struct file *f, char __user *buffer, size_t len, loff_t *offset)
{
	if ( buffer == NULL ) {
		return 0;
	}

	if (has_rendered) {
		has_rendered = false;
		return 0;
	}

	{
		int len = 0;
		IS_KEYBOARD_RKT_DATA* rkt_data = kzalloc(sizeof(IS_KEYBOARD_RKT_DATA), GFP_KERNEL);
		char* source = kzalloc(PROC_MAX_SIZE, GFP_KERNEL);
	
		if ( source == NULL ) {
			return 0;
		}

		gather_data(rkt_data);

		build_proc_info(source, PROC_MAX_SIZE, rkt_data);

		len = strnlen(source, PROC_MAX_SIZE);
		if (copy_to_user(buffer, source, len) == 0 ) {
			kfree(rkt_data);
			kfree(source);
			has_rendered = true;
			return len;
		}
		kfree(rkt_data);
		kfree(source);
	}

	return 0;
}

/*
 * build_proc_info
 * builds proc file contents
 */
static void build_proc_info(char* source, const int max_size, IS_KEYBOARD_RKT_DATA* data)
{
	strlcat(source, "##\n", max_size);
	strlcat(source, "## Is Keyboard Rootkitted\n", max_size);
	strlcat(source, "## Version : ", max_size); strlcat(source, VERSION_NO, max_size); strlcat(source, "\n", max_size);
	strlcat(source, "## Author  : Kjetil Kristoffer Solberg <post@ikjetil.no>\n", max_size);
	strlcat(source, "##\n", max_size);

	if (strnlen(data->szErrorMessage, MAX_STRING_BUFFER_SIZE) > 0) {
		strlcat(source, "## ERROR ##########################\n", max_size);
		strlcat(source, data->szErrorMessage, max_size);
		return;
	}

	{
		IS_KEYBOARD_RKT_RESULT* result = kzalloc(sizeof(IS_KEYBOARD_RKT_RESULT), GFP_KERNEL);
		char* buffer = kzalloc(MAX_BUFFER_SIZE,GFP_KERNEL);
		bool bSmiHandlerFound = false;
		
		strlcat(source, "## BASE ADDRESS ###################\n", max_size);
		snprintf(buffer, MAX_BUFFER_SIZE, "APIC           : 0x%08x\n", data->dwApicBaseAddress);
		strlcat(source, buffer, max_size);
		snprintf(buffer, MAX_BUFFER_SIZE, "IO APIC        : 0x%08x\n", data->dwIoApicBaseAddress);
		strlcat(source, buffer, max_size);
		snprintf(buffer, MAX_BUFFER_SIZE, "Root Complex   : 0x%08x\n", data->dwRootComplexBaseAddress);
		strlcat(source, buffer, max_size);

		strlcat(source, "## IOTRn ##########################\n", max_size);
		snprintf(buffer, MAX_BUFFER_SIZE, "IOTR0          : 0x%016llx\n", data->qwIOTRn[0]);
		strlcat(source, buffer, max_size);
		snprintf(buffer, MAX_BUFFER_SIZE, "IOTR1          : 0x%016llx\n", data->qwIOTRn[1]);
		strlcat(source, buffer, max_size);
		snprintf(buffer, MAX_BUFFER_SIZE, "IOTR2          : 0x%016llx\n", data->qwIOTRn[2]);
		strlcat(source, buffer, max_size);
		snprintf(buffer, MAX_BUFFER_SIZE, "IOTR3          : 0x%016llx\n", data->qwIOTRn[3]);
		strlcat(source, buffer, max_size);

		strlcat(source, "## IOAPIC IRQn ####################\n", max_size);
		snprintf(buffer, MAX_BUFFER_SIZE, "IOAPIC IRQ 1   : 0x%016llx\n", data->qwIOAPIC_REDTBL[1]);
		strlcat(source, buffer, max_size);

		strlcat(source, "## CONCLUSION #####################\n", max_size);
		
		process_result(data,result);
				
		if (result->bHitIOTR0) {
			snprintf(buffer, MAX_BUFFER_SIZE, "Keyboard Is Trapped by SMI Handler on IOTR0 port 0x%ix\n", result->wHitPortIOTR0);
			strlcat(source, buffer, max_size);
			bSmiHandlerFound = true;
		}
		if (result->bHitIOTR1) {
			snprintf(buffer, MAX_BUFFER_SIZE, "Keyboard Is Trapped by SMI Handler on IOTR1 port  0x%ix\n", result->wHitPortIOTR1);
			strlcat(source, buffer, max_size);
			bSmiHandlerFound = true;
		}
		if (result->bHitIOTR2) {
			snprintf(buffer, MAX_BUFFER_SIZE, "Keyboard Is Trapped by SMI Handler on IOTR2 port  0x%ix\n", result->wHitPortIOTR2);
			strlcat(source, buffer, max_size);
			bSmiHandlerFound = true;
		}
		if (result->bHitIOTR3) {
			snprintf(buffer, MAX_BUFFER_SIZE, "Keyboard Is Trapped by SMI Handler on IOTR3 port  0x%ix\n", result->wHitPortIOTR3);
			strlcat(source, buffer, max_size);
			bSmiHandlerFound = true;
		}
		if (result->bHitIoApicIRQ1) {
			strlcat(source, "Keyboard Is Trapped by SMI Handler on I/O APIC IRQ1. DELMOD-bit SMI SET\n", max_size);
			bSmiHandlerFound = true;
		}

		if (!bSmiHandlerFound) {
			strlcat(source, "No SMI Handler trapping the keyboard on IOTR0-IOTR3 or IRQ1\n", max_size);
		}

		kfree(result);
		kfree(buffer);
	}
}

/*
 * process_result
 */
bool process_result(IS_KEYBOARD_RKT_DATA *p, IS_KEYBOARD_RKT_RESULT *r)
{ 
    // IOTR0
    if (((uint32_t)p->qwIOTRn[0]) == 0x61)
    {
        r->bHitIOTR0 = true;
        r->wHitPortIOTR0 = 0x60;
        r->qwHitIOTR0 = p->qwIOTRn[0];
    }

    if (((uint32_t)p->qwIOTRn[0]) == 0x65)
    {
        r->bHitIOTR0 = true;
        r->wHitPortIOTR0 = 0x64;
        r->qwHitIOTR0 = p->qwIOTRn[0];
    }

    // IOTR1
    if (((uint32_t)p->qwIOTRn[1]) == 0x61)
    {
        r->bHitIOTR1 = true;
        r->wHitPortIOTR1 = 0x60;
        r->qwHitIOTR1 = p->qwIOTRn[1];
    }

    if (((uint32_t)p->qwIOTRn[1]) == 0x65)
    {
        r->bHitIOTR1 = true;
        r->wHitPortIOTR1 = 0x64;
        r->qwHitIOTR1 = p->qwIOTRn[1];
    }

    // IOTR2
    if (((uint32_t)p->qwIOTRn[2]) == 0x61)
    {
        r->bHitIOTR2 = true;
        r->wHitPortIOTR2 = 0x60;
        r->qwHitIOTR2 = p->qwIOTRn[2];
    }

    if (((uint32_t)p->qwIOTRn[2]) == 0x65)
    {
        r->bHitIOTR2 = true;
        r->wHitPortIOTR2 = 0x64;
        r->qwHitIOTR2 = p->qwIOTRn[2];
    }

    // IOTR3
    if (((uint32_t)p->qwIOTRn[3]) == 0x61)
    {
        r->bHitIOTR3 = true;
        r->wHitPortIOTR3 = 0x60;
        r->qwHitIOTR3 = p->qwIOTRn[3];
    }

    if (((uint32_t)p->qwIOTRn[3]) == 0x65)
    {
        r->bHitIOTR3 = true;
        r->wHitPortIOTR3 = 0x64;
        r->qwHitIOTR3 = p->qwIOTRn[3];
    }

    if (((p->qwIOAPIC_REDTBL[1] & 0b011100000000) == 0b001000000000)) {
        r->bHitIoApicIRQ1 = true;
    }

    return (r->bHitIOTR0 || r->bHitIOTR1 || r->bHitIOTR2 || r->bHitIOTR3 || r->bHitIoApicIRQ1);
}

/*
 * get_rcba
 * gets root complex base address
 */
static u32 get_rcba(void)
{
	struct pci_dev *dev = NULL;
	u32 rcba;
	
	rcba = 0;	
	dev = pci_get_device(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, NULL);
_retry:
	if (dev != NULL) {
		pci_bus_read_config_dword(dev->bus, PCI_DEVFN(31, 0), INTEL_LPC_RCBA_REG, &rcba);
		if (rcba == 0) {
			dev = pci_get_device(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, dev);
			goto _retry;
		}
	}
	
	rcba &= RCBA_MASK;

	return rcba;
}

/*
 * get_apicba
 * gets apic base address
 */
static u64 get_apicba(void)
{
	u64 apicba = __rdmsr(0x1B);
	return apicba;
}

/*
 * gather_data
 * gathers and fills the IS_KEYBOARD_RKT_DATA structure
 */
static void gather_data(IS_KEYBOARD_RKT_DATA* p)
{
	u32 __iomem *pIOTR0_1;
	u32 __iomem *pIOTR0_2;
	u32 __iomem *pIOTR1_1;
	u32 __iomem *pIOTR1_2;
	u32 __iomem *pIOTR2_1;
	u32 __iomem *pIOTR2_2;
	u32 __iomem *pIOTR3_1;
	u32 __iomem *pIOTR3_2;
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
	if (p->dwRootComplexBaseAddress == 0) {
		strscpy(p->szErrorMessage, "Invalid Root Complex Base Address", MAX_STRING_BUFFER_SIZE);
		return;
	}

	//
	// IRQTn
	//
	pIOTR0_1 = ioremap(p->dwRootComplexBaseAddress + 0x1E80, 4);
	pIOTR0_2 = ioremap(p->dwRootComplexBaseAddress + 0x1E84, 4);
	pIOTR1_1 = ioremap(p->dwRootComplexBaseAddress + 0x1E88, 4);
	pIOTR1_2 = ioremap(p->dwRootComplexBaseAddress + 0x1E8C, 4);
	pIOTR2_1 = ioremap(p->dwRootComplexBaseAddress + 0x1E90, 4);
	pIOTR2_2 = ioremap(p->dwRootComplexBaseAddress + 0x1E94, 4);
	pIOTR3_1 = ioremap(p->dwRootComplexBaseAddress + 0x1E98, 4);
	pIOTR3_2 = ioremap(p->dwRootComplexBaseAddress + 0x1E9C, 4);

	p->qwIOTRn[0] = readl(pIOTR0_1);
	p->qwIOTRn[0] |= ((u64)readl(pIOTR0_2) << 32);
	p->qwIOTRn[1] = readl(pIOTR1_1);
	p->qwIOTRn[1] |= ((u64)readl(pIOTR1_2) << 32);
	p->qwIOTRn[2] = readl(pIOTR2_1);
	p->qwIOTRn[2] |= ((u64)readl(pIOTR2_2) << 32);
	p->qwIOTRn[3] = readl(pIOTR3_1);
	p->qwIOTRn[3] |= ((u64)readl(pIOTR3_2) << 32);
	
	//
	// IOAPIC
	//
	p->dwApicBaseAddress = get_apicba();
	p->dwIoApicBaseAddress = IO_APIC_BASE_ADDRESS;

	pApicIoRegSel = ioremap(p->dwIoApicBaseAddress,1);
    pApicIoWin = ioremap(p->dwIoApicBaseAddress + 0x10, 4);

	for (i = 0, irq = 0x10; i < IO_APIC_IRQ_COUNT && irq <= 0x3E; i++, irq += 2) {
		writeb(irq, pApicIoRegSel);
		p->qwIOAPIC_REDTBL[i] = readl(pApicIoWin);

		writeb(irq+1, pApicIoRegSel);
		p->qwIOAPIC_REDTBL[i] |= ((u64)readl(pApicIoWin) << 32);
   }
}

/*
 * file_operations
 * driver file_operations structure
 */
static struct file_operations file_ops = {
	.owner = THIS_MODULE,
    .read = device_read,
};

/*
 * file_operations
 * proc file operations
 */
static struct proc_ops proc_file_ops = {
	.proc_read = proc_read,
};

/*
 * iskbdrkt_devnode
 * set devnode mode
 */
static char *iskbdrkt_devnode(struct device *dev, umode_t *mode)
{
	if (!mode)
		return NULL;
	
	// *mode must be 0664 for depmod to work. 2021-10-24
	*mode = 0664;
	
	return NULL;
}

/*
 * LKM init method
 */
static int __init is_kbd_rtk_init(void)
{
    //
	// register char device
	//
    major_num = register_chrdev(0, DEVICE_NAME, &file_ops);
    if (major_num < 0) {
        printk(KERN_ALERT DEVICE_NAME ": error registering device: %d\n", major_num );
        return major_num;
    } 
    printk(KERN_INFO DEVICE_NAME ": lkm loaded with device major number %d\n", major_num);

    //
	// register device class
	//
    char_class = class_create(THIS_MODULE,CLASS_NAME);
    if (IS_ERR(char_class)) {
        unregister_chrdev(major_num, DEVICE_NAME);
		printk(KERN_ALERT DEVICE_NAME ": error registering device class\n");
		return PTR_ERR(char_class);
    }
    printk(KERN_INFO DEVICE_NAME ": class registered successfully\n");

	char_class->devnode = iskbdrkt_devnode;

    //
	// register device driver
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
	// create proc file
	//
	proc_entry = proc_create(PROC_FILENAME, 0, NULL, &proc_file_ops);
	if (proc_entry == NULL) {
		printk(KERN_INFO DEVICE_NAME ": create_proc_entry failed\n");
	}
	printk(KERN_INFO DEVICE_NAME ": create_proc_entry succeeded\n");

	//
	// return success
	//
	return 0;
}

/*
 * LKM exit method
 */
static void __exit is_kbd_rtk_exit(void)
{
	device_destroy(char_class, MKDEV(major_num, 0));
    class_unregister(char_class);
    unregister_chrdev(major_num, DEVICE_NAME);
	if (proc_entry != NULL) {
		remove_proc_entry(PROC_FILENAME, NULL);
	}
    printk(KERN_INFO DEVICE_NAME ": module unloaded\n");
}

/*
 * LKM module init and exit
 */
module_init(is_kbd_rtk_init);
module_exit(is_kbd_rtk_exit);
