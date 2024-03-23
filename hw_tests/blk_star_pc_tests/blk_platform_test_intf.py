#!/usr/bin/env python3
"""Blackstar Platform Test Interface"""

# -----------------------------------------------------------------------------
# Imports
# -----------------------------------------------------------------------------

# stdlib imports --------------------------------------------------------------
from __future__ import print_function
import logging
import ipaddress
import platform
from os import popen
import sys

# Our own imports -------------------------------------------------------------
from ssh import SSH
sys.path.append('C:/workspace/blackstar/hw_tests/test_equipment/')
# import a module using its name as a string
# my_module = importlib.import_module('visa_test_equipment')
from visa_test_equipment import *

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
class BlkPlatformTestInt:
    PYTHON_PATH = "python3"
    SUDO = "sudo"
    BLK_TEST_SCRIPT_PATH = "/home/blackstar-admin/tests/"
    BLK_SED_APPS_PATH = "/mnt/sed/admin-data/apps/"
    READ_SLOT_COMMAND = "BlackStarECM -s"
    SERIAL_COMMAND = "serial_msg_test.py"
    RPI_CAN_MESSAGE_COMMAND = "rpi_can_test.py"
    BLK_CAN_MESSAGE_COMMAND = "blk_can_test.py"

    # SUDO_MODE_COMMAND = "SUDO_ASKPASS=./pass.sh sudo -A su - blackstar-admin"
    SUDO_MODE_COMMAND = "export SUDO_ASKPASS=./pass.sh"

    # Set logging level to INFO to see test pass/fail results and DEBUG
    # to see detailed serial process information
    fmt = "%(asctime)s: %(message)s"
    logging.basicConfig(format=fmt, level=logging.INFO, datefmt="%H:%M:%S")

    def __init__(self, username, hostname=None):
        """
        Class constructor
        :param username: User name for SSH login :type String
        :param hostname: Hostname of the board/unit :type String
        """
        self._ssh_conn = None
        self._username = username
        self._hostname = hostname

        if hostname is not None:
            self.open_ssh_connection(hostname)

    def __repr__(self):
        """ :return: string representing the class """
        return "CsmPlatformTest({!r})".format(self._hostname)

    def __del__(self):
        """ Class destructor - close the SSH connection """
        if self._hostname != "":
            self.close_ssh_connection()

    def __enter__(self):
        """ Context manager entry """
        return self

    def __exit__(self, exc_ty, exc_val, tb):
        """ Context manager exit - close the SSH connection """
        self.close_ssh_connection()

    def open_ssh_connection(self, hostname):
        """
        Opens the specified serial port
        :param hostname: Hostname of the KT-001-0213-00 :type string
        :return: N/A
        """
        # Perform a ping to help the test computer find the host
        self._ping(hostname, retries=4)
        self._ssh_conn = SSH(hostname, self._username)
        log.debug("Opened SSH connection {}".format(hostname))
        self._hostname = hostname

    def close_ssh_connection(self):
        """ Closes _ssh_conn if it is open """
        if self._ssh_conn is not None:
            log.debug("Closing SSH connection {}".format(self._hostname))
            self._ssh_conn.close()
        self._ssh_conn = None
        self._hostname = ""

    def check_ssh_connection(self):
        """ Raise a Runtime Error if the SSH connection is not open """
        if self._ssh_conn is None:
            raise RuntimeError("SSH Connection is not open!")
        else:
            return True
    
    def get_ECM_slot_position(self):
        """
        Gets the Blackstar ECM slot position from BlackstarECM
        """
        slot_position_resp = ""

        if self.check_ssh_connection():

            # # # place the connection in sudo mode
            # self._ssh_conn.send_command(self.SUDO_MODE_COMMAND)
            # reply = self._ssh_conn.send_command("echo $SUDO_ASKPASS")
            # print("reply: ", reply)
            # for lines in (reply.stdout.splitlines() or reply.stderr.splitlines()):
            #     print("lines: ", lines)
            # # print("reply: ", reply)
            cmd_str = "{} {}{}".format(self.SUDO, self.BLK_SED_APPS_PATH, self.READ_SLOT_COMMAND)
            print("Command: ", cmd_str)
            resp = self._ssh_conn.send_command(cmd_str)
            print("res: ", resp)

            # cmd_str = "./pass.sh"
            # print("Command: ", cmd_str)
            # resp = self._ssh_conn.send_command(cmd_str)
            # print("res: ", resp)

            for a_line in resp.stdout.splitlines():
                print("a_line: ", a_line)
                if "ECM slot number:" in a_line:
                    
                    slot_position_resp = a_line.split(':')[1]
                    log.info("slot_position_resp: {}".format(slot_position_resp))
                    break

        return slot_position_resp

    def run_serial_message_test(self):
        """
        Serial message test method
        """
        if self.check_ssh_connection():

            cmd_str = "{} {}{}".format(self.PYTHON_PATH, self.BLK_TEST_SCRIPT_PATH, self.SERIAL_COMMAND)
            print(cmd_str)
            resp = self._ssh_conn.send_command(cmd_str, timeout=95, retries=3)
            log.debug("res: ", resp)

            for a_line in resp.stderr.splitlines():
                log.debug(a_line)
                if "Pass Count:" in a_line:
                    pass_count = a_line.split(':')[4]
                    print("pass count = ", pass_count)

                if "Fail Count:" in a_line:
                    fail_count = a_line.split(':')[4]
                    print("fail count = ", fail_count)

                    if pass_count == " 40":
                        return True, pass_count
                    else:
                        return False, fail_count
        
    def run_blk_can_message_test(self):
        """
        """
        if self.check_ssh_connection():

            cmd_str = "{} {}{}".format(self.PYTHON_PATH, self.BLK_TEST_SCRIPT_PATH, self.BLK_CAN_MESSAGE_COMMAND)
            resp = self._ssh_conn.send_command(cmd_str, timeout=25)
            log.debug("res: ", resp)

            # for a_line in resp.stderr.splitlines():
            #     log.debug(a_line)
            #     if "Pass Count:" in a_line:
            #         pass_count = a_line.split(':')[4]



    def run_rpi_can_message_test(self):

        cmd_str = "{} {}{}".format(self.PYTHON_PATH, self.BLK_TEST_SCRIPT_PATH, self.RPI_CAN_MESSAGE_COMMAND)
        resp = self._ssh_conn.send_command(cmd_str, timeout=25)
        log.debug("res: ", resp)


    @staticmethod
    def _ping(ip_address, retries=1):
        """
        Calls the system ping command for the specified IP address
        :param ip_address: ip address/hostname to ping :type: string
        :param retries: number of times to retry failed ping before giving up :type: integer
        :return: True if the IP address is successfully pinged with retries attempts, else False
        """
        return_val = False

        # This will throw a ValueError exception if ip_address is NOT a valid IP address
        try:
            a = ipaddress.ip_address(ip_address)
        except Exception as ex:
            log.debug("Using hostname for ping rather than IP address".format(ex))
            a = ip_address

        ping_type = ""

        if platform.system().lower() == "windows":
            count_param = "n"
            if type(a) == ipaddress.IPv6Address:
                ping_type = "-6"
            else:
                ping_type = "-4"
        else:
            count_param = "c"

        for i in range(0, retries):
            output = popen("ping {} -{} 1 {}".format(ping_type, count_param, a)).read()
            log.debug("Ping {}:".format(i))
            log.debug(output)

            if "unreachable" in output or "0 packets received" in output or "could not find" in output:
                return_val = False
            else:
                return_val = True
                break

        return return_val




    