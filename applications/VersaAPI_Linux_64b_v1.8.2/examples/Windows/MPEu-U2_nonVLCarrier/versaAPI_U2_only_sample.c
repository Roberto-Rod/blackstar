/* VersaLogic VersaAPI Sample Code - 20160225
*
* This code is given with no guarantees and is intended to be used as an 
* example of how to use the VersaAPI software package and to check the 
* operation post install. This code will  work with various VersaLogic 
* single board computers and the VersaLogic VL-MPEe-A1/A2 miniPCIe board.
*
* Default Program Description: 
* This application will:
*    Report the VersaAPI Library version number
*    Report if the application is running on a VersaAPI or non-VersaAPI SBC
*    Report if there is an MPEe-U2E connected to the SBC
*    Try and toggle DIO_U2_CHANNEL_1 from LOW to HIGH, checking for the 
*        appropriate DIO level and reporting SUCCESS or FAILED as appropriate.
*
* This example application was compiled with:
*     Visual Studio 2017 (v141)
*     Windows SDK Version: 10.0.177763.0
*     Target Platform:     Windows 10
*     Configuration:       Debug
*     Platform:            x64
* 
* Command Line Options: 
* Run versaAPI_U2_only_sample -h for a list of command line options.
* 
*  */


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
	printf("  -v => Set API debug level to verbose. Default is no debug messages.\n");
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
    unsigned long vl_ReturnStatus;	    // Return status.
    unsigned char majorRev;			    // Major revision number.
    unsigned char minorRev;			    // Minor revision number.
    unsigned char releaseRev;		    // Release revision number.
    unsigned char  currentDIO;			// Use this DIO channel.
    char           currentDIOName[48];	// Name of current DIO channel.
    int			   opt;					// Command line option helper.
	int			   doSimpleU2DIO;		// Execute simple U2 DIO code. Default is No.
	unsigned long  curHwProds;          // What hardware are we running on and with.
    /* ********************************** */

    /* ***** Variable initializations. ***** */
    vl_ReturnStatus = 0xffffff;	// Assume a failure on open.
    majorRev		= (unsigned char)0;
    minorRev		= (unsigned char)0;
    releaseRev		= (unsigned char)0;
	curHwProds      = 0;

    currentDIO 	   = DIO_CHANNEL_1;
	sprintf_s(currentDIOName, sizeof("On-board DIO_CHANNEL_1"), "On-board DIO_CHANNEL_1");
	
	opt	= 0;  // Process command line options.
	
	doSimpleU2DIO = YES;  // By default do simple U2 DIO

	// Set default debug level.
	gApiDebugLevel = VL_DEBUG_OFF;  // No debug info by default.
    /* ************************************* */

    /* ***** Process command line options. ***** */
	if(argc > 2)
	{
		showUsage();
		exit(-1);
	}

	// There is some functionality to be shown.
    while ((opt = getopt(argc, argv, "vh")) != -1)
    {
		switch(opt)
		{
			case 'v':	gApiDebugLevel = VL_DEBUG_1;
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

	/* ***** Check hardware. ***** */
	// VSL_FindProducts returns an unsigned long:
	//   Bit 0 (LSB): 0 => Non VersaLogic SBC; 1 => VersaLogic SBC;
	//   Bit 1      : 0 => MPEe-A1/A2 not present; 1 => Present;
	//   Bit 21      : 0 => MPEe-U2e  not present; 1 => Present;
	curHwProds = VSL_FindProducts();

	// Report if we are running an a VersaLogic SBC.
	printf("Running on a VersaLogic SBC ...");
	if ((curHwProds & 0x01) == 1)
	{
		printf("... YES\n");
	}
	else
	{
		printf("... NO\n");
	}
	
	// Check to make sure an MPEe-U2E was found.
	printf("Looking for an MPEe-U2E device ...");
	if (VSL_FindProducts() >= 4)
	{
		printf("... Found\n");
	}
	else
	{
		printf("... ERROR: Did NOT Find an MPEe-U2E device!");
		exit(-1);
	}
	/* ************************* */

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


    /* ***** VL_Close() should be called at the end of the program. ***** */
    VL_Close();
    
    printf("\nEnd of VersaLogic Example Application execution.\n");

    return(0);
}





