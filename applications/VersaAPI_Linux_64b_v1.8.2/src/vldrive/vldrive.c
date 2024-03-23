/*
DIO
 * vldrive.c
 *
 *  Created on: Aug 20, 2013
 *      Author: Andy Dowie
 *      20221202: VersaAPI Release 1.8.2
 *          Added DIO interrupt handling for boards with more than 8 DIO/GPIO.
 *          This will be improved upon next release of the API.
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/siginfo.h>
#include <asm/io.h>
#include <linux/rcupdate.h>
#include <linux/debugfs.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#else
#include <asm/uaccess.h>
#include <linux/sched.h>
#endif

#include "vldrive.h"

static dev_t dev;
static struct cdev c_dev;
static struct class *VLDriveClass;
pid_t	VLTMRPid;
pid_t	VLDIOPid;

unsigned char IRQNum = FPGA_IRQ_DEF;
unsigned short FPGA_BASE = FPGA_BASE_DEF;
module_param(IRQNum, byte, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(FPGA_BASE, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

MODULE_PARM_DESC(IRQNum, " This parameter tells the SBC FPGA which IRQ to use when interrupting the SBC. Default = IRQ 5");
MODULE_PARM_DESC(FPGA_BASE, " This parameter tells the SBC FPGA which I/O space to use on the LPC bus. Default = 0xCA0");
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
	long	   IRQData;

    switch (cmd)
    {
    	case VSL_IOCTL_PASS_TIMER_PID:
			if (arg <= 0)
			{
			   printk(KERN_INFO "vldrive: PID for 8254 Timers not specified, error in API command\n");
        	   return -EACCES;
			}
			else
			{
			   printk(KERN_INFO "vldrive: PID for 8254 Timers specified\n");
			}

			VLTMRPid = arg;
    		break;

    	case VSL_IOCTL_PASS_DIO_PID:
			if (arg < 0)
			{
			   printk(KERN_INFO "vldrive: PID for DIO/GPIOs not specified, error in API command\n");
        	   return -EACCES;
			}
			else
			{
			   printk(KERN_INFO "vldrive: PID for DIO/GPIOs specified\n");
			}
			VLDIOPid = arg;
    		break;

    	case VSL_IOCTL_GET_TMR_IRQ:
    	case VSL_IOCTL_GET_DIO_IRQ:
        	if (copy_from_user(&IRQData, (long *)arg, sizeof(long)))
        	{
			   printk(KERN_INFO "vldrive: IRQ: Error from copy_from_user();\n");
        	   return -EACCES;
        	}

			printk(KERN_INFO "vldrive: RQ: Successful from copy_from_user();\n");
        	IRQData = IRQNum;
            printk(KERN_INFO "vldrive: IRQData=%ld;\n", IRQData);

            if (copy_to_user((long *)arg, &IRQData, sizeof(long)))
            {
			    printk(KERN_INFO "vldrive: IRQNUM: Error from copy_to_user();\n");
                return -EACCES;
            }
			printk(KERN_INFO "vldrive: IRQNUM: Successful from copy_to_user();\n");
            break;

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


static 

irq_handler_t VSL_IntHandler(int irq, void * dev_id, struct pt_regs *regs)
{
	// Declarations and Initializations. 
    volatile unsigned char CurrentTMRInts   = 0;
    volatile unsigned char CurrentDIOInts   = 0;
    volatile unsigned char IntState         = 0;
    volatile unsigned char FPGAValue        = 0;
    volatile unsigned int  CurrentFPGABase  = 0;
    volatile unsigned int  TMRIRQStatusAdd  = 0;
    volatile unsigned int  TMRIRQControlAdd = 0;
    volatile unsigned int  DIOIRQStatusAdd  = 0;
    volatile unsigned int  DIOIRQControlAdd = 0;
	volatile irq_handler_t returnValue      = (irq_handler_t)IRQ_NONE;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0))
    struct siginfo TMRSig, DIOSig;
#else
    struct kernel_siginfo TMRSig, DIOSig;
#endif
    struct task_struct *Task;

	// Set the correct FPGA_BASE value. The EPME-51 has the DIO and 8254 
	// Timer on the IO board (not the CPU board), so the FPGA_BASE needs
	// to be adjusted appropriately.
	FPGAValue = inb(FPGA_BASE);
	if ((FPGAValue & 0x7F) == 0x24)
	{
		CurrentFPGABase  = FPGA_BASE + 0x40;
	}
	else
	{
		CurrentFPGABase = FPGA_BASE;
	}

	// Gather 8254 Timer interrupt status.
	TMRIRQControlAdd = CurrentFPGABase + IRQ_CONTROL_OFF;
	TMRIRQStatusAdd  = CurrentFPGABase + IRQ_STATUS_OFF;

	// Gather DIO interrupt status.
	/* ***** Uncomment to set the desired DIO channels that will generate 
	 *       interrupts. Must also comment-out the two current lines.
	 *       This will be automated in the next VersaAPI release.
	// Channels 1-8.
	DIOIRQControlAdd = CurrentFPGABase + IRQ_DIO_MASK_1_8_OFF;
	DIOIRQStatusAdd  = CurrentFPGABase + IRQ_DIO_STATUS_1_8_OFF;
	// Channels 9-16.
	DIOIRQControlAdd = CurrentFPGABase + IRQ_DIO_MASK_9_16_OFF;
	DIOIRQStatusAdd  = CurrentFPGABase + IRQ_DIO_STATUS_9_16_OFF;
	// AUX channels.
	DIOIRQControlAdd = CurrentFPGABase + IRQ_DIO_MASK_AUX_OFF;
	DIOIRQStatusAdd  = CurrentFPGABase + IRQ_DIO_STATUS_AUX_OFF;
	***** */
	// DIO AUX channels.
	DIOIRQControlAdd = CurrentFPGABase + IRQ_DIO_MASK_AUX_OFF;
	DIOIRQStatusAdd  = CurrentFPGABase + IRQ_DIO_STATUS_AUX_OFF;

	// Retrieve values.
    CurrentTMRInts  = inb(TMRIRQStatusAdd);
    CurrentDIOInts  = inb(DIOIRQStatusAdd);
    IntState        = inb((CurrentFPGABase + SPI_STATUS_OFF));

	if (CurrentTMRInts & 0x07)
	{
	    printk(KERN_INFO "vldrive: VSL_IntHandler: 8254 timer interrupt generated:0x%x\n", CurrentTMRInts);

		// turn off any timers that have signalled
		outb((volatile unsigned char)((inb(TMRIRQControlAdd)) | (CurrentTMRInts & 7)), TMRIRQControlAdd);

		// Some TMR bit is set. Clear the bits and signal the user's signal handler with the data
		memset(&TMRSig, 0, sizeof(TMRSig));
		TMRSig.si_signo = SIG_VL_TIMER;
		TMRSig.si_code  = SI_QUEUE;
		TMRSig.si_int   = CurrentTMRInts; // Sets the counter that triggered the IRQ.
		rcu_read_lock();
		//
		// find_vl_pidId();
		Task = pid_task(find_pid_ns(VLTMRPid, &init_pid_ns), PIDTYPE_PID);
		if(Task == NULL)
		{
			// Nothing to do. User task has gone.
			printk(KERN_INFO "vldrive: No application to signal, user task has terminated.\n");
			rcu_read_unlock();
			
			// Clear the interrupt status register.
			outb(0xFF, TMRIRQStatusAdd);
			returnValue = IRQ_NONE;
		}
		else
		{
		    rcu_read_unlock();
		    send_sig_info(SIG_VL_TIMER, &TMRSig, Task);
		    outb((IntState | 0x08), (CurrentFPGABase + 9));
		    outb((CurrentTMRInts & 7), (CurrentFPGABase + 4));		
		    returnValue = (irq_handler_t)IRQ_HANDLED;
		}
	}

    if (CurrentDIOInts >= 1)
    {
	    printk(KERN_INFO "vldrive: VSL_IntHandler: GPIO/DIO interrupt generated:0x%x\n", CurrentDIOInts);
        memset(&DIOSig, 0, sizeof(DIOSig));
        DIOSig.si_signo = SIG_VL_DIO;
        DIOSig.si_code  = SI_QUEUE;
        DIOSig.si_int   = CurrentDIOInts;

        rcu_read_lock();
        Task = pid_task(find_pid_ns(VLDIOPid, &init_pid_ns), PIDTYPE_PID);
        if(Task == NULL)
        {
            // Nothing to do. User task has gone.
            printk(KERN_INFO "vldrive: No application to signal, user task has terminated.\n");
            rcu_read_unlock();
			
			// Clear the interrupt status register.
            printk(KERN_INFO "vldrive: Found application to signal, clearing IRQ Status register.\n");
			outb(0xFF, DIOIRQStatusAdd);
            returnValue = IRQ_NONE;
        }
		else
		{
            rcu_read_unlock();
            send_sig_info(SIG_VL_DIO, &DIOSig, Task);

			// Clear the Status regisiter.
			outb(0xFF, DIOIRQStatusAdd);
            returnValue = (irq_handler_t)IRQ_HANDLED;
		}
    }

	// Finished.
    return (irq_handler_t)returnValue;
}


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

	// Print some useful information. 
	printk(KERN_INFO "vldrive: Installing VersaLogic baseboard driver"); 

    if ((ret = alloc_chrdev_region(&dev, VLDRIVE_MINOR, VLDRIVE_CNT, "vldrive")) < 0)
    {
    	printk(KERN_INFO "vldrive: Could not allocate driver region");
        return ret;
    }

    cdev_init(&c_dev, &query_fops);

    if ((ret = cdev_add(&c_dev, dev, VLDRIVE_CNT)) < 0)
    {
    	printk(KERN_INFO "vldrive: Could not initialize device");
        return ret;
    }

    if (IS_ERR(VLDriveClass = class_create(THIS_MODULE, "versa")))
    {
    	printk(KERN_INFO "vldrive: Failed to create class versa");

    }
	else
	{
    	printk(KERN_INFO "vldrive: Creating device");

		/* Need control of the actual /dev device created. */
		VLDriveClass->devnode = vldevnode;

		if (IS_ERR(dev_ret = device_create(VLDriveClass, NULL, dev, NULL, "vldrive")))
		{
			printk(KERN_INFO "vldrive: Failed to create device vldrive");
			class_destroy(VLDriveClass);
			cdev_del(&c_dev);
			unregister_chrdev_region(dev, VLDRIVE_CNT);
			return PTR_ERR(dev_ret);
		}
		else
		{
			printk(KERN_INFO "vldrive: Device created");
		}
	}

    /*
     *  Grab access to the FPGA I/O area so the reads and writes can have access
     */
    printk(KERN_INFO "vldrive: Requesting FPGA region for use: 0x%04x\n", FPGA_BASE);

    if(!request_region(FPGA_BASE, 32, "versalogic-vldrive"))
    {
         printk(KERN_INFO "vldrive: Could not obtain requested FPGA region\n");
         return(-1);
    }
	else
    {
         printk(KERN_INFO "vldrive: Obtain requested FPGA region\n");
    }

    /*
     *  Grab access to the IRQ so the Timers and DIO can use the interrupt to signal user code
     */
	printk(KERN_INFO "vldrive: Requesting IRQ for use: %04x", IRQNum);
	
    if(IRQNum)
    {
    	ret = request_irq(IRQNum, (void *) VSL_IntHandler, 0, "vldrive", NULL);
    	if (ret) 
		{
             printk(KERN_INFO "vldrive: Could NOT obtain requested IRQ\n");
             return(-1);
    	}
		else
		{
             printk(KERN_INFO "vldrive: Obtained requested IRQ\n");
		}
    }
    printk(KERN_INFO "vldrive: VersaLogic driver installation complete.\n");

    return 0;
}


static void __exit vldrive_exit(void)
{
    device_destroy(VLDriveClass, dev);
    class_destroy(VLDriveClass);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, VLDRIVE_CNT);
    release_region(FPGA_BASE, 32);
    free_irq(IRQNum, NULL);
    printk(KERN_INFO "vldrive: VersaLogic driver removed.\n");

}

module_init(vldrive_init);
module_exit(vldrive_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VersaLogic Corporation <Support@Versalogic.com>");
MODULE_DESCRIPTION("VersaAPI Driver");
