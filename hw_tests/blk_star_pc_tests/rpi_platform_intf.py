#!/usr/bin/env python3
"""Raspberry Pi Platform Test Interface"""

# -----------------------------------------------------------------------------
# Imports
# -----------------------------------------------------------------------------

# stdlib imports --------------------------------------------------------------
from __future__ import print_function
import logging
import ipaddress
import platform
from os import popen

# Our own imports -------------------------------------------------------------
from ssh import SSH
from raspberry_pi_interface import RaspberryPiInterface

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
class RpiPlatformTestInt:
    PYTHON_PATH = "python3"
    SUDO = "sudo"
    RPI_TEST_SCRIPT_PATH = "/home/blackstar-test/test_scripts/"
    RPI_SLOT_POS_COMMAND = "slot_position_test.py -p"
    POWER_0N_COMMAND = "rpi_power_on_test.py"


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
        :param hostname: Hostname of the KT-000-0140-00 :type string
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
    
    def run_power_on_test(self):
        """
        Blackstar Power On/OFF Test
        """  
        if self.check_ssh_connection():
            cmd_str = "{} {}{}".format(self.PYTHON_PATH, self.RPI_TEST_SCRIPT_PATH, self.POWER_0N_COMMAND)
            print("Command: ", cmd_str)
            resp = self._ssh_conn.send_command(cmd_str, timeout=60)
            # print("res: ", resp)

            for a_line in resp.stderr.splitlines():
                print("a_line: ", a_line)
                if "INFO - Blackstar EPU Power ON/OFF Test Result - " in a_line:
                    
                    test_result = a_line.split('-')[2]
                    log.info("Test result: {}".format(test_result))
                
                    if test_result == " PASS":
                        return True
                    else:
                        return False

    
    def set_blackstar_slot_position(self, position):

        slot_position = position

        if self.check_ssh_connection():
            cmd_str = "{} {}{} {}".format(self.PYTHON_PATH, self.RPI_TEST_SCRIPT_PATH, self.RPI_SLOT_POS_COMMAND, slot_position)
            # print("Command: ", cmd_str)
            self._ssh_conn.send_command(cmd_str, timeout=10)
            # print("res: ", resp)

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




    