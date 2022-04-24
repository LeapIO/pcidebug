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
int pcidebug_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

// Aliasing
struct file_operations pcidebug_ops={
    unlocked_ioctl : pcidebug_ioctl,
    open : pcidebug_open,
    release : pcidebug_release,
};

long pcidebug_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int rc = (ERROR);
    switch(cmd){
        case IOCTL_RDBAR8:
            break;
        case IOCTL_RDBAR16:
            break;
        case IOCTL_RDBAR32:
            break;
        case IOCTL_RDBAR64:
            break;
        case IOCTL_WRBAR8:
            break;
        case IOCTL_WRBAR16:
            break;
        case IOCTL_WRBAR32:
            break;
        case IOCTL_WRBAR64:
            break;
    }
}

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

static int pcidebug_map_mem_bar(int bar_id)
{
    pcidebug.baseHards[bar_id] = pci_resource_start(pcidebug.dev, bar_id);
    printk(KERN_INFO "%s: map_mem_bar: BAR %d hw addr 0x%016lX\n",DEVICE_NAME, bar_id, pcidebug.baseHards[bar_id]);
    pcidebug.baseLens[bar_id] = pci_resource_len(pcidebug.dev, bar_id);
    printk(KERN_INFO "%s: map_mem_bar: BAR %d hw len %lu\n",DEVICE_NAME, bar_id, pcidebug.baseLens[bar_id]);
    pcidebug.baseVirts[bar_id] = ioremap(pcidebug.baseHards[bar_id],pcidebug.baseLens[bar_id]);
    if(!pcidebug.baseVirts[bar_id]){
        printk(KERN_WARNING "%s: map_mem_bar: BAR %d can't remap memory.\n",DEVICE_NAME,bar_id);
        return (ERROR);
    }
    printk(KERN_INFO "%s: map_mem_bar: BAR %d virt addr %lX.\n",DEVICE_NAME, bar_id, (size_t)pcidebug.baseVirts[bar_id]);

    // request region
    // todo
    return (SUCCESS);
}

static int pcidebug_map_io_bar(int bar_id)
{
    pcidebug.baseHards[bar_id] = pci_resource_start(pcidebug.dev, bar_id);
    printk(KERN_INFO "%s: map_io_bar: BAR %d hw addr 0x%016lX\n",DEVICE_NAME, bar_id, pcidebug.baseHards[bar_id]);
    pcidebug.baseLens[bar_id] = pci_resource_len(pcidebug.dev, bar_id);
    printk(KERN_INFO "%s: map_io_bar: BAR %d hw len %lu\n",DEVICE_NAME, bar_id, pcidebug.baseLens[bar_id]);
    pcidebug.baseVirts[bar_id] = ioport_map(pcidebug.baseHards[bar_id],pcidebug.baseLens[bar_id]);
    if(!pcidebug.baseVirts[bar_id]){
        printk(KERN_WARNING "%s: map_io_bar: BAR %d can't remap memory.\n",DEVICE_NAME,bar_id);
        return (ERROR);
    }
    printk(KERN_INFO "%s: map_io_bar: BAR %d virt addr %lX.\n",DEVICE_NAME, bar_id, (size_t)pcidebug.baseVirts[bar_id]);

    // request region
    // to do
    return (SUCCESS);
}

static int pcidebug_getResource(void)
{
    int bar_id = 0;
    unsigned long flags;

    // enable device
    if(0 > pci_enable_device(pcidebug.dev)){
        printk(KERN_CRIT"%s: getResource: Device not enable.\n", DEVICE_NAME);
        return (ERROR);
    }

    // map all bars
    for(bar_id = 0; bar_id < BARS_MAXNUM; bar_id++){
        if(pci_resource_len(pcidebug.dev,bar_id)>0 &&pci_resource_start(pcidebug.dev,bar_id)>0){  // valid BAR
            flags = pci_resource_flags(pcidebug.dev,bar_id);
            if(flags & IORESOURCE_MEM){         // Memory
                if(0 > pcidebug_map_mem_bar(bar_id)){
                    return (ERROR);
                }
            }else if(flags & IORESOURCE_IO){    // I/O ports
                if(0 > pcidebug_map_io_bar(bar_id)){
                    return (ERROR);
                }
            }
        }else{
            printk(KERN_WARNING "%s: getResource: BAR %d invalid\n",DEVICE_NAME,bar_id);
        }
    }

    return (SUCCESS);
}

static int __init pcidebug_init(void)
{
    int i=0;
    pcidebug.dev = NULL;
    pcidebug.used = false;
    for(i=0; i<BARS_MAXNUM; i++){
        pcidebug.baseHards[i] = 0;
        pcidebug.baseLens[i] = 0;
        pcidebug.baseVirts[i] = NULL;
    }

    printk(KERN_INFO"%s: Init: try to found first device, Vendor=0x%x Device=0x%x\n",DEVICE_NAME,vendor,device);
    pcidebug.dev = pci_get_device(vendor,device,NULL);
    if(pcidebug.dev){
        printk(KERN_INFO"%s: Init: found device\n",DEVICE_NAME);
        if(pcidebug_getResource() == SUCCESS){
            pcidebug.used = 1;
        }else{
            printk(KERN_WARNING"%s: Init: can't get resources!\n",DEVICE_NAME);
        }
    }else{
        printk(KERN_INFO"%s: Init: not found device\n",DEVICE_NAME);
        return (ERROR);
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
    int id = 0;
    for(id = 0; id<BARS_MAXNUM; id++){
        //release region
        // to do

        // unmap virtual device address
        if(pcidebug.baseVirts[id] != NULL){
            iounmap(pcidebug.baseVirts[id]);
            printk(KERN_INFO "%s: unmap bar %d",DEVICE_NAME,id);
        }
    }
    if(pcidebug.dev){
        pci_disable_device(pcidebug.dev);
        printk(KERN_INFO "%s: disable device",DEVICE_NAME);
    }

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