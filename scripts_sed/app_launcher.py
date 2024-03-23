#!/usr/bin/env python3
import datetime
import json
import glob
import sys
import operator
import os
import re

import generate_ephemerides as ephem
from get_pa_calibration import GetPACalibration

APP_FULL_PATH = {
    "iq_transmit": "/lib/uhd/examples/tx_samples_from_file",
    "gnss_spoof": "/mnt/sed/admin-data/apps/gps-gen-realtime"
}
CMD_FILE = "/opt/blackstar/command"
DATA_DIR = "/opt/blackstar/data"
EPHEM_DIR = "/mnt/sed/admin-data/ephem"
VALID_FILE_PATTERN = r"[^\.\/_\-A-Za-z0-9]"

FREQ_MIN_HZ = 400e6
FREQ_MAX_HZ = 2700e6
GPS_L1_HZ = 1575.42e6
RATE_MIN_SPS = 1e3
RATE_MAX_SPS = 61.44e6
ATT_MIN_DB = 0
ATT_MAX_DB = 89
FORMAT_VALUES = ["double", "float", "short"]
GAIN_MAX = 89.75
GPS_SIM_STATIC_DURATION_MAX = 86400

if __name__ == "__main__":
    ok = False
    command = ""
    if len(sys.argv) > 1:
        filename = sys.argv[1]
        try:
            with open(filename) as json_file:
                data = json.load(json_file)
            app_name = data["app_name"]
            if data["app_name"] in APP_FULL_PATH.keys():
                # Get the parameters associated with the app, let the 'try' block
                # fail if an expected key does not exist
                if app_name == "iq_transmit":
                    freq_Hz = int(data["centre_freq_hz"])
                    rate_sps = int(data["sample_rate_sps"])
                    iq_format = data["iq_format"]
                    iq_file = data["iq_file"]
                    repeat = data["repeat"]
                    try:
                        extra_gain_dB = data["iq_gain_db"]
                    except:
                        extra_gain_dB = 0.0
                    gain_switch = "--gain"
                    # Validate the parameters
                    if FREQ_MIN_HZ <= freq_Hz <= FREQ_MAX_HZ:
                        if RATE_MIN_SPS <= rate_sps <= RATE_MAX_SPS:
                            if iq_format in FORMAT_VALUES:
                                if not re.search(VALID_FILE_PATTERN, iq_file):
                                    if isinstance(repeat, bool):
                                        command = APP_FULL_PATH[app_name]
                                        command += " --freq {}".format(freq_Hz)
                                        command += " --rate {}".format(rate_sps)
                                        command += " --type {}".format(iq_format)
                                        command += " --file {}".format(iq_file)
                                        command += " --ref external"
                                        if repeat:
                                            command += " --repeat"
                                    else:
                                        print("ERROR: 'repeat' value failed validation")
                                else:
                                    print("ERROR: 'iq_file' value failed validation")
                            else:
                                print("ERROR: 'iq_format' value failed validation")
                        else:
                            print("ERROR: 'rate' value failed validation")
                    else:
                        print("ERROR: 'freq' value failed validation")
                elif app_name == "gnss_spoof":
                    now = datetime.datetime.now(datetime.timezone.utc)
                    spoof_year = now.year
                    spoof_month = now.month
                    spoof_day = now.day
                    spoof_hour = now.hour
                    spoof_minute = now.minute
                    spoof_second = now.second
                    # Get the requested spoof time, if provided
                    if "time" in data.keys():
                        spoof_time = data["time"]
                        try:
                            spoof_year = spoof_time["year"]
                        except:
                            pass
                        try:
                            spoof_month = spoof_time["month"]
                        except:
                            pass
                        try:
                            spoof_day = spoof_time["day"]
                        except:
                            pass
                        try:
                            spoof_hour = spoof_time["hour"]
                        except:
                            pass
                        try:
                            spoof_minute = spoof_time["minute"]
                        except:
                            pass
                        try:
                            spoof_second = spoof_time["second"]
                        except:
                            pass

                    # Find the ephemeris file
                    # try:
                    #    ephem_file = max(glob.glob(EPHEM_DIR + "/*"), key=os.path.getctime)
                    # except Exception:
                    #    ephem_file = None

                    # Generate a new ephemeris file using nearest (earlier) 2-hour block to spoof time
                    ephem_hour = spoof_hour
                    if ephem_hour % 2:
                        ephem_hour -= 1
                    ephem_datetime = datetime.datetime(spoof_year, spoof_month, spoof_day, ephem_hour)
                    ephem_file = ephem.generateEphemerides(EPHEM_DIR, ephem_datetime)

                    gain_switch = "-G"
                    # Set the frequency
                    freq_Hz = GPS_L1_HZ
                    try:
                        extra_gain_dB = data["iq_gain_db"]
                    except:
                        extra_gain_dB = 36

                    if ephem_file:
                        # Validate the parameters
                        if "location" in data.keys():
                            location = data["location"]
                            latitude = float(location["latitude"])
                            longitude = float(location["longitude"])
                            height = float(location["height"])
                            if -90.0 <= latitude <= 90.0:
                                if -180.0 <= longitude <= 180.0:
                                    if -1000.0 <= height <= 20000.0:
                                        command = APP_FULL_PATH[app_name]
                                        command += " -l {},{},{}".format(latitude, longitude, height)
                                    else:
                                        print("ERROR: 'latitude' value failed validation")
                                else:
                                    print("ERROR: 'longitude' value failed validation")
                            else:
                                print("ERROR: 'height' value failed validation")
                            # For static mode, set duration to maximum
                            command += " -d {}".format(GPS_SIM_STATIC_DURATION_MAX)
                        elif "motion_file" in data.keys():
                            motion_file = data["motion_file"]
                            if not re.search(VALID_FILE_PATTERN, motion_file):
                                command = APP_FULL_PATH[app_name]
                                command += " -x {}".format(motion_file)
                            else:
                                print("ERROR: 'motion_file' value failed validation")
                        # Add the ephemeris file option
                        command += " -e {}".format(ephem_file)
                        # Add the date/time option
                        command += " -t {}/{}/{},{}:{}:{}".format(spoof_year, spoof_month, spoof_day,
                                                                  spoof_hour, spoof_minute, spoof_second)
                    else:
                        print("ERROR: could not find an ephemeris file in {}".format(EPHEM_DIR))
                        command = ""
                # If we have a command then add the PA calibration correction factor
                if command:
                    att_dB, slope_mv_per_dB, offset_dBm = GetPACalibration.get_cal(freq_Hz)
                    if ATT_MIN_DB <= att_dB <= ATT_MAX_DB:
                        command += " {} {}".format(gain_switch, GAIN_MAX - float(att_dB) + float(extra_gain_dB))
                    else:
                        print("ERROR: invalid attenuation value calculated: {}".format(att_dB))
                        command = ""
            else:
                print("ERROR: app_name '{}' not recognised".format("app_name"))
        except Exception as e:
            print(e)
    else:
        print("ERROR: no filename provided")

    if command:
        print(command)
        os.system(command)
        sys.exit(1)
    else:
        sys.exit(0)
