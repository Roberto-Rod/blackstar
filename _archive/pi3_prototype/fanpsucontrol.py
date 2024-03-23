#!/usr/bin/env python

from smbus import SMBus
from time import sleep

class FanPSUControl:
    # Bus/slave addresses
    BUS = 1
    BUS_ADDRESS = 0x15
    
    # Register addresses
    CONTROL = 0x00
    FAN1_CONTROL = 0x01
    FAN2_CONTROL = 0x02
    STATUS1 = 0x40
    RESET_REG = 0x7F
    
    # Register values
    CONTROL_PSU_ENABLE = 0x01
    CONTROL_PA_ENABLE = 0x05
    CONTROL_PSU_DISABLE = 0x00
    CONTROL_FAN_ENABLE = 0x13
    CONTROL_FAN_DISABLE = 0x00
    RESET_CODE = 0x34

    def __init__(self):
        self.bus = SMBus(self.BUS)
        print("Resetting Fan/PSU Controller... ", end="", flush=True)
        if self.reset_registers():
            print("OK")
        

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
        
    def reset_registers(self):
        if self.write_register(self.RESET_REG, self.RESET_CODE):
            sleep(1)
            return True
        else:
            return False        

    def enable_psu(self):
        return self.write_register(self.CONTROL, self.CONTROL_PSU_ENABLE)
        
    def enable_pa(self):
        return self.write_register(self.CONTROL, self.CONTROL_PA_ENABLE)
    
    def disable_psu(self):
        return self.write_register(self.CONTROL, self.CONTROL_PSU_DISABLE)
        
    def enable_fans(self):
        return self.write_register(self.FAN1_CONTROL, self.CONTROL_FAN_ENABLE) and \
               self.write_register(self.FAN2_CONTROL, self.CONTROL_FAN_ENABLE)
               
    def disable_fans(self):
        return self.write_register(self.FAN1_CONTROL, self.CONTROL_FAN_DISABLE) and \
               self.write_register(self.FAN2_CONTROL, self.CONTROL_FAN_DISABLE)

if __name__ == "__main__":
    fan_psu = FanPSUControl()
    print("STATUS1: 0x{:02X}".format(fan_psu.read_register(FanPSUControl.STATUS1)))
    print("CONTROL: 0x{:02X}".format(fan_psu.read_register(FanPSUControl.CONTROL)))
    print("FAN1_CONTROL: 0x{:02X}".format(fan_psu.read_register(FanPSUControl.FAN1_CONTROL)))
    print("FAN2_CONTROL: 0x{:02X}".format(fan_psu.read_register(FanPSUControl.FAN2_CONTROL)))

