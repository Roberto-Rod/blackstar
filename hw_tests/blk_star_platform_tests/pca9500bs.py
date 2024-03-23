#!/usr/bin/env python
"""
Module for communicating with PCA9500BS EEPROM IC
"""
# -----------------------------------------------------------------------------
# Copyright (c) 2024, Kirintec
#
# -----------------------------------------------------------------------------
"""
OPTIONS ------------------------------------------------------------------
None
"""

# stdlib imports -------------------------------------------------------
import logging
from os import popen
from smbus import SMBus
import time


# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

# -----------------------------------------------------------------------------
# CLASSES
# -----------------------------------------------------------------------------

class PCA9500BS:

    # Bus/slave addresses
    BUS = 0
    BUS_ADDRESS = 0x52
    # Start address
    START_ADDRESS = 0x00


    def __init__(self):
        self.bus = SMBus(self.BUS)
        log.debug("Enabling PCA9500 EEPROM reading/writing...")
    
    def erase_eeprom(self):

        address_list    = [0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0]
        erase_data      = [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

        log.debug("Erasing PCA9500BS EEPROM...")
        for address in address_list:
            self.write_data_block(address, erase_data)
            time.sleep(0.02)
    
    def read_eeprom(self):
        cmd = "/usr/sbin/i2cdump -y {} {}".format(int(self.BUS), hex(self.BUS_ADDRESS))
        try:
            # Read the output so that we block and wait for command to return
            ret = popen(cmd).read()
        except:
            return False
        return True, ret


    def read_register(self, address):
        try:
            return self.bus.read_byte_data(self.BUS_ADDRESS, address)
        except OSError:
            print("Error reading bus {}, slave 0x{:02X}, register 0x{:02X}".format(self.BUS, self.BUS_ADDRESS, address))
            return -1

    def write_register(self, address, value):
        try:
            self.bus.write_byte_data(self.BUS_ADDRESS, address, value)
            return True
        except OSError:
            print("Error write bus {}, slave 0x{:02X}, register 0x{:02X}".format(self.BUS, self.BUS_ADDRESS, address))
            return False
    
    def write_data_block(self, address, value):
        try:
            self.bus.write_i2c_block_data(self.BUS_ADDRESS, address, self.convert_to_bytes(value))
            return True
        except OSError:
            print("Error write bus {}, slave 0x{:02X}, register 0x{:02X}".format(self.BUS, self.BUS_ADDRESS, address))
            return False
    
    def read_data_block(self, cmd):
        try:
            return self.bus.read_i2c_block_data(self.BUS_ADDRESS, cmd)
        except OSError:
            print("Error reading bus {}, slave 0x{:02X}, register 0x{:02X}".format(self.BUS, self.BUS_ADDRESS, cmd))
            return False
    
    def read_word(self, cmd):
        try:
            return self.bus.read_word_data(self.BUS_ADDRESS, cmd)
        except OSError:
            print("Error reading bus {}, slave 0x{:02X}, register 0x{:02X}".format(self.BUS, self.BUS_ADDRESS, cmd))
            return False
    
    def write_word(self, address, value):
        try:
            self.bus.write_word_data(self.BUS_ADDRESS, address, value)
            return True
        except OSError:
            print("Error write bus {}, slave 0x{:02X}, register 0x{:02X}".format(self.BUS, self.BUS_ADDRESS, address))
            return False
    
    def convert_to_bytes(self, string):

        hex_string_list = []

        if isinstance(string, int):
            num = str(string)
            num = num.encode('utf-8').hex(sep=",")
            num = int(num, 16)
            num = string.to_bytes(1, byteorder='big')
            hex_string_list += num
            return hex_string_list
        elif len(string) == 0:
            return [0x00]
        elif isinstance(string[0], int):
            return string
        elif isinstance(string, str):
            for char in string:
                encoded_char = char.encode('utf-8').hex(sep=",")
                encoded_char_int = int(encoded_char, 16)
                encoded_char_byte = encoded_char_int.to_bytes(1, byteorder='big')
                hex_string_list += encoded_char_byte
            return hex_string_list
        else:
            print("Unknown data type")

    def convert_hex_string_to_ascii(self, hex_values):
        ascii_string = []
        for value in hex_values:
            hex_values = str(value)
            print("Hex value is ", hex(value), "; ASCII char is ", chr(value))
            ascii_string += chr(value)

            return ascii_string

if __name__ == "__main__":
    """
        Module is not intended to be executed stand-alone
    """
    print("Module is not intended to be executed stand-alone")

