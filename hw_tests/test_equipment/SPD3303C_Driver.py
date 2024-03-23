
#*IDN\sSiglent\sTechnologies,SPD3303C,NPD3ECAQ1R0177,1.02.01.01.03R6,V1.3\n
# Manufacturer Name:    Siglent Technologies
# Model Name:           SPD3303C
# Manufacturer ID:      0x0483
# Model Code:           0x7540           
# USB Serial Number:    SDG00001130273
# Firmware Version:     1.02.01.01.03R6
# Hardware Version:     V1.3

#!/usr/bin/env python3
"""Pachymeter Automated Software Verification Rig SPD 3303C Power Supply Device Driver"""


import pyvisa
import time
import sys
import logging

# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

class SPD330Class():
    def __init__(self, reset_device=True, debug=False):
        super().__init__()
        log.info("INFO - Instanciating SPD3303C Power Supply Class!")
        self.rm = pyvisa.ResourceManager()
        self.resource = None
        self.psu = None
        self.reset_device = reset_device
        self.debug = debug
        # self.psu = rm.open_resource('USB0::0x0483::0x7540::NPD3ECAQ1R0177::INSTR')
        # log.info(self.psu)
        # self.psu.write("*IDN?")
        # time.sleep(0.05)
        # reply = self.psu.read()
        # log.info("\nConnection established with  the SPD3303C PSU: " + reply)

        # Set logging level to INFO initially
        self._logging_fmt = "%(asctime)s: %(message)s"
        self._logging_level = logging.INFO
        logging.basicConfig(format=self._logging_fmt, datefmt="%H:%M:%S", stream=sys.stdout, level=self._logging_level)

    def find_and_initialise(self):
        self.resource = None
        resources = self.rm.list_resources()
        for res in resources:
            # This instrument does not fully support VXI-11, we need to use the COM port interface
            # find COM ports, open and query to look for device ID
            if res.startswith("USB0") or res.startswith("ASRL"):
                self.resource = res
                if self.initialise_device():
                    return True
                else:
                    self.resource = None
        log.info("ERROR: did not find a SPD3303C Power Supply!")
        return False

    def initialise_device(self):
        try:
            self.psu = self.rm.open_resource(self.resource)
            log.info("INFO - Found and initialised SPD3303C Power Supply: {}".format(self.details()))
        except:
            if self.psu:
                self.psu.close()
                log.info("ERROR - Could not open resource: {}".format(self.resource))
            else:
                log.info("ERROR - Resource busy: {}".format(self.resource))
            return False
        details = self.details()
        try:
            model = details.split(",")[1].strip()
            if model.startswith("SPD"):
                if self.reset_device:
                    return self.send_command("*RST")
                else:
                    return True
        except:
            pass
        self.psu.close()
        return False
    
    def details(self):
        self.psu.write("*IDN?")
        time.sleep(0.05)
        return self.psu.read()
    
    def send_command(self, cmd):
        if self.debug:
            log.info("INFO - send_command: {}".format(cmd))
        try:
            self.psu.write(cmd)
            time.sleep(0.05)
            return True
        except:
            log.info("ERROR - could not send command")
            return False
    
    def send_query(self, query):
        if self.debug:
            log.info("INFO - send_query: {}".format(query))
        try:
            self.psu.write(query)
            time.sleep(0.05)
            response = self.psu.read()
            return response
        except:
            log.info("ERROR - could not send query")
            return False
    
    def set_enabled(self, enabled):
        if enabled:
            return self.send_command("OUTP CH1, ON")
        else:
            return self.send_command("OUTP CH1, OFF")
    
    def set_voltage(self, voltage):
        return self.send_command("CH1:VOLT {}".format(voltage))
    
    def get_voltage(self):
        response = self.send_query("CH1:VOLT?")
        return float(response)
    
    def get_voltage_out(self):
        response = self.send_query("MEAS:VOLT? CH1")
        # return float(response.replace("V", ""))
        return float(response)
    
    def set_current(self, voltage):
        return self.send_command("CH1:CURR {}".format(voltage))

    def get_current(self):
        response = self.send_query("CH1:CURR?")
        # return float(response.split(" ")[1])
        return float(response)
    
    def get_current_out(self):
        response = self.send_query("MEAS:CURR? CH1")
        return float(response)

    def get_average_current_out(self, nr_readings=16, delay_s=0.05):
        readings = []
        for i in range(0, nr_readings):
            time.sleep(delay_s)
            readings.append(self.get_current_out())
        return round(sum(readings) / len(readings), 4)

    def get_power_out(self):
        return round(self.get_voltage_out() * self.get_current_out(), 4)

    def get_average_power_out(self, nr_readings=16, delay_s=0.1):
        # Use average current and an instantaneous voltage measurement as voltage
        # is stabilised by the PSU whilst current varies
        return round(self.get_voltage_out() * self.get_average_current_out(nr_readings, delay_s), 4)

    
# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == "__main__":
    """ This module is NOT intended to be executed stand-alone """
    psu = SPD330Class(reset_device=False)
    log.info("INFO - Power Supply SPD3303C Test:")
    if psu.find_and_initialise(): 
        time.sleep(2)
        psu.set_voltage(24)
        psu.set_current(3)
        psu.set_enabled(True)
        time.sleep(2)
        log.info("INFO - Details:           {}".format(psu.details()))
        log.info("INFO - Voltage Setting:   {} V".format(psu.get_voltage()))
        log.info("INFO - Voltage Out:       {} V".format(psu.get_voltage_out()))
        log.info("INFO - Current Setting:   {} A".format(psu.get_current()))
        log.info("INFO - Current Out:       {} A".format(psu.get_current_out()))
        log.info("INFO - Power Out:         {} W".format(psu.get_power_out()))
        log.info("INFO - Average Power Out: {} W".format(psu.get_average_power_out()))

        while True:
            log.info("INFO - Average Current:   {} A".format(psu.get_average_current_out(nr_readings=100)))
            log.info("INFO - Average Power Out: {} W".format(psu.get_average_power_out()))
