/*
 * vldrivep.h
 *
 *  Created on: Aug 19, 2013
 *      Author: Andy Dowie
 */

#ifndef VLDRIVE_IOCTL_H
#define VLDRIVE_IOCTL_H
#include <linux/ioctl.h>

typedef struct
{
	unsigned char	Address;
	union {
	unsigned char  	Data8;
	unsigned short 	Data16;
	unsigned long  	Data32;
	}Passed;
} VLDriveArg;


#define	IOCTL_VSL_READ_PORT_UCHAR	_IOWR(0xAA, 1, VLDriveArg *)
#define	IOCTL_VSL_READ_PORT_USHORT  _IOWR(0xAA, 2, VLDriveArg *)
#define	IOCTL_VSL_READ_PORT_ULONG   _IOWR(0xAA, 3, VLDriveArg *)

#define	IOCTL_VSL_WRITE_PORT_UCHAR	_IOWR(0xAA, 4, VLDriveArg *)
#define	IOCTL_VSL_WRITE_PORT_USHORT _IOWR(0xAA, 5, VLDriveArg *)
#define	IOCTL_VSL_WRITE_PORT_ULONG  _IOWR(0xAA, 6, VLDriveArg *)

#define	IOCTL_VSL_READ_PORT_UCHAR_AX	_IOWR(0xAA, 11, VLDriveArg *)
#define	IOCTL_VSL_READ_PORT_USHORT_AX  	_IOWR(0xAA, 12, VLDriveArg *)
#define	IOCTL_VSL_READ_PORT_ULONG_AX   	_IOWR(0xAA, 13, VLDriveArg *)

#define	IOCTL_VSL_WRITE_PORT_UCHAR_AX	_IOWR(0xAA, 14, VLDriveArg *)
#define	IOCTL_VSL_WRITE_PORT_USHORT_AX 	_IOWR(0xAA, 15, VLDriveArg *)
#define	IOCTL_VSL_WRITE_PORT_ULONG_AX  	_IOWR(0xAA, 16, VLDriveArg *)

#define VLDRIVE_MINOR	0xAD
#define VLDRIVE_CNT		1
#endif






