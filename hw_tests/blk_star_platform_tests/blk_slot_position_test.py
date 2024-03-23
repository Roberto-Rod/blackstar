#!/usr/bin/env python3
"""
Blah...
"""

# -----------------------------------------------------------------------------
# Copyright (c) 2024, Kirintec
#
# -----------------------------------------------------------------------------
"""
OPTIONS ------------------------------------------------------------------
None

ARGUMENTS -------------------------------------------------------------
None
"""

# -----------------------------------------------------------------------------
# Imports
# -----------------------------------------------------------------------------

# stdlib imports -------------------------------------------------------
import logging
import time
import sys
import subprocess

# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

# Set logging level to INFO initially
logging_fmt = "%(asctime)s: %(message)s"
logging_level = logging.INFO
logging.basicConfig(format=logging_fmt, datefmt="%H:%M:%S", stream=sys.stdout, level=logging_level)

DEBUG_CMD = True

def command_run(command):
    # Issues system command, checks last returned line matches expect
    # result = subprocess.run([command], capture_output=True, shell=True)
    subprocess.run([command], capture_output=True, shell=True)
    result = subprocess.getoutput(command)

    log.info(result)

    return result
    
def main():
    
    BLACKSTAR_GET_ECM_SLOT= "/mnt/sed/admin-data/apps/BlackStarECM -s"

    test_pass = True
    pass_count = 0

    for i in range(8):

        log.info("Expecting ECM slot postion : {}".format(i))
        time.sleep(2)
        result = command_run(BLACKSTAR_GET_ECM_SLOT)
        if result == "ECM slot number: {}".format(i):
            log.info("Pass")
            pass_count += 1
            test_pass &= True
        else:
            test_pass &= False
        
        time.sleep(1)
    
    if test_pass:
        log.info("PASS - Blackstar ECM Slot Position Test")
    else:
        log.info("FAIL - Blackstar ECM Slot Position Test")

    return test_pass, pass_count

# ECM slot number: 7
# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    """
    Call runtime procedure and execute test
    """
    main()