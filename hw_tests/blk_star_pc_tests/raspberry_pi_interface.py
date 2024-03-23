#!/usr/bin/env python3
"""Interface for the Raspberry Pi 4"""

# -----------------------------------------------------------------------------
# Imports
# -----------------------------------------------------------------------------

# stdlib imports --------------------------------------------------------------
import logging
import sys
from gpiozero import LED, Button

# Our own imports -------------------------------------------------------------
# sys.path.append('/home/blackstar-test/test_equipment/')
# from visa_test_equipment import *
# from visa_power_supply import *

# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

# -----------------------------------------------------------------------------
# Raspberry PI 4 Pins 
# -----------------------------------------------------------------------------
# _MCP2515_CS      = 8
# _MCP2515_MISO    = 9
# _MCP2515_MOSI    = 10
# _MCP2515_SCK     = 11
# _CTL_GPIO1       = 27
# _CTL_GPIO0       = 17
# _CTL_GPIO2       = 22
# _SHDN_A          = 24
# _MCP2515_INT     = 25
# _SHDN_K          = 39
# _MCP2515_3_3V    = 9
# _MCP2515_GROUND  = 10
# _BLK_GROUND      = 11

# -----------------------------------------------------------------------------
# Raspberry PI 4 Interface Class
# -----------------------------------------------------------------------------
class RaspberryPiInterface:
    def __init__(self, power_on_test=False):
        self.initialised = False
        self.power_on_test = power_on_test

        # Set logging level to INFO initially
        self._logging_fmt = "%(asctime)s: %(message)s"
        self._logging_level = logging.INFO
        logging.basicConfig(format=self._logging_fmt, datefmt="%H:%M:%S", stream=sys.stdout, level=self._logging_level)

        if self.power_on_test:
            self.CTL_GPIO0_PIN   = Button(17, pull_up=False)
            self.CTL_GPIO1_PIN   = LED(27, initial_value=False)
            self.CTL_GPIO2_PIN   = LED(22, initial_value=False)
            self.SHDN_A_PIN      = LED(24, initial_value=True)
        else:
            self.CTL_GPIO0_PIN   = LED(17, initial_value=False)
            self.CTL_GPIO1_PIN   = LED(27, initial_value=False)
            self.CTL_GPIO2_PIN   = LED(22, initial_value=False)
            self.SHDN_A_PIN      = LED(24, initial_value=False)
  
    def __del__(self):
        self.CTL_GPIO0_PIN.close()
        self.CTL_GPIO1_PIN.close()
        self.CTL_GPIO2_PIN.close()
        self.SHDN_A_PIN.close()
        log.info("INFO - Closing Raspberry Pi Interface...")
    
    def initialise(self):
        if self.power_on_test:
            log.info("INFO - Initializing Raspberry Pi for Blackstar Power ON Test...")
            self.CTL_GPIO1_PIN.off()
            self.CTL_GPIO2_PIN.off()
            self.initialised = True
            log.info("INFO - Done!")
        else:
            log.info("INFO - Initializing Raspberry Pi...")
            self.CTL_GPIO0_PIN.off()
            self.CTL_GPIO1_PIN.off()
            self.CTL_GPIO2_PIN.off()
            self.initialised = True
            log.info("INFO - Done!")
    
    def deinitialise(self):
        log.info("INFO - Deinitializing Raspberry Pi Interface...")
        self.initialised = False
    
    def is_initialised(self):
        return self.initialised
    
    def set_ctl_gpio0_pin(self, state):
        if state:
            self.CTL_GPIO0_PIN.on()
        else:
            self.CTL_GPIO0_PIN.off()
    
    def set_ctl_gpio1_pin(self, state):
        if state:
            self.CTL_GPIO1_PIN.on()
        else:
            self.CTL_GPIO1_PIN.off()
    
    def set_ctl_gpio2_pin(self, state):
        if state:
            self.CTL_GPIO2_PIN.on()
        else:
            self.CTL_GPIO2_PIN.off()
    
    def set_shdn_a_pin(self, state):
        if state:
            self.SHDN_A_PIN.on()
        else:
            self.SHDN_A_PIN.off()
    
    def get_ctl_gpio0_pin_state(self):
        if self.CTL_GPIO0_PIN.is_pressed:
            return True
        else:
            return False
    
# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == "__main__":
    """
    This module is NOT intended to be executed stand-alone
    """
    log.info("Module is NOT intended to be executed stand-alone")
