import numpy as np
import os
import sys

SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))
DIR_NAME = "{}/../calibration/".format(SCRIPT_PATH)
FILE_NAME = "pa_cal.csv"


class GetPACalibration:
    def get_cal(cal_freq_Hz):
        ret_att_dB = -1000
        ret_slope_mv_per_dB = -1000
        ret_offset_dBm = -1000
        try:
            # Open file for writing
            headerLinesSkipped = False
            freq_points = []
            att_points = []
            slope_points = []
            offset_points = []

            with open(DIR_NAME + FILE_NAME, "r") as f:
                for line in f:
                    if headerLinesSkipped:
                        if line.strip():
                            fields = line.rstrip().split(",")
                            # path = int(fields[0])
                            freq_Hz = int(fields[1])
                            #att_sat_dB = float(fields[2]) / 4.0
                            att_lev_dB = float(fields[3]) / 4.0
                            pm_slope_mV_per_dB = float(fields[4]) / 1000.0
                            pm_offset_dBm = float(fields[5]) / 1000.0

                            freq_points.append(freq_Hz)
                            att_points.append(att_lev_dB)
                            slope_points.append(pm_slope_mV_per_dB)
                            offset_points.append(pm_offset_dBm)
                    else:
                        if line.startswith("path"):
                            headerLinesSkipped = True

                ret_att_dB = np.interp(cal_freq_Hz, freq_points, att_points)
                # Round attenuation to nearest 0.25 dB
                ret_att_dB = int(ret_att_dB * 4) / 4.0
                ret_slope_mv_per_dB = np.interp(cal_freq_Hz, freq_points, slope_points)
                ret_offset_dBm = np.interp(cal_freq_Hz, freq_points, offset_points)
        except Exception as e:
            print(e)
        
        return [ret_att_dB, ret_slope_mv_per_dB, ret_offset_dBm]


if __name__ == "__main__":
    cal_freq_Hz = int(sys.argv[1])
    print(GetPACalibration.get_cal(cal_freq_Hz))
