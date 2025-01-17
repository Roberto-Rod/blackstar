
from __future__ import print_function
import can


def send_one():
    bus = can.interface.Bus(bustype='socketcan', channel='can0', bitrate=500000)
    # bus = can.interface.Bus(bustype='pcan', channel='PCAN_USBBUS1', bitrate=500000)
    # bus = can.interface.Bus(bustype='ixxat', channel=0, bitrate=250000)
    # bus = can.interface.Bus(bustype='vector', app_name='CANalyzer', channel=0,˓→bitrate=250000)

    msg = can.Message(arbitration_id=0xc0ffee,
    # data=[0, 25, 0, 1, 3, 1, 4, 1],
    data=[8, 1, 0, 7,101],
    is_extended_id=True)
    try:
        bus.send(msg)
        print("Message sent on {}".format(bus.channel_info))
    except can.CanError:
        print("Message NOT sent")

if __name__ == "__main__":
    send_one()