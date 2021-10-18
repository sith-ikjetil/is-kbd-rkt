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

//
// MODULE metadata
//
MODULE_AUTHOR("Kjetil Kristoffer Solberg <post@ikjetil.no>");
MODULE_DESCRIPTION("Linux Kernel Module for detecting SMM keyboard rootkit");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.2");

//
// function prototypes
//
//static int device_open(struct inode *, struct file *);
//static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
//static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int GatherData(IS_KEYBOARD_RKT_DATA* p);

//
// static variables
//
static int major_num;
//static int device_open_count = 0;
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
// device write
//
//static ssize_t device_write(struct file *file, const char *buffer, size_t len, loff_t *offset) 
//{
//   return 0;
//}

//
// device open
//
//static int device_open(struct inode *inode, struct file *file) 
//{
//    if ( device_open_count ) {
//        return -EBUSY;
//    }

//    device_open_count++;
//	try_module_get(THIS_MODULE);
    
//	return 0;
//}

//
// device_release
//
//static int device_release(struct inode *inode, struct file *file) 
//{
//   device_open_count--;
//    module_put(THIS_MODULE);
//    
//	return 0;
//}

//
// GatherData
//
static int GatherData(IS_KEYBOARD_RKT_DATA* p)
{
	memset((void*)p, 0, sizeof(IS_KEYBOARD_RKT_DATA));

	p->cbSize = sizeof(IS_KEYBOARD_RKT_DATA);
	strcpy(p->szErrorMessage, "Hello World from Kjetil Kristoffer Solberg");

	return 0;
}

//
// file_operations
//
static struct file_operations file_ops = {
	.owner = THIS_MODULE,
    .read = device_read,
//    .write = device_write,
//    .open = device_open,
//    .release = device_release,
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
