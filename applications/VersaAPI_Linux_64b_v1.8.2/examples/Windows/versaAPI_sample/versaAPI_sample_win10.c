/* VersaLogic VersaAPI Sample Code - 20160225
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
#include "stdafx.h"
#include <windows.h>

// VersaAPI header.
#include "VL_OSALib.h"
#include "Cgos.h"
/* *************************** */

/* ***** Defines. ***** */
//#define SLEEP_TIME 3000 		// Time to sleep between setting the DIO. In Windows time is in milliseconds.
#define SLEEP_TIME 3 		// Time to sleep between setting the DIO. In Windows time is in milliseconds.
#define VL_RETURN_SUCCESS 0	// A successful return status from a VL API call.

#define YES 1				// Define yes.
#define NO  0				// Define no.

#define TIMER_MAX_LOOP_CNT 60 // Maximum count to wait for timer to generate signal.
/* ******************** */


/* ***** Global variables. ***** */
int TimerTriggered;		// Global variable: Initial condition = NO,
						// After timer has triggered = YES.
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
unsigned char gWDTSupport;	// Does board support WDT.
unsigned char gBIOSInfo;	// BIOS information;

int			gApiDebugLevel;	// Current debug level.
/* ***************************** */


/* ***** Function to display application help message. ***** */
void showUsage()
{
	printf("Usage: versaAPI_sample [-abdDilstuvxwh]\n");
	printf("Application to show basic usage of VersaAPI commands.\n");
	printf("  -a => Use MPEe-A1/A2 DIO.\n");
	printf("  -b => Read from both BIOS Banks.\n");
	printf("  -d => Use onboard DIO.\n");
	printf("  -D => Run DIO loopback (DIO_CHANNEL_1 connected to DIO_CHANNEL_3).\n");
	printf("  -i => Execute some I2C API code.\n");
	printf("  -l => Number of loops to perform each tests.\n");
	printf("  -s => Run VL-SPX-2 DIO simple code. Default is to not run  code.\n");
	printf("  -t => Run 8254 Timer code. Default is to not run timer code.\n");
	printf("  -u => Use MPEe-U2 DIO. Default is to not run this code.\n");
	printf("  -v => Set API debug level to verbose. Default is no debug messages.\n");
	printf("  -x => Run VL-SPX-2 DIO loopback (DIO_SS0_CHANNEL-1 looped DIO_SS0_CHANNEL_9 code). Default is to not run  code.\n");
	printf("  -w => Run Watchdog Timer code. Default is to not run WDT code.\n");
	printf("  -h => Print this help message and exit.\n");
	printf("Default is to run no examples\n");
	
	exit(-1);
}
/* ********************************************************* */


/* ***** Function to run the DIO example. ***** */
/* Description: Each loop will initialize a DIO channel to an OUTPUT, set the
 *              DIO channel level HIGH, check it's value, set the DIO channel
 *              level LOW and check it's value. 'SUCCESS' on the command line 
 *              means the DIO level read is the same value as what was set,
 *              otherwise 'FAILED is outputted.                              */
void runDIOExample(int channel, char *channelName)
{

	/* ***** Variable declarations. ***** */
    unsigned char dioLevel;		// DIO level: DIO_CHANNEL_LOW or DIO_CHANNEL_HIGH.
	/* ********************************** */

	/* ***** Variable initializations. ***** */
	/* ************************************* */

    /* Must initialize the channels to input or output before accessing DIO. */
    VL_DIOSetChannelDirection(channel, DIO_OUTPUT);

	/* Run the loop. */
    gLoopCount = 0;
    while(gLoopCount <= gMaxNumLoops)
    {
	    // Set the DIO high.
	    printf("Setting %s (0x%x) high (1);\n", channelName, channel);
        VL_DIOSetChannelLevel(channel, DIO_CHANNEL_HIGH);

	    // Report on the DIO level.
        dioLevel = VL_DIOGetChannelLevel(channel);
	    printf("\tLevel=0x%x; ", dioLevel);
	    if (dioLevel == DIO_CHANNEL_HIGH)
	        printf("SUCCESS!\n");
	    else
	        printf("FAILED!\n");

	    // Pause.
	    Sleep(SLEEP_TIME);
	    
	    // Set the DIO Low.
	    printf("Setting %s (0x%x) low (0);\n", channelName, channel);
        VL_DIOSetChannelLevel(channel, DIO_CHANNEL_LOW);

	    // Report on the DIO level.
        dioLevel = VL_DIOGetChannelLevel(channel);
	    printf("\tLevel=0x%x; ", dioLevel);
	    if (dioLevel == DIO_CHANNEL_LOW)
	        printf("SUCCESS!\n");
	    else
	        printf("FAILED!\n");
	    
	    // Pause.
	    Sleep(SLEEP_TIME);
    
        // Increment the loop control so we don't run forever.
        gLoopCount++;
    }

	/* ***** Done. ***** */
	return;
	/* ***************** */

}
/* ******************************************** */


/* ***** Function to run the DIO loopback example. ***** */
/* Description: Each loop will initialize the specified DIO channel to an
 * OUTPUT/Input, set the output channel level HIGH, check it's value and the
 * loopback Input channel level, set the DIO channel level LOW and check it's
 * value and the loopback Input channel level. 'SUCCESS' on the command line
 * means the DIO level read is the same value as what was set, otherwise
 * 'FAILED' is outputted.                              */
void runDIOLoopbackExample(int outputChannel, char *outputChannelName,
	int inputChannel, char *inputChannelName)
{

	/* ***** Variable declarations. ***** */
	unsigned char    dioLevel;	// DIO level: DIO_CHANNEL_LOW or DIO_CHANNEL_HIGH.
	/* ********************************** */

	/* ***** Variable initializations. ***** */
	/* ************************************* */

	/* Must initialize the channels to input or output before accessing DIO. */
	VL_DIOSetChannelDirection(outputChannel, DIO_OUTPUT);
	VL_DIOSetChannelDirection(inputChannel, DIO_INPUT);

	/* Run the loop. */
	gLoopCount = 0;
	while (gLoopCount <= gMaxNumLoops)
	{
		// Set the DIO high.
		printf("Setting %s (0x%x) high (1);\n", outputChannelName, outputChannel);
		VL_DIOSetChannelLevel(outputChannel, DIO_CHANNEL_HIGH);

		// Report on the DIO level.
		dioLevel = VL_DIOGetChannelLevel(outputChannel);
		printf("\tLevel=0x%x; ", dioLevel);
		if (dioLevel == DIO_CHANNEL_HIGH)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");

		// Check if the cooresponding DIO changed as well.
		dioLevel = VL_DIOGetChannelLevel(inputChannel);
		printf("\t%s Level=0x%x; ", inputChannelName, dioLevel);
		if (dioLevel == DIO_CHANNEL_HIGH)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");

		// Pause.
		Sleep(SLEEP_TIME);

		// Set the DIO Low.
		printf("Setting %s (0x%x) low (0);\n", outputChannelName, outputChannel);
		VL_DIOSetChannelLevel(outputChannel, DIO_CHANNEL_LOW);

		// Report on the DIO level.
		dioLevel = VL_DIOGetChannelLevel(outputChannel);
		printf("\tLevel=0x%x; ", dioLevel);
		if (dioLevel == DIO_CHANNEL_LOW)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");

		// Check if the cooresponding DIO changed as well.
		dioLevel = VL_DIOGetChannelLevel(inputChannel);
		printf("\t%s Level=0x%x; ", inputChannelName, dioLevel);
		if (dioLevel == DIO_CHANNEL_LOW)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");

		// Pause.
		Sleep(SLEEP_TIME);

		// Set the DIO high.
		printf("Setting %s (0x%x) high (1);\n", outputChannelName, outputChannel);
		VL_DIOSetChannelLevel(outputChannel, DIO_CHANNEL_HIGH);

		// Report on the DIO level.
		dioLevel = VL_DIOGetChannelLevel(outputChannel);
		printf("\tLevel=0x%x; ", dioLevel);
		if (dioLevel == DIO_CHANNEL_HIGH)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");

		// Check if the cooresponding DIO changed as well.
		dioLevel = VL_DIOGetChannelLevel(inputChannel);
		printf("\t%s Level=0x%x; ", inputChannelName, dioLevel);
		if (dioLevel == DIO_CHANNEL_HIGH)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");

		// Increment the loop control so we don't run forever.
		gLoopCount++;
	}

	//printf("Number of VersaLogic Interrupts post-test:\n");
	//system("cat /proc/interrupts | grep vldrive");

	/* ***** Done. ***** */
	return;
	/* ***************** */

}
/* **************************************************** */


/* ***** Function to run the SPX loopback DIO example. ***** */
/* Description: Each loop will initialize the specified DIO channel to an 
 * OUTPUT/Input, set the output channel level HIGH, check it's value and the 
 * loopback Input channel level, set the DIO channel level LOW and check it's 
 * value and the loopback Input channel level. 'SUCCESS' on the command line
 * means the DIO level read is the same value as what was set, otherwise 
 * 'FAILED' is outputted.                              */
void runSPXLoopBackDIOExample(int outputChannel, char *outputChannelName,
	                          int inputChannel, char *inputChannelName)
{

	/* ***** Variable declarations. ***** */
	unsigned char dioLevel;		// DIO level: DIO_CHANNEL_LOW or DIO_CHANNEL_HIGH.
	/* ********************************** */

	/* ***** Variable initializations. ***** */
	/* ************************************* */

	/* Must initialize the channels to input or output before accessing DIO. */
	VL_DIOSetChannelDirection(outputChannel, DIO_OUTPUT);
	VL_DIOSetChannelDirection(inputChannel, DIO_INPUT);

	/* Report on which DIO channel is which. */
	printf("DIO Channel %s (0x%x) is Output channel;\n", outputChannelName, outputChannel);
	printf("DIO Channel %s (0x%x) is Input channel;\n", inputChannelName, inputChannel);

	/* Run the loop. */
	gLoopCount = 0;
	while (gLoopCount <= gMaxNumLoops)
	{
		// Set the Output DIO high.
		printf("Setting Output %s high (1);\n", outputChannelName);
		VL_DIOSetChannelLevel(outputChannel, DIO_CHANNEL_HIGH);

		// Report on the DIO level on the Output.
		dioLevel = VL_DIOGetChannelLevel(outputChannel);
		printf("\tOutput Channel Level=0x%x; ", dioLevel);
		if (dioLevel == DIO_CHANNEL_HIGH)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");
		
		// Report on the DIO level on the loopback Input.
		dioLevel = VL_DIOGetChannelLevel(inputChannel);
		printf("\tCorresponding Input Channel Level=0x%x; ", dioLevel);
		if (dioLevel == DIO_CHANNEL_HIGH)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");

		// Pause.
		Sleep(SLEEP_TIME);

		// Set the Output DIO Low.
		printf("Setting Output %s low (0);\n", outputChannelName);
		VL_DIOSetChannelLevel(outputChannel, DIO_CHANNEL_LOW);

		// Report on the Output DIO level.
		dioLevel = VL_DIOGetChannelLevel(outputChannel);
		printf("\tOutput Channel Level=0x%x; ", dioLevel);
		if (dioLevel == DIO_CHANNEL_LOW)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");

		// Report on the Input DIO level.
		dioLevel = VL_DIOGetChannelLevel(inputChannel);
		printf("\tInput Channel %s Level=0x%x; ", inputChannelName, dioLevel);
		if (dioLevel == DIO_CHANNEL_LOW)
			printf("SUCCESS!\n");
		else
			printf("FAILED!\n");

		// Pause.
		Sleep(SLEEP_TIME);

		// Increment the loop control so we don't run forever.
		gLoopCount++;
	}

	/* ***** Done. ***** */
	return;
	/* ***************** */

}
/* ******************************************** */

/* ***** Function to run the 8254 Timer example. ***** */
/* Description: Each loop operated on the first two timers on the board.
 * The timer is stopped, a callback is registered for the timer, then the
 * timer is started in Mode 0 based on the internal clock. When the timer
 * value reaches 0 the timer stops and an interrupt is generated.            */
void run8254TimerExample()
{
	/* ***** Variable Declarations. ***** */
    unsigned short timerValue;		// 8254 Timer current count value.
	short          countDownCount;	// Current timer count down value.
	int            i;				// Simple counter.
	unsigned short initialTimerVal; // Initial timer count down value.
	/* ********************************** */

	/* ***** Variable Initialization. ***** */
    timerValue = 0;
	/* ************************************ */

	/* Run the loop. */
    gLoopCount = 0;
    while(gLoopCount <= gMaxNumLoops)
    {
        /* ***** Loop through each timer. ***** */
	    for(i = 0; i < 2; i ++)
	    {
	        printf("----- Using timer VL_TIMER%i -----\n", i);

			TimerTriggered = NO;  // Timmer has not triggered.

	        // Make sure timer is stopped.
	        printf("Stopping VL_TIMER%i\n", i);
	        VL_TMRClear(i);
   
	        printf("Starting timer VL_TIMER%i in Mode 0, value of 2000 with type of internal.\n", i);
	        VL_TMRSet(i, VL_TIMER_MODE0, 2000, VL_TIMER_TYPE_INTERNAL);
			initialTimerVal = VL_TMRGet(i);
			timerValue     = VL_TMRGet(i);
    
	        countDownCount = 1;
	        while ((countDownCount <= TIMER_MAX_LOOP_CNT) && 
				   (timerValue <= initialTimerVal))
	        {
		        // Get the current timer value.
		        timerValue = VL_TMRGet(i);
		        printf("Timer Count %i(%i): Timer Value=%d;\n", countDownCount, 
				        TIMER_MAX_LOOP_CNT, timerValue);
		
		        // Do not want to loop forever.
		        countDownCount++;
	        }
	
	        // Stop the timer.
	        VL_TMRClear(i);
		
	        if (countDownCount != TIMER_MAX_LOOP_CNT)
	        {
		        printf("VersaLogic Timer Triggered.\n");
	        }
	        else
	        {
		        printf("VersaLogic Timer did not Trigger.\n");
	        }
		
	        printf("\n");
	        Sleep(SLEEP_TIME);
	    }

        // Increment the loop control so we don't run forever.
        gLoopCount ++;
     }
	/* ************************************ */

	/* ***** Done. ***** */
	return;
	/* ***************** */
}
/* *************************************************** */


/* ***** BIOS example helper function. ***** */
void reportActiveBIOSBank(unsigned char activeBank)
{
	switch(activeBank)
	{
		case BIOS_PRIMARY_BANK: 
			printf("\tCurrent Active BIOS Bank is PRIMARY\n");
			break;
		case BIOS_SECONDARY_BANK: 
			printf("\tCurrent Active BIOS Bank is SECONDARY\n");
			break;
		default: 
			printf("\tCurrent Active BIOS Bank is UNKNOWN\n");
			break;
	}
}
/* ***************************************** */

/* ***** Function to run the BIOS bank example. ***** */
/* Description: Each loop will report the current (active) BIOS bank, set the
 * BIOS bank to the backup (secondary) bank, report the newly activated bank, 
 * set the BIOS bank to primary and report the newly activated bank.         */
void runBIOSBankExample()
{
	/* ***** Variable Declarations. ***** */
    unsigned char currentActiveBIOSBank;	// Active BIOS Bank.
	VL_APIStatusT retCode;					// API return status.
	/* ********************************** */

	/* ***** Variable Initialization. ***** */
    currentActiveBIOSBank = 0;
	/* ************************************ */

	/* Run the loop. */
    gLoopCount = 0;
    while(gLoopCount <= gMaxNumLoops)
    {
		// Report the current BIOS bank.
	    printf("Getting current Active BIOS Bank:\n");	
		retCode = VSL_GetActiveBIOSBank(&currentActiveBIOSBank);
		if (retCode != VL_API_OK)
		{
			printf("\tVSL_GetActiveBIOSBank() returned error %d; Stopping Example.\n", 
					retCode);
			return;
		}
		reportActiveBIOSBank(currentActiveBIOSBank);
		
		// Set the active BIOS bank to be the secondary bank.
		printf("Setting the Active BIOS Bank to be Secondary bank\n");
		retCode = VSL_SetActiveBIOSBank(BIOS_SECONDARY_BANK);
		if (retCode != VL_API_OK)
		{
			printf("\tVSL_GetActiveBIOSBank() returned error %d; Stopping Example.\n", 
					retCode);
			return;
		}
		 
		printf("Getting current Active BIOS Bank:\n");
		retCode = VSL_GetActiveBIOSBank(&currentActiveBIOSBank);
		if (retCode != VL_API_OK)
		{
			printf("\tVSL_GetActiveBIOSBank() returned error %d; Stopping Example.\n",
				retCode);
			return;
		}
		
		// Report the current BIOS bank.
		reportActiveBIOSBank(currentActiveBIOSBank);
		  
		// Set the current BIOS bank to be primary.
		printf("Setting the Active BIOS Bank to be Primary bank\n");
		retCode = VSL_SetActiveBIOSBank(BIOS_PRIMARY_BANK);
		if (retCode != VL_API_OK)
		{
			printf("VSL_GetActiveBIOSBank() returned error %d; Stopping Example.\n", 
					retCode);
			return;
		}
		 
		printf("Getting current Active BIOS Bank:\n");
		retCode = VSL_GetActiveBIOSBank(&currentActiveBIOSBank);
		if (retCode != VL_API_OK)
		{
			printf("\tVSL_GetActiveBIOSBank() returned error %d; Stopping Example.\n",
				retCode);
			return;
		}

		// Report the current BIOS bank.
		
		// Report the active BIOS bank.
		reportActiveBIOSBank(currentActiveBIOSBank);
		  
	    printf("\n");
	    Sleep(SLEEP_TIME);

        // Increment the loop control so we don't run forever.
        gLoopCount ++;
     }
	/* ************************************ */

	/* ***** Done. ***** */
	return;
	/* ***************** */
}
/* ************************************************** */


/* ***** WDT example helper function. ***** */
void reportWDTState()
{
	unsigned char curState;

	VSL_GetWDTEnable(&curState);
	switch(curState)
	{
		case 0x00: 
			printf("\tWDT is DISABLED\n");
			break;
		case 0x02: 
			printf("\tWDT is ENABLED\n");
			break;
		default: 
			printf("\tWDT in UNKOWN STATE:0x%x.\n", curState);
			break;
	}
}

/* ***** WDT example helper function. ***** */
void reportWDTStatus()
{
	unsigned char curState = VSL_WDTStatus();
	switch(curState)
	{
		case 0x00: 
			printf("\tWDT NOT FIRED\n");
			break;
		case 0x01: 
			printf("\tWDT FIRED\n");
			break;
		default: 
			printf("\tWDT has UNKOWN STATUS.\n");
			break;
	}
}
/* **************************************** */

/* ***** Function to run the WDT example. ***** */
/* Description: Each loop will shows the WDT expire and how to 'pet' the WDT.
 * Expire example configures the WDT for 10 seconds, enables (starts) the WDT
 * and let's it expire, showing how the state changes once it expires (fires).
 * The second part of the loop test again sets the WDT for an expire time, but
 * 'pets' it every 5 seconds, showing that the WDT never expires (fires).    */
void runSimpleWDTExample()
{
	/* ***** Variable Declarations. ***** */
    unsigned char currentWDTState;	// Active BIOS Bank.
	/* ********************************** */

	/* ***** Variable Initialization. ***** */
    currentWDTState = 0;
	/* ************************************ */

	/* Run the loop. */
    gLoopCount = 0;
    while(gLoopCount <= gMaxNumLoops)
    {
		// Start WDT Expirey test
		printf("Start of the WDT Expirey example\n");
		
		// Configure the expirey action of the WDT to be disabled.
		VSL_WDTSetValue(10);
		  
		// Get the expiery action of the WDT.
		VSL_WDTResetEnable(DISABLE);
		  
		// Disable the WDT.
		VSL_WDTEnable(DISABLE);
		reportWDTState(currentWDTState);
		  
		// Set expirey time to 10 seconds.
		printf("\tWDT expirey time set to %d seconds\n", 10);
		VSL_WDTSetValue(10);
		  
		// Enable (start) WDT
		printf("\tStarting WDT...\n");
		VSL_WDTEnable(ENABLE);
		reportWDTState();
		reportWDTStatus();
		  
		// Poll the WDT state. For ~14 seconds it should be 0 (=> not fired).
		int i;
		for (i = 0; i <= 24; i ++)
		{
			printf("\tloop %i: ", i);
			reportWDTStatus();
			Sleep(1000);
		}
		  
		// Start the WDT example of petting the WDT.
		printf("Start of the WDT pet example\n");

		// Disable the WDT.
		VSL_WDTEnable(DISABLE);
		reportWDTState(currentWDTState);
		  
		// Configure the expirey action to disable.
		VSL_WDTResetEnable(DISABLE);
		  
		// Set the expirey time to 10 seconds.
		printf("\tWDT expirey time set to %d seconds\n", 10);
		VSL_WDTSetValue(10);
		  
		// Enable (start) WDT.
		printf("\tStarting WDT...\n");
		VSL_WDTEnable(ENABLE);
		reportWDTState();
		  
		// Pet the WDT every 5 seconds for 25 seconds (enough time for 3 expirey's).
		for (i = 0; i <= 25; i ++)
		{
			printf("\tloop %i: ", i);
			reportWDTStatus();

			if ((i % 5) == 0)
			{
				printf("\t\tPetting WDT in loop %d\n", i);
		        VSL_WDTSetValue(10);
			}

			Sleep(1000);
		}

        // Increment the loop control so we don't run forever.
        gLoopCount ++;
     }
	/* ************************************ */

	/* ***** Done. ***** */
	return;
	/* ***************** */
}
/* ******************************************** */

/* ***** Function to run the I2C example. ***** */
/* Description: I2C bus is checked to see if it is available. The frequency
 * od the bus is set and checked. The registers reads and performed.         */
void runI2CExample()
{
	/* ***** Variable declarations. ***** */
	VL_APIStatusT status;
	unsigned long targetFreq;
	unsigned long curFreq;
	unsigned char value;
	/* ********************************** */

	/* ***** Variable initializations. ***** */
	status = VL_API_OK;
	targetFreq = VL_I2C_FREQUENCY_100KHZ;
	curFreq = 0;
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
	/* ********************************************** */

	/* ***** Set/Get the I2C bus frequence. ***** */
	// Set the frequency
	status = VSL_I2CSetFrequency(VL_I2C_BUS_TYPE_PRIMARY, targetFreq);
	if (status == VL_API_OK)
	{
		printf("\tSet the I2C frequency to %d;\n", targetFreq);
	}
	else 
	{
		printf("\tCould NOT set the I2C frequency to %d;\n", targetFreq);
	}

	// Get the frequency.
	status = VSL_I2CGetFrequency(VL_I2C_BUS_TYPE_PRIMARY, &curFreq);
	if (status == VL_API_OK)
	{
		printf("\tCurrent I2C frequency is %d;\n", curFreq);
	}
	else
	{
		printf("\tCould NOT get the I2C frequency;\n");
	}
	/* ****************************************** */

	/* ***** Run the loop. ***** */
	gLoopCount = 0;
	int maxIntWrap = 0;

	printf("\t\n\tStarting the I2C loop read loop\n");
	while (gLoopCount <= gMaxNumLoops)
	{
		// Initalize the read value;
		value = 0x0;

		// Increment the loop control so we don't run forever.
		printf("\t\tLoop=%d:", gLoopCount);
		status = VSL_I2CReadRegister(VL_I2C_BUS_TYPE_PRIMARY, 0xc1, 0x0c, &value);
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
	if ((gAttributes & 0x2) == 1)
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

int     opterr = 1,     /* if error message should be printed */
optind = 1,             /* index into parent argv vector */
optopt,                 /* character checked for validity */
optreset;               /* reset getopt */
char    *optarg;        /* argument associated with option */

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""
/*
* getopt --
*      Parse argc/argv argument vector.
*/
int getopt(int nargc, char * const nargv[], const char *ostr)
{
	static char *place = EMSG;    /* option letter processing */
	const char *oli;              /* option letter list index */

	if (optreset || !*place)
	{              /* update scanning pointer */
		optreset = 0;
		if (optind >= nargc || *(place = nargv[optind]) != '-')
		{
			place = EMSG;
			return (-1);
		}
		if (place[1] && *++place == '-')
		{      /* found "--" */
			++optind;
			place = EMSG;
			return (-1);
		}
	}                                       /* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
		!(oli = strchr(ostr, optopt)))
	{
		/*
		* if the user didn't specify '-' as an option,
		* assume it means -1.
		*/
		if (optopt == (int)'-')
			return (-1);
		if (!*place)
			++optind;
		if (opterr && *ostr != ':')
			(void)printf("illegal option -- %c\n", optopt);
		return (BADCH);
	}
	if (*++oli != ':')
	{                    /* don't need argument */
		optarg = NULL;
		if (!*place)
			++optind;
	}
	else
	{                                  /* need an argument */
		if (*place)                     /* no white space */
			optarg = place;
		else if (nargc <= ++optind)
		{   /* no arg */
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (opterr)
				(void)printf("option requires an argument -- %c\n", optopt);
			return (BADCH);
		}
		else                            /* white space */
			optarg = nargv[optind];
		place = EMSG;
		++optind;
	}
	return (optopt);                        /* dump back option letter */
}

/* ***** main program. ***** */
int main(int argc, char *argv[])
{
    /* ***** Variable declarations. ***** */
    unsigned long vl_ReturnStatus;	// Return status.
    
    unsigned char majorRev;			// Major revision number.
    unsigned char minorRev;			// Minor revision number.
    unsigned char releaseRev;		// Release revision number.
    
    unsigned char  currentDIO;				// Use this DIO channel.
    char           currentDIOName[48];		// Name of current DIO channel.
	unsigned char  currentInputDIO;			// Use this DIO channel.
	char           currentInputDIOName[48];	// Name of current DIO channel.
    
    int			   opt;					// Command line option helper.
    int			   do8254Timer;			// Execute 8254 timer code. Default is No.
    int			   doSimpleDIO;			// Execute simple DIO code. Default is No.
	int			   doLoopbackDIO;		// Execute loopback DIO code. Default is No.
	int			   doSimpleU2DIO;		// Execute simple U2 DIO code. Default is No.
	int			   doLoopbackU2DIO;		// Execute loopback U2 DIO code. Default is No.
    int			   doAxSimpleDIO;		// Execute loopback DIO code using A1/2. Default is No.
    int			   doSimpleWDT;			// Execute Watch Dog Timer code. Default is No.
    int			   doBIOSBank;			// Execute BIOS bank test code. Default is No.
	int			   doSimpleI2C;			// Execute I2C code. Default is No.
	int			   doSpxSimpleDIO;		// Execute simple DIO code on VL-SPX-2. Default is No.
	int			   doSpxLoopbackDIO;	// Execute simple DIO code on VL-SPX-2. Default is No.
    /* ********************************** */

    /* ***** Variable initializations. ***** */
    vl_ReturnStatus = 0xffffff;	// Assume a failure on open.
    majorRev		= (unsigned char)0;
    minorRev		= (unsigned char)0;
    releaseRev		= (unsigned char)0;
    currentDIO 	   = DIO_CHANNEL_1;
	sprintf_s(currentDIOName, sizeof("On-board DIO_CHANNEL_1"), "On-board DIO_CHANNEL_1");
	
	opt	= 0;  // Process command line options.
	
	// By default, do not execute any tests.
	do8254Timer			= NO;
    doSimpleDIO			= NO;
	doLoopbackDIO		= NO;
	doSimpleU2DIO		= NO;
	doLoopbackU2DIO		= NO;
    doAxSimpleDIO		= NO;
    doSimpleWDT			= NO;
    doBIOSBank			= NO;
	doSimpleI2C			= NO;
	doSpxSimpleDIO		= NO;
	doSpxLoopbackDIO	= NO;
	
	TimerTriggered	= NO; // Default value.

	// Sert default debug level.
	gApiDebugLevel = VL_DEBUG_OFF;  // No debug info by default.
    /* ************************************* */

    /* ***** Process command line options. ***** */
	if(argc <= 1)
	{
		showUsage();
		exit(-1);
	}

	// There is some functionality to be shown.
    while ((opt = getopt(argc, argv, "abdDil:sStuUvwxh")) != -1)
    {
		switch(opt)
		{
			case 'a': 	doAxSimpleDIO = YES;
						break;
			case 'b': 	doBIOSBank = YES;
						break;
			case 'd': 	doSimpleDIO = YES;
						break;
			case 'D': 	doLoopbackDIO = YES;
						break;
			case 'i': 	doSimpleI2C = YES;
						break;
			case 'l': 	gMaxNumLoops = atoi(optarg);
						break;
			case 's': 	doSpxSimpleDIO = YES;
						break;
			case 'S': 	doSpxLoopbackDIO = YES;
						break;
			case 't':	do8254Timer = YES;
						break;
			case 'u': 	doSimpleU2DIO = YES;
						break;
			case 'U': 	doLoopbackU2DIO = YES;
						break;
			case 'v':	gApiDebugLevel = VL_DEBUG_1;
						break;
			case 'w':	doSimpleWDT = YES;
						break;
			case 'x': 	doSpxLoopbackDIO = YES;
						break;
			default:	showUsage();
						exit(-1);
		}
	}
    /* ***************************************** */

	/* ***** Set the debug level. ****** */
	VSL_DebugLevelSet(gApiDebugLevel);
	/* ********************************* */

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


	/* ***** Setup the VL 8254 Timer Signal Handler ***** */
	/* According to Microsoft:
	    https://docs.microsoft.com/en-us/previous-versions/xdkz3x12(v%3Dvs.140)
	    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/signal?view=msvc-160
	   only a few signals are allowed in the signal() api call. Not sure what to do with
	   Timer generated signals.
	*/
	/* ************************************************** */

	/* ***** Simple DIO Example. ***** */
	if (doSimpleDIO == YES)
	{
	    printf("\n********* Start VersaLogic Onboard Simple DIO Example *********\n");

        // Run the loop.
		runDIOExample(currentDIO, currentDIOName);
    
		printf("********* End VersaLogic Simple DIO Example *********\n");
	}
	/* ******************************* */

	/* ***** Loopback DIO Example. ***** */
	if (doLoopbackDIO == YES)
	{
		printf("\n********* Start VersaLogic Loopback DIO Example *********\n");

		// Setup the DIO.
		currentDIO = DIO_CHANNEL_1;
		currentInputDIO = DIO_CHANNEL_3;
		snprintf(currentDIOName, sizeof("DIO_CHANNEL_1"), "DIO_CHANNEL_1");
		snprintf(currentInputDIOName, sizeof("DIO_CHANNEL_3"), "DIO_CHANNEL_3");

		// Run the loop.
		runDIOLoopbackExample(currentDIO, currentDIOName,
			currentInputDIO, currentInputDIOName);

		printf("********* End VersaLogic Loopback DIO Example *********\n");
	}
	/* ********************************* */
	
	/* ***** Simple A1/A2 DIO Example. ***** */
	if (doAxSimpleDIO == YES)
	{
	    printf("\n********* Start VersaLogic A1/A2 Simple DIO Example *********\n");

		// Setup the Ax DIO.
		currentDIO = DIO_AX_CHANNEL_3;
		sprintf_s(currentDIOName, sizeof("Ax DIO_AX_CHANNEL_3"), "Ax DIO_AX_CHANNEL_3");

        // Run the loop.
		runDIOExample(currentDIO, currentDIOName);
    
		printf("********* End VersaLogic Simple Ax DIO Example *********\n");
	}
	/* ******************************* */

	/* ***** Simple U2 DIO Example. ***** */
	if (doSimpleU2DIO == YES)
	{
		printf("\n********* Start VersaLogic Onboard Simple U2 DIO Example *********\n");

		// Setup the U2 DIO.
		currentDIO = DIO_U2_CHANNEL_1;
		sprintf_s(currentDIOName, sizeof("U2 DIO_U2_CHANNEL_1 (UART_MPIO0)"), "U2 DIO_U2_CHANNEL_1 (UART_MPIO0)");

		// Run the loop.
		runDIOExample(currentDIO, currentDIOName);

		printf("********* End VersaLogic Simple U2 DIO Example *********\n");
	}
	/* ******************************* */

	/* ***** Loopback MPEe-U2 DIO Example. ***** */
	if (doLoopbackU2DIO == YES)
	{
		printf("\n********* Start VersaLogic MPEe-U2 Loopback DIO Example *********\n");

		// Setup the U2 DIO.
		currentDIO      = DIO_U2_CHANNEL_9;
		currentInputDIO = DIO_U2_CHANNEL_2;
		sprintf_s(currentDIOName, sizeof("U2 DIO_U2_CHANNEL_9"), "U2 DIO_U2_CHANNEL_9");
		sprintf_s(currentInputDIOName, sizeof("U2 DIO_U2_CHANNEL_2"), "U2 DIO_U2_CHANNEL_2");

		// Run the loop.
		runDIOLoopbackExample(currentDIO, currentDIOName,
			currentInputDIO, currentInputDIOName);

		printf("********* End VersaLogic Loopback MPEe-U2 DIO Example *********\n");
	}
	/* ******************************* */

	/* ***** Simple SPX-2 DIO Example. ***** */
	if (doSpxSimpleDIO == YES)
	{
		printf("\n********* Start VersaLogic VL-SPX-2 Simple DIO Example *********\n");

		// Setup the Ax DIO.
		currentDIO = DIO_SS0_CHANNEL_1;
		sprintf_s(currentDIOName, sizeof("SPX DIO_SS0_CHANNEL_1"), "SPX DIO_SS0_CHANNEL_1");

		// Run the loop.
		runDIOExample(currentDIO, currentDIOName);

		printf("********* End VersaLogic Simple VL-SPX-2 DIO Example *********\n");
	}
	/* ******************************* */

	/* ***** Simple Loopback SPX-2 DIO Example. ***** */
	if (doSpxLoopbackDIO == YES)
	{
		printf("\n********* Start VersaLogic VL-SPX-2 Loopback DIO Example *********\n");

		// Setup the Ax DIO.
		currentDIO = DIO_SS0_CHANNEL_1;
		currentInputDIO = DIO_SS0_CHANNEL_9;
		sprintf_s(currentDIOName, sizeof("SPX DIO_SS0_CHANNEL_1"), "SPX DIO_SS0_CHANNEL_1");
		sprintf_s(currentInputDIOName, sizeof("SPX DIO_SS0_CHANNEL_9"), "SPX DIO_SS0_CHANNEL_9");

		// Run the loop.
		runSPXLoopBackDIOExample(currentDIO, currentDIOName, currentInputDIO, currentInputDIOName);

		printf("********* End VersaLogic Loopback VL-SPX-2 DIO Example *********\n");
	}
	/* ******************************* */

	
    
    /* ***** VersaLogic 8254 Timer Example Section. ***** */
    if (do8254Timer == YES)
	{
		printf("***** Start VersaLogic 8254 Timer Example. *****\n");

        run8254TimerExample();
		
		printf("\n***** End VersaLogic 8254 Timer Example. *****\n");
	}
    /* ************************************************** */
    

    /* ***** VersaLogic BIOS Bank Read Example Section. ***** */
    if (doBIOSBank == YES)
	{
		printf("***** Start VersaLogic BIOS Bank Read Example. *****\n");

        runBIOSBankExample();
		
		printf("\n***** End VersaLogic BIOS Bank Read Example. *****\n");
	}
    /* ************************************************** */

    /* ***** VersaLogic Simple WDT Example Section. ***** */
    if (doSimpleWDT == YES)
	{
		printf("***** Start VersaLogic Simple WDT Example. *****\n");

        runSimpleWDTExample();
		
		printf("\n***** End VersaLogic Simple WDT Example. *****\n");
	}
    /* ************************************************** */

	/* ***** Simple I2C Example. ***** */
	if (doSimpleI2C == YES)
	{
		printf("\n********* Start VersaLogic Simple I2C Example *********\n");

		// Run the loop.
		runI2CExample();

		printf("********* End VersaLogic Simple I2C Example *********\n");
	}
	/* ******************************* */

    /* ***** VL_Close() should be called at the end of the program. ***** */
    VL_Close();
    
    printf("\nEnd of VersaLogic Example Application execution.\n");

    return(0);
}





