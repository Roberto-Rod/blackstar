#!/bin/bash

#
# Written by VersaLogic Coporation, June 2019.
# Updated February 2020 - Added MPEu-C1E, EPU-401x, ESU-5070
#                       - Release 1.6.0
#                       - Release 1.6.1;
# Updated March 2021    - Added MPEe-51
# 						- Moved MPEu-C1E install into it's own script.
# Updated July 2021     - 1.7.2 VersaAPI Release.  Changed vldrive install
#                         to no longer require FPGA_BASE to be passed in.
#                         Added vldrive.h.template to package.
#                         Removed vldrive.h, it is now created from
#                         vldrive.h.template during the make process.
#                         Added the EPMe-51 and EPMe-5021.
#


# ***** Variable declarations. *****
VERSION=1.8.2;
LIB_DIR=/usr/local/lib;
VL_LIB_NAME=libVL_OSALib.$VERSION.so;
CG_LIB_NAME=$VL_LIB_NAME;
MCU_LIB_NAME=libVL_OSALib_mcu.$VERSION.so;
ARM_LIB_NAME=libVL_OSALib_arm.$VERSION.so;
SRC_LIB_NAME="";
LIB_CONF=/etc/ld.so.conf;

LIB_CG="no";
LIB_CG_NAME="libcgos.so";
VL_CG_DEV_NAME="/dev/cgos";
VL_PERM_FILE="S04vl_check_drivers";

BOARD_NAME="";
FPGA_ADDR="0xC80";
# **********************************

printUsage ()
{
    echo "Usage: $0 <VERSALOGIC_BOARD_NAME>";
    echo "    Where VERSALOGIC_BOARD_NAME must be one of the following:";
    echo "      EBX-18, EBX-38, EPM-31, EPM-43, EPMe-30, EPM-39, EPMe-42, EPMe-51, EPMe-5120, "
    echo "      EPM-19, EPU-3311, EPU-3312, EPU-4x62, EPU-4460, EPU-4011, EPU-4012, ";
    echo "      MPEe-A1/A2, MPEe-U2, ESU-5070";
    echo "    Install the VersaLogic API library \"$TARGET_LIB_NAME\".";
    echo "    Compile and install the VersaLogic API drivers \"vldrive*\".";
    echo "    This script should be run as user root, or a user with similar access.";
}

echo "VerasLogic Corporation VersaAPI Library and Driver Installation script";
echo "***** VerasLogic Corporation VersaAPI Library Installation *****";
echo "Version:$VERSION";
echo `date`;
echo "";

# ***** Command line parameter checking. ***** 
if [ $# -eq 1 ] 
then
    BOARD_NAME=$1;
    echo "Board: $BOARD_NAME";
 
    case "$1" in
	EPM-31|EPMe-30|EPM-39|EPMe-42|EPM-43|EBX-38|EPMe-51|EPMe-5120)
	    FPGA_ADDR="0xC80";
	    SRC_LIB_NAME=$VL_LIB_NAME;
        LIB_CG="no";
	    ;;
	EPU-3311|EPU-3312)
	    FPGA_ADDR="0x1C80";
	    SRC_LIB_NAME=$CG_LIB_NAME;
        LIB_CG="yes";
	    ;;
	EPU-4011|EPU-4012)
	    FPGA_ADDR="0x1C80";
	    SRC_LIB_NAME=$VL_LIB_NAME;
        LIB_CG="no";
	    ;;
	EPU-4x62|EPU-4460)
	    FPGA_ADDR="0xC80";
	    SRC_LIB_NAME=$CG_LIB_NAME;
        LIB_CG="yes";
	    ;;
	EPM-19|EBX-18|MPEe-A1/A2|MPEe-U2)
	    echo "FPGA_BASE does not need to be set for this board.";
	    FPGA_ADDR="0xCA0";
	    SRC_LIB_NAME=$VL_LIB_NAME;
        LIB_CG="no";
	    ;;
	ESU-5070)
	    FPGA_ADDR="0xD80";
	    SRC_LIB_NAME=$MCU_LIB_NAME;
        LIB_CG="no";
	    ;;
	* )
	    echo "Unrecognized board name!";
	    FPGA_ADDR="0x000";
        printUsage;
        exit -1;
	    ;;
    esac

    echo "Setting FPGA_BASE=$FPGA_ADDR";
else
    printUsage;

    exit -1;
fi
# ******************************************** 

# ***** Check for write permissions for the necessary files/directories. *****
if [ ! -w "$LIB_CONF" ]
then
    echo "Write permission denied for file: '$LIB_CONF'.";
    echo "Installation Incomplete.";
    exit -1;
fi
# ****************************************************************************

# ***** Continuing with the installation. *****
# Library.
echo -n "Installing VersaLogic API library '"$SRC_LIB_NAME"' to '$LIB_DIR'...";
cp $SRC_LIB_NAME $LIB_DIR/$SRC_LIB_NAME;
echo "...Done.";

echo -n "Installing VersaLogic Carrier Group API library 'libcgos' to '$LIB_DIR'...";
install "$LIB_CG_NAME" "$LIB_DIR";
echo "...Done.";

echo -n "Linking library...";
ln -sf "$LIB_DIR"/"$SRC_LIB_NAME" "$LIB_DIR"/libVL_OSALib.so;
ln -sf "$LIB_DIR"/"$SRC_LIB_NAME" "$LIB_DIR"/libVL_OSALib.so.1;
echo "...Done.";

echo -n "Loading library configurations...";
grep "$LIB_DIR" "$LIB_CONF" >> /dev/null;
if [ $? -ne "0" ]
then
    echo "...Done."
    echo -n "Updating $LIB_CONF...";
    echo "$LIB_DIR" >> "$LIB_CONF";
fi
echo "...Done.";

echo -n "Running ldconfig...";
ldconfig;
echo "...Done.";

echo;
ls -l libVL_OSALib*;
ls -l "$LIB_DIR"/libVL_OSALib* "$LIB_DIR"/"$LIB_CG_NAME";

echo;
echo "****************************************************************";
# *********************************************


echo "***** VerasLogic Corporation VersaAPI Driver Installation *****";
echo `date`;
echo "";

# ***** Variable declarations. *****
KERNEL_DIR=`uname -r`;
KERNEL_HEADERS="/lib/modules/$KERNEL_DIR/build";
DRIVER_DIR="/lib/modules/$KERNEL_DIR";
DRIVER_ARGS="$FPGA_ADDR";
# **********************************

# ***** OS overrides. ***** 
grep "CentOS|Rocky" /etc/redhat-release >> /dev/null;
if [ $? == "0" ]
then
    sed -i 's/5,0,0/4,0,0/' ./src/vldrive/vldrive.c
    sed -i 's/5,14,9/4,14,9/' ./src/vldrivecb/Lx/DrvOsaLx.c
fi
# ************************* 

# ***** Check for existence of kernel source directories. *****
if [ ! -d "$KERNEL_HEADERS" ]
then
    echo "Kernel header file source does not exist at: '$KERNEL_HEADERS'.";
    echo "Driver Installation Incomplete.";
    echo "Please install kernel headers and ensure they are symlinked at '$KERNEL_HEADERS'.";
    exit -1;
fi
echo "Found kernel source at:"$KERNEL_HEADERS"";
# **************************************************************

# ***** Check for existence of driver installation directory. *****
if [ ! -d "$DRIVER_DIR" ]
then
    echo "Kernel header file source does not exist at: '$DRIVER_DIR'.";
    echo "Driver Installation Incomplete.";
    exit -1;
fi
echo "Will install drivers at:"$DRIVER_DIR"";
# *****************************************************************

# ***** Compile and install drivers. *****
# ***** Look for driver source code.
if [ ! -d "./src" ]
then
    echo "Driver source does not exist at: '`pwd`/src'.";
    echo "Driver Installation Incomplete.";
    echo "Please ensure all VersaAPI files are extracted and the directory structure is kept in tact.";
    exit -1;
fi

# Look for kernel source code.
# First one it finds, use it.
kDir=`uname -r`;
ubuntuKSrc="/usr/src/linux-headers-$kDir";
centOSKSrc="/usr/src/kernels/$kDir";
userKSrc="";
for currentKSrc in "$userKSrc" "$ubuntuKSrc" "$centOSKSrc"
do
	echo "CHECKing for:$currentKSrc;";
done;

curDir=`pwd`;
for currentDriver in "vldrive" "vldriveax" "vldrivep" 
do
    echo "***** Compiling driver: "$currentDriver"";
    driverDir="./src/"$currentDriver"";
    if [ ! -d "$driverDir" ]
    then
        echo "Driver source directory does not exist: "$driverDir".";
        echo "Not building driver in directory: "$driverDir".";
    else
        cd $driverDir;

	    # vldrive is just a bit different from the others.
		if [ "$currentDriver" = "vldrive" ]
		then
            make clean; make build_"$FPGA_ADDR";
	    else
            make clean; make;
		fi

		# Remove any older driver that may be loaded.
		echo -n ""$currentDriver": Checking if already loaded and running...";
		lsmod | grep "$currentDriver " &> /dev/null;
		if [ $? == "0" ]
		then
            echo -n "...found driver loaded/running, removing it...";
            rmmod $currentDriver;
		else
			echo -n "...driver NOT loaded/running...";
		fi
		echo "...Done.";

        echo ""$currentDriver": Adding the driver to the kernel.";
        insmod "$currentDriver".ko;

		# Installing the driver for reboot.
		echo ""$currentDriver": Installing the driver for reboot";
		systemctl &> /dev/null;
		if [ $? == "0" ]
		then
	        make install;
		else
			make install5;
		fi

        cd "$curDir";
	fi
done;

if [ "$LIB_CG" == "yes" ]
then
    currentDriver="vldrivecb";
    echo "***** Compiling driver: "$currentDriver"";
    driverDir="./src/vldrivecb/Lx";
    if [ ! -d "$driverDir" ]
    then
        echo "Driver source directory does not exist: "$driverDir".";
        echo "Not building driver in directory: "$driverDir".";
    else
        cd "$driverDir";
        make clean; make;

        echo "Removing the driver to the kernel: "$currentDriver".";
        rmmod "cgosdrv";

        echo "Adding the driver to the kernel: "$currentDriver".";
        insmod "cgosdrv".ko;
        make install;

    	echo -n "Configuring VersaLogic Carrier Group device '$VL_CG_DEV_NAME'..."; 
		driveDir="/etc/rc5.d";
    	if [ ! -d "$driveDir" ]
    	then
        	echo "Device directory does not exist: "$driverDir".";
        	echo "VersaAPI applications must be run as user 'root'.";
    	else
    		cp "$VL_PERM_FILE" "$driveDir";
			./"$VL_PERM_FILE";
    		echo "...Done.";
		fi

        cd "$curDir";
    fi
fi
# ****************************************

echo "***************************************************************";

echo "VersaAPI Driver Installation Complete.";
echo `date`;
exit 0;

