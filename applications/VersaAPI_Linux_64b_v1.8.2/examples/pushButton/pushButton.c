/* VersaLogic pushbutton Sample Code 
*
* 20210411 - Initial release.
*
* This code is given with no guarantees and is intended to be used as an 
* example of how to use the VersaAPI software package and to check the 
* operation post install. This code will  work with various VersaLogic 
* single board computers. 
*
* Default Program Description: 
* Example of using the VersaAPI Administration, and LCD panel control
* calls.
* 
* Command Line Options: 
* Run pushbutton -h for a list of command line options.
* 
* This code has been compiled and run using gcc on Ubuntu 18.04.03 LTS, 
* kernel 4.15.0-88-generic, with VersaAPI version 1.7.0.  A Linux
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
#define VL_RETURN_SUCCESS 0   // A successful return status from a VL API call.
/* ******************** */


/* ***** Global variables. ***** */
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

unsigned long    gMaxBacklight;  // Maximum value for backlight.
int              gBlChangeValue; // Value to increase/decrease backlight.

// Signal hander stuff.
struct sigaction gVlDIOSig;   // Signal (event).
/* ***************************** */


/* ***** Signal Handler for the DIO signals. ***** */
void VL_DIO_SignalHandler(int Signal, siginfo_t *Info, void *Nothing)
{
	// Declarations. 
    unsigned long currentBlValue = 0;	
    unsigned long newBlValue     = 0;	

	if (Info)
	{
        printf("*** Recieved Signal %d from:%d\n", Signal, Info->si_int );

		// Get the current backlight value.
        if (VSL_LCDGetBacklight(&currentBlValue) != VL_API_OK)
	    {
	        printf("\nCould not get backlight value.\n");
		    currentBlValue = 50;
	    }
		
		switch (Info->si_int)
		{
			case 1: newBlValue = currentBlValue + gBlChangeValue;
					if (newBlValue > gMaxBacklight)
					{
					    newBlValue = gMaxBacklight; 
					}
					printf("Increasing Bl from %lu to %lu\n", currentBlValue, newBlValue);
					break;
			case 2: newBlValue = currentBlValue - gBlChangeValue;
					if (newBlValue < 0)
					{
						newBlValue = 0;
					}
					printf("Decreasing Bl from %lu to %lu\n", currentBlValue, newBlValue);
					break;
			default: break;
		}

		// Now set the new value.
		if (VSL_LCDSetBacklight((unsigned long)newBlValue) != VL_API_OK)
		{
	        printf("FAILED to set backlight value to:%lu;\n", newBlValue);
		}
	}
	else
	{
        printf("*** Recieved Signal %d from unknown source data; *** ;\n", Signal);
	}
}
/* *********************************************************** */


/* ***** Function to display application help message. ***** */
void showUsage()
{
	printf("Usage: pushButton [-vh]\n");
	printf("Application to show basic usage of VersaAPI commands.\n");
	printf("  -v => Set API debug level to verbose. Default is no debug messages.\n");
	printf("  -h => Print this help message and exit.\n");
	
	exit(-1);
}
/* ********************************************************* */


/* ***** Function to report generic board information. ***** */
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


/* ***** Toggle the given DIO. ***** */
// This function "simulates" a button push.
void toggleDIO(unsigned char dio)
{
	// Toggel DIO from LOW to HIGH.
	// This should cause an interrupt to be generated.
	VSL_DIOSetChannelLevel(dio, DIO_CHANNEL_HIGH);
	sleep(1); 
	
	// Toggel DIO from HIGH to LOW (for next time).
	VSL_DIOSetChannelLevel(dio, DIO_CHANNEL_LOW);
}
/* ********************************* */


/* ***** Setup the system to generate interrupts on DIO transitions. ***** */
void setupInterrupts(unsigned char dioIncrease, unsigned char dioDecrease)
{
	// Declarations. 
	pid_t curPid;

	// Initializations.
	curPid = getpid();

	// Set both DIO as Outputs.
    VSL_DIOSetChannelLevel(dioIncrease, DIO_OUTPUT);	
    VSL_DIOSetChannelLevel(dioDecrease, DIO_OUTPUT);	

	// Set both DIO to low.
	VSL_DIOSetChannelLevel(dioIncrease, DIO_CHANNEL_LOW);
	VSL_DIOSetChannelLevel(dioDecrease, DIO_CHANNEL_LOW);

	// Setup the DIO signal handler.
	gVlDIOSig.sa_sigaction = VL_DIO_SignalHandler;
	gVlDIOSig.sa_flags     = SA_SIGINFO;
	sigaction(SIG_VL_DIO, &gVlDIOSig, NULL);

	// Associate the signal handler with this process for each DIO.
	printf("Registering DIO/GPIO interrupt handler for process %ld.\n", (long)curPid);
	(void)VSL_DIOSetupInterrupts(curPid);
	(void)VSL_DIOSetupInterrupts(curPid);
}
/* *********************************************************************** */


/* ***** main program. ***** */
int main(int argc, char *argv[])
{
    /* ***** Variable declarations. ***** */
    unsigned long vl_ReturnStatus;	// Return status.
    
    unsigned char majorRev;			// Major revision number.
    unsigned char minorRev;			// Minor revision number.
    unsigned char releaseRev;		// Release revision number.
    int			   opt;				// Command line option helper.
    
    unsigned char  increaseDIO;     // Use this DIO channel.
	unsigned char  decreaseDIO;		// Use this DIO channel.
    
	char           curChar;         // Character from stdin.
	unsigned long  currentBlValue;  // Current value of the backlight.
    /* ********************************** */

    /* ***** Variable initializations. ***** */
    vl_ReturnStatus = 0xffffff;	// Assume a failure on open.
    majorRev		= (unsigned char)0;
    minorRev		= (unsigned char)0;
    releaseRev		= (unsigned char)0;
	opt	            = 0;
	
    increaseDIO    = DIO_CHANNEL_1;
    decreaseDIO    = DIO_CHANNEL_2;
    /* ************************************* */

    /* ***** Process command line options. ***** */
	// There is some functionality to be shown.
    while ((opt = getopt(argc, argv, "hv")) != -1)
    {
		switch(opt)
		{
			case 'v': 	gApiDebugLevel = VL_DEBUG_1;
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

	/* ***** Setup DIO to generate interrupts. ***** */
	setupInterrupts(increaseDIO, decreaseDIO);
	/* ********************************************* */

	/* ***** Setup display. ***** */
	// Set display name.
	// Display name can be found in the /sys/class/backlight directory.
	if (VSL_LCDSetDisplayName("acpi_video0") != VL_API_OK)
	{
		printf("Could not find specified display name.\n");
		exit(-1);
	}

	// Get and report the maximum backlight value.
	if (VSL_LCDGetBacklightMaximum(&gMaxBacklight) != VL_API_OK)
	{
	    printf("\nCould not get maximium backlight value.\n");
		gMaxBacklight = 100; 
	}
	else
	{
	    printf("\nMaximum backlight value = %lu;\n", gMaxBacklight);
	}

	// Get and report the current backlight value.
    if (VSL_LCDGetBacklight(&currentBlValue) != VL_API_OK)
	{
	    printf("\nCould not get backlight value.\n");
	}
	else
	{
		printf("Current backlight value = %lu;\n", currentBlValue);
	}

	// Set the backlight increment level.
	gBlChangeValue = 5;
	/* ************************ */

	/* ***** Get character input from stdin and process accordingly. ***** */
	// Get a character from stdin.
	printf("Press i to increase brightness, d to decrease brightness, q to quit\n");
	while ((curChar = getchar()) != EOF)
	{
		// Only process certian characters.
		if (curChar != '\n')
		{
		    switch (curChar)
		    {
			    case 'i': toggleDIO(increaseDIO);
						  break;
			    case 'd': toggleDIO(decreaseDIO);
						  break;
			    case 'q': printf("\tDone\n"); exit(0);
			    default:  printf("\tIgnoring\n");   break;
	        }

		// Next.
	    printf("i(ncrease), d(ecrease), q(uit)\n");
		}
	}
	/* ******************************************************************* */


    /* ***** VL_Close() should be called at the end of the program. ***** */
    VL_Close();
    /* ****************************************************************** */
    
    printf("\nEnd of VersaLogic Example Application execution.\n");

    return(0);
}

