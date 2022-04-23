#include <linux/init.h>
#include <linux/module.h>     
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include "pcidebug_driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hwj");
MODULE_DESCRIPTION("A driver for debug pci");

static unsigned int vendor = 0x10ee;
static unsigned int device = 0x7038;
module_param(vendor,uint,S_IRUSR);
module_param(device,uint,S_IRUSR);

static bool gKernelRegFlag = false; 
static dev_t dev;           // Global variable for device number
static struct cdev chardev; // Global variable for character device 
static struct class *cl;    // Global variable for device class


struct pcidebug_state{
    struct pci_dev *dev;
    bool used;
    unsigned long baseHards[BARS_MAXNUM];  // BARs Hardware address
    unsigned long baseLens[BARS_MAXNUM];   // BARs Length
    void *baseVirts[BARS_MAXNUM];          // BRAs Virtual address
};

static struct pcidebug_state pcidebug;

//Prorotypes
int pcidebug_open(struct inode *inode,struct file *file);
int pcidebug_release(struct inode *inode,struct file *file);

// Aliasing
struct file_operations pcidebug_ops={
    open : pcidebug_open,
    release : pcidebug_release,
};

int pcidebug_open(struct inode *inode,struct file *file)
{
    printk(KERN_INFO "%s: Open: module opened\n",DEVICE_NAME);
    return (SUCCESS);
}

int pcidebug_release(struct inode *inode,struct file *file)
{
    printk(KERN_INFO "%s: Release: module released\n",DEVICE_NAME);
    return (SUCCESS);
}

static int pcidebug_getReource(void)
{
    //todo
    return (SUCCESS);
}

static int __init pcidebug_init(void)
{
    printk(KERN_INFO"%s: Init: try to found first device, Vendor=0x%x Device=0x%x\n",DEVICE_NAME,vendor,device);
    pcidebug.dev = pci_get_device(vendor,device,NULL);
    if(pcidebug.dev){
        printk(KERN_INFO"%s: Init: found device\n",DEVICE_NAME);
        if(pcidebug_getReource() == SUCCESS){
            pcidebug.used = 1;
        }else{
            printk(KERN_WARNING"%s: Init: can't get resources!\n",DEVICE_NAME);
        }
    }else{
        printk(KERN_INFO"%s: Init: not found device\n",DEVICE_NAME);
    }
    printk(KERN_INFO"%s: Init: finish found device\n",DEVICE_NAME);

    // Register driver as a character device
    if(0 > alloc_chrdev_region(&dev,0,1,DEVICE_NAME)){
        printk(KERN_ALERT "%s: Device Registration failed\n",DEVICE_NAME);
        unregister_chrdev_region(dev,1);
        return (ERROR);
    }
    if( NULL == (cl = class_create(THIS_MODULE,"chardevice"))){
        printk(KERN_ALERT "%s: Class creation failed\n",DEVICE_NAME);
        class_destroy(cl);
        unregister_chrdev_region(dev,1);
        return (ERROR);
    }
    printk(KERN_INFO "%s: Init: module registered\n",DEVICE_NAME);

    if( NULL == device_create(cl, NULL, dev, NULL, DEVICE_NAME)){
        printk(KERN_ALERT "%s: Device creation failed\n",DEVICE_NAME);
        class_destroy(cl);
        unregister_chrdev_region(dev,1);
        return (ERROR);
    }

    cdev_init(&chardev, &pcidebug_ops);

    if( 0 > cdev_add(&chardev, dev, 1)){
        printk(KERN_ALERT "%s: Device addition failed\n",DEVICE_NAME);
        device_destroy(cl, dev);
        class_destroy(cl);
        unregister_chrdev_region(dev,1);
        return (ERROR);
    }

    gKernelRegFlag = true;
    printk(KERN_INFO "%s: Init: Driver is load\n",DEVICE_NAME);

    printk(KERN_INFO "%s: Init: Done\n",DEVICE_NAME);
    return (SUCCESS);
}

static void __exit pcidebug_exit(void)
{
    if(gKernelRegFlag){
        cdev_del(&chardev);
        device_destroy(cl, dev);
        class_destroy(cl);
        unregister_chrdev_region(dev,1);
        printk(KERN_ALERT "%s: Device unregistered\n",DEVICE_NAME);
    }
    printk(KERN_ALERT "%s: Driver is unloaded\n",DEVICE_NAME);
}

module_init(pcidebug_init);
module_exit(pcidebug_exit);