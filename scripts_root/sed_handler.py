#!/usr/bin/env python3

from serial_msg_intf import SerialMsgInterface, MsgId, MsgPayloadLen
import argparse
import os
import subprocess
import time

DEBUG_CMD = True
DEBUG_KEY = False

MAX_UNLOCK_ATTEMPTS = 10
SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))
SED_PATH = "/dev/sda"
MOUNT_POINT = "/mnt/sed"

CMD_FILE_WATCHER = "/mnt/sed/admin-data/scripts/watch_active_file.sh"
CMD_BLACKSTAR_ECM = "/mnt/sed/admin-data/apps/BlackStarECM"
CMD_UBX_EPHEMERIDES = "/mnt/sed/admin-data/apps/ubx-ephemerides"

USER = "root"
GROUP = "sed"
SERIAL_PORT = "/dev/ttyS0"
BAUD_RATE = 115200

def command_expect(command, expect):
    # Issues system command, checks last returned line matches expect
    result = subprocess.getoutput("{} 2>&1 | tail -1".format(command)).strip()
    if result == expect:
        return True
    else:
        if DEBUG_CMD:
            print("expected: \"{}\", got: \"{}\"".format(expect, result))
        return False

def get_key_from_bscm():
    if DEBUG_KEY:
        return "1234567890ABCDEFGHIJKLMNOPQRSTUV"
    else:
        with SerialMsgInterface(SERIAL_PORT, BAUD_RATE) as smi:
                result, msg = smi.get_command(MsgId.GET_KEY, MsgPayloadLen.GET_KEY)
                if result:
                    payload_version, key = smi.unpack_get_key_response(msg)
                    try:
                        return key.decode("utf8")
                    except:
                        pass
    # If we reach here then something didn't work...
    return None

def send_key_to_bscm(key):
    with SerialMsgInterface(SERIAL_PORT, BAUD_RATE) as smi:
        key_ba = bytearray()
        key_ba.extend(map(ord, key))
        return smi.send_set_key(key_ba)

def make_key():
    if DEBUG_KEY:
        return "1234567890ABCDEFGHIJKLMNOPQRSTUV"
    else:
        return subprocess.getoutput("/bin/cat /proc/sys/kernel/random/uuid | /usr/bin/sha256sum | base64 -w 0 | /usr/bin/cut -c -32")

def sed_detected():
    return command_expect("{}/sedutil-cli --isValidSED {} | cut -d ' ' -f 2".format(SCRIPT_PATH, SED_PATH), "SED")

def is_mounted():
    return command_expect("/usr/bin/mount | grep {} | cut -d ' ' -f 1".format(SED_PATH), SED_PATH)

def initialise_sed(sid, admin1):
    ok = True
    unmount_sed()
    ok = ok and command_expect("{}/sedutil-cli --initialSetup {} {}".format(SCRIPT_PATH, sid, SED_PATH), "Initial setup of TPer complete on {}".format(SED_PATH))
    ok = ok and command_expect("{}/sedutil-cli --enableLockingRange 0 {} {}".format(SCRIPT_PATH, sid, SED_PATH), "LockingRange0 enabled ReadLocking,WriteLocking")
    ok = ok and command_expect("{}/sedutil-cli --setAdmin1Pwd {} {} {}".format(SCRIPT_PATH, sid, admin1, SED_PATH), "Admin1 password changed")
    ok = ok and command_expect("{}/sedutil-cli --setMBREnable off {} {}".format(SCRIPT_PATH, admin1, SED_PATH), "MBREnable set off")
    return ok

def unlock_sed(admin1):
    return command_expect("{}/sedutil-cli --setLockingRange 0 rw {} {}".format(SCRIPT_PATH, admin1, SED_PATH), "LockingRange0 set to RW")

def revert_sed(sid):
    return command_expect("{}/sedutil-cli --revertTPer {} {}".format(SCRIPT_PATH, sid, SED_PATH), "revertTper completed successfully")

def get_sed_hashed_msid():
    hashed_msid = subprocess.getoutput("{}/sedutil-cli --printDefaultPassword {} | /usr/bin/sha256sum | base64 -w 0 | /usr/bin/cut -c -32".format(SCRIPT_PATH, SED_PATH))
    return hashed_msid

def format_sed():
    subprocess.getoutput("mkfs.ext4 {}".format(SED_PATH))
    return True

def unmount_sed():
    return command_expect("umount {}".format(SED_PATH), "")

def mount_sed():
    ok = True
    unmount_sed()
    ok = ok and command_expect("mkdir -p {};mount {} {}".format(MOUNT_POINT, SED_PATH, MOUNT_POINT), "")
    ok = ok and command_expect("chown {}:{} {}".format(USER, GROUP, MOUNT_POINT), "")
    ok = ok and command_expect("chmod 770 {}".format(MOUNT_POINT), "")
    ok = ok and command_expect("mount | grep {}".format(SED_PATH), "{} on {} type ext4 (rw,relatime)".format(SED_PATH, MOUNT_POINT))
    return ok

def reinitialise_and_send_key():
    ok = True
    admin1 = make_key()
    sid = get_sed_hashed_msid()
    print("revert: ", end="", flush=True)
    revert_sed(sid)
    if ok:
        print("ok")
        print("initialise: ", end="", flush=True)
        ok = initialise_sed(sid, admin1)
    if ok:
        print("ok")
        print("unlock: ", end="", flush=True)
        ok = unlock_sed(admin1)
    if ok:
        print("ok")
        print("format: ", end="", flush=True)
        ok = format_sed()
    if ok:
        print("ok")
        print("send: ", end="", flush=True)
        ok = send_key_to_bscm(admin1)
    if ok:
        print("ok")
        return True
    else:
        print("fail")
        return False

def get_key_and_unlock():
    ok = False
    # Get key
    print("get key: ", end="", flush=True)
    admin1 = get_key_from_bscm()
    if admin1:
        print("ok")
        # Unlock SED with the key
        print("unlock: ", end="", flush=True)
        ok = unlock_sed(admin1)
        if ok:
            print("ok")
            print("mount: ")
            ok = mount_sed()
    if ok:
        print("ok")
        return True
    else:
        print("fail")
        return False


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--reinitialise", help="generate new key and reinitialise SED",
                        action="store_true")
    args = parser.parse_args()
    reinitialise = False
    locked = True

    if sed_detected():
        if args.reinitialise:
            print("reinitialise")
            reinitialise_and_send_key()
            exit(1)
        else:
            if is_mounted():
                locked = False
                print("not locked")
            unlock_attempt = 1
            while locked and unlock_attempt <= MAX_UNLOCK_ATTEMPTS:
                if get_key_and_unlock():
                    print("unlocked")
                    locked = False
                else:
                    print("unlock failed on try {}".format(unlock_attempt))
                    unlock_attempt+=1
    else:
        print("no SED detected")

    if not locked:
        # If SED has been detected and unlocked then remove all previous ephemeris files
        if False:
            ephem_dir = "/mnt/sed/admin-data/ephem"
            for filename in os.listdir(ephem_dir):
                filepath = os.path.join(ephem_dir, filename)
                try:
                    if os.path.isfile(filepath):
                        os.unlink(filepath)
                except Exception as e:
                    print("Failed to delete {}: {}".format(filepath, e))
        
        # Now run the apps which should never exit
        print("starting BlackStar file watcher...")
        proc1 = subprocess.Popen(CMD_FILE_WATCHER, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print("starting BlackStarECM...")
        proc2 = subprocess.Popen(CMD_BLACKSTAR_ECM, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        # Sleep for 2 seconds to ensure GNSS receiver is out of reset
        time.sleep(2)
        print("starting ubx-ephemerides...")
        proc3 = subprocess.Popen(CMD_UBX_EPHEMERIDES, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        while True:
            time.sleep(1)
            if proc1.poll():
                # Process 1 ended
                print("{} ended".format(CMD_FILE_WATCHER))
            if proc2.poll():
                # Process 2 ended
                print("{} ended".format(CMD_BLACKSTAR_ECM))
            if proc3.poll():
                # Process 2 ended
                print("{} ended".format(CMD_UBX_EPHEMERIDES))

    # If we reach this point then spin here, slowly
    print("spinning...")
    while True:
        time.sleep(10)
