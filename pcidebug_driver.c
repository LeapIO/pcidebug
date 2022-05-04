#include <linux/init.h>
#include <linux/module.h>     
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/file.h>
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

static char KprintfBuff[KPRINTF_MAX] = {0,}; // kernal printf buffer

struct pcidebug_state{
    struct pci_dev *dev;
    bool used;
    bool baruseds[BARS_MAXNUM];            // BAR is or not used
    bool haveRegion[BARS_MAXNUM];          
    unsigned long baseHards[BARS_MAXNUM];  // BARs Hardware address
    unsigned long baseLens[BARS_MAXNUM];   // BARs Length
    void *baseVirts[BARS_MAXNUM];          // BRAs Virtual address
};

static struct pcidebug_state pcidebug;

//Prorotypes
int pcidebug_open(struct inode *inode,struct file *file);
int pcidebug_release(struct inode *inode,struct file *file);
long pcidebug_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

// Aliasing
struct file_operations pcidebug_ops={
    unlocked_ioctl : pcidebug_ioctl,
    open : pcidebug_open,
    release : pcidebug_release,
};

long Kprintf_SysWrite(unsigned int fd, char * buf, unsigned int buf_len)
{
    long len = -EBADF;
    struct file * file = NULL;
    //mm_segment_t old_fs;

    //check arguments
    if(NULL == buf){
        printk(KERN_ALERT"Kprintf_SysWrite: write buffer is empty!\n");
        return len;
    }

    // set file system status to kernal
    // old_fs = get_fs(); 
    // set_fs(KERNEL_DS); 

    file = fget(fd);

    if(NULL!=file){
        loff_t pos = file->f_pos;
        // len = vfs_write(file, buf, buf_len, &pos);
        len = kernel_write(file, buf, buf_len, &pos); // suit for linux 5.10
        file->f_pos = pos;
        fput(file);
    }

    // recovery file system status
    // set_fs(old_fs);
    return len;
}

// printf to user's terminal
int Kprintf(const char * fmt, ...)
{
    int printed_len = 0;
    va_list args; 

    if(NULL == fmt){
        printk(KERN_ALERT"Kprintf: fmt is empty!\n");
        return -EBADF;
    }

    memset(KprintfBuff, 0, sizeof(KprintfBuff)); //reset buff

    /* Emit the output into the temporary buffer */
    va_start(args, fmt);
    printed_len = vsnprintf(KprintfBuff, KPRINTF_MAX, fmt, args);
    va_end(args);
    if( 0 < printed_len){
        printed_len = (int)Kprintf_SysWrite(0,KprintfBuff,(unsigned int)printed_len);
    }else{
        printk(KERN_ALERT"Kprintf: something is wrong with fmt or snprintf\n");
    }
    return printed_len;
}

static u64 pcidebug_readbar(int id, u64 offset, size_t bitwidth){
    u64 result = 0;
    u32 low = 0;
    switch(bitwidth){
        case 8:
            result = ioread8(pcidebug.baseVirts[id]+offset);
            Kprintf("[KERN_INFO] %s: read8 at 0x%016llx = 0x%02x.\n",DEVICE_NAME, (size_t)pcidebug.baseVirts[id]+offset, (u8)result);
            printk(KERN_INFO"%s: read8 at 0x%016llx = 0x%02x.\n",DEVICE_NAME, (size_t)pcidebug.baseVirts[id]+offset, (u8)result);
            break;
        case 16:
            result = ioread16(pcidebug.baseVirts[id]+offset);
            Kprintf("[KERN_INFO] %s: read16 at 0x%016llx = 0x%04x.\n",DEVICE_NAME, (size_t)pcidebug.baseVirts[id]+offset, (u16)result);
            printk(KERN_INFO"%s: read16 at 0x%016llx = 0x%04x.\n",DEVICE_NAME, (size_t)pcidebug.baseVirts[id]+offset, (u16)result);
            break;
        case 32:
            result = ioread32(pcidebug.baseVirts[id]+offset);
            Kprintf("[KERN_INFO] %s: read32 at 0x%016llx = 0x%08x.\n",DEVICE_NAME, (size_t)pcidebug.baseVirts[id]+offset, (u32)result);
            printk(KERN_INFO"%s: read32 at 0x%016llx = 0x%08x.\n",DEVICE_NAME, (size_t)pcidebug.baseVirts[id]+offset, (u32)result);
            break;
        case 64:
            low = ioread32(pcidebug.baseVirts[id]+offset);
            result = ioread32(pcidebug.baseVirts[id]+offset+4);
            result = (result<<32) + low;
            Kprintf("[KERN_INFO] %s: read64 at 0x%016llx = 0x%016llx.\n",DEVICE_NAME, (size_t)pcidebug.baseVirts[id]+offset, result);
            printk(KERN_INFO"%s: read64 at 0x%016llx = 0x%016llx.\n",DEVICE_NAME, (size_t)pcidebug.baseVirts[id]+offset, result);
            break;
        default:
            Kprintf("[KERN_WARNING] %s: don't support this bitwidth!\n",DEVICE_NAME);
            printk(KERN_WARNING "%s: don't support this bitwidth!\n",DEVICE_NAME);
            return 0;
    }
    return result;
}

static u64 pcidebug_writebar(int id, u64 offset, u64 val, size_t bitwidth){
    u64 result = 0;
    u32 low = 0;
    //printk(KERN_INFO"%s: pcidebug_writebar val=0x%016llx\n",DEVICE_NAME, val);
    switch(bitwidth){
        case 8:
            Kprintf("[KERN_INFO] %s: write8 0x%02x at 0x%016llx.\n",DEVICE_NAME, (u8)val, (size_t)pcidebug.baseVirts[id]+offset);
            printk(KERN_INFO"%s: write8 0x%02x at 0x%016llx.\n",DEVICE_NAME, (u8)val, (size_t)pcidebug.baseVirts[id]+offset);
            iowrite8((u8)val,pcidebug.baseVirts[id]+offset);
            result = ioread8(pcidebug.baseVirts[id]+offset);
            Kprintf("[KERN_INFO] %s: %s, 0x%016llx = 0x%02x.\n",DEVICE_NAME, result==val?"Success":"Failure",(size_t)pcidebug.baseVirts[id]+offset, (u8)result);
            printk(KERN_INFO"%s: %s, 0x%016llx = 0x%02x.\n",DEVICE_NAME, result==val?"Success":"Failure",(size_t)pcidebug.baseVirts[id]+offset, (u8)result);
            break;
        case 16:
            Kprintf("[KERN_INFO] %s: write16 0x%04x at 0x%016llx\n",DEVICE_NAME, (u16)val, (size_t)pcidebug.baseVirts[id]+offset);
            printk(KERN_INFO"%s: write16 0x%04x at 0x%016llx\n",DEVICE_NAME, (u16)val, (size_t)pcidebug.baseVirts[id]+offset);
            iowrite16((u16)val,pcidebug.baseVirts[id]+offset);
            result = ioread16(pcidebug.baseVirts[id]+offset);
            Kprintf("[KERN_INFO] %s: %s, 0x%016llx = 0x%04x.\n",DEVICE_NAME, result==val?"Success":"Failure",(size_t)pcidebug.baseVirts[id]+offset, (u16)result);
            printk(KERN_INFO"%s: %s, 0x%016llx = 0x%04x.\n",DEVICE_NAME, result==val?"Success":"Failure",(size_t)pcidebug.baseVirts[id]+offset, (u16)result);
            break;
        case 32:
            Kprintf("[KERN_INFO] %s: write32 0x%08x at 0x%016llx\n",DEVICE_NAME, (u32)val, (size_t)pcidebug.baseVirts[id]+offset);
            printk(KERN_INFO"%s: write32 0x%08x at 0x%016llx\n",DEVICE_NAME, (u32)val, (size_t)pcidebug.baseVirts[id]+offset);
            iowrite32((u32)val,pcidebug.baseVirts[id]+offset);
            result = ioread32(pcidebug.baseVirts[id]+offset);
            Kprintf("[KERN_INFO] %s: %s, 0x%016llx = 0x%08x.\n",DEVICE_NAME, result==val?"Success":"Failure",(size_t)pcidebug.baseVirts[id]+offset, (u32)result);
            printk(KERN_INFO"%s: %s, 0x%016llx = 0x%08x.\n",DEVICE_NAME, result==val?"Success":"Failure",(size_t)pcidebug.baseVirts[id]+offset, (u32)result);
            break;
        case 64:
            Kprintf("[KERN_INFO] %s: write64 0x%016llx at 0x%016llx\n",DEVICE_NAME, val, (size_t)pcidebug.baseVirts[id]+offset);
            printk(KERN_INFO"%s: write64 0x%016llx at 0x%016llx\n",DEVICE_NAME, val, (size_t)pcidebug.baseVirts[id]+offset);
            iowrite32((u32)val,pcidebug.baseVirts[id]+offset); // write low
            iowrite32((u32)(val>>32),pcidebug.baseVirts[id]+offset+4); // write high
            low = ioread32(pcidebug.baseVirts[id]+offset); //read low
            result = ioread32(pcidebug.baseVirts[id]+offset+4); //read high
            result = (result<<32) + low;
            Kprintf("[KERN_INFO] %s: %s, 0x%016llx = 0x%016llx.\n",DEVICE_NAME, result==val?"Success":"Failure",(size_t)pcidebug.baseVirts[id]+offset, (u64)result);
            printk(KERN_INFO"%s: %s, 0x%016llx = 0x%016llx.\n",DEVICE_NAME, result==val?"Success":"Failure",(size_t)pcidebug.baseVirts[id]+offset, (u64)result);
            break;
        default:
            Kprintf("[KERN_WARNING] %s: don't support this bitwidth!\n",DEVICE_NAME);
            printk(KERN_WARNING "%s: don't support this bitwidth!\n",DEVICE_NAME);
            return 0;
    }
    return result;
}

long pcidebug_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int rc = ERROR;
    rwbar_t cmdarg;

    Kprintf("[KERN_INFO] %s: Enter pcidebug_ioctl.\n",DEVICE_NAME);
    printk(KERN_INFO"%s: Enter pcidebug_ioctl.\n",DEVICE_NAME);
    
    if(copy_from_user(&cmdarg,(rwbar_t*)arg,sizeof(rwbar_t))){
        Kprintf("[KERN_ALERT] %s: can't access cmd arg\n",DEVICE_NAME);
        printk(KERN_ALERT"%s: can't access cmd arg\n",DEVICE_NAME);
        return rc;
    }
    // check args
    if(cmdarg.barid<0 || cmdarg.barid>BARS_MAXNUM){
        Kprintf("[KERN_WARNING] %s: BAR id invalid!\n",DEVICE_NAME);
        printk(KERN_WARNING "%s: BAR id invalid!\n",DEVICE_NAME);
        return rc;
    }
    if(!pcidebug.baruseds[cmdarg.barid]){
        Kprintf("[KERN_WARNING] %s: BAR %d don't used!\n",DEVICE_NAME, cmdarg.barid);
        printk(KERN_WARNING "%s: BAR %d don't used!\n",DEVICE_NAME, cmdarg.barid);
        return rc;
    }
    if(cmdarg.offset < 0 || cmdarg.offset > pcidebug.baseLens[cmdarg.barid]){
        Kprintf("[KERN_WARNING] %s: Offset out of range!\n",DEVICE_NAME);
        printk(KERN_WARNING "%s: Offset out of range!\n",DEVICE_NAME);
        return rc;
    }
    
    Kprintf("[KERN_INFO] %s: barid=%d, offset=0x%llx, bitwidth=%d\n",DEVICE_NAME, cmdarg.barid, cmdarg.offset, cmdarg.bitwidth);
    printk(KERN_INFO"%s: barid=%d, offset=0x%llx, bitwidth=%d\n",DEVICE_NAME, cmdarg.barid, cmdarg.offset, cmdarg.bitwidth);

    switch(cmd){
        case IOCTL_RDBAR:
            cmdarg.value = pcidebug_readbar(cmdarg.barid, cmdarg.offset, cmdarg.bitwidth);
            rc = SUCCESS;
            break;
        case IOCTL_WRBAR:
            cmdarg.value = pcidebug_writebar(cmdarg.barid, cmdarg.offset, cmdarg.value, cmdarg.bitwidth);
            break;
        default:
            Kprintf(KERN_WARNING "%s: Don't support this ioctl cmd!\n",DEVICE_NAME);
            printk(KERN_WARNING "%s: Don't support this ioctl cmd!\n",DEVICE_NAME);
            break;
    }
    
    if(copy_to_user((rwbar_t*)arg,&cmdarg,sizeof(rwbar_t))){
        Kprintf("[KERN_ALERT] %s: return fail\n",DEVICE_NAME);
        printk(KERN_ALERT"%s: return fail\n",DEVICE_NAME);
        return rc;
    }
    return rc;
}

int pcidebug_open(struct inode *inode,struct file *file)
{
    if(!pcidebug.used){
        Kprintf("[KERN_WARNING] %s: device don't initialized!\n",DEVICE_NAME);
        printk(KERN_WARNING "%s: device don't initialized!\n",DEVICE_NAME);
        return (ERROR);
    }
    Kprintf("[KERN_INFO] %s: Open: module opened\n",DEVICE_NAME);
    printk(KERN_INFO "%s: Open: module opened\n",DEVICE_NAME);
    return (SUCCESS);
}

int pcidebug_release(struct inode *inode,struct file *file)
{
    if(!pcidebug.used){
        Kprintf("[KERN_WARNING] %s: device don't initialized!\n",DEVICE_NAME);
        printk(KERN_WARNING "%s: device don't initialized!\n",DEVICE_NAME);
        return (ERROR);
    }
    Kprintf("[KERN_INFO] %s: Release: module released\n",DEVICE_NAME);
    printk(KERN_INFO "%s: Release: module released\n",DEVICE_NAME);
    return (SUCCESS);
}

static int pcidebug_get_bar(int bar_id)
{
    pcidebug.baseHards[bar_id] = pci_resource_start(pcidebug.dev, bar_id);
    Kprintf("[KERN_INFO] %s: pcidebug_get_bar: BAR %d hw addr 0x%016lx\n",DEVICE_NAME, bar_id, pcidebug.baseHards[bar_id]);
    printk(KERN_INFO "%s: pcidebug_get_bar: BAR %d hw addr 0x%016lx\n",DEVICE_NAME, bar_id, pcidebug.baseHards[bar_id]);
    pcidebug.baseLens[bar_id] = pci_resource_len(pcidebug.dev, bar_id);
    Kprintf("[KERN_INFO] %s: pcidebug_get_bar: BAR %d hw len %lu\n",DEVICE_NAME, bar_id, pcidebug.baseLens[bar_id]);
    printk(KERN_INFO "%s: pcidebug_get_bar: BAR %d hw len %lu\n",DEVICE_NAME, bar_id, pcidebug.baseLens[bar_id]);
    pcidebug.baseVirts[bar_id] = pci_iomap(pcidebug.dev, bar_id, pcidebug.baseLens[bar_id]);
    if(!pcidebug.baseVirts[bar_id]){
        Kprintf("[KERN_WARNING] %s: pcidebug_get_bar: BAR %d can't remap memory.\n",DEVICE_NAME,bar_id);
        printk(KERN_WARNING "%s: pcidebug_get_bar: BAR %d can't remap memory.\n",DEVICE_NAME,bar_id);
        return (ERROR);
    }
    Kprintf("[KERN_INFO] %s: pcidebug_get_bar: BAR %d virt addr 0x%016lx.\n",DEVICE_NAME, bar_id, (size_t)pcidebug.baseVirts[bar_id]);
    printk(KERN_INFO "%s: pcidebug_get_bar: BAR %d virt addr 0x%016lx.\n",DEVICE_NAME, bar_id, (size_t)pcidebug.baseVirts[bar_id]);

    // request region
    if(pci_request_region(pcidebug.dev, bar_id, "pcidebug_Driver")!=0){
        Kprintf("[KERN_WARNING] %s: pcidebug_get_bar: Mem/IO in use.\n",DEVICE_NAME);
        printk(KERN_WARNING"%s: pcidebug_get_bar: Mem/IO in use.\n",DEVICE_NAME);
        return (ERROR);
    }
    pcidebug.haveRegion[bar_id] = true;
    Kprintf("[KERN_INFO] %s: pcidebug_get_bar done.\n",DEVICE_NAME);
    printk(KERN_INFO "%s: pcidebug_get_bar done.\n",DEVICE_NAME);
    return (SUCCESS);
}

static int pcidebug_getResource(void)
{
    int bar_id = 0;

    // enable device
    if(0 > pcim_enable_device(pcidebug.dev)){
        Kprintf("[KERN_CRIT] %s: getResource: Device not enable.\n", DEVICE_NAME);
        printk(KERN_CRIT"%s: getResource: Device not enable.\n", DEVICE_NAME);
        return (ERROR);
    }

    // map all bars
    for(bar_id = 0; bar_id < BARS_MAXNUM; bar_id++){
        if(pci_resource_len(pcidebug.dev,bar_id)>0 &&pci_resource_start(pcidebug.dev,bar_id)>0){  // used BAR
            if(0 > pcidebug_get_bar(bar_id)){
                Kprintf("[KERN_WARNING] %s: getResource: Can't get BAR %d!\n",DEVICE_NAME,bar_id);
                printk(KERN_WARNING"%s: getResource: Can't get BAR %d!\n",DEVICE_NAME,bar_id);
            }
            pcidebug.baruseds[bar_id] = true;
        }else{
            Kprintf("[KERN_WARNING] %s: getResource: BAR %d don't used\n",DEVICE_NAME,bar_id);
            printk(KERN_WARNING "%s: getResource: BAR %d don't used\n",DEVICE_NAME,bar_id);
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
        pcidebug.baruseds[i] = false;
        pcidebug.haveRegion[i] = false;
        pcidebug.baseHards[i] = 0;
        pcidebug.baseLens[i] = 0;
        pcidebug.baseVirts[i] = NULL;
    }
    Kprintf("[KERN_INFO] %s: Init: try to found first device, Vendor=0x%x Device=0x%x\n",DEVICE_NAME,vendor,device);
    printk(KERN_INFO"%s: Init: try to found first device, Vendor=0x%x Device=0x%x\n",DEVICE_NAME,vendor,device);
    pcidebug.dev = pci_get_device(vendor,device,NULL);
    if(pcidebug.dev){
        Kprintf("[KERN_INFO] %s: Init: found device\n",DEVICE_NAME);
        printk(KERN_INFO"%s: Init: found device\n",DEVICE_NAME);
        if(pcidebug_getResource() == SUCCESS){
            pcidebug.used = 1;
        }else{
            Kprintf("[KERN_WARNING] %s: Init: can't get resources!\n",DEVICE_NAME);
            printk(KERN_WARNING"%s: Init: can't get resources!\n",DEVICE_NAME);
        }
    }else{
        Kprintf("[KERN_INFO] %s: Init: not found device\n",DEVICE_NAME);
        printk(KERN_INFO"%s: Init: not found device\n",DEVICE_NAME);
        return (-ENODEV);
    }
    Kprintf("[KERN_INFO] %s: Init: finish found device\n",DEVICE_NAME);
    printk(KERN_INFO"%s: Init: finish found device\n",DEVICE_NAME);

    // Register driver as a character device
    if(0 > alloc_chrdev_region(&dev,0,1,DEVICE_NAME)){
        Kprintf("[KERN_ALERT] %s: Device Registration failed\n",DEVICE_NAME);
        printk(KERN_ALERT "%s: Device Registration failed\n",DEVICE_NAME);
        unregister_chrdev_region(dev,1);
        return (ERROR);
    }
    if( NULL == (cl = class_create(THIS_MODULE,"chardevice"))){
        Kprintf("[KERN_ALERT] %s: Class creation failed\n",DEVICE_NAME);
        printk(KERN_ALERT "%s: Class creation failed\n",DEVICE_NAME);
        class_destroy(cl);
        unregister_chrdev_region(dev,1);
        return (ERROR);
    }
    Kprintf("[KERN_INFO] %s: Init: module registered\n",DEVICE_NAME);
    printk(KERN_INFO "%s: Init: module registered\n",DEVICE_NAME);

    if( NULL == device_create(cl, NULL, dev, NULL, DEVICE_NAME)){
        Kprintf("[KERN_ALERT] %s: Device creation failed\n",DEVICE_NAME);
        printk(KERN_ALERT "%s: Device creation failed\n",DEVICE_NAME);
        class_destroy(cl);
        unregister_chrdev_region(dev,1);
        return (ERROR);
    }

    cdev_init(&chardev, &pcidebug_ops);

    if( 0 > cdev_add(&chardev, dev, 1)){
        Kprintf("[KERN_ALERT] %s: Device addition failed\n",DEVICE_NAME);
        printk(KERN_ALERT "%s: Device addition failed\n",DEVICE_NAME);
        device_destroy(cl, dev);
        class_destroy(cl);
        unregister_chrdev_region(dev,1);
        return (ERROR);
    }

    gKernelRegFlag = true;
    Kprintf("[KERN_INFO] %s: Init: Driver is load\n",DEVICE_NAME);
    printk(KERN_INFO "%s: Init: Driver is load\n",DEVICE_NAME);

    Kprintf("[KERN_INFO] %s: Init: Done\n",DEVICE_NAME);
    printk(KERN_INFO "%s: Init: Done\n",DEVICE_NAME);
    return (SUCCESS);
}

static void __exit pcidebug_exit(void)
{
    int id = 0;
    
    for(id = 0; id<BARS_MAXNUM; id++){
        if(pcidebug.used){
            //release region
            if(pcidebug.haveRegion[id]){
                pci_release_region(pcidebug.dev,id);
                Kprintf("[KERN_INFO] %s: release bar %d\n",DEVICE_NAME,id);
                printk(KERN_INFO "%s: release bar %d\n",DEVICE_NAME,id);
            }
        
            // unmap virtual device address
            if(pcidebug.baseVirts[id] != NULL){
                pci_iounmap(pcidebug.dev, pcidebug.baseVirts[id]);
                Kprintf("[KERN_INFO] %s: unmap bar %d\n",DEVICE_NAME,id);
                printk(KERN_INFO "%s: unmap bar %d\n",DEVICE_NAME,id);
            }

            pcidebug.baseVirts[id] = NULL;
        }
    }
    pcidebug.used = 0;

    if(pcidebug.dev){
        pci_disable_device(pcidebug.dev);
        Kprintf("[KERN_INFO] %s: disable device\n",DEVICE_NAME);
        printk(KERN_INFO "%s: disable device\n",DEVICE_NAME);
    }

    if(gKernelRegFlag){
        cdev_del(&chardev);
        device_destroy(cl, dev);
        class_destroy(cl);
        unregister_chrdev_region(dev,1);
        Kprintf("[KERN_ALERT] %s: Device unregistered\n",DEVICE_NAME);
        printk(KERN_ALERT "%s: Device unregistered\n",DEVICE_NAME);
    }
    Kprintf("[KERN_ALERT] %s: Driver is unloaded\n",DEVICE_NAME);
    printk(KERN_ALERT "%s: Driver is unloaded\n",DEVICE_NAME);
}

module_init(pcidebug_init);
module_exit(pcidebug_exit);