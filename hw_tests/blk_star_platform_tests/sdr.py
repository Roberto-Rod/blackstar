#!/usr/bin/env python3
import freq_gen
import time

class SDR:
    SDR_MAX_GAIN = 89.75
    
    def __init__(self):
        self.freq_gen = freq_gen.freq_gen()
        
    def __exit__(self):
        self.stop()
        
    def initialise(self):
        self.freq_gen.start()
    
    def stop(self):
        self.freq_gen.stop()
        self.freq_gen.wait()
        
    def set_frequency(self, freq_Hz):
        self.freq_gen.set_freq_Hz(freq_Hz)
        time.sleep(1.0)
    
    def set_att_dB(self, att_dB):
        gain = float(self.SDR_MAX_GAIN) - float(att_dB)
        self.freq_gen.set_tx_gain(gain)
        time.sleep(0.25)
        

if __name__ == "__main__":
    sdr = SDR()
    sdr.initialise()
    freq_Hz = 1.5e9
    att_dB = 30
    while att_dB >= 0:
        print("Set SDR to {} Hz, att {} dB".format(freq_Hz, att_dB))
        sdr.set_frequency(freq_Hz)
        sdr.set_att_dB(att_dB)
        freq_Hz += 1e6
        att_dB -= 0.5
        time.sleep(1)
    sdr.stop()
    