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
#include "stdafx.h"
#include <windows.h>

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
/* ***************************** */


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

void changeBlValue(char direction)
{
	/* ***** Declarations. ***** */
	unsigned long currentBLValue;
	unsigned long newBLValue;
	/* ************************* */

	/* ***** Initializations. ***** */
	newBLValue = 0;
	/* **************************** */

	/* ***** Change the value according to which button was pressed. ***** */
	// Get the current LCD backlight level.
	if (VSL_LCDGetBacklight(&currentBLValue) == VL_API_OK)
	{
		// Calculate the new backlight value.
		if (direction == 'i')
		{
			// Increase backlight level.
			newBLValue = currentBLValue + 1;
		}
		else if (direction == 'd')
		{
			// Decrease backlight level.
			newBLValue = currentBLValue - 1;
		}
	}
	else
	{
		printf("Could not retrieve current LCD backlight value.");
	}

	// Change the value based on user input.
	printf("Changing backlight value from %d to %d\n", currentBLValue, newBLValue);
	if (VSL_LCDSetBacklight(newBLValue) != VL_API_OK)
	{
		if (gAPIDebugLevel >= VL_DEBUG_1)
		{
			printf("Error setting Backlight value from %d to %d\n", currentBLValue, newBLValue);
		}
	}
	/* ******************************************************************* */
}


/* ***** main program. ***** */
int main(int argc, char *argv[])
{
    /* ***** Variable declarations. ***** */
    unsigned long vl_ReturnStatus;	// Return status.
    
    unsigned char majorRev;			// Major revision number.
    unsigned char minorRev;			// Minor revision number.
    unsigned char releaseRev;		// Release revision number.
    int			   opt;				// Command line option helper.

    
	char           curChar;         // Character from stdin.
	unsigned long  currentBlValue;  // Current value of the backlight.

	char* pPathToDisplay = "\\\\\.\\LCD";   // Window's path to the display
										    // to change the backlight
										    // value of.
    /* ********************************** */

    /* ***** Variable initializations. ***** */
    vl_ReturnStatus = 0xffffff;	// Assume a failure on open.
    majorRev		= (unsigned char)0;
    minorRev		= (unsigned char)0;
    releaseRev		= (unsigned char)0;
	opt	            = 0;
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

	// Open the LCD Panel.
	if (VSL_LCDSetDisplayName(pPathToDisplay) != VL_API_OK)
	{
		printf("Could not open LCD panel \"%s\"\n", pPathToDisplay);
		return(-1);
	}

	// Get and report the maximum backlight value.
	if (VSL_LCDGetBacklightMaximum(&gMaxBacklight) != VL_API_OK)
	{
	    printf("\nCould not get maximium backlight value.\n");
		// MM - gMaxBacklight = 100; 
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
			    case 'i': 
				case 'd': changeBlValue(curChar);
						  break;
			    case 'q': printf("\tDone\n"); exit(0);
			    default:  printf("\tInvalid input\n");   
					      break;
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

