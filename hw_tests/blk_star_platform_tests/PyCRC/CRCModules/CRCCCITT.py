#!/usr/bin/env python3
# -*- coding: utf8 -*-

# CRC CCITT
#
# comes in 3 flavous
# (XModem)  starting value: 0x0000
#           starting value: 0xffff
#           starting value: 0x1d0f
# 
# Cristian NAVALICI cristian.navalici at gmail dot com
#
# Modified by rharrison@kirintec.com on 17/09/2020
# changed format of exceptions for compatibility with Python 3

import sys
from ctypes import c_ushort

class CRCCCITT:
    crc_ccitt_tab = []

    # The CRC's are computed using polynomials. Here is the most used coefficient for CRC CCITT
    crc_ccitt_constant = 0x1021

    def __init__(self, version = 'XModem'):
        try:
            dict_versions = { 'XModem':0x0000, 'FFFF':0xffff, '1D0F':0x1d0f }
            if version not in dict_versions.keys():
                raise Exception("Your version parameter should be one of the {} options".format("|".join(dict_versions.keys())))

            self.starting_value = dict_versions[version]
            if not len(self.crc_ccitt_tab): self.init_crc_ccitt() # initialize the precalculated tables
        except:
            print(sys.exc_info()[0])


    def calculate(self, string = ''):
        try:
            if not isinstance(string, str): raise Exception("Please provide a string as argument for calculation.")
            if not string: return 0

            crcValue = self.starting_value

            for c in string:
                tmp = (c_ushort(crcValue >> 8).value) ^ ord(c)
                crcValue = (c_ushort(crcValue << 8).value) ^ int(self.crc_ccitt_tab[tmp], 0)

            return crcValue
        except:
            print("EXCEPTION(calculate): {}".format(sys.exc_info()[0]))


    def init_crc_ccitt(self):
        '''The algorithm use tables with precalculated values'''
        for i in range(0, 256):
            crc = 0
            c = i << 8

            for j in range(0, 8):
                if ((crc ^ c) & 0x8000):  crc = c_ushort(crc << 1).value ^ self.crc_ccitt_constant
                else: crc = c_ushort(crc << 1).value

                c = c_ushort(c << 1).value # equiv c = c << 1
            self.crc_ccitt_tab.append(hex(crc))




