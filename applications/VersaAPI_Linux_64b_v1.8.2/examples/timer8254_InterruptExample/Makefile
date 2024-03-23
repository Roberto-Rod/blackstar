# VersaLogic VersaAPI Sample Code Makefile - 20160225
#
# This code is given with no guarantees and is intended to be used as an 
# example of how to use the VersaAPI software package and to check the 
# operation post install. 
#
# Usage: 	make clean	=> Removes all dependant .o files.
#		make 		=> Builds timer8254_InterruptExample application.
#
# Build fruit is an example application 'timer8254_InterruptExample'

# ********** Library section *********
# CG_LIBS=/usr/local/lib/libcgos.so
LIBS=	/usr/local/lib/libVL_OSALib.so

# ************************************

INCLUDEPATHS=	-I. -I/usr/include

O_FILES=	timer8254_InterruptExample.o
SRC_FILES=	timer8254_InterruptExample.c

all:	example

example:	$(O_FILES)
	@echo 'Building target: $@'
#	if [ -f '$(CG_LIBS)' ]; then \
    		LIBS=$(LIBS)
#	fi
	gcc -o timer8254_InterruptExample $(DIO_FLAG) $(O_FILES) $(INCLUDEPATHS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

clean:	
	rm *.o timer8254_InterruptExample

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Wall -c $(DIO_FLAG) $(INCLUDEPATHS) -fmessage-length=0 -o "$@" "$<"
	@echo 'Finished building: $<'


