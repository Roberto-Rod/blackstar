/* VersaLogic VersaAPI Sample Code - 20220126
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
* will be reinitialized and started again.  This will continue until the 
* specifed number of interrupts are generated. Timing information is
* printed to the screen: time captured immedidiately before the timer
* is started, and immediately when the timer interrupt service routine
* is called. This allows for capturing real-time timing information for
* the timer count-down.
* 
* Command Line Options: 
* Run timer8254_InterruptExample -h for a list of command line options.
* 
* This code has been compiled and run using gcc on Ubuntu 18.04.03 LTS, 
* kernel 4.15.0-88-generic, with VersaAPI version 1.8.b.  A Linux
* compatible Makefile has been provided with this .c file for an example
* of how to compile this code. */


/* ***** Includes Files. ***** */
// System.
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include <inttypes.h>
#include <time.h>
#include <math.h>

// VersaAPI header.
#include "VL_OSALib.h"
/* *************************** */

/* ***** Defines. ***** */
#define SLEEP_TIME 1 		// Time to sleep between setting the DIO.
#define VL_RETURN_SUCCESS 0	// A successful return status from a VL API call.

#define SIG_VL_TIMER 56		// VersaLogic 8256 Signal identifier.

#define YES 1				// Define yes.
#define NO  0				// Define no.

#define TIMER_MAX_LOOP_CNT 50000 // Maximum count to wait for timer to generate interrupt.
/* ******************** */


/* ***** Global variables. ***** */
int gTimerTriggered;	    // Global variable: Initial condition = NO,
						    // After timer has triggered = YES.
int gMaxNumInterrupts;	    // Number of interrupts to watch for before test stops.
int gTimerStartCount;       // Start counting down value.
int gCurrentInterruptCount; // Number of interrupts generated.
int gTimerMode;             // Timer Mode.
int gStopAllTimers;         // Whether to stop all timers pre-example start.
unsigned short gCurrentTimer;    // Current timer, needed in callback function.

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


/* ***** Helper Functions. ***** */
// Function to print current clock time.
void print_current_time_with_ms (void)
{
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;

    printf("%ld.%09ld,", (intmax_t)s, spec.tv_nsec);
}

// Function to display application help message.
void showUsage()
{
	printf("Usage: timer8254_LoopExample [-c <start_count> -i <interrupt_count> tvh]\n");
	printf("Application to show basic usage of VersaAPI commands.\n");
	printf("  -c => Start count of the timer. Default is 400.\n");
	printf("  -i => Run the example until this number if interrupts are generated. Default is 10.\n");
	printf("  -m => Set the 8254 Mode. Valid values are 0-5. Default is mode 0.\n");
	printf("  -t => Set 8254 Timer number to use. Valid values are 0-2. Default is 0.\n");
	printf("  -v => Set API debug level to verbose. Default is no debug messages.\n");
	printf("  -h => Print this help message and exit.\n");
	printf("Default is to run with Timer 0 in Mode 0.\n");
	
	exit(-1);
}

// Function to report generic board information.
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
/* ***************************** */


/* ***** Simple Signal Handler for the 8254 timer calls. ***** */
void VL_TMR_SignalHandler(int Signal, siginfo_t *Info, void *Nothing)
{
	int timerNum = -1;  // Timer that generated the Signal/IRQ.

	if (gApiDebugLevel > VL_DEBUG_1)
	{
	    printf("*** Entering VL_TMR_SignalHandler()\n");
	}

	// Set flag that a timer sent a signal.
	gTimerTriggered = YES;

	// Stop the timer.
	VL_TMRClear(gCurrentTimer);
	  
	// Make sure the signal came from the correct timer.
	if (Info)
	{
		switch (Info->si_int)
		{
		    case 1: timerNum = 0; break;
			case 2: timerNum = 1; break;
			case 4: timerNum = 2; break;
			default: timerNum = -1; break;
		}

		// Print time.
	    print_current_time_with_ms();
	    printf("\n");
		  
		// Report which timer sent the signal.
		if ((timerNum >= 0) && (timerNum <= 7))
		{
			if (gApiDebugLevel > VL_DEBUG_OFF)
			{
                printf("*** Recieved Signal %d from Timer number:%d ***\n", Signal, timerNum);
                printf("**********************************************\n");
			}
		}
		else
		{
            printf("*** Recieved Signal %d with invalid source data 0x%x; *** ;\n", 
				   Signal, Info->si_int);
		}
	}
	else
	{
        printf("*** Recieved Signal %d from unknown source data; *** ;\n", Signal);
	}

	// Increment the number of times an interrupt has been generated.
	gCurrentInterruptCount ++;

	if (gApiDebugLevel > VL_DEBUG_1)
	{
	    printf("*** Leaving VL_TMR_SignalHandler(%d)\n", timerNum);
	}
}
/* *********************************************************** */


/* ***** Function to run the 8254 Timer example. ***** */
/* Description: One timer is used in this example.
 * The timer is stopped, a callback is registered for the timer, then the
 * timer is started in Mode 0 based on the internal clock. When the timer
 * count value reaches 0 an interupt is generated and the timer is stopped. */
void run8254TimerExample(unsigned short timer)
{
	/* ***** Variable Declarations. ***** */
    unsigned short timerValue;		// 8254 Timer current count value.
	short          countDownCount;	// Current timer count down value.
    unsigned char  timerMode;		// 8254 Timer mode.
	/* ********************************** */

	/* ***** Variable Initialization. ***** */
    timerValue      = 0;   // Used if verbose mode only.
	countDownCount  = 0;   // Reset every loop.
	gTimerTriggered = NO;  // Updated in timer callback function.
	/* ************************************ */

	/* ***** Determine which timer mode to use. ***** */
	switch (gTimerMode)
	{
		case 0: timerMode = VL_TIMER_MODE0; break;
		case 1: timerMode = VL_TIMER_MODE1; break;
		case 2: timerMode = VL_TIMER_MODE2; break;
		case 3: timerMode = VL_TIMER_MODE3; break;
		case 4: timerMode = VL_TIMER_MODE4; break;
		case 5: timerMode = VL_TIMER_MODE5; break;
		default: printf("Invalid timer mode specified: %d\n", gTimerMode);
				 showUsage();
				 break;
	}
	/* ********************************************** */

    /* ***** Use specified timer. ***** */
	printf("\n----- Using timer VL_TIMER%i in mode VL_TIMER_MODE%d (%d) -----\n", 
		   timer, gTimerMode, timerMode);
	printf("\nRun until this number of interrupts are received: %d\n", 
	       gMaxNumInterrupts);
	printf("Current number of VersaLogic Timer Interrupts pre-example:\n");
	system("cat /proc/interrupts | grep vldrive");
	printf("\n");
    
    // Register the VersaLogic 8254 Timer Signal Handler.
    printf("Registering timer callback.\n");
    VL_TMRGetEvent(timer, getpid());

	// No interrupts have been generated yet.
	gCurrentInterruptCount = 0;
	
	// Make sure the current timer is stopped.
    printf("Stopping 8254 Timer VL_TIMER%d ...", timer);
    VL_TMRClear(timer);
    printf("... Done\n\n");

	/* Run the loop until the desired number of interrupts are generated. */
    while(gCurrentInterruptCount < gMaxNumInterrupts)
    {
		// Interrupt has not been generated (yet).
		// Set to YES in interrupt handler.
	    gTimerTriggered = NO;
		
		// Initialize timer wait time.
	    countDownCount = 1;
   
	    printf("Starting timer VL_TIMER%i in Mode %d, value of %d with type of internal.\n", 
                   timer, timerMode, gTimerStartCount);
		printf("    Loop %d,", gCurrentInterruptCount);
		print_current_time_with_ms();
	    VL_TMRSet(timer, timerMode, gTimerStartCount, VL_TIMER_TYPE_INTERNAL);
    
	    while ((countDownCount <= TIMER_MAX_LOOP_CNT) && (gTimerTriggered == NO))
	    {
		    // Get the current timer value.
		    if (gApiDebugLevel > VL_DEBUG_OFF)
			{
				// Do not waste time doing this if the output is not desired.
		        timerValue = VL_TMRGet(timer);
		        printf("Timer Count %i(%i): Timer Value=%d\n", countDownCount, 
				        TIMER_MAX_LOOP_CNT, timerValue);
			}
			
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
		/* ***************************************************************** */

		// Sleep a bit before we start up again.
	    sleep(SLEEP_TIME);
	}
	/* ************************************ */

	/* ***** Done. ***** */
	// Make sure timer is stopped.
	printf("Stopping VL_TIMER%i\n", timer);
	VL_TMRClear(timer);

	printf("Number of 8254 Timer Interrupts post-test, should be %d more than when example started:\n",
			gMaxNumInterrupts);
	system("cat /proc/interrupts | grep vldrive");
	printf("\n");

	return;
	/* ***************** */
}
/* *************************************************** */


/* ***** main program. ***** */
int main(int argc, char *argv[])
{
    /* ***** Variable declarations. ***** */
    unsigned long vl_ReturnStatus;	// Return status.
    
    unsigned char majorRev;			// Major revision number.
    unsigned char minorRev;			// Minor revision number.
    unsigned char releaseRev;		// Release revision number.
    
    int			   opt;				// Command line option helper.
    int			   cTimer;  		// Execute 8254 timer code. Default is No.
    /* ********************************** */

    /* ***** Variable initializations. ***** */
    vl_ReturnStatus = 0xffffff;	// Assume a failure on open.
    majorRev	= (unsigned char)0;
    minorRev	= (unsigned char)0;
    releaseRev	= (unsigned char)0;
	
	// Process command line options.
	opt	= 0; 
	
	// By default, do not execute any tests.
	cTimer = 0;
	
    // Default start counting down value. Can override on command line.
    gTimerStartCount = 400;

	// Default number of interrupts to watch for before stopping test.
	gMaxNumInterrupts = 10;

	// Timer mode to run.
	gTimerMode = 0;

	// Keep timers in current state at the start of the example.
	gStopAllTimers = NO;
    /* ************************************* */

    /* ***** Process command line options. ***** */
    while ((opt = getopt(argc, argv, "c:i:m:st:vh")) != -1)
    {
		switch(opt)
		{
			case 'c': 	gTimerStartCount  = atoi(optarg);
						break;
			case 'i': 	gMaxNumInterrupts = atoi(optarg);
						break;
			case 'm': 	gTimerMode        = atoi(optarg);
						break;
			case 's': 	gStopAllTimers    = YES;
						break;
			case 't':	cTimer            = atoi(optarg);
						break;
			case 'v': 	gApiDebugLevel    = VL_DEBUG_1;
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

	/* **** Separate timers 4 and 5 (1 & 2) => Set TCR bit 5 to 1. **** */
	printf("Separating timers 1 and 2 into two 16 bit timers.\n");
	(void) VSL_TMRCascadeTimers(FALSE);
	/* **************************************************************** */

	/* ***** Setup the VL 8254 Timer Signal Handler ***** */
	struct sigaction vlSig;
	vlSig.sa_sigaction	= VL_TMR_SignalHandler;
	vlSig.sa_flags		= SA_SIGINFO;
	
	// Register the signal handler.
	sigaction(SIG_VL_TIMER, &vlSig, NULL);
	/* ************************************************** */

	/* ***** Initial condition setup. ***** */
	if (gStopAllTimers == YES)
	{
	    printf("Stopping all timers pre-example start.\n");
		VL_TMRClear(VL_TIMER0);
		VL_TMRClear(VL_TIMER1);
		VL_TMRClear(VL_TIMER2);
	}
	else
	{
	    printf("Leaving timers in current state.\n");
	}
	/* ************************************ */

    /* ***** VersaLogic 8254 Timer Example Section. ***** */
	printf("***** Start VersaLogic 8254 Timer Loop Example. *****\n");
	switch (cTimer)
	{
        case 0: gCurrentTimer = VL_TIMER0; 
				run8254TimerExample(VL_TIMER0); 
				break;
        case 1: gCurrentTimer = VL_TIMER1; 
				run8254TimerExample(VL_TIMER1); 
				break;
        case 2: gCurrentTimer = VL_TIMER2; 
				run8254TimerExample(VL_TIMER2); 
				break;
		default: printf("Invalid timer number specified: %d\n", cTimer);
		showUsage();
	}

	printf("\n***** End VersaLogic 8254 Timer Loop Example. *****\n");
    /* ************************************************** */


    /* ***** VL_Close() should be called at the end of the program. ***** */
    VL_Close();
    
    printf("\nEnd of VersaLogic Example Application execution.\n");

    return(0);
}

