#!/usr/bin/env python3
import os

class PAControl:
    def initialise():
        cmd = "/mnt/sed/admin-data/apps/BlackStarECM -i"
        response = os.popen(cmd).read()
        return response.strip() == "OK"
        
    def enable_power(enable):
        if enable:
            cmd = "/mnt/sed/admin-data/apps/BlackStarECM -E"
        else:
            cmd = "/mnt/sed/admin-data/apps/BlackStarECM -e"
        response = os.popen(cmd).read()
        return response.strip() == "OK"

    def force_mute(mute):
        if mute:
            cmd = "/mnt/sed/admin-data/apps/BlackStarECM -M"
        else:
            cmd = "/mnt/sed/admin-data/apps/BlackStarECM -m"
        response = os.popen(cmd).read()
        return response.strip() == "OK"
    
    def get_power_monitor_readings():
        ret_val = {"fwd": -1000, "rev": -1000}
        cmd = "/mnt/sed/admin-data/apps/BlackStarECM -r"
        response = os.popen(cmd).read()
        try:        
            values = response.strip().split(",")
            ret_val["fwd"] = int(values[0])
            ret_val["rev"] = int(values[1])
        except Exception as e:
            print(e)
        return ret_val
        

if __name__ == "__main__":
    if PAControl.initialise():
        print("Initialised")
    if PAControl.enable_power(True):
        print("Power enabled")
    if PAControl.force_mute(False):
        print("PA unmuted")
    print("Readings: {}".format(PAControl.get_power_monitor_readings()))
    input("Press enter to terminate")
    PAControl.force_mute(True)
    PAControl.enable_power(False)
    