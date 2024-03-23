SETTINGS="app_settings.json"
APP_LAUNCHER="/mnt/sed/admin-data/scripts/app_launcher.py"

if [ ! -f "active_file" ]; then
    echo "ERROR: active_file does not exist"
    exit 0
fi
   
ARCHIVE=$(head -n 1 active_file)
if [ ! -f ${ARCHIVE} ]; then
    echo "ERROR: ${ARCHIVE} does not exist"
    exit 0
fi

# Reset Ettus device
#BUS=$(lsusb | grep Ettus | cut -d ' ' -f 2)
#DEVICE=$(lsusb | grep Ettus | cut -d ' ' -f 4 | cut -d ':' -f 1)
#./usbreset /dev/bus/usb/${BUS}/${DEVICE}

rm -rf active_app
mkdir active_app
unzip ${ARCHIVE} -d active_app
cd active_app
if [ -f ${SETTINGS} ]; then
    python3 ${APP_LAUNCHER} ${SETTINGS}
    
else
    echo "ERROR: no ${SETTINGS} file found"
fi
