#!/usr/bin/env python3
"""Raspberry Pi file to test CAN communications with the Blackstar module"""

# -----------------------------------------------------------------------------
# Imports
# -----------------------------------------------------------------------------

# stdlib imports --------------------------------------------------------------
# from __future__ import print
import can
import logging
import time
import os
import filecmp

# Our own imports -------------------------------------------------------------


# -----------------------------------------------------------------------------
# GLOBALS
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# CONSTANTS
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())


# -----------------------------------------------------------------------------
# CLASSES
# -----------------------------------------------------------------------------
class RpiCanTest:
    """
    Class which tests the CAN module by sending/receiving data between the Blackstar module and the Raspberry Pi
    """
    def __init__(self, debug=False):
        """
        Class constructor
        :param None
        """
        self.debug = debug
        self.can_bus = can.interface.Bus(bustype='socketcan', channel='can0', bitrate=500000)

        fmt = "%(asctime)s: %(message)s"
        # Set logging level to INFO to see test pass/fail results and DEBUG
        # to see detailed serial process information
        logging.basicConfig(format=fmt, level=logging.INFO, datefmt="%H:%M:%S")

    def send_one_msg(self, arb_id, data, ext_id):

        msg = can.Message(arbitration_id=arb_id, data=data, is_extended_id=ext_id)

        try:
            self.can_bus.send(msg)
            log.debug("Message sent on {}".format(self.can_bus.channel_info))
            return True
        except can.CanError:
            log.debug("Message NOT sent")
            return False
    
    def compare_can_msg_files(self, f1, f2):
 
        # reading files
        file1 = open(f1, "r")  
        file2 = open(f2, "r") 
        file1_data = file1.readlines()
        file2_data = file2.readlines()
        # files to write modified lines to
        CMP_FILE1_PATH = '/home/blackstar-test/test_scripts/cmp_file1.txt'
        CMP_FILE2_PATH = '/home/blackstar-test/test_scripts/cmp_file2.txt'

        if os.path.exists(CMP_FILE1_PATH) or os.path.exists(CMP_FILE2_PATH):
            os.remove(CMP_FILE1_PATH)
            os.remove(CMP_FILE2_PATH)
            log.info("The files {} and {} have been deleted.".format(CMP_FILE1_PATH, CMP_FILE2_PATH))

        log.info("Creating new compare files...")
        out_file1 = open(CMP_FILE1_PATH, "w")
        out_file2 = open(CMP_FILE2_PATH, "w")

        result = True

        for line1 in file1_data:
            # split to remove timestamps
            line1 = line1.split('can0 ')[1]
            out_file1.write(line1)
        # Done, close the file
        out_file1.close()
            
        for line2 in file2_data:
            # split to remove timestamps
            line2 = line2.split('can0 ')[1] 
            out_file2.write(line2)
        # Done, close the file
        out_file2.close()  

        # closing files
        file1.close()                                       
        file2.close()  
           
        # Now compare the text files
        result = filecmp.cmp(CMP_FILE1_PATH, CMP_FILE2_PATH, shallow = False)      
                     
        return result

    def run_test(self):
        """
        Test the unit under test CAN interface
        Prerequisites:
        - Blackstar unit and Raspberry Pi are powered up
        - Linux is booted on both boards
        :return: True if test passes, else False :type Boolean
        """
        log.info("Blackstar CAN Test")
        ret_val = True
        log.info("Transmitting CAN messages to Blackstar module...")

        time.sleep(2)
        os.system("cat can_msgs_to_send | canplayer")
        log.info("CAN messages transmission complete.")

        time.sleep(2)
        if os.path.exists("blk_received_msgs"):
            log.info("Removing previous blk_received_msgs file...")
            os.remove("blk_received_msgs")
            
        log.info("Copying CAN log file from the Blackstar module...")
        os.system("scp -r blackstar-admin@192.168.6.2:/home/blackstar-admin/tests/blk_received_msgs blackstar-test@192.168.6.99:/home/blackstar-test/test_scripts")

        log.info("Comparing the sent messages with the messages received by the Blackstar module...")
        ret_val &= self.compare_can_msg_files('can_msgs_to_send', 'blk_received_msgs')

        if ret_val:
            log.info("PASS - Blackstar CAN test succeeded.")
        else:
            log.info("FAIL - Blackstar CAN test failed.")

        return ret_val


# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    """
    Call runtime procedure and execute test
    """

    rpi_can_test = RpiCanTest()
    rpi_can_test.run_test()
    
    
 