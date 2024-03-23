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
import argparse
import time
from raspberry_pi_interface import RaspberryPiInterface

# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

def main(position):

    rasp_pi_interface = RaspberryPiInterface()
    rasp_pi_interface.initialise()

    if position == "0":
        log.info("INFO - Setting slot number to 0...")
        rasp_pi_interface.set_ctl_gpio0_pin(False)
        rasp_pi_interface.set_ctl_gpio1_pin(False)
        rasp_pi_interface.set_ctl_gpio2_pin(False)
        time.sleep(5)
    elif position == "1":
        log.info("INFO - Setting slot number to 1...")
        rasp_pi_interface.set_ctl_gpio0_pin(True)
        rasp_pi_interface.set_ctl_gpio1_pin(False)
        rasp_pi_interface.set_ctl_gpio2_pin(False)
        time.sleep(5)
    elif position == "2":
        log.info("INFO - Setting slot number to 2...")
        rasp_pi_interface.set_ctl_gpio0_pin(False)
        rasp_pi_interface.set_ctl_gpio1_pin(True)
        rasp_pi_interface.set_ctl_gpio2_pin(False)
        time.sleep(5)
    elif position == "3":
        log.info("INFO - Setting slot number to 3...")
        rasp_pi_interface.set_ctl_gpio0_pin(True)
        rasp_pi_interface.set_ctl_gpio1_pin(True)
        rasp_pi_interface.set_ctl_gpio2_pin(False)
        time.sleep(5)
    elif position == "4":
        log.info("INFO - Setting slot number to 4...")
        rasp_pi_interface.set_ctl_gpio0_pin(False)
        rasp_pi_interface.set_ctl_gpio1_pin(False)
        rasp_pi_interface.set_ctl_gpio2_pin(True)
        time.sleep(5)
    elif position == "5":
        log.info("INFO - Setting slot number to 5...")
        rasp_pi_interface.set_ctl_gpio0_pin(True)
        rasp_pi_interface.set_ctl_gpio1_pin(False)
        rasp_pi_interface.set_ctl_gpio2_pin(True)
        time.sleep(5)
    elif position == "6":
        log.info("INFO - Setting slot number to 6...")
        rasp_pi_interface.set_ctl_gpio0_pin(False)
        rasp_pi_interface.set_ctl_gpio1_pin(True)
        rasp_pi_interface.set_ctl_gpio2_pin(True)
        time.sleep(5)
    elif position == "7":
        log.info("INFO - Setting slot number to 7...")
        rasp_pi_interface.set_ctl_gpio0_pin(True)
        rasp_pi_interface.set_ctl_gpio1_pin(True)
        rasp_pi_interface.set_ctl_gpio2_pin(True)
        time.sleep(5)
    else:
        log.info("ERROR - Unknown slot position...")

# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    """
    Call runtime procedure and execute test
    """
    parser = argparse.ArgumentParser(description="Blackstar Rack Slot Position Test")
    parser.add_argument("-p", "--position", required=True, dest="position", action="store",
                        help="Serial port to test")

    args = parser.parse_args()

    fmt = "%(asctime)s: %(message)s"
    # Set logging level to INFO to see test pass/fail results and DEBUG
    # to see detailed serial process information
    logging.basicConfig(format=fmt, level=logging.INFO, datefmt="%H:%M:%S")

    main(args.position)