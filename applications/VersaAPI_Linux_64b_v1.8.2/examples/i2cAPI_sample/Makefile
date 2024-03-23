# VersaLogic VersaAPI Sample Code Makefile - 20160225
#
# This code is given with no guarantees and is intended to be used as an 
# example of how to use the VersaAPI software package and to check the 
# operation post install. 
#
# Usage: 	make clean	=> Removes all dependant .o files.
#		make 		=> Builds versaAPI_I2CSample application.
#
# Build fruit is an example application 'versaAPI_I2CSample'

# ********** Library section *********
LIBS=/usr/local/lib/libVL_OSALib.so /usr/local/lib/libcgos.so

# ************************************

INCLUDEPATHS=	-I. -I/usr/include

O_FILES=	versaAPI_I2CSample.o
SRC_FILES=	versaAPI_I2CSample.c

all:	example

example:	$(O_FILES)
	@echo 'Building target: $@'
	gcc -o versaAPI_I2CSample $(DIO_FLAG) $(O_FILES) $(INCLUDEPATHS) $(LIBS);
	@echo 'Finished building target: $@'
	@echo ' '

clean:	
	rm *.o versaAPI_I2CSample

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Wall -c $(DIO_FLAG) $(INCLUDEPATHS) -fmessage-length=0 -o "$@" "$<"
	@echo 'Finished building: $<'


