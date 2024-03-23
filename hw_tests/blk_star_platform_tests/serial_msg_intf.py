#!/usr/bin/env python3
"""
Module for handling serial messages using the K-CEMA serial protocol
specified in KT-957-0143-00. Implements the Active Backplane command set for
communicating with the Active Backplane Firmware, KT-956-0194-00.
"""
# -----------------------------------------------------------------------------
# Copyright (c) 2020, Kirintec
#
# -----------------------------------------------------------------------------
"""
OPTIONS ------------------------------------------------------------------
None

ARGUMENTS -------------------------------------------------------------
None
"""

# -----------------------------------------------------------------------------
# Imports
# -----------------------------------------------------------------------------

# stdlib imports -------------------------------------------------------
from enum import Enum
import logging
import struct
import time


# Third-party imports -----------------------------------------------
from PyCRC.CRCCCITT import CRCCCITT

# Our own imports ---------------------------------------------------
import serial_message_handler as smh

# -----------------------------------------------------------------------------
# GLOBALS
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# CONSTANTS
# -----------------------------------------------------------------------------
SERIAL_TIMEOUT = 2.0

# -----------------------------------------------------------------------------
# LOCAL UTILITIES
# -----------------------------------------------------------------------------
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())


# -----------------------------------------------------------------------------
# CLASSES
# -----------------------------------------------------------------------------
class MsgId(Enum):
    """
    Enumeration for message identifiers
    """
    PING = 0x0
    GET_SOFTWARE_VERSION_NUMBER = 0x1
    GET_SLOT_NO = 0x7
    GET_HARDWARE_INFO = 0x8
    GET_BIT_INFO = 0xA
    GET_UNIT_INFO = 0xB
    SET_UNIT_INFO = 0xC
    SET_KEY = 0xFE
    GET_KEY = 0xFF


class MsgPayloadLen(Enum):
    """
    Enumeration for payload lengths
    """
    PING = 0
    GET_SOFTWARE_VERSION_NUMBER = 11
    GET_HARDWARE_INFO = 73
    GET_BIT_INFO = 10
    GET_UNIT_INFO = 66
    SET_UNIT_INFO = 65
    GET_SLOT_NO_CMD = 7
    GET_SLOT_NO_RESP = 8
    SET_KEY = 33
    GET_KEY = 33


class SerialMsgInterface:
    """
    Class for handling serial messages to the Active Backplane board firmware
    firmware KT-956-0194-00.  The serial message ICD is defined in document
    KT-957-0413-00
    """

    def __init__(self, serial_port, baud_rate=115200):
        """
        Class constructor
        :param None
        :return: NA
        """
        self._serial_port = serial_port
        self._baud_rate = baud_rate
        self._smh = None

    def __enter__(self):
        if self._smh is not None:
            raise RuntimeError("Already started SMH!")
        self._smh = smh.MessageHandler()
        if not self._smh.start(self._serial_port, self._baud_rate):
            raise RuntimeError("Failed to start SMH!")
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._smh.stop()

    def wait_for_ack(self, ack_no, msg_id):
        """
        Wait for an ack to be received in response to a transmitted message
        :param ack_no: sequence no of message that is being acked
        :param msg_id: ID of message that is being acked
        :return: True if the ack received before SERIAL_TIMEOUT, else False
        """
        ret_val = False
        rx_timeout = time.time() + SERIAL_TIMEOUT
        while True:
            time.sleep(0.1)
            rx_msg = self._smh.get_from_rx_queue()
            if rx_msg:
                log.debug("Wait for Ack Rx Msg: {}".format(" ".join(format(x, '02x') for x in rx_msg)))

                if len(rx_msg) == smh.TOTAL_HEADER_LENGTH:
                    if rx_msg[smh.HeaderOffset.ACKNOWLEDGEMENT_NO.value] == ack_no and \
                            rx_msg[smh.HeaderOffset.MESSAGE_ID.value] == msg_id and \
                            ((rx_msg[smh.HeaderOffset.MESSAGE_STATUS_PROTOCOL_VERSION.value] & 0xE0) == 0x40):
                        ret_val = True
                        break
                    else:
                        log.debug("Wait for Ack Rx Msg Failed Consistency Check! {} - {}; {} - {}; {}".format(
                            rx_msg[smh.HeaderOffset.ACKNOWLEDGEMENT_NO.value], ack_no,
                            rx_msg[smh.HeaderOffset.MESSAGE_ID.value], msg_id,
                            rx_msg[smh.HeaderOffset.MESSAGE_STATUS_PROTOCOL_VERSION.value] & 0xE0))
                else:
                    log.debug("Wait for Ack Rx Msg Failed Length Check!")

            if time.time() > rx_timeout:
                log.debug("Wait for Ack Rx Msg Timed Out! - {:02x}".format(msg_id))
                break

        return ret_val

    def send_ping(self):
        """
        Sends a Ping message, waits for the response and processes it
        :return: True if Ping is successfully acked, else False :type Boolean
        """
        seq_no = self._smh.get_next_sequence_number()
        header_bytes = self._smh.build_message_header(
            seq_no=seq_no,
            ack_no=0,  # '0' for new message
            status=0,  # Status = '0' for new message and Protocol = '0'
            msg_id=MsgId.PING.value,
            pl_len=0)  # No payload

        log.debug("Tx Ping: {}".format(" ".join(format(x, '02x') for x in header_bytes)))
        self._smh.send_to_tx_queue(header_bytes)

        return self.wait_for_ack(seq_no, MsgId.PING.value)

    def send_set_key(self, key):
        """
        Utility method to pack the payload of a SetKey command and send it
        :param key: :type bytearray
        :return: True if message added to serial message handler tx queue, else False
        """
        if type(key) is not bytearray:
            log.critical("The key is not of type bytearray!")
            return False

        seq_no = self._smh.get_next_sequence_number()
        header_bytes = self._smh.build_message_header(seq_no,
                                                      0,
                                                      smh.MessageStatus.NEW_MESSAGE.value,
                                                      MsgId.SET_KEY.value,
                                                      MsgPayloadLen.SET_KEY.value)

        payload_bytes = bytearray(b"\x01") # Payload version
        payload_bytes.extend(key)

        # Add the payload CRC
        payload_crc = CRCCCITT(version="FFFF").calculate(bytes(payload_bytes))
        payload_crc_bytes = payload_crc.to_bytes(2, byteorder="little")
        payload_bytes.append(payload_crc_bytes[0])  # CRC LSB
        payload_bytes.append(payload_crc_bytes[1])  # CRC MSB

        # Build and send the message
        msg_bytes = header_bytes + payload_bytes
        self._smh.send_to_tx_queue(msg_bytes)

        return self.wait_for_ack(seq_no, MsgId.SET_KEY.value)

    def get_command(self, get_cmd, resp_payload_len):
        """
        Sends a command that expects a response, processes ACK response and returns response message
        :return: [0] True if response received, else False; [1] the received message
        """
        if get_cmd not in MsgId:
            raise ValueError("get_cmd must be one of MsgId enumerated values")
        if resp_payload_len not in MsgPayloadLen:
            raise ValueError("get_cmd must be one of MsgPayloadLen enumerated values")

        seq_no = self._smh.get_next_sequence_number()
        header_bytes = self._smh.build_message_header(
            seq_no=seq_no,
            ack_no=0,  # '0' for new message
            status=0,  # Status = '0' for new message and Protocol = '0'
            msg_id=get_cmd.value,
            pl_len=0)  # No payload

        log.debug("Tx Get Cmd: {}".format(" ".join(format(x, '02x') for x in header_bytes)))
        self._smh.send_to_tx_queue(header_bytes)

        # Wait for Ack, if it's successful wait for response
        if not self.wait_for_ack(seq_no, get_cmd.value):
            log.debug("No Ack")
            return False, None

        ret_val = False
        rx_timeout = time.time() + SERIAL_TIMEOUT
        while True:
            time.sleep(0.1)
            rx_msg = self._smh.get_from_rx_queue()
            if rx_msg:
                log.debug("Get Cmd Rx Msg: {}".format(" ".join(format(x, '02x') for x in rx_msg)))

                if len(rx_msg) == smh.TOTAL_HEADER_LENGTH + resp_payload_len.value + smh.CRC_LENGTH:
                    if rx_msg[smh.HeaderOffset.ACKNOWLEDGEMENT_NO.value] == seq_no and \
                            rx_msg[smh.HeaderOffset.MESSAGE_ID.value] == get_cmd.value and \
                            (((rx_msg[smh.HeaderOffset.MESSAGE_STATUS_PROTOCOL_VERSION.value] & 0xE0) == 0x80) or
                                ((rx_msg[smh.HeaderOffset.MESSAGE_STATUS_PROTOCOL_VERSION.value] & 0xE0) == 0x00)):
                        ret_val = True
                        break
                    else:
                        log.debug("Get Cmd Rx Msg Failed Consistency Check! {} - {}; {} - {}; {}".format(
                            rx_msg[smh.HeaderOffset.ACKNOWLEDGEMENT_NO.value], seq_no,
                            rx_msg[smh.HeaderOffset.MESSAGE_ID.value], get_cmd.value,
                            rx_msg[smh.HeaderOffset.MESSAGE_STATUS_PROTOCOL_VERSION.value] & 0xE0))
                else:
                    log.debug("Get Cmd Rx Msg Failed Length Check!")

            if time.time() > rx_timeout:
                log.debug("Get Cmd Rx Msg Timed Out! - {:02x}".format(get_cmd.value))
                break

        return ret_val, rx_msg

    @staticmethod
    def unpack_get_software_version_number_response(ba):
        """
        Utility method to unpack the payload of a GetSoftwareVersionNumber Response
        :param ba: ByteArray payload of response message to unpack
        :return: None if ba is wrong type or length, else 5x unpacked values
        """
        if type(ba) != bytearray or len(ba) != \
                smh.TOTAL_HEADER_LENGTH + MsgPayloadLen.GET_SOFTWARE_VERSION_NUMBER.value + smh.CRC_LENGTH:
            log.critical("Unpack GetSoftwareVersionNumber Response expecting {}-byte array".format(
                smh.TOTAL_HEADER_LENGTH + MsgPayloadLen.GET_SOFTWARE_VERSION_NUMBER.value + smh.CRC_LENGTH))
            return None
        else:
            payload_version, sw_major, sw_minor, sw_patch, sw_build = \
                struct.unpack("<BHHHI", ba[smh.TOTAL_HEADER_LENGTH:len(ba) - smh.CRC_LENGTH])
            return payload_version, sw_major, sw_minor, sw_patch, sw_build

    @staticmethod
    def unpack_get_key_response(ba):
        """
        Utility method to unpack the payload of a GetKey Response
        :param ba: ByteArray payload of response message to unpack
        :return: None if ba is wrong type or length, else key payload version, key
        """
        if type(ba) != bytearray or len(ba) != \
                smh.TOTAL_HEADER_LENGTH + MsgPayloadLen.GET_KEY.value + smh.CRC_LENGTH:
            log.critical("Unpack GetKey Response expecting {}}-byte array".format(
                smh.TOTAL_HEADER_LENGTH + MsgPayloadLen.GET_KEY.value + smh.CRC_LENGTH
            ))
            return None
        else:
            read_position = smh.TOTAL_HEADER_LENGTH

            payload_version = ba[read_position: read_position + 1]
            read_position += 1

            key = ba[read_position: read_position + 32]
            read_position += 32

            return payload_version, key

    # def start(self, serial_port, baud_rate):
    #     """
    #     Starts the MessageHandler running
    #     :param serial_port:
    #     :param baud_rate:
    #     :return: True if started, else False :type: Boolean
    #     """
    #     return self._smh.start(serial_port, baud_rate)

    # def stop(self):
    #     """
    #     Stop the MessageHandler from running
    #     :return:
    #     """
    #     self._smh.stop()


# -----------------------------------------------------------------------------
# FUNCTIONS
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# RUNTIME PROCEDURE
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    """
    Module is NOT intended to be executed stand-alone, print warning message
    """
    print("Module is NOT intended to be executed stand-alone")
