#!/usr/bin/env python3
"""Interface for the Raspberry Pi 4"""

# -----------------------------------------------------------------------------
# Imports
# -----------------------------------------------------------------------------

# stdlib imports --------------------------------------------------------------
import logging
import RPi.GPIO as GPIO
import time
import sys
from enum import Enum

# Our own imports -------------------------------------------------------------
sys.path.append('/home/blackstar-test/test_equipment/')
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
_MCP2515_CS      = 8
_MCP2515_MISO    = 9
_MCP2515_MOSI    = 10
_MCP2515_SCK     = 11
_CTL_GPIO1       = 16
_CTL_GPIO0       = 23
_SHDN_A          = 24
_MCP2515_INT     = 25
_SHDN_K          = 39
# _MCP2515_3_3V    = 9
# _MCP2515_GROUND  = 10
# _BLK_GROUND      = 11

# -----------------------------------------------------------------------------
# Raspberry PI 4 Pins Setup
# -----------------------------------------------------------------------------
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(_CTL_GPIO0, GPIO.IN)       # CTL_GPIO0
GPIO.setup(_CTL_GPIO1, GPIO.IN)       # CTL_GPIO1
GPIO.setup(_SHDN_A, GPIO.OUT)         # SHDN_A

# -----------------------------------------------------------------------------
# Raspberry PI 4 Interface Class
# -----------------------------------------------------------------------------
class RaspberryPiInterface:
    def __init__(self, debug=False):
        log.info("INFO - Setting up EPU Power ON/OFF test!")
        self.debug = debug
        self._CTL_GPIO0_PIN_STATE    = GPIO.input(_CTL_GPIO0)
        self._CTL_GPIO1_PIN_STATE    = GPIO.input(_CTL_GPIO1)
        self._SHDN_A_PIN_STATE       = False
        GPIO.output(_SHDN_A, self._SHDN_A_PIN_STATE)

        # Set logging level to INFO initially
        self._logging_fmt = "%(asctime)s: %(message)s"
        self._logging_level = logging.INFO
        logging.basicConfig(format=self._logging_fmt, datefmt="%H:%M:%S", stream=sys.stdout, level=self._logging_level)

    # def __del__(self):
    #     if self.blk_epu_test:
    #         self.blk_epu_test.close()
    
    def get_ctl_gpio0_pin_state(self):
        self._CTL_GPIO0_PIN_STATE = GPIO.input(_CTL_GPIO0)
        return self._CTL_GPIO0_PIN_STATE
    
    def get_ctl_gpio1_pin_state(self):
        self._CTL_GPIO1_PIN_STATE = GPIO.input(_CTL_GPIO1)
        return self._CTL_GPIO1_PIN_STATE
    
    def set_shdn_a_pin_high(self):
        self._SHDN_A_PIN_STATE = True
        GPIO.output(_SHDN_A, self._SHDN_A_PIN_STATE)
        return self._SHDN_A_PIN_STATE

    def set_shdn_a_pin_low(self):
        self._SHDN_A_PIN_STATE = False
        GPIO.output(_SHDN_A, False)
        return not self._SHDN_A_PIN_STATE






