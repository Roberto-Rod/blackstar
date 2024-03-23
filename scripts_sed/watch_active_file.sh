#!/bin/sh

# Trap ctrl-c
trap terminate INT

WATCH_DIR="/home/blackstar"
WATCH_FILE="active_file"
APP_DIR="active_app"
SCRIPTS_DIR="/mnt/sed/admin-data/scripts"
PID=0

cd ${WATCH_DIR}
rm -f ${WATCH_FILE}

kill_descendants() {
    if [ $PID != 0 ]; then
        DESC=$(pstree -p ${PID} | grep -o '([0-9]\+)' | grep -o '[0-9]\+')
        echo "Killing PIDs: ${DESC}"
        kill ${DESC}
    fi
}

terminate() {
    echo "Terminate application..."
    kill_descendants
    rm -rf ${APP_DIR}
}

# Probe USRP device to load firmware before launching applications
uhd_usrp_probe

# Watch this directory for changes
inotifywait -m -e close_write,delete ./ | while read DIRECTORY EVENT FILE; do
    if [ ${FILE} = ${WATCH_FILE} ]; then
        case $EVENT in
            CLOSE_WRITE*)
                # WATCH_FILE modified
                kill_descendants
                ${SCRIPTS_DIR}/unpack_and_run_active_app.sh &
                PID=$!
                echo "Launched process ID ${PID}"
                sleep 1
                ;;
            DELETE*)
                # WATCH_FILE deleted
                terminate
                ;;
        esac
    fi
done
