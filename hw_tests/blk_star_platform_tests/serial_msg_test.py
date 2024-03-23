#!/usr/bin/env python3
"""
Simple script file to test the functionality of th SerialMsgInterface class
"""
# -----------------------------------------------------------------------------
# Copyright (c) 2021, Kirintec
#
# -----------------------------------------------------------------------------
from serial_msg_intf import SerialMsgInterface, MsgId, MsgPayloadLen
import logging

log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

NO_TESTS = 10
COM_PORT = "/dev/ttyS0"
BAUD_RATE = 115200

if __name__ == "__main__":
    fmt = "%(asctime)s: %(message)s"
    # Set logging level DEBUG to see detailed information
    logging.basicConfig(format=fmt, level=logging.INFO, datefmt="%H:%M:%S")

    pass_count = 0
    fail_count = 0

    for i in range(0, NO_TESTS):
        with SerialMsgInterface(COM_PORT, BAUD_RATE) as smi:
            result = smi.send_ping()
            log.info("{} Ping {}".format(result, i))
            if result:
                pass_count += 1
            else:
                fail_count += 1
                
            result, msg = smi.get_command(MsgId.GET_SOFTWARE_VERSION_NUMBER,
                                          MsgPayloadLen.GET_SOFTWARE_VERSION_NUMBER)
            if result:
                payload_version, sw_major, sw_minor, sw_patch, sw_build = \
                    smi.unpack_get_software_version_number_response(msg)
                log.info("{} Sw Ver: V{}.{}.{}:{}".format(result, sw_major, sw_minor, sw_patch, sw_build))
                pass_count += 1
            else:
                log.info("{} Sw Ver: none".format(result))
                fail_count += 1
            
            key_ba = bytearray()
            key_ba.extend(map(ord, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345"))
            result = smi.send_set_key(key_ba)
            
            log.info("{} Set Key".format(result))
            if result:
                pass_count += 1
            else:
                fail_count += 1
                
            result, msg = smi.get_command(MsgId.GET_KEY,
                                          MsgPayloadLen.GET_KEY)
            if result:
                payload_version, key = smi.unpack_get_key_response(msg)
                try:
                    key_str = key.decode("utf8")
                    log.info("{} Get Key: {}".format(result, key_str))
                except:
                    log.info("{} Get Key: decode failed".format(result))
                pass_count += 1
            else:
                log.info("{} Get Key: none".format(result))
                fail_count += 1

    log.info("Pass Count: {}".format(pass_count))
    log.info("Fail Count: {}".format(fail_count))
    # smi.stop()
