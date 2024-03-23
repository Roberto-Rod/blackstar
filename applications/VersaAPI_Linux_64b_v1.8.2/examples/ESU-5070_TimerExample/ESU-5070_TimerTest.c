/* VersaLogic Sample Code - using low-level Linux port functions.
*
* Date			Version
* ---------------------
* 07/16/2019	 1.0 This example is intended to test DIO interupts on Grizzly 
* 09/30/2019	 2.0 Used example as a basis for a detailed timer/counter test.
* 10/01/2019	 2.1 Now have better understanding of the API (specifically the
* 					 originator of the IRQ and the EnableRest bit on the Set()
* 					 call. Also added a test PASS/FAIL section.
* 
*
* Command Line Options: 
* None.
* 
* */

/* ***** Includes Files. ***** */
// System.
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/io.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

// VersaAPI header.
#include "VL_OSALib.h"

#include <sys/time.h>

struct timeval stop, start;

int	gIrqGenerated;

/* *************************** */

#define VL_RETURN_SUCCESS 0	// A successful return status from a VL API call.
#define FPGA_BASE 0x0d80

#define       VSL_IOCTL_PASS_TIMER_PID    _IOWR(0xAA, 7, pid_t *)

char *VLDriveName           = "/dev/vldrive";

/* ***** Simple Signal Handler for the Grizzly timer/DIO interrupts. ***** */
void VL_GPIO_SignalHandler(int Signal, siginfo_t *Info, void *Nothing)
{
	// Increment the global IRQ count for this test.
	gIrqGenerated ++;

	if (Info)
	{
			// Bits 7 and 6 are used to tell which Timer generated the interrupt.
			int timerNum = 0;
			if (Info->si_int > 0)
				timerNum = 1;
			else
				timerNum = 0;
            printf("\t** Received Signal %d from Timer%d due to reaching a particular count ** ;\n", 
				Signal, timerNum);
            VSL_FPGAWriteRegister(0x26, Info->si_int);
	}
	else
	{
        printf("\t** Received Signal %d from an UNKNOWN SOURCE (%x); ** ;\n", Signal, Info->si_int);
	}
}

/* ***** main program. ***** */
int main(int argc, char *argv[])
{
    /* ***** Variable declarations. ***** */
    unsigned long vl_ReturnStatus;	// Return status.

    unsigned char majorRev;			// Major revision number.
    unsigned char minorRev;			// Minor revision number.
    unsigned char releaseRev;		// Release revision number.

    pid_t		  callingPId;     // Process ID (PID) of this process.

	int			curTimer;		// Current Timer number (0, or 1).
	int			numOfTimers; 	// Number of Timers for this board.
	int			sleepTime;		// Time to let counter count, in useconds.
	int			origCount;		// Timer count to start.
	int			newCount;		// Timer count after a sleep.
	int			deltaCount;		// Count difference between orig and new.
	int			basicFunctionsResults;	// 0 => fail, 1 => pass.
	int			matchFunctionsResults;	// 0 => fail, 1 => pass.

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

    /* For Grizzly disable port 80 and enable IRQ for GPIOs */
    VSL_FPGAWriteRegister(0x12, 0x01);

    /* Enable IRQ register for IRQ 5 */
    VSL_FPGAWriteRegister(0x2B, 0xA0);

    struct sigaction DIOSig;			// Signal structure.
    
    /* ***** Setup the VL GPIO Signal Handler ***** */
    DIOSig.sa_sigaction =	VL_GPIO_SignalHandler;
    DIOSig.sa_flags =		SA_SIGINFO;
    
    // Register the signal handler.
    printf("Register handler\n");
    sigaction(SIG_VL_DIO, &DIOSig, NULL);

    // Open a path to the driver.
    int driverHandle = open(VLDriveName, O_RDWR);

    callingPId = getpid();
    
    // Register the calling process PId with the driver.
    if(ioctl(driverHandle, VSL_IOCTL_PASS_TIMER_PID, callingPId))
    {
            printf("TimerTest: Failed to issue VSL_IOCTL_PASS_TIMER_PID to vldrive!");
            printf("\tInterrpt callbacks disabled.\n");
    }
	
	// Remember for Grizzly there are 2 timers, 0 and 1.
	numOfTimers		= 2;
	sleepTime		= 1000000;	// 1 seconds.

	// Turn on some debug info.
	//VSL_DebugLevelSet(VL_DEBUG_1);

	// Timer Start(), Stop() and Get() tests.
    printf("\n**************** Start(), Stop(), Reset() and Get() Tests **************************\n");
	printf("This test will:\n");
	printf("\t1. Reset timer.\n");
	printf("\t2. Get an initial timer count value.\n");
	printf("\t3. Start timer.\n");
	printf("\t4. Stop timer.\n");
	printf("\t5. Get a current timer count value.\n");
	printf("\t6. Get a second current timer count value.\n");
	printf("\t7. Reset timer.\n");
	printf("\t8. Get a reset timer count value.\n");
	printf("\t   Test is a PASS if count current count value > initial count value AND\n");
	printf("\t   current count value equals second current count value AND\n");
	printf("\t   reset count value equals 0\n");
	printf("\t9. Continue until all Timers have been run.\n");

    for (curTimer = 0; curTimer < numOfTimers; curTimer++)
    {
		printf("\n*** Running test on Timer%d\n", curTimer);

		// Assume test passes.
		basicFunctionsResults = 1;

		printf("\tReset timer.\n");
    	VSL_CTimerStop(curTimer);
   		VSL_CTimerClear(curTimer);
		VSL_CTimerReset(curTimer);

		int initialCount = VSL_CTimerGet(curTimer);
		printf("\tInitial count = %d;\n", initialCount); 

		printf("\tStarting timer ...");
    	VSL_CTimerStart(curTimer);
		usleep(sleepTime);
    	VSL_CTimerStop(curTimer);
		printf("... timer stopped\n");

		int currentCount = VSL_CTimerGet(curTimer);
		printf("\tcurrentCount = %d;\n", currentCount); 

		usleep(sleepTime);
		int secondCurrentCount = VSL_CTimerGet(curTimer);
		printf("\tsecondCurrentCount = %d;\n", secondCurrentCount); 

		printf("\tReset timer.\n");
		VSL_CTimerReset(curTimer);
		printf("\tGet a reset timer count value.\n");
		int resetCurrentCount = VSL_CTimerGet(curTimer);
		printf("\tresetCurrentCount = %d;\n", resetCurrentCount); 

		// Determine PASS/FAIL
		if ((currentCount > initialCount) 			&&
			(currentCount == secondCurrentCount)	&&
			(resetCurrentCount == 0))
		{
			basicFunctionsResults = 1;
		}
		else
		{
			basicFunctionsResults = 0;
		}
		printf("\t... Done\n");

		// Next Timer;
		printf("\n");
		sleep(1);
	}
    printf("***************************************************************************\n");


	// IRQ generation test.
    printf("\n**************** Timer IRQ Generation Test **************************\n");
	printf("This test will:\n");
	printf("\t1. Run a Timer for a known period of time capturing a count\n");
	printf("\t2. Divide that count by 2 to get match count value.\n");
	printf("\t3. Run a Timer for a known period of time looking for an interrupt to be geneated.\n");
	printf("\t4. Continue until all Timers have been run.\n");

	/* ***** General Timer setup. ***** */
	gIrqGenerated	      = 0;	// Number of IRQs generated by test. Updated in signal handler.
	matchFunctionsResults = 1;  // Assume pass.


    //for (curTimer = 1; curTimer >= 0; curTimer--)
    for (curTimer = 0; curTimer < numOfTimers; curTimer++)
    {
		printf("\n*** Running test on Timer%d\n", curTimer);

		// Initialize counters.
		origCount 	= 0;
		newCount  	= 0;
		deltaCount	= 0;
		
		// Initialize Timer.
		printf("\tInitializing Timer%d ...", curTimer);
    	VSL_CTimerStop(curTimer);
    	VSL_CTimerClear(curTimer);
		VSL_CTimerReset(curTimer);
		printf("... Done\n");

        VSL_FPGAWriteRegister(0x26, 0xFF);

		// Run the Timer for a known length of time.
		origCount = VSL_CTimerGet(curTimer);
		printf("\tRunning Timer%d for %d useconds to get count ...", curTimer, sleepTime);
        VSL_CTimerStart(curTimer);
		usleep(sleepTime);

		// Stop the Timer.
        VSL_CTimerStop(curTimer);

		// Get the Timer's count value.
		newCount = VSL_CTimerGet(curTimer);
		printf("... Done\n");
		deltaCount = newCount - origCount;
		printf("\t\tOriginal count=%d; Count after sleep=%d; Delta=%d\n",
			origCount, newCount, deltaCount);
		
		// Reset the Timer's count value.
		printf("\tReinitializing Timer%d ...", curTimer);
    	VSL_CTimerStop(curTimer);
    	VSL_CTimerClear(curTimer);
		VSL_CTimerReset(curTimer);
		printf("... Done\n");

		// Set a target count value to match and generate an interrupt when reached.
		int matchCount = deltaCount/4;

        printf("\tSetting Timer%d for a stop at match %d\n", curTimer, matchCount);
        printf("\t(Interrupt should be generated before the Timer is stopped.)\n");
		// VSL_CTimerSet(Timer,    ResetEnabled, StopEnabled, MatchValue, OutControl, OutInitState, IntEnable);
        //VSL_CTimerSet(   curTimer, 0           , 0          , matchCount, 0         , 0             , 1);

		// Used for resetting board.
        VSL_CTimerSet(   curTimer, 0           , 1          , matchCount, 0         , 0             , 1);

		// Run the Timer for the known length of time. Interrupt should be generated about
		// half way through.
		// Could use a global variable in the interrupt handler AND check the file system.

		// Run the Timer for a known length of time.
		origCount = VSL_CTimerGet(curTimer);
		printf("\tRunning Timer%d for %d useconds...\n", curTimer, sleepTime);
        VSL_CTimerStart(curTimer);
		usleep(sleepTime);

		// Stop the Timer.
        VSL_CTimerStop(curTimer);

		// Check if the timer stopped at the match value.
        newCount = VSL_CTimerGet(curTimer);
		if (newCount == matchCount)
		{
			printf("\tTimer stopped at match count %d\n", newCount);
			matchFunctionsResults = 1;
		}
		else
		{
			printf("\tTimer DID NOT stopped at match count: match=%d, current count=%d\n",
					newCount, matchCount);
			matchFunctionsResults = 0;
		}

		printf("\t... Done\n");

		// Next Timer;
		printf("\n");
		sleep(1);
    }
    printf("*********************************************************************\n");


    /* ***** VL_Close() should be called at the end of the program. ***** */
    VL_Close();

	// Print closing stats and PASS/FAIL results.
	printf("\n***** TEST RESULTS *****\n");

	// Basic functions.
	if (basicFunctionsResults == 0)
	{
		printf("Basic Funtions results: FAIL -");
		printf("Start(), Stop(), Reset() and/or Get() functions\n");
	}
	else
	{
		printf("Basic Funtions results: PASSED -");
		printf("Start(), Stop(), Reset() and/or Get() functions\n");
	}

	// Stop on match.
	if (matchFunctionsResults == 0)
	{
		printf("Match Funtions results: FAIL -");
		printf("Stopping timer are desired value\n");
	}
	else
	{
		printf("Match Funtions results: PASSED -");
		printf("Stopping timer are desired value\n");
	}

	// IRQ generation.
	if (gIrqGenerated != numOfTimers)
	{
		printf("IRQ Generation test results: FAIL -");
		printf("Total number if IRQs generated (should be %d): %d\n", numOfTimers, gIrqGenerated);
	}
	else
	{
		printf("IRQ Generation test results: PASSED -");
		printf("\nTotal number if IRQs generated (should be %d): %d\n", numOfTimers, gIrqGenerated);
	}
	printf("\n************************\n");

    printf("\nEnd of VersaLogic Example Application execution.\n");

    return(0);
}


