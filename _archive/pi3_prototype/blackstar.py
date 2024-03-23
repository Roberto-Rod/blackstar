#!/usr/bin/env python
from gpiozero import LED, Button
from time import sleep
from fanpsucontrol import FanPSUControl
import subprocess

class BlackStar:
    def __init__(self):
        self.initialised = False
        self.system_off = Button(16, pull_up=False)
        self.usrp_reset = LED(17, initial_value=False)
        self.green_led = LED(18, initial_value=False)
        self.red_led = LED(22, initial_value=False)
        self.fan_psu = FanPSUControl()
        self.deinitialise()

    def __del__(self):
        print("Shutting Down...")
        self.system_off.close()
        self.usrp_reset.close()
        self.green_led.close()
        self.red_led.close()

    def initialise(self):
        self.red_led.off()
        self.green_led.blink(on_time=0.5, off_time=0.5)
        self.fan_psu.enable_fans()
        print("Resetting USRP...")
        self.usrp_reset.on()
        sleep(0.1)
        self.usrp_reset.off()
        process = subprocess.Popen("uhd_usrp_probe", shell=True, stdout=subprocess.PIPE)
        process.wait()
        self.green_led.on()
        self.initialised = True
        print("Done")

    def deinitialise(self):
        print("Shutting Down...")
        self.disable_tx()
        self.red_led.off()
        self.green_led.off()
        sleep(2)
        self.fan_psu.disable_fans()
        self.initialised = False

    def enable_tx(self):
        print("Enable Tx...")
        self.fan_psu.enable_psu()
        sleep(0.5)
        self.fan_psu.enable_pa()
        self.fan_psu.enable_fans()
        print("Done")

    def disable_tx(self):
        print("Disable Tx...")
        process = subprocess.Popen("killall tx_samples_from_file", shell=True, stdout=subprocess.PIPE)
        process.wait()
        self.fan_psu.disable_psu()
        print("Done")

    def run(self):
        try:
            while True:
                if self.system_off.is_active:
                    if self.initialised:
                        self.deinitialise()
                else:
                    if not self.initialised:
                        self.initialise()
                        self.enable_tx()
                sleep(1)
        except KeyboardInterrupt:
            print("Interrupted!")

if __name__ == "__main__":
    bs = BlackStar()
    bs.run()
