/* VersaLogic VersaAPI Sample Code - 20210107
*
* This code is given with no guarantees and is intended to be used as an 
* example of how to use the VersaAPI software package and to check the 
* operation post install. This code will work with various VersaLogic 
* single board computers.
*
* Default Program Description: 
* This application will report the VersaAPI library version number, and
* then run a 8254 Timer example that will count down from a specifed count.
* When 0 is reached an interrupt (56) will be generated and the counter
* will be reinitialized and started again.  This will continue until a 
* specifed number of interrupts are generated.
* 
* Command Line Options: 
* Run timer8254_LoopExample -h for a list of command line options.
* 
* This code has been compiled and run using gcc on Ubuntu 18.04.03 LTS, 
* kernel 4.15.0-88-generic, with VersaAPI version 1.7.a.  A Linux
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
#define SLEEP_TIME 2 		// Time to sleep between setting the DIO.
#define VL_RETURN_SUCCESS 0	// A successful return status from a VL API call.

#define SIG_VL_TIMER 56		// VersaLogic 8256 Signal identifier.

#define YES 1				// Define yes.
#define NO  0				// Define no.

#define TIMER_MAX_LOOP_CNT 30 // Maximum count to wait for timer to generate interrupt.
/* ******************** */


/* ***** Global variables. ***** */
int gTimerTriggered;	// Global variable: Initial condition = NO,
						// After timer has triggered = YES.
int gMaxNumInterrupts;	// Number of interrupts to watch for before test stops.
int gTimerStartCount;   // Start counting down value.
int gCurrentInterruptCount; // Number of interrupts generated.

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


/* ***** Simple Signal Handler for the 8254 timer calls. ***** */
void VL_TMR_SignalHandler(int Signal, siginfo_t *Info, void *Nothing)
{
	int timerNum = -1;  // Timer that generated the Signal/IRQ.

	if (Info)
	{
		switch (Info->si_int)
		{
		    case 1: timerNum = 0; break;
			case 2: timerNum = 1; break;
			case 3: timerNum = 2; break;
			default: timerNum = -1; break;
		}
		
		// Set flag that a timer sent a signal.
		gTimerTriggered = YES;
		
		// Report which timer sent the signal.
		if ((timerNum >= 0) && (timerNum <=2))
		{
            printf("*** Recieved Signal %d from Timer number:%d ***\n", Signal, timerNum);
            printf("**********************************************\n");
            printf("********* Control Processing Here ************\n");
			printf("Number of VersaLogic Timer Interrupts post-test (should be one more than when the example startetd):\n");
		    system("cat /proc/interrupts | grep vldrive");
            printf("**********************************************\n");
		}
		else
		{
            printf("*** Recieved Signal %d with invalid source data; *** ;\n", Signal);
		}
	}
	else
	{
        printf("*** Recieved Signal %d from unknown source data; *** ;\n", Signal);
	}

	// Increment the number of times an interrupt has been generated.
	gCurrentInterruptCount ++;

	// Stop the timer.
    VL_TMRClear(0);
}
/* *********************************************************** */


/* ***** Function to display application help message. ***** */
void showUsage()
{
	printf("Usage: timer8254_LoopExample [-c <start_count> -i <interrupt_count> tvh]\n");
	printf("Application to show basic usage of VersaAPI commands.\n");
	printf("  -c => Start count of the timer. Default is 2000.\n");
	printf("  -i => Run the example until this number if interrupts are generated. Default is 10.\n");
	printf("  -t => Run 8254 Timer code. Default is to not run timer code.\n");
	printf("  -v => Set API debug level to verbose. Default is no debug messages.\n");
	printf("  -h => Print this help message and exit.\n");
	printf("Default is to run no examples\n");
	
	exit(-1);
}
/* ********************************************************* */


/* ***** Function to run the 8254 Timer example. ***** */
/* Description: One timer, VL_TIMER0, is used in this example.
 * The timer is stopped, a callback is registered for the timer, then the
 * timer is started in Mode 0 based on the internal clock. When the timer
 * value reaches 0 the timer is stopped and an interrupt is generated.            */
void run8254TimerExample()
{
	/* ***** Variable Declarations. ***** */
    unsigned short timerValue;		// 8254 Timer current count value.
	short          countDownCount;	// Current timer count down value.
	/* ********************************** */

	/* ***** Variable Initialization. ***** */
    timerValue = 0;
	/* ************************************ */

    /* ***** Use VL_TIMER0. ***** */
	printf("\n----- Using timer VL_TIMER%i -----\n", VL_TIMER0);
	printf("\nRun until this number of interrupts are received: %d\n", 
	       gMaxNumInterrupts);
	printf("Current number of VersaLogic Timer Interrupts pre-example:\n");
	system("cat /proc/interrupts | grep vldrive");
	printf("\n");
    
    // Start the VersaLogic 8254 Timer Signal Handler.
    printf("Registering timer callback.\n");
    VL_TMRGetEvent(VL_TIMER0, getpid());

	// No interrupts have been generated yet.
	gCurrentInterruptCount = 0;

	/* Run the loop until the desired number of interrupts are generated. */
    while(gCurrentInterruptCount < gMaxNumInterrupts)
    {
		// Interrupt has not been generated (yet).
	    gTimerTriggered = NO;

	    // Make sure timer is stopped.
	    printf("Stopping VL_TIMER%i\n", VL_TIMER0);
	    VL_TMRClear(VL_TIMER0);
   
	    printf("Starting timer VL_TIMER%i in Mode 0, value of %d with type of internal.\n", 
                   VL_TIMER0, gTimerStartCount);
	    VL_TMRSet(VL_TIMER0, VL_TIMER_MODE0, gTimerStartCount, VL_TIMER_TYPE_INTERNAL);
    
		// Initialize timer wait time.
	    countDownCount = 1;

	    while ((countDownCount <= TIMER_MAX_LOOP_CNT) && (gTimerTriggered == NO))
	    {
		    // Get the current timer value.
		    timerValue = VL_TMRGet(VL_TIMER0);
		    printf("Timer Count %i(%i): Timer Value=%d\n", countDownCount, 
				    TIMER_MAX_LOOP_CNT, timerValue);
		
		    // Do not want to loop forever.
		    countDownCount++;
	    }
	
		// Either the timer count has been exceeded, or an interrupt was generate.
	    if (countDownCount >= TIMER_MAX_LOOP_CNT)
		{
		    printf("Timer expired without an interrupt generated.\n");
	        printf("Current number of VersaLogic Timer Interrupts post-example:\n");
	        system("cat /proc/interrupts | grep vldrive");
	        printf("\n");
		    printf("Halting example.\n");

			// No reason to continue if interrupts are not being generated.
			break;
		}

		// Sleep a bit before we start up again.
	    sleep(SLEEP_TIME);
	}
	/* ************************************ */

	/* ***** Done. ***** */
	return;
	/* ***************** */
}
/* *************************************************** */


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


/* ***** main program. ***** */
int main(int argc, char *argv[])
{
    /* ***** Variable declarations. ***** */
    unsigned long vl_ReturnStatus;	// Return status.
    
    unsigned char majorRev;			// Major revision number.
    unsigned char minorRev;			// Minor revision number.
    unsigned char releaseRev;		// Release revision number.
    
    int			   opt;				// Command line option helper.
    int			   do8254Timer;		// Execute 8254 timer code. Default is No.
    /* ********************************** */

    /* ***** Variable initializations. ***** */
    vl_ReturnStatus = 0xffffff;	// Assume a failure on open.
    majorRev	= (unsigned char)0;
    minorRev	= (unsigned char)0;
    releaseRev	= (unsigned char)0;
	
	// Process command line options.
	opt	= 0; 
	
	// By default, do not execute any tests.
	do8254Timer	= NO;
	
    // Default start counting down value. Can override on command line.
    gTimerStartCount = 400;

	// Default number of interrupts to watch for before stopping test.
	gMaxNumInterrupts = 10;
    /* ************************************* */

    /* ***** Process command line options. ***** */
	if(argc <= 1)
	{
		showUsage();
		exit(-1);
	}

	// There is some functionality to be shown.
    while ((opt = getopt(argc, argv, "c:i:tv")) != -1)
    {
		switch(opt)
		{
			case 'c': 	gTimerStartCount = atoi(optarg);
						break;
			case 'i': 	gMaxNumInterrupts = atoi(optarg);
						break;
			case 't':	do8254Timer = YES;
						break;
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


	/* ***** Setup the VL 8254 Timer Signal Handler ***** */
	struct sigaction vlSig;
	vlSig.sa_sigaction	= VL_TMR_SignalHandler;
	vlSig.sa_flags		= SA_SIGINFO;
	
	// Register the signal handler.
	sigaction(SIG_VL_TIMER, &vlSig, NULL);
	/* ************************************************** */

    /* ***** VersaLogic 8254 Timer Example Section. ***** */
    if (do8254Timer == YES)
	{
		printf("***** Start VersaLogic 8254 Timer Loop Example. *****\n");

        run8254TimerExample();
		
		printf("\n***** End VersaLogic 8254 Timer Loop Example. *****\n");
	}
    /* ************************************************** */
    

    /* ***** VL_Close() should be called at the end of the program. ***** */
    VL_Close();
    
    printf("\nEnd of VersaLogic Example Application execution.\n");

    return(0);
}

