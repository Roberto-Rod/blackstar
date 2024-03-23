# VersaLogic VersaAPI Sample Code Makefile - 20180412
#
# This code is given with no guarantees and is intended to be used as an 
# example of how to use the VersaAPI software package and to check the 
# operation post install. 
#
# This code has been compiled and run using gcc verions 4.4.5, Debian 
# QSP-LNX-VRTX-2.01, Advanced User Package QSP-LNX-VRTX-R2.01-AUP and
# kernel 2.6.39-vl-vrtx-r2.00.
#
# Usage: 	make clean	=> Removes all dependant .o files.
#		make all	=> Builds versaAPI_sample application
#				   for onboard DIO and versaAPI_sample_spx2
#				   for SPX-2 DIO.
#
# Build fruit is an example application 'versaAPI_sample' and/or 
# 'versaAPI_sample_spx2'
#

LIBS=	/usr/local/lib/libVL_OSALib.so
INCLUDEPATHS=	-I. \

O_TIMER_TEST_FILES=	ESU-5070_TimerTest.o
TIMER_TEST_C_FILES=	ESU-5070_TimerTest.c

all:	timerTest

timerTest:	$(O_TIMER_TEST_FILES)
	@echo 'Building target: $@'
	gcc -o timerTest $(O_TIMER_TEST_FILES) $(INCLUDEPATHS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

clean:	
	rm *.o timerTest 

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -g -Wall -c $(INCLUDEPATHS) -fmessage-length=0 -o "$@" "$<"
	@echo 'Finished building: $<'


