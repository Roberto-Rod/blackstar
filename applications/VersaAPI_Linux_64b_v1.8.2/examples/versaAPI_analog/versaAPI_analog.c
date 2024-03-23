/* VersaLogic VersaAPI Sample Code 
*
* 20211217 - Initial release.
*
* This code is given with no guarantees and is intended to be used as an 
* example of how to use the VersaAPI software package and to check the 
* operation post install. This code will work with various VersaLogic 
* single board computers that support onboard DIO and ADC.
*
* Hardware setup 
* This example requires DIO1 to be connected to Analog1.
*
* Default Program Description: 
* This application will report the library version number and then toggle 
* an on-board DIO from HIGH to LOW and read the voltage level on the on-board
* analog pin.
* 
* Command Line Options: 
* Run versaAPI_sample -h for a list of command line options.
* 
* This code has been compiled and run using gcc on Ubuntu 18.04.03 LTS, 
* kernel 4.15.0-88-generic, with VersaAPI version 1.a.a pre-release.  
* A Linux compatible Makefile has been provided with this .c file for an 
* example of how to compile this code. */


/* ***** Includes Files. ***** */
// System.
#include <stdio.h>
//#include <signal.h>
#include <stdlib.h>

// VersaAPI header.
#include "VL_OSALib.h"
/* *************************** */

/* ***** Defines. ***** */
#define SLEEP_TIME 3 		// Time to sleep between setting the DIO.
#define VL_RETURN_SUCCESS 0	// A successful return status from a VL API call.

#define YES 1				// Define yes.
#define NO  0				// Define no.
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
unsigned char gName[24];		// Board name.
unsigned char gAttributes;	// Generic board attributes.
unsigned char gWDTSupport;	// Does board support WDT?;
unsigned char gBIOSInfo;		// BIOS information;

int gApiDebugLevel = VL_DEBUG_OFF;  // No debugging info by default.
/* ***************************** */


/* ***** Function that will display the currently selected Aanlog Input Range. ***** */
void printADCRangeValues(unsigned char currentAnalogRange)
{
	printf("Analog ADC Range:");
	switch(currentAnalogRange)
	{
	    case SPI_RANGE_PM0P625V: printf("SPI_RANGE_PM0P625V (+/-0.625V)\n"); break;
        case SPI_RANGE_PM1P25V:  printf("SPI_RANGE_PM1P25V (+/-1.25V)\n");  break;
        case SPI_RANGE_PM2P5V:   printf("SPI_RANGE_PM2P5V (+/-2.50V)\n");   break;
        case SPI_RANGE_PM5V:     printf("SPI_RANGE_PM5V (+/-5V)\n");    break;
        case SPI_RANGE_PM10V:    printf("SPI_RANGE_PM10V (+/-10V)\n");    break;
        case SPI_RANGE_0_10V:    printf("SPI_RANGE_0_10V (0-10V)\n");    break;
        case SPI_RANGE_0_5V:     printf("SPI_RANGE_0_5V (0-5V)\n");     break;
        case SPI_RANGE_0_2P5V:   printf("SPI_RANGE_0_2P5V (0-2.5V)\n");   break;
        case SPI_RANGE_0_1P25V:  printf("SPI_RANGE_0_1P25V (0-1.25V)\n");  break;
        default:                 printf("SPI_RANGE_0_5V (0-5V)\n");     break;
	}
	printf("\n");
}


/* ***** Function to display application help message. ***** */
void showUsage()
{
	printf("Usage: versaAPI_sample [-dvh]\n");
	printf("Application to show basic usage of VersaAPI commands.\n");
	printf("Must specify use case(s) to run.\n");
	printf("  -d => Run onboard DIO.\n");
	printf("  -v => Set API debug level to verbose. Default is no debug messages.\n");
	printf("  -h => Print this help message and exit.\n");
	printf("Default is to run no examples\n");
	
	exit(-1);
}
/* ********************************************************* */


/* ***** Function to run the simple DIO example. ***** */
/* Description: This example requires DIO1 to be connected to AnalogIn1.
 *              Each loop will initialize a DIO channel to an OUTPUT, set the
 *              DIO channel level HIGH, check it's value, check the AnalogIn value,
 *              set the DIO channel
 *              level LOW and check it's value and the AnalogIn value. 
 *              'SUCCESS' on the command line 
 *              means the DIO level read is the same value as what was set, and
 *              that the analog value read appropriately.
 *              otherwise 'FAILED is outputted.                              */
void runDIOADCExample(int channel, char *channelName)
{

	/* ***** Variable declarations. ***** */
    unsigned char    dioLevel;	          // DIO level: DIO_CHANNEL_LOW or DIO_CHANNEL_HIGH.
	double           analogValueRead;     // Value of the analog signal connect to the DIO.
	unsigned char    currentAnalogRange;  // Current Analog input range.
	/* ********************************** */

	/* ***** Variable initializations. ***** */
	analogValueRead    = 0;

	currentAnalogRange = SPI_RANGE_PM0P625V;
	//currentAnalogRange = SPI_RANGE_PM1P25V;
	//currentAnalogRange = SPI_RANGE_PM2P5V;
	//currentAnalogRange = SPI_RANGE_PM5V;
	//currentAnalogRange = SPI_RANGE_PM10V;
	//currentAnalogRange = SPI_RANGE_0_10V;
	//currentAnalogRange = SPI_RANGE_0_5V;
	//currentAnalogRange = SPI_RANGE_0_2P5V;
	//currentAnalogRange = SPI_RANGE_0_1P25V;
    /* ************************************* */

    /* Must initialize the channels to input or output before accessing DIO. */
    VL_DIOSetChannelDirection(channel, DIO_OUTPUT);

	/* Must initialize the analog input range. */
	VSL_ADCSetAnalogInputRange(SPI_AO_CHANNEL_1, currentAnalogRange);

	/* Remind the user as to what Input Range is being used. */
	printADCRangeValues(currentAnalogRange);

	/* Run the loop. */
    gLoopCount = 0;
    while(gLoopCount <= gMaxNumLoops)
    {
	    // Set the DIO high.
	    printf("Setting %s (0x%x) high (1);\n", channelName, channel);
        VL_DIOSetChannelLevel(channel, DIO_CHANNEL_HIGH);

	    // Report on the DIO level.
        dioLevel = VL_DIOGetChannelLevel(channel);
	    printf("\tLevel read=0x%x; ", dioLevel);
	    if (dioLevel == DIO_CHANNEL_HIGH)
		{
	        printf("SUCCESS!\n");
		}
	    else
		{
	        printf("FAILED!\n");
		}

		// Report on the analog level.
	    VSL_DebugLevelSet(gApiDebugLevel);
		analogValueRead = VSL_ADCGetAnalogInput(SPI_AO_CHANNEL_1, AI_VOLTS);
	    VSL_DebugLevelSet(0);
		printf("\tAnalog value read = %4f\n", analogValueRead);

	    // Pause.
	    sleep(SLEEP_TIME);
	    
	    // Set the DIO Low.
	    printf("Setting %s (0x%x) low (0);\n", channelName, channel);
        VL_DIOSetChannelLevel(channel, DIO_CHANNEL_LOW);

	    // Report on the DIO level.
        dioLevel = VL_DIOGetChannelLevel(channel);
	    printf("\tLevel read=0x%x; ", dioLevel);
	    if (dioLevel == DIO_CHANNEL_LOW)
		{
	        printf("SUCCESS!\n");
		}
	    else
		{
	        printf("FAILED!\n");
		}

		// Report on the analog level.
		analogValueRead = VSL_ADCGetAnalogInput(SPI_AO_CHANNEL_1, AI_VOLTS);
		printf("\tAnalog value read = %4f\n", analogValueRead);

        // Increment the loop control so we don't run forever.
        gLoopCount++;
    }

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
    unsigned long vl_ReturnStatus;	    // Return status.
    unsigned char majorRev;			    // Major revision number.
    unsigned char minorRev;			    // Minor revision number.
    unsigned char releaseRev;		    // Release revision number.
    unsigned char  currentDIO;			// Use this DIO channel.
    char           currentDIOName[24];	// Name of current DIO channel.
    int			   opt;				    // Command line option helper.
    int			   doSimpleDIO;		    // Execute simple DIO code. Default is No.
    /* ********************************** */

    /* ***** Variable initializations. ***** */
    vl_ReturnStatus = 0xffffff;	// Assume a failure on open.
    majorRev		= (unsigned char)0;
    minorRev		= (unsigned char)0;
    releaseRev		= (unsigned char)0;
    currentDIO 	   = DIO_CHANNEL_1;
	snprintf(currentDIOName, sizeof("On-board DIO_CHANNEL_1"), "On-board DIO_CHANNEL_1");
	
	// Process command line options.
	opt	= 0;
	
	// By default, do not execute any tests.
    doSimpleDIO	= NO;	
    /* ************************************* */

    /* ***** Process command line options. ***** */
	if(argc <= 1)
	{
		showUsage();
		exit(-1);
	}

	// There is some functionality to be shown.
    while ((opt = getopt(argc, argv, "abdDil:rsthuUvwx")) != -1)
    {
		switch(opt)
		{
			case 'd': 	doSimpleDIO = YES;
						break;
			case 'l': 	gMaxNumLoops = atoi(optarg);
						break;
			case 'v': 	gApiDebugLevel = VL_DEBUG_1;
						break;
			default:	showUsage();
						exit(-1);
		}
	}
    /* ***************************************** */

	/* ***** Set the debug level. ****** */
	//VSL_DebugLevelSet(gApiDebugLevel);
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

	/* ***** Is there on-board analog to test? ***** */
	if (gNumAOut <= 0)
	{
		printf("This board does not have on-board analog signals.\n");
		printf("Example terminating.\n");
		return(-1);
	}
	/* ********************************************* */

	/* ***** Simple DIO Example. ***** */
	if (doSimpleDIO == YES)
	{
	    printf("\n********* Start VersaLogic Onboard Simple DIO Example *********\n");

        // Run the loop.
		runDIOADCExample(currentDIO, currentDIOName);
    
		printf("********* End VersaLogic Simple DIO Example *********\n");
	}
	/* ******************************* */

    /* ***** VL_Close() should be called at the end of the program. ***** */
    VL_Close();
    
    printf("\nEnd of VersaLogic Example Application execution.\n");

    return(0);
}

