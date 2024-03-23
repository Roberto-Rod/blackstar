/* VersaLogic VersaAPI Sample Code 
*
* 20220201 - Initial release.
*
*
* This code is given with no guarantees and is intended to be used as an 
* example of how to use the VersaAPI software package and to check the 
* operation post install. This code will  work with various VersaLogic 
* single board computers and the VersaLogic VL-MPEe-A1/A2 miniPCIe board.
*
* Default Program Description: 
* This application will report the library version number and then toggle 
* an on-board DIO from HIGH to Low every three seconds for 30 seconds. 
* 
* Command Line Options: 
* Run versaAPI_sample -h for a list of command line options.
* 
* This code has been compiled and run using gcc on Ubuntu 18.04.03 LTS, 
* kernel 4.15.0-88-generic, with VersaAPI version 1.6.0.  A Linux
* compatible Makefile has been provided with this .c file for an example
* of how to compile this code. */


/* ***** Includes Files. ***** */
// System.
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

// VersaAPI header.
#include "VL_OSALib.h"
/* *************************** */

/* ***** Defines. ***** */
#define SLEEP_TIME 3 		// Time to sleep between setting the DIO.
#define VL_RETURN_SUCCESS 0	// A successful return status from a VL API call.

#define YES 1				// Define yes.
#define NO  0				// Define no.

#define TIMER_MAX_LOOP_CNT 60 // Maximum count to wait for timer to generate signal.

#ifndef CGOS_I2C_TYPE_PRIMARY
#define CGOS_I2C_TYPE_PRIMARY 0x00010000  // primary I2C bus
#endif  // CGOS_I2C_TYPE_PRIMARY
/* ******************** */


/* ***** Global variables. ***** */
int gLoopCount;			// Loop control.
int gMaxNumLoops;		// Number of times to toggle the DIO.

// Board information - set in VSL_GetProductInfo().
short gNumDIOs		= 0;
short gNumAIn		= 0;
short gNumAOut		= 0;
short gNumSerial	= 0;
short gNum8254Timers= 0;
short gFanSupport	= 0;	// Fan not supported.
unsigned char gName[24];	// Board name.
unsigned char gAttributes;	// Generic board attributes.
unsigned char gWDTSupport;	// Does board support WDT?;
unsigned char gBIOSInfo;	// BIOS information;

int gApiDebugLevel = VL_DEBUG_OFF;  // No debugging info by default.
/* ***************************** */


/* ***** Function to display application help message. ***** */
void showUsage()
{
	printf("Usage: versaAPI_sample [-bcdlrRwWxvh]\n");
	printf("Application to show basic usage of VersaAPI commands.\n");
	printf("Must specify examples(s) to run.\n");
	printf("  -b <data-byte>    => Value to write to the address of the chip.\n");
	printf("  -c <chip-address> => Specifies the address of the chip on the bus.\n");
	printf("                       Must be a hexidecimal integer between 0x03 and 0x77.\n");
	printf("  -d <data-address> => Specifies the data address (register) on the chip.\n");
	printf("                       Must be a hexidecimal integer between 0x00 and 0xFF\n");
	printf("  -l <number>       => Number of times to run each example.\n");
	printf("  -n <number>       => Number of bytes to write to the address\n");
	printf("                       of the chip.\n");
	printf("  -r => Perform a read register on the specified chip (-c) and\n");
	printf("        data (-d) addresses. Values must be in hex format (0x##).\n");
	printf("  -R => Perform a read address on the specified chip (-c) and\n");
	printf("        data (-d) addresses this number of bytes (-n). Values must\n");
	printf("        be in hex format (0x##).\n");
	printf("  -w => Perform a write register on the specified chip (-c) and\n");
	printf("        data (-d) addresses this byte value (-b). Values must be in\n");
	printf("        hex format (0x##).\n");
	printf("        The same byte value will be written to each address.\n");
	printf("  -W => Perform a write address on the specified chip (-c) and\n");
	printf("        data (-d) addresses this number of bytes (-n) this byte\n");
	printf("        value (-b). Values must be in hex format (0x##).\n");
	printf("  -x => Perform a writeread address on the specified chip (-c)\n");
	printf("        and data (-d) addresses this number of bytes (-n) this byte\n");
	printf("        value (-b). Values must be in hex format (0x##).\n");
	printf("  -v => Set API debug level to verbose. Default is no debug messages.\n");
	printf("  -h => Print this help message and exit.\n");
	printf("Default is to run no examples\n");
	
	exit(-1);
}
/* ********************************************************* */


/* ***** Function to run the I2C Read Address example. ***** */
/* Description: I2C bus is checked to see if it is available. The frequency
 * od the bus is set and checked. The registers reads and performed.         */
void runI2CAddressReadExample(unsigned char chipAddress, 
				              unsigned char dataAddress,
							  unsigned char dataLength)
{
	/* ***** Variable declarations. ***** */
	VL_APIStatusT status;
	/* ********************************** */

	/* ***** Variable initializations. ***** */
	status = VL_API_OK;
	/* ************************************* */

	/* ***** Check if the I2C bus is available. ***** */
	status = VSL_I2CIsAvailable(VL_I2C_BUS_TYPE_PRIMARY);
	if (status == VL_API_OK)
	{
		printf("\tPRIMARY I2C bus is available\n");
	}
	else
	{
		printf("\tPRIMARY I2C bus is NOT available\n");
	}

	/* ***** Run the loop. ***** */
	gLoopCount = 0;

	printf("\t\n\tStarting the I2C read loop\n");

	// The first byte of the buff MUST contain the data (register) address 
	// where the read is to start from.
	unsigned char buff[(dataLength+1)];
	buff[0] = dataAddress;

	while (gLoopCount <= gMaxNumLoops)
	{
		// Increment the loop control so we don't run forever.
		status = VSL_I2CReadAddress(VL_I2C_BUS_TYPE_PRIMARY, chipAddress, buff, dataLength);
		if (status == VL_API_OK) 
		{
			for (int i = 0; i < dataLength; i++)
		    {
				printf("\t\tLoop=%d:", gLoopCount);
			    printf("Byte %d: Value=0x%x;\n", i, buff[i]);
			}
		}
		else
		{
			printf("Value read=ERROR;\n");
		}
		// Next loop.
		gLoopCount++;
	}
	/* ************************* */

	/* ***** Done. ***** */
	return;
	/* ***************** */
}
/* ********************************************************* */

/* ***** Function to run the I2C example. ***** */
/* Description: I2C bus is checked to see if it is available. The frequency
 * od the bus is set and checked. The registers reads and performed.         */
void runI2CRegisterReadExample(unsigned char chipAddress, unsigned char registerAddress)
{
	/* ***** Variable declarations. ***** */
	VL_APIStatusT status;
	unsigned char value;
	/* ****************eclarations. ***** */

	/* ***** Variable initializations. ***** */
	status = VL_API_OK;
	value  = 0;
	/* ************************************* */

	/* ***** Check if the I2C bus is available. ***** */
	status = VSL_I2CIsAvailable(VL_I2C_BUS_TYPE_PRIMARY);
	if (status == VL_API_OK)
	{
		printf("\tPRIMARY I2C bus is available\n");
	}
	else
	{
		printf("\tPRIMARY I2C bus is NOT available\n");
	}

	/* ***** Run the loop. ***** */
	gLoopCount = 0;

	printf("\t\n\tStarting the I2C register read loop: Chip address=0x%x, Register address=0x%x\n",
		   chipAddress, registerAddress);
	while (gLoopCount <= gMaxNumLoops)
	{
	    value = 0;

		// Increment the loop control so we don't run forever.
		printf("\t\tLoop=%d:", gLoopCount);
		status = VSL_I2CReadRegister(VL_I2C_BUS_TYPE_PRIMARY, chipAddress, registerAddress, &value);
		if (status == VL_API_OK) 
		{
			printf("Value read=0x%x;\n", value);
		}
		else
		{
			printf("Value read=ERROR;\n");
		}
		// Next loop.
		gLoopCount++;
	}
	/* ************************* */

	/* ***** Done. ***** */
	return;
	/* ***************** */
}
/* ******************************************** */


/* ***** I2C Register write example. ***** */
void runI2CRegisterWriteExample(unsigned char chipAddress, 
				               unsigned char registerAddress,
							   unsigned char dataToWrite)
{
	/* ***** Variable declarations. ***** */
	VL_APIStatusT status;
	/* ****************eclarations. ***** */

	/* ***** Variable initializations. ***** */
	status = VL_API_OK;
	/* ************************************* */

	/* ***** Check if the I2C bus is available. ***** */
	status = VSL_I2CIsAvailable(VL_I2C_BUS_TYPE_PRIMARY);
	if (status == VL_API_OK)
	{
		printf("\tPRIMARY I2C bus is available\n");
	}
	else
	{
		printf("\tPRIMARY I2C bus is NOT available\n");
	}

	/* ***** Run the loop. ***** */
	gLoopCount = 0;

	printf("\t\n\tStarting the I2C register read loop: Chip address=0x%x, Register address=0x%x\n",
		   chipAddress, registerAddress);
	while (gLoopCount <= gMaxNumLoops)
	{
		// Increment the loop control so we don't run forever.
		printf("\t\tLoop=%d:", gLoopCount);
		status = VSL_I2CWriteRegister(VL_I2C_BUS_TYPE_PRIMARY, chipAddress, 
						              registerAddress, dataToWrite);
		if (status == VL_API_OK) 
		{
			printf("\tValue written OK.\n");
		}
		else
		{
			printf("\tValue write ERROR;\n");
		}
		// Next loop.
		gLoopCount++;
	}
	/* ************************* */

	/* ***** Done. ***** */
	return;
	/* ***************** */
}
/* *************************************** */


/* ***** I2C Register write example. ***** */
void runI2CAddressWriteExample(unsigned char chipAddress, 
				               unsigned char dataLength,
							   unsigned char *dataToWrite)
{
	/* ***** Variable declarations. ***** */
	VL_APIStatusT status;
	/* ********************************** */

	/* ***** Variable initializations. ***** */
	status = VL_API_OK;
	/* ************************************* */

	/* ***** Check if the I2C bus is available. ***** */
	status = VSL_I2CIsAvailable(VL_I2C_BUS_TYPE_PRIMARY);
	if (status == VL_API_OK)
	{
		printf("\tPRIMARY I2C bus is available\n");
	}
	else
	{
		printf("\tPRIMARY I2C bus is NOT available\n");
	}

	/* ***** Run the loop. ***** */
	gLoopCount = 0;

	printf("\t\n\tStarting the I2C address write example: Chip address=0x%x, data length=0x%x\n",
		   chipAddress, dataLength);
	while (gLoopCount <= gMaxNumLoops)
	{
		// Increment the loop control so we don't run forever.
		printf("\t\tLoop=%d:", gLoopCount);
		status = VSL_I2CWriteAddress(VL_I2C_BUS_TYPE_PRIMARY, chipAddress, 
						             dataToWrite, dataLength);
		if (status == VL_API_OK) 
		{
			printf("\tValue written OK.\n");
		}
		else
		{
			printf("\tValue write ERROR;\n");
		}
		// Next loop.
		gLoopCount++;
	}
	/* ************************* */

	/* ***** Done. ***** */
	return;
	/* ***************** */
}
/* *************************************** */


/* ***** I2C Register write example. ***** */
void runI2CAddressWriteReadExample(unsigned char chipAddress, 
				               unsigned char dataLength,
							   unsigned char *dataToWrite)
{
	/* ***** Variable declarations. ***** */
	VL_APIStatusT status;
	unsigned char dataRead[(dataLength+1)];
	/* ********************************** */

	/* ***** Variable initializations. ***** */
	status = VL_API_OK;
	/* ************************************* */

	/* ***** Check if the I2C bus is available. ***** */
	status = VSL_I2CIsAvailable(VL_I2C_BUS_TYPE_PRIMARY);
	if (status == VL_API_OK)
	{
		printf("\tPRIMARY I2C bus is available\n");
	}
	else
	{
		printf("\tPRIMARY I2C bus is NOT available\n");
	}

	/* ***** Run the loop. ***** */
	gLoopCount = 0;

	printf("\t\n\tStarting the I2C address write example: Chip address=0x%x, data length=0x%x\n",
		   chipAddress, dataLength);
	while (gLoopCount <= gMaxNumLoops)
	{
		// Create the data to write.
		printf("\t\tWriting data:\n");
		for (int i = 0; i <= dataLength; i ++)
		{
		    printf("\t\t\t%d:0x%x;\n", i , dataToWrite[i]);
		}
		
		// Remember the first byte of a write/readAddress() is the data address
		// (register). It was set in the write buffer, now set it in the read
		// buffer.
	    dataRead[0] = dataToWrite[0];	
		status = VSL_I2CWriteReadCombined(VL_I2C_BUS_TYPE_PRIMARY, chipAddress, 
						             dataToWrite, dataLength,
									 dataRead,    dataLength);
		if (status != VL_API_OK) 
		{
			printf("\tValue(s) written OK.\n");
		}

		// Report what was written and what was read.
		printf("\t\tReading data\n");
		for (int i = 0; i < dataLength; i ++)
		{
		    printf("\t\t\t%d:0x%x;\n", i , dataRead[i]);
		}
		
		
		// Next loop.
		gLoopCount++;
	}
	/* ************************* */

	/* ***** Done. ***** */
	return;
	/* ***************** */
}
/* *************************************** */


/* ***** Function to report generic board information. ***** */
/* Description: Each loop will shows the WDT expire and how to 'pet' the WDT.
 * Expire example configures the WDT for 10 seconds, enables (starts) the WDT
 * and let's it expire, showing how the state changes once it expires (fires).
 * The second part of the loop test again sets the WDT for an expire time, but
 * 'pets' it every 5 seconds, showing that the WDT never expires (fires).    */
void reportBoardInfo()
{
    VSL_GetProductInfo((unsigned long)0x1, gName, &gAttributes, &gNumDIOs,
					   &gNum8254Timers, &gWDTSupport, &gNumAIn, &gNumAOut, &gNumSerial,
					   &gFanSupport, &gBIOSInfo);
	printf("Board Info:\n");
	printf("  Name:%s\n  Number of DIO/GPIOs:%d\n  Number of Analog Inputs:%d\n",
			gName, gNumDIOs, gNumAIn);
	printf("  Number of Analog Outputs:%d\n  Number of Serial ports:%d\n",
			gNumAOut, gNumSerial);
	printf("  Number of 8254 Timers:%d\n", gNum8254Timers);

	if (gWDTSupport == 0)
		printf("  Watchdog Timer Support:No\n");
	else
		printf("  Watchdog Timer Support:Yes\n");

	if (gFanSupport == 0)
		printf("  Onboard Fan Command Support:No\n");
	else
		printf("  Onboard Fan Command Support:Yes\n");

	printf("  Board Attributes:0x%x\n", gAttributes);
	printf("\t");				
	if ((gAttributes & 0x1) == 0)
	{
		printf("Production, ");
	}
	else
	{
		printf("Beta, ");
	}
	if ((gAttributes & 0x2) == 2)
	{
		printf("Custom board, ");
	}
	else
	{
		printf("Standard board, ");
	}
	if ((gAttributes & 0x4) == 4)
	{
		printf("Extended Temperature, ");
	}
	else
	{
		printf("Standard Temperature, ");
	}
	printf("Board Revision Level: 0x%x\n", 
           ((gAttributes & 0xF8) >> 3));
}
/* ********************************************************* */


/* ***** main program. ***** */
int main(int argc, char *argv[])
{
    /* ***** Variable declarations. ***** */
    unsigned long vl_ReturnStatus;	// Return status.
    
    unsigned char majorRev;			// Major revision number.
    unsigned char minorRev;			// Minor revision number.
    unsigned char releaseRev;		// Release revision number.
    
    int			   opt;				// Command line option helper.
    int			   doRegRead;		// Execute 8254 timer code. Default is No.
    int			   doAddRead;		// Execute simple DIO code. Default is No.
    int			   doRegWrite;	    // Execute loopback DIO code. Default is No.
    int			   doAddWrite;	    // Execute loopback DIO code using A1/2. Default is No.
    int			   doRegWriteRead;	// Execute simple DIO code using U2. Default is No.

    unsigned char dataLength;  // Number of bytes of data to read/write.
    unsigned char chipAddress; // I2C chip address.
    unsigned char dataAddress; // Register/data address.
	unsigned char actualData;  // Data to write to register/address.
    /* ********************************** */

    /* ***** Variable initializations. ***** */
    vl_ReturnStatus = 0xffffff;	// Assume a failure on open.
    majorRev		= (unsigned char)0;
    minorRev		= (unsigned char)0;
    releaseRev		= (unsigned char)0;
	
	opt	= 0;  // Process command line options.
	
	// By default, do not execute any tests.
	doRegRead			= NO;
    doAddRead			= NO;	
    doRegWrite  		= NO;	
    doAddWrite  		= NO;
    doRegWriteRead		= NO;

	// Set default addresses.
	dataLength  = 0;
	dataAddress = 0;
	chipAddress = 0;
	actualData  = 0;
    /* ************************************* */

    /* ***** Process command line options. ***** */
	if(argc <= 1)
	{
		showUsage();
		exit(-1);
	}

	// There is some functionality to be shown.
    while ((opt = getopt(argc, argv, "b:c:d:l:n:rRhvwWxv")) != -1)
    {
		switch(opt)
		{
			case 'b': 	actualData     = strtol(optarg, NULL, 16);
						break;
			case 'c': 	chipAddress    = strtol(optarg, NULL, 16);
						break;
			case 'd': 	dataAddress    = strtol(optarg, NULL, 16);
						break;
			case 'l': 	gMaxNumLoops    = atoi(optarg);
						break;
			case 'n': 	dataLength     = atoi(optarg);
						break;
			case 'r': 	doRegRead       = YES;
						break;
			case 'R': 	doAddRead       = YES;
						break;
			case 'v': 	gApiDebugLevel  = VL_DEBUG_1;
						break;
			case 'w': 	doRegWrite      = YES;
						break;
			case 'W': 	doAddWrite      = YES;
						break;
			case 'x':	doRegWriteRead  = YES;
						break;
			default:	showUsage();
						exit(-1);
		}
	}
    /* ***************************************** */

	/* ***** Set the debug level. ****** */
	VSL_DebugLevelSet(gApiDebugLevel);
	/* ********************************* */

	/* ***** Make sure command line arguments are OK. */
	// All commands need a valid chip and address addresses. Check once here.
	if ((chipAddress < 0x03) || (chipAddress > 0x77))
	{
		printf("Error: Chip address must be specified and 0x00 <= chip-address <= 0x77.\n");
		printf("    Specified chip address = 0x%x\n", chipAddress);
		showUsage();
		exit(-1);
	}

	// Register functions need data (register) address.
	if ((doRegRead  == YES)  || 
		(doRegWrite == YES))
	{
	    if ((dataAddress < 0x00) || (dataAddress > 0xFF))
	    {
		    printf("Error: Data (register) address must be specified and 0x00 <= data-address <= 0xFF.\n");
		    printf("    Specified data address = 0x%x\n", dataAddress);
		    showUsage();
		    exit(-1);
	    }
	}
	
	// Address Read.
	if ((doAddRead  == YES) || 
		(doAddWrite == YES) || 
	    (doRegWriteRead == YES))
	{
		// Doing an address read. 
		if (dataLength < 1)
		{
			printf("Error: Data length must be specified and > 0.\n");
		    printf("    Specified data length = 0x%x\n", dataLength);
			showUsage();
			exit(-1);
		}
	}
	/* ********************************************** */

    /* **** Open VersaAPI library. **** */
    /* The library must be successfully opened before it can be used. */
    vl_ReturnStatus = VL_Open();
    if (vl_ReturnStatus == VL_RETURN_SUCCESS)
    {
        // Opened the library OK, display it's version number.
        VL_GetVersion(&majorRev, &minorRev, &releaseRev);
        printf("VersaAPI library version: %x.%x.%x;\n", majorRev, minorRev, releaseRev);
    }
    else
    {
        printf("VersaAPI Library did NOT opened successfully: rc=0x%lx.\n", vl_ReturnStatus);
	    return(-1);
    }
    /* ******************************** */

	/* ***** Get and report on board information. ***** */
	reportBoardInfo();
	/* ************************************************ */

	/* ***** I2C Register Read Example. ***** */
	if (doRegRead == YES)
	{
		printf("\n********* Start VersaLogic I2C Register Read Example *********\n");

		// Run the loop.
		runI2CRegisterReadExample(chipAddress, dataAddress);

		printf("********** End VersaLogic I2C Register Read Example ***********\n");
	}
	/* ************************************** */

	/* ***** I2C Address Read Example. ***** */
	if (doAddRead == YES)
	{
		printf("\n********* Start VersaLogic I2C Address Read Example *********\n");

		// Run the loop.
		runI2CAddressReadExample(chipAddress, dataAddress, dataLength);

		printf("*********** End VersaLogic I2C Address Read Example ***********\n");
	}
	/* ******************************* */

	/* ***** I2C Register Write Example. ***** */
	if (doRegWrite == YES)
	{
		printf("\n********* Start VersaLogic I2C Register Write Example *********\n");

		// Run the loop.
		runI2CRegisterWriteExample(chipAddress, dataAddress, actualData);

		printf("*********** End VersaLogic I2C Register Write Example ***********\n");
	}
	/* *************************************** */

	/* ***** I2C Register Address Write Example. ***** */
	if (doAddWrite == YES)
	{
		printf("\n********* Start VersaLogic I2C Address Write Example *********\n");

		// Fill in the data array.
		unsigned char dataArray[100];

		// First byte MUST be the offset (register) to start the write.
		dataArray[0] = dataAddress;
		for (int i = 1; i <= dataLength; i++)
		{
			dataArray[i] = actualData;
		}
		
		// Run the loop.
		runI2CAddressWriteExample(chipAddress, dataLength, dataArray);

		printf("*********** End VersaLogic I2C Address Write Example ***********\n");
	}
	/* *********************************************** */

	/* ***** I2C Register Write/Read Example. ***** */
	if (doRegWriteRead == YES)
	{
		printf("\n********* Start VersaLogic I2C Register Write/Read Example *********\n");

		// Fill in the data array.
		unsigned char dataArray[100];
		dataArray[0] = dataAddress;
		for (int i = 1; i <= dataLength; i++)
		{
			dataArray[i] = actualData;
		}
		
		// Run the loop.
		runI2CAddressWriteReadExample(chipAddress, dataLength, dataArray);

		printf("*********** End VersaLogic I2C Register Write/Read Example ***********\n");
	}
	/* ******************************************** */

    /* ***** VL_Close() should be called at the end of the program. ***** */
    VL_Close();
    
    printf("\nEnd of VersaLogic Example Application execution.\n");

    return(0);
}

