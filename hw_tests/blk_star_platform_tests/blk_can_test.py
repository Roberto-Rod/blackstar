#!/usr/bin/env python
"""Blackstar CAN Test"""

import time
import can
import logging

# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())


# -----------------------------------------------------------------------------
# CLASSES
# -----------------------------------------------------------------------------
class BlkCanTest:
    """
    Class which tests the CAN module by sending/receiving data between the Blackstar module and the Raspberry Pi
    """
    def __init__(self, debug=False):
        """
        Class constructor
        :param debug: Type boolean, True to display CAN messages, False otherwise
        """
        self.debug = debug
        self.can_bus = can.interface.Bus(bustype='socketcan', channel='can0', bitrate=500000)
        self.can_log_file = can.io.CanutilsLogWriter('blk_received_msgs', channel='can0')

        if self.debug:
            self.notifier = can.Notifier(self.can_bus, [can.Printer(), self.can_log_file])
        else:
            self.notifier = can.Notifier(self.can_bus, [self.can_log_file])

        fmt = "%(asctime)s: %(message)s"
        # Set logging level to INFO to see test pass/fail results and DEBUG
        # to see detailed serial process information
        logging.basicConfig(format=fmt, level=logging.INFO, datefmt="%H:%M:%S")

    def send_one_msg(self, arb_id, data, ext_id):

            msg = can.Message(arbitration_id=arb_id, data=data, is_extended_id=ext_id)

            try:
                self.can_bus.send(msg, timeout=1)
                log.debug("Message sent on {}".format(self.can_bus.channel_info))
                return True
            except can.CanError:
                log.debug("Message NOT sent")
                return False


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

        start_time = time.time()
        test_time = 10
        elapsed_time = 0

        log.info("Started logging CAN messages...")  

        while True:
            elapsed_time = time.time()
            # log.info("Elapsed time = ", elapsed_time)
            if (elapsed_time - start_time) >= test_time:  
                self.notifier.stop()
                break

        log.info("Logging CAN messages complete.")   

        return ret_val


# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    """
    Call runtime procedure and execute test
    """
    blk_can_test = BlkCanTest()
    blk_can_test.run_test()