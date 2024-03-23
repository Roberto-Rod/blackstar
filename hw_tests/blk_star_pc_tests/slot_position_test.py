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

def main():

    rasp_pi_interface = RaspberryPiInterface()
    rasp_pi_interface.initialise()

    for position in range(8):

        if position == 0:
            log.info("INFO - Setting slot number to 0...")
            rasp_pi_interface.set_ctl_gpio0_pin(False)
            rasp_pi_interface.set_ctl_gpio1_pin(False)
            rasp_pi_interface.set_ctl_gpio2_pin(False)
            time.sleep(3)
        elif position == 1:
            log.info("INFO - Setting slot number to 1...")
            rasp_pi_interface.set_ctl_gpio0_pin(True)
            rasp_pi_interface.set_ctl_gpio1_pin(False)
            rasp_pi_interface.set_ctl_gpio2_pin(False)
            time.sleep(3)
        elif position == 2:
            log.info("INFO - Setting slot number to 2...")
            rasp_pi_interface.set_ctl_gpio0_pin(False)
            rasp_pi_interface.set_ctl_gpio1_pin(True)
            rasp_pi_interface.set_ctl_gpio2_pin(False)
            time.sleep(3)
        elif position == 3:
            log.info("INFO - Setting slot number to 3...")
            rasp_pi_interface.set_ctl_gpio0_pin(True)
            rasp_pi_interface.set_ctl_gpio1_pin(True)
            rasp_pi_interface.set_ctl_gpio2_pin(False)
            time.sleep(3)
        elif position == 4:
            log.info("INFO - Setting slot number to 4...")
            rasp_pi_interface.set_ctl_gpio0_pin(False)
            rasp_pi_interface.set_ctl_gpio1_pin(False)
            rasp_pi_interface.set_ctl_gpio2_pin(True)
            time.sleep(3)
        elif position == 5:
            log.info("INFO - Setting slot number to 5...")
            rasp_pi_interface.set_ctl_gpio0_pin(True)
            rasp_pi_interface.set_ctl_gpio1_pin(False)
            rasp_pi_interface.set_ctl_gpio2_pin(True)
            time.sleep(3)
        elif position == 6:
            log.info("INFO - Setting slot number to 6...")
            rasp_pi_interface.set_ctl_gpio0_pin(False)
            rasp_pi_interface.set_ctl_gpio1_pin(True)
            rasp_pi_interface.set_ctl_gpio2_pin(True)
            time.sleep(3)
        elif position == 7:
            log.info("INFO - Setting slot number to 7...")
            rasp_pi_interface.set_ctl_gpio0_pin(True)
            rasp_pi_interface.set_ctl_gpio1_pin(True)
            rasp_pi_interface.set_ctl_gpio2_pin(True)
            time.sleep(3)
        else:
            log.info("ERROR - Unknown slot position...")

# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    """ This module runs the Blackstar ECM position Test"""
    log.info("INFO - Starting Blackstar ECM position Test...")
    main()