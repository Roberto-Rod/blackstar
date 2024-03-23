#!/usr/bin/env python3
"""Blackstar EPU Power Test"""

# -----------------------------------------------------------------------------
# Imports
# -----------------------------------------------------------------------------

# stdlib imports --------------------------------------------------------------
import logging
import time
import sys

# Our own imports -------------------------------------------------------------
from raspberry_pi_interface import *

# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

# -----------------------------------------------------------------------------
# Global Definition
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# FUNCTIONS
# -----------------------------------------------------------------------------

class RpiPowerOnTest:
    def __init__(self) -> None:
        # Set logging level to INFO to see test pass/fail results and DEBUG
        # to see detailed serial process information
        fmt = "%(asctime)s: %(message)s"
        logging.basicConfig(format=fmt, level=logging.INFO, datefmt="%H:%M:%S")

    def run_test(self):

        rasp_pi_interface = RaspberryPiInterface(power_on_test=True)
        rasp_pi_interface.initialise()
        test_pass = True

        # Check EPU power state - should be OFF
        epu_power_state = rasp_pi_interface.get_ctl_gpio0_pin_state()

        if epu_power_state:
            log.info("FAIL - Blackstar should be powered OFF!")
            test_pass &= False
            # return test_pass
        
        log.info("INFO - EPU is powered OFF!")
        log.info("INFO - Veryfing Blackstar is powering ON...")
        rasp_pi_interface.set_shdn_a_pin(False) 
        time.sleep(15)
        epu_power_state = rasp_pi_interface.get_ctl_gpio0_pin_state()

        if not epu_power_state:
            log.info("FAIL - Blackstar should be powered ON!")
            test_pass &= False
        else:
            log.info("PASS - Blackstar is powered ON")
            
        # Blackstar should be ON now - try to power OFF
        log.info("INFO - Powering OFF Blackstar...")
        rasp_pi_interface.set_shdn_a_pin(True)     
        time.sleep(10)
        rasp_pi_interface.set_shdn_a_pin(False)
        log.info("INFO - Verifying Blackstar is powered OFF!")
        epu_power_state = rasp_pi_interface.get_ctl_gpio0_pin_state()

        if epu_power_state:
            log.info("FAIL - Blackstar should be powered OFF!")
            test_pass &= False
            # return test_pass
        else:
            log.info("PASS - Blackstar is powered OFF.")
        
        # Verify the Blackstar does not power ON when SHDN_A is asserted
        log.info("INFO - Asserting SHDN_A...")
        rasp_pi_interface.set_shdn_a_pin(True)  
        log.info("INFO - Veryfing Blackstar remains powered OFF...")
        time.sleep(10)
        epu_power_state = rasp_pi_interface.get_ctl_gpio0_pin_state()

        if epu_power_state:
            log.info("FAIL - Blackstar should be powered OFF!")
            test_pass &= False
            # return test_pass
        else:
            log.info("PASS - Blackstar is powered OFF.")
            # psu.visa_te.psu.set_enabled(False)
            rasp_pi_interface.set_shdn_a_pin(False)

        log.info("INFO - Veryfing Blackstar is powering ON...")
        time.sleep(20)
        epu_power_state = rasp_pi_interface.get_ctl_gpio0_pin_state()

        if not epu_power_state:
            log.info("FAIL - Blackstar should be powered ON!")
            test_pass &= False
        else:
            log.info("PASS - Blackstar is powered ON")       

        if test_pass:
            # Test has passed, return result
            log.info("INFO - Blackstar EPU Power ON/OFF Test Result - PASS")
            test_pass &= True
        else:
            log.info("INFO - Blackstar EPU Power ON/OFF Test Result - FAIL")
            test_pass &= False
        
        return test_pass

# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == "__main__":
    """
    Call runtime procedure and execute test
    """
    rpi_pwr_on = RpiPowerOnTest()
    rpi_pwr_on.run_test()
    


