/*
 * vldrivep.c
 *
 *  Created on: Aug 20, 2013
 *      Author: Andy Dowie
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <asm/uaccess.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#else
#include <asm/uaccess.h>
#include <linux/sched.h>
#endif

#include "vldrivep.h"

#define VENDOR_ID 0x11AA
#define DEVICE_ID 0x0847
#define MPEE_AX_SUBSYSTEM_DEVICE_ID 0x847
#define EBX_18_SUBSYSTEM_DEVICE_ID 0x10
#define EPM_19_SUBSYSTEM_DEVICE_ID 0x12


dev_t dev;
struct cdev c_dev;
static struct class *VLDriveClass;


unsigned long FPGA_BASE = 0x00;


struct pci_device_id  VLDrive_PCI_Table[] =
{
  {VENDOR_ID, DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
  {}  // end of list
};

/* This function is called by the PCI core when it has a struct pci_dev 
 * that it thinks this driver wants to control. A pointer to the struct 
 * pci_device_id that the PCI core used to make this decision is also 
 * passed to this function. If the PCI driver claims the struct pci_dev 
 * that is passed to it, it should initialize the device properly and 
 * return 0. If the driver does not want to claim the device, or an error 
 * occurs, it should return a negative error value.
 * */
int device_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
  int ret;
  
  // Make sure this driver is only called for any PCI device except A1 or A2.
  if ((*dev).subsystem_device == MPEE_AX_SUBSYSTEM_DEVICE_ID)
  {
	  printk(KERN_INFO "vldrivep: Ignoring device: venId=0x%x; devId=0x%x; svenId=0x%x; sdevId=0x%x.\n", 
	         (*dev).vendor, (*dev).device, (*dev).subsystem_vendor, (*dev).subsystem_device);
	  return -1;
  }
  else
  {
	  printk(KERN_INFO "vldrivep: Found device: venId=0x%x; devId=0x%x; svenId=0x%x; sdevId=0x%x.\n", 
	         (*dev).vendor, (*dev).device, (*dev).subsystem_vendor, (*dev).subsystem_device);
  }

  ret = pci_enable_device(dev);
  if (ret < 0) return ret;

  ret = pci_request_regions(dev, "vldrivep");
  if (ret < 0)
  {
    pci_disable_device(dev);
    return ret;
  }

  FPGA_BASE = pci_resource_start(dev, 0);
  printk(KERN_INFO "vldrivep: Setting FPGA_BASE=0x%x;\n", (unsigned int)FPGA_BASE);

  return 0;
}

void device_remove(struct pci_dev *dev)
{
  pci_release_regions(dev);
  pci_disable_device(dev);
}

struct pci_driver VLDrive_PCI =
{
  name: "vldrivep",
  id_table: VLDrive_PCI_Table,
  probe: device_probe,
  remove: device_remove
};


static int vl_open(struct inode *i, struct file *f)
{
    return 0;
}
static int vl_close(struct inode *i, struct file *f)
{
    return 0;
}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int vl_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long vl_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
	VLDriveArg VLArgs;

    switch (cmd)
    {
        case IOCTL_VSL_READ_PORT_UCHAR:
        	if (copy_from_user(&VLArgs, (VLDriveArg *)arg, sizeof(VLDriveArg)))
        	{
        	   return -EACCES;
        	}
			VLArgs.Passed.Data8 = inb((FPGA_BASE + VLArgs.Address));

            if (copy_to_user((VLDriveArg *)arg, &VLArgs, sizeof(VLDriveArg)))
            {
                return -EACCES;
            }
            break;
        case IOCTL_VSL_READ_PORT_USHORT:

        	if (copy_from_user(&VLArgs, (VLDriveArg *)arg, sizeof(VLDriveArg)))
        	{
        	   return -EACCES;
        	}

        	VLArgs.Passed.Data16 = inw((FPGA_BASE + VLArgs.Address));


            if (copy_to_user((VLDriveArg *)arg, &VLArgs, sizeof(VLDriveArg)))
            {
                return -EACCES;
            }
            break;
        case IOCTL_VSL_READ_PORT_ULONG:

        	if (copy_from_user(&VLArgs, (VLDriveArg *)arg, sizeof(VLDriveArg)))
        	{
        	   return -EACCES;
        	}

        	VLArgs.Passed.Data32 = inl((FPGA_BASE + VLArgs.Address));


            if (copy_to_user((VLDriveArg *)arg, &VLArgs, sizeof(VLDriveArg)))
            {
                return -EACCES;
            }
            break;

        case IOCTL_VSL_WRITE_PORT_UCHAR:

            if (copy_from_user(&VLArgs, (VLDriveArg *)arg, sizeof(VLDriveArg)))
            {
                return -EACCES;
            }
            outb(VLArgs.Passed.Data8, (FPGA_BASE + VLArgs.Address));
            break;

        case IOCTL_VSL_WRITE_PORT_USHORT:

            if (copy_from_user(&VLArgs, (VLDriveArg *)arg, sizeof(VLDriveArg)))
            {
            	return -EACCES;
            }
            outw(VLArgs.Passed.Data16, (FPGA_BASE + VLArgs.Address));
            break;

        case IOCTL_VSL_WRITE_PORT_ULONG:

            if (copy_from_user(&VLArgs, (VLDriveArg *)arg, sizeof(VLDriveArg)))
            {
                return -EACCES;
            }
            outl(VLArgs.Passed.Data32, (FPGA_BASE + VLArgs.Address));
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static struct file_operations query_fops =
{
    .owner = THIS_MODULE,
    .open = vl_open,
    .release = vl_close,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl = vl_ioctl
#else
    .unlocked_ioctl = vl_ioctl
#endif
};


/* Function to change "properties" of the created /dev device. */
static char *vldevnode(struct device *dev, umode_t *mode)
{
	if (mode)
	{
		*mode = 0666;  /* (read/write to all); */
	}

	/* Finished. */
	return NULL;

}


static int __init vldrive_init(void)
{
    int ret;
    struct device *dev_ret;

    if ((ret = alloc_chrdev_region(&dev, VLDRIVE_MINOR, VLDRIVE_CNT, "vldrivep")) < 0)
    {
        return ret;
    }

    cdev_init(&c_dev, &query_fops);

    if ((ret = cdev_add(&c_dev, dev, VLDRIVE_CNT)) < 0)
    {
        return ret;
    }


    if (IS_ERR(VLDriveClass = class_create(THIS_MODULE, "versap")))
    {

    	printk(KERN_INFO "vldrivep: Failed to create class versap");

    }
    else
    {
		/* Need control of the actual /dev device create. */
		VLDriveClass->devnode = vldevnode;

		if (IS_ERR(dev_ret = device_create(VLDriveClass, NULL, dev, NULL, "vldrivep")))
		{
	    	printk(KERN_INFO "vldrivep: Failed to create device vldrivep");
			class_destroy(VLDriveClass);
			cdev_del(&c_dev);
			unregister_chrdev_region(dev, VLDRIVE_CNT);
			return PTR_ERR(dev_ret);
		}
    }

    printk(KERN_INFO "vldrivep: VersaLogic PCI Express driver installed.\n");

    return pci_register_driver(&VLDrive_PCI);
}

static void __exit vldrive_exit(void)
{
    device_destroy(VLDriveClass, dev);
    class_destroy(VLDriveClass);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, VLDRIVE_CNT);
    pci_unregister_driver(&VLDrive_PCI);
    printk(KERN_INFO "vldrivep: VersaLogic PCI Express driver removed.\n");

}

module_init(vldrive_init);
module_exit(vldrive_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VersaLogic Corporation <Support@Versalogic.com>");
MODULE_DESCRIPTION("VersaAPI PCI Express Driver");
