#!/usr/bin/env python3
"""
Script to test the ECM slot position of the Blackstar module
"""

# -----------------------------------------------------------------------------
# Imports
# -----------------------------------------------------------------------------

# stdlib imports --------------------------------------------------------------
import logging
import sys
import time
import threading
import sys

# Our own imports -------------------------------------------------------------
sys.path.append("C:/workspace/blackstar/hw_tests/test_equipment/")
from rpi_platform_intf import RpiPlatformTestInt
from blk_platform_test_intf import BlkPlatformTestInt
from visa_test_equipment import VisaTestEquipment

# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

# Set logging level to INFO initially
logging_fmt = "%(asctime)s: %(message)s"
logging_level = logging.INFO
logging.basicConfig(format=logging_fmt, datefmt="%H:%M:%S", stream=sys.stdout, level=logging_level)

class BlkProdTest:

    BLK_HOSTNAME = "192.168.6.2"
    BLK_USERNAME = "blackstar-admin"
    BLK_PASSWORD = "@toqPvE3ms"
    RPI_HOSTNAME = "raspberrypi"
    RPI_USERNAME = "blackstar-test"
    RPI_PASSWORD = "blackstar"

    # Set logging level to INFO to see test pass/fail results and DEBUG
    # to see detailed serial process information
    fmt = "%(asctime)s: %(message)s"
    logging.basicConfig(format=fmt, level=logging.INFO, datefmt="%H:%M:%S")

    def __init__(self):
        # Set test environment initial state
        log.info("INFO - Initialising test environment...")
        self.psu = None
    
    def blackstar_power_on_test(self):
        log.info("INFO - Start Blackstar Power ON/OFF Test...")

        self.psu = VisaTestEquipment("Power Supply")
        log.info("INFO - Attempting to initialise PSU...")
        # Initialise PSU
        try:
            [is_device_initalized, model] = self.psu.device_specific_initialisation()
        except:
            log.info("ERROR: Could not initialise PSU!")

        if is_device_initalized:
            with RpiPlatformTestInt(self.RPI_USERNAME, self.RPI_HOSTNAME) as rpissh:
                log.info("INFO - Setting PSU voltage to 24V: ")
                self.psu.visa_te.psu.set_voltage(24)
                log.info("INFO - Setting PSU current to 3A: ")
                self.psu.visa_te.psu.set_current(3)
                log.info("INFO - Enabling PSU output...")
                self.psu.visa_te.psu.set_enabled(True)
                test_pass = rpissh.run_power_on_test()

        if test_pass:
                log.info("PASS - Blackstar Power ON/OFF Complete.")
        else:
            log.info("FAIL - Blackstar Power ON/OFF Complete.")

        

    def ecm_slot_position_test(self):

        with BlkPlatformTestInt(self.BLK_USERNAME, self.BLK_HOSTNAME) as blkssh:
            with RpiPlatformTestInt(self.RPI_USERNAME, self.RPI_HOSTNAME) as rpissh:
                
                log.info("INFO - Start Blackstar ECM Slot Position Test...")
                for p in range (8):

                    log.info("INFO - Setting ECM slot position to {}".format(p))
                    threading.Thread(target=rpissh.set_blackstar_slot_position, args=(p,)).start()
                    # thread1.start()
                    # rpissh.set_blackstar_slot_position(5)
                    time.sleep(3)
                    pos = threading.Thread(target=blkssh.get_ECM_slot_position).start()
                    # pos = blkssh.get_ECM_slot_position()
                    # time.sleep(1)
                    log.info("ECM slot position set to: {}; Blackstar reported :{}".format(p, pos))
                    if pos == p:
                        log.info("ECM Slot Position Test - PASS")
                    else:
                        log.info("ECM Slot Position Test - FAIL")
                    # thread2.start()
                    # pos = blkssh.get_ECM_slot_position()
                    time.sleep(3)
                    # log.info("Slot position: ".format(pos))

    def serial_message_test(self):

        with BlkPlatformTestInt(self.BLK_USERNAME, self.BLK_HOSTNAME) as blkssh:
            log.info("INFO - Serial Message Test Started...")
            test_pass = False
            # [test_pass, test_results] = 
            blkssh.run_serial_message_test()
            
            # if test_pass:
            #     log.info("PASS - Serial Messsage Test Complete.")
            # else:
            #     log.info("FAIL - Serial Messsage Test Complete. {} tests FAILED".format(test_results))
    
    def can_message_test(self):

        with BlkPlatformTestInt(self.BLK_USERNAME, self.BLK_HOSTNAME) as blkssh:
            with RpiPlatformTestInt(self.RPI_USERNAME, self.RPI_HOSTNAME) as rpissh:

                log.info("INFO - Start Blackstar CAN Message Test...")


# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    blk_prod_test = BlkProdTest()
    # blk_prod_test.blackstar_power_on_test()
    # blk_prod_test.ecm_slot_position_test()
    blk_prod_test.serial_message_test()
    # blk_prod_test.can_message_test()


