#!/usr/bin/env python3
# PA calibration routine based on the algorithm in:
# http://bitbucket.kirintec.local/projects/MS/repos/mercury/browse/UI/desktop/mercuryrftest/PaCalibration.cpp
# commit ID: 53443ff544d
#
# NOTE: Mercury hunts for levelled and saturated power. K-CEMA does not hunt for saturated power as the
# Power Amplifier stage output could over-power the Transmit/Receive switch which follows the PA stage
import argparse
import datetime
import math
import os

from band import *
from sdr import *
from external_attenuator import *
from pa_control import *
from logger import *
from power_meter import *

DEBUG = False
DIR_NAME = "../calibration/"
FILE_NAME = "pa_cal.csv"
ATT_MAX_DB = 80
ATT_STEP_MIN_DB = 0.25
COMPRESSION_FAIL_COUNT = 2

band_name = {Band.LOW: "low", Band.MID: "mid", Band.HIGH: "high"}

voltage = {Band.LOW:2830, Band.MID: 28, Band.HIGH: 28}

base_target_power_dBm = {Band.LOW: 44, Band.MID: 44, Band.HIGH: 44}

TEST_POINTS = [
    # Band,     Path, Start Freq., Stop Freq., Step, Start Att, Att Step per, Power Monitor, Min Pass,  Target
    #                       (MHz)       (MHz) (MHz)       (dB)  per Freq (dB)    Range (dB)  Power (dB) Offset (dB)
    [Band.MID,    0,         400,        2700,   25,      46.0,         15.0,           30,       -1.0,     0.0]
]
# Note that Power Monitor Range is used to define the minimum power below the target power that
# is used to calculate the power monitor slope and offset.


def run_test():
    ok = True
    serial = "000000"
    start = time.process_time()
    print()
    if not os.path.exists("../log"):
        os.makedirs("../log")
    sys.stdout = Logger("../log/pa_cal.log")
    print("test_pa_calibrate")
    print("-----------------")
    print("Serial number: {}".format(serial))
    print("Disable Tx power supplies: ", end = "")
    print("Initialise PA Control: ", end = "")
    if PAControl.initialise():
        print("OK")
    else:
        print("ERROR")
        terminate_test()
        return False
    
    print("Disable PA power: ", end = "")    
    if PAControl.enable_power(False):
        print("OK")
    else:
        print("ERROR")
        terminate_test()
        return False
        
    time.sleep(2)
    pm = PowerMeter()
    print("Searching for Power Meter Service: ", end = "", flush = True)
    if pm.find():
        print("Power Meter Service found at {}:{}".format(pm.address,str(pm.port)))
        print("Connect to power meter: ", end = "", flush = True)
        if pm.connect():
            print("OK")
            print("Power meter details: {}".format(pm.description))
            # Zero power meter
            print("Zero power meter: ", end = "", flush = True)
            if pm.zero():
                print("OK")
            else:
                print("FAIL")
                terminate_test()
                return False
            # Set power meter offset
            print("Set offset to 0.0 dB: ", end = "", flush = True)
            if pm.set_offset(0):
                print("OK")
            else:
                print("FAIL")
                terminate_test()
                return False
        else:
            print("FAIL")
            terminate_test()
            return False
    else:
        print("FAIL - could not find PMS")
        terminate_test()
        return False

    print("Enable PA: ", end = "", flush = True)    
    if PAControl.enable_power(True):
        print("OK")
    else:
        print("FAIL - PA PSU did not become ready")
        terminate_test()
        return False
    print("Get PA band: ", end = "", flush = True)
    # Fixed mid-band for now
    band = Band.MID
    if band != Band.UNKNOWN:
        print("OK [{}]".format(band))
    else:
        print("FAIL [{}]".format(band))
        terminate_test()
        return False

    print("Open {}{} for writing: ".format(DIR_NAME, FILE_NAME), end = "")
    try:
        # Create directory if it does not exist
        os.makedirs(DIR_NAME, exist_ok = True)

        # Open file for writing
        f = open(DIR_NAME + FILE_NAME, "w")

        # Write header (v0.3)
        f.write("file_version,0.3\n")
        f.write("serial,{}\n".format(serial))
        f.write("band,{}\n".format(band_name[band]))
        f.write("power,{}\n".format(base_target_power_dBm[band]))
        f.write("voltage,{}\n".format(voltage[band]))
        f.write("\n")
        f.write("path,freq_Hz,att_sat,att_level,pm_slope,pm_offset\n")
        print("OK")
    except:
        print("FAIL")
        terminate_test()
        return False

    # File opened, Power Meter connected now initialise the hardware
    sdr = SDR()
    print("Initialise SDR: ", end = "")
    sdr.initialise()
    print("OK")

    print("Unmute PA: ", end = "", flush = True)
    PAControl.force_mute(False)
    print("OK")

    target_power_W = (10 ** (base_target_power_dBm[band]/10)) / 1000
    print("Target Power: {} dBm ({:.1f} W)".format(base_target_power_dBm[band], target_power_W))

    min_achieved_power_dBm = 100
    max_achieved_power_dBm = -100
    min_power_freq_MHz = 0
    max_power_freq_MHz = 0
    
    warmed_up = False

    # Loop through test points
    for test_point in TEST_POINTS:
        # Skip this test point if it is not in the band we are calibrating
        if test_point[0] != band:
            continue
        path = test_point[1]
        start_MHz = test_point[2]
        stop_MHz = test_point[3]
        step_MHz = test_point[4]
        att_dB = test_point[5]
        att_step_per_freq_dB = test_point[6]
        power_monitor_range_dB = test_point[7]
        min_pass_power_dBm = base_target_power_dBm[band] + test_point[8]
        target_power_dBm = base_target_power_dBm[band] + test_point[9]  # Apply target power offset
        att_step_dB = 0

        # Loop through the frequencies
        for freq_MHz in range(start_MHz, stop_MHz + step_MHz, step_MHz):
            # Skip this frequency if it is beyond the stop frequency (the table shouldn't be configured like this
            # as it means that there are not a whole number of steps between the start and stop frequencies)
            if freq_MHz > stop_MHz:
                continue
            freq_Hz = int(freq_MHz * 1e6)
            sdr_freq_Hz = freq_Hz

            # Set Power Meter offset
            power_meter_offset_dB = ExternalAttenuator.get_att(band, freq_MHz)
            print("Set power meter offset to {:.2f} dB: ".format(power_meter_offset_dB), end = "", flush = True)
            if pm.set_offset(power_meter_offset_dB) and pm.set_average_count(1):
                print("OK")
                power_meter_average_enabled = False
            else:
                print("FAIL")
                terminate_test()
                return False
                
            # Set SDR frequency
            print("Set SDR frequency: {} MHz".format(freq_MHz))
            sdr.set_frequency(sdr_freq_Hz)

            cal_point_found = False
            compression_fail = 0
            last_power_dBm = -100.0
            min_error_dB = 100.0
            final_narrowing = False
            narrowing_error_dB = 100.0

            # Storage for power monitor cal points
            pm_point = []

            while not cal_point_found and compression_fail < COMPRESSION_FAIL_COUNT:
                # Set attenuation
                sdr.set_att_dB(att_dB)

                # Read power
                pm.frequency_Hz = freq_Hz
                power_dBm = pm.get_reading_dBm()
                error_dB = abs(power_dBm - target_power_dBm)
                delta_dBm = power_dBm - last_power_dBm
                
                print("p {:.2f}, t {:.2f}, e {:.2f}, a {:.2f}".format(power_dBm, target_power_dBm, error_dB, att_dB), flush=True)

                # Do we have the first power monitor reading yet?
                # Use the first point we find that is at an output power within x dB of the target
                if len(pm_point) == 0 and warmed_up and error_dB <= power_monitor_range_dB:
                    # Read and store PA forward/reverse power
                    pm_values = PAControl.get_power_monitor_readings()
                    print("pm point 0: {}".format(pm_values["fwd"]))
                    pm_point.append({"dBm": power_dBm, "mV": pm_values["fwd"]})

                # If the point we are on is the closest to target that we have seen then store the values.
                # If we are in the final narrowing phase then continue for as long as we are finding smaller errors
                # as soon as the error is growing again, stop searching, we have found the cal point and
                # it will already be stored in min_error_dB / att_lev_dB / power_lev_dBm
                if error_dB < min_error_dB:
                    min_error_dB = error_dB
                    att_lev_dB = att_dB
                    power_lev_dBm = power_dBm
                if final_narrowing:
                    if error_dB > narrowing_error_dB:
                        cal_point_found = True
                    else:
                        narrowing_error_dB = error_dB
                # If we have hit or exceeded the target power then enter final narrowing phase:
                # change direction, use minimum attenuator step and reset stored minimum error
                elif power_dBm >= target_power_dBm:
                    narrowing_error_dB = error_dB
                    final_narrowing = True
                # Check for compression, when we are using 1.0 dB steps a <= 0.1 dB change indicates compression
                # check that power is high enough to ensure we are not just at low power and in a non-linear
                # part of the NTM attenuator range
                elif att_step_dB == 1.0 and delta_dBm <= 0.1:
                    if power_dBm > min_pass_power_dBm:
                        cal_point_found = True
                    elif att_dB < 27.0:
                        compression_fail += 1
                else:
                    compression_fail = 0
                # Check for max. attenuation - if we have hit maximum attenuation and are achieving power then
                # set cal point found to true
                if att_dB == ATT_MAX_DB and power_dBm > min_pass_power_dBm:
                    cal_point_found = True

                # We have hit target power, have we warmed up yet?
                if cal_point_found and not warmed_up:
                    # Wind back to the start conditions so that the first point is calibrated whilst warm
                    cal_point_found = False
                    att_dB = test_point[5]
                    att_step_dB = 0
                    last_power_dBm = -100.0
                    min_error_dB = 100.0
                    final_narrowing = False
                    narrowing_error_dB = 100.0

                    # Having got to full power, wait 1 minute for warm up
                    print("Warming up for 60 seconds...")
                    for i in range(6):
                        for j in range(10):
                            print(".", end="", flush=True)
                            time.sleep(1.0)
                        print("")
                    warmed_up = True

                # If we are about to leave the loop due to PA compression but we have already seen minimum power
                # then set cal point found to True
                if compression_fail == COMPRESSION_FAIL_COUNT and power_lev_dBm >= min_pass_power_dBm:
                    cal_point_found = True
                    # Set attenuation back to the "cal point" before the power monitor reading is taken
                    sdr.set_att_dB(att_lev_dB)

                if cal_point_found:
                    # Read and store PA forward/reverse power
                    pm_values = PAControl.get_power_monitor_readings()
                    print("pm point 1: {}".format(pm_values["fwd"]))
                    pm_point.append({"dBm": power_dBm, "mV": pm_values["fwd"]})
                    break;

                # If we have run out of input power then just stop, this should never happen
                # and it will cause the test to fail
                if att_dB == 0:
                    break;

                # If we are in the final narrowing-in phase then step attenuator up (input power down)
                # by the smallest step
                if final_narrowing:
                    att_dB += ATT_STEP_MIN_DB
                    compression_check = False
                    if att_dB > ATT_MAX_DB:
                        # Attenuator reached maximum, this should never happen and it will cause the test to fail
                        break;
                else:
                    # Pick nearest step to 1.0 dB and use 1.0 dB steps once we are within 3.0 dB of the target
                    # to facilitate the compression check.
                    att_step_dB = round(error_dB, 0)
                    att_step_dB -= 3
                    if att_step_dB <= 3.0:
                        att_step_dB = 1.0
                        
                    # On High Band don't make any steps bigger than 6 dB
                    if band == Band.HIGH:
                        if att_step_dB > 6.0:
                            att_step_dB = 6.0

                    # Input power heading up, reduce attenuation
                    att_dB -= att_step_dB
                    if att_dB < 0:
                        att_dB = 0
                    
                    if not power_meter_average_enabled and error_dB <= 1.0:
                        pm.set_average_count(8)
                        power_meter_average_enabled = True

                last_power_dBm = power_dBm
            # end while not cal_point_found and compression_fail < COMPRESSION_FAIL_COUNT

            if cal_point_found:
                delta_dB = pm_point[1]["dBm"] - pm_point[0]["dBm"]
                delta_mV = pm_point[1]["mV"] - pm_point[0]["mV"]
                slope_mv_per_dB = delta_mV / delta_dB
                if slope_mv_per_dB != 0:
                    offset_dBm = pm_point[0]["dBm"] - (pm_point[0]["mV"] / slope_mv_per_dB)
                else:
                    offset_dBm = pm_point[0]["dBm"]
                if power_lev_dBm < min_achieved_power_dBm:
                    min_achieved_power_dBm = power_lev_dBm
                    min_power_freq_MHz = freq_MHz
                if power_lev_dBm > max_achieved_power_dBm:
                    max_achieved_power_dBm = power_lev_dBm
                    max_power_freq_MHz = freq_MHz

                    
                print("> path {}, freq {} MHz, att {} dB, power {:.2f} dBm, slope {:.2f} mV/dB, offset {:.2f} dBm".format(
                                                                          path, freq_MHz, att_lev_dB, power_lev_dBm,
                                                                          slope_mv_per_dB, offset_dBm), flush=True)
                try:
                    # Write line to file
                    pm_slope = int(round(slope_mv_per_dB * 1000.0, 0))
                    pm_offset = int(round(offset_dBm * 1000.0, 0))
                    f.write("{},{},{},{},{},{}\n".format(path, freq_Hz, int(att_lev_dB * 4), int(att_lev_dB * 4), pm_slope, pm_offset))
                except:
                    print("FAIL - file write error")
                    terminate_test()
                    return False                
            else:
                print("FAIL - could not find calibration point (PA may not be achieving minimum power)")
                if not DEBUG:
                    terminate_test()
                    return False

            # Increase attenuation before moving to next frequency point and set to nearest whole dB
            att_dB += att_step_per_freq_dB
            att_dB = round(att_dB, 0)
            if att_dB > ATT_MAX_DB:
                att_dB = math.floor(ATT_MAX_DB)

    # If we got this far then all tests passed
    print("\nMinimum achieved power: {:.2f} dBm (at {} MHz)".format(min_achieved_power_dBm, min_power_freq_MHz))
    print("\nMaximum achieved power: {:.2f} dBm (at {} MHz)".format(max_achieved_power_dBm, max_power_freq_MHz))
    terminate_test()
    return ok


def terminate_test():
    PAControl.enable_power(False)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Control timing register")
    parser.add_argument("-n", "--no_kill_no_duration", help="Don't kill apps and don't log test duration", action="store_true")
    args = parser.parse_args()
    if not args.no_kill_no_duration:
        os.system("/usr/bin/killall BlackStarECM")
        start_time = time.time()
    pm = PowerMeter()
    if not pm.find():
        print("Could not find Power Meter Service, terminating test...")
    elif run_test():
        print("\n*** OK - PA calibration passed ***\n")
        if not args.no_kill_no_duration:
            print("\n(PA calibration duration: {} h:m:s)\n".format(str(datetime.timedelta(seconds = round(time.time() - start_time, 0)))))
            print("\n*** Unmount mmcblk0p2 to ensure calibration and log files are saved ***\n")
    else:
        if not args.no_kill_no_duration:
            print("\n(PA calibration duration: {} h:m:s)\n".format(str(datetime.timedelta(seconds = round(time.time() - start_time, 0)))))
        print("\n*** TEST FAILED ***\n")
