#include <linux/init.h>
#include <linux/module.h>      /* Needed by all modules */
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include "pcidebug_driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hwj");
MODULE_DESCRIPTION("A driver for debug pci");

static unsigned int vendor = 0x10ee;
static unsigned int device = 0x7038;
module_param(vendor,uint,S_IRUSR);
module_param(device,uint,S_IRUSR);

static int __init pcidebug_init(void)
{
    printk(KERN_INFO "%s:Init:try to found Vendor=0x%x Device=0x%x\n",DEVICE_NAME,vendor,device);
    return 0;
}

static void __exit pcidebug_exit(void)
{
    printk(KERN_ALERT "%s:driver is unloaded\n",DEVICE_NAME);
}

module_init(pcidebug_init);
module_exit(pcidebug_exit);