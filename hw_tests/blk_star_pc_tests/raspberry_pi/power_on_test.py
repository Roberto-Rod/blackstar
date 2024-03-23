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
sys.path.append('/home/blackstar-test/test_equipment/')
from raspberry_pi_interface import *
from visa_test_equipment import *


# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

# -----------------------------------------------------------------------------
# Global Definition
# -----------------------------------------------------------------------------
psu = VisaTestEquipment("Power Supply")

# -----------------------------------------------------------------------------
# FUNCTIONS
# -----------------------------------------------------------------------------

def initialise_psu():
    log.info("INFO - Attempting to initialise PSU...")
    # Initialise PSU
    # psu = VisaTestEquipment("Power Supply")
    try:
        [is_device_initalized, model] = psu.device_specific_initialisation()
        if is_device_initalized:
            return True
    except:
        log.info("ERROR: Could not initialise PSU!")
        return False

def run_test():

    rasp_pi_interface = RaspberryPiInterface()
    test_pass = True

    log.info("INFO - Start EPU Power ON/OFF test...")
    log.info("INFO - Looking for a power supply...")

    # Initialise PSU
    try:
        if initialise_psu():
            log.info("INFO - Initialised Power supply.")
            log.info("INFO - Setting PSU voltage to 24V: ")
            psu.visa_te.psu.set_voltage(24)
            log.info("INFO - Setting PSU current to 3A: ")
            psu.visa_te.psu.set_current(3)
            test_pass &= True
    except:
        log.info("ERROR: Could not initialise Power supply. Exiting EPU Power ON/OFF test!")
        test_pass &= False
        return test_pass
    # rasp_pi_interface.set_shdn_a_pin_low()
    # Check EPU power state - should be OFF
    epu_power_state = rasp_pi_interface.get_ctl_gpio0_pin_state()

    if epu_power_state:
        log.info("FAIL - Blackstar should be powered OFF!")
        test_pass &= False
        return test_pass
    
    log.info("INFO - EPU is powered OFF!")
    log.info("INFO - Enabling PSU output...")
    psu.visa_te.psu.set_enabled(True)
    log.info("INFO - Veryfing Blackstar is powering ON...")
    time.sleep(20)
    epu_power_state = rasp_pi_interface.get_ctl_gpio0_pin_state()

    if not epu_power_state:
        log.info("FAIL - Blackstar should be powered ON!")
        # psu.visa_te.psu.set_enabled(False)
        test_pass &= False
        # return test_pass
    else:
        log.info("PASS - Blackstar is powered ON")
        
    # Blackstar should be ON now - try to power OFF
    log.info("INFO - Powering OFF Blackstar...")
    rasp_pi_interface.set_shdn_a_pin_high()    
    time.sleep(10)
    rasp_pi_interface.set_shdn_a_pin_low()
    log.info("INFO - Verifying Blackstar is powered OFF!")
    epu_power_state = rasp_pi_interface.get_ctl_gpio0_pin_state()

    if epu_power_state:
        log.info("FAIL - Blackstar should be powered OFF!")
        # psu.visa_te.psu.set_enabled(False)
        test_pass &= False
        # return test_pass
    else:
        log.info("PASS - Blackstar is powered OFF.")
        # log.info("INFO - Disabling PSU output...")
    
    # Verify the Blackstar does not power ON when SHDN_A is asserted
    log.info("INFO - Asserting SHDN_A...")
    rasp_pi_interface.set_shdn_a_pin_high()  
    log.info("INFO - Veryfing Blackstar remains powered OFF...")
    time.sleep(10)
    epu_power_state = rasp_pi_interface.get_ctl_gpio0_pin_state()

    if epu_power_state:
        log.info("FAIL - Blackstar should be powered OFF!")
        # psu.visa_te.psu.set_enabled(False)
        test_pass &= False
        # return test_pass
    else:
        log.info("PASS - Blackstar is powered OFF.")
        log.info("INFO - Disabling PSU output...")
        psu.visa_te.psu.set_enabled(False)
        rasp_pi_interface.set_shdn_a_pin_low()

    if test_pass:
        # Test has passed, return result
        log.info("INFO - Blackstar EPU Power ON/OFF Test Result - PASS")
        return test_pass
    else:
        log.info("INFO - Blackstar EPU Power ON/OFF Test Result - FAIL")
        return test_pass

# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == "__main__":
    """
    Call runtime procedure and execute test
    """
    # Set logging level to INFO to see test pass/fail results and DEBUG
    # to see detailed serial process information
    fmt = "%(asctime)s: %(message)s"
    logging.basicConfig(format=fmt, level=logging.INFO, datefmt="%H:%M:%S")

    run_test()
    


