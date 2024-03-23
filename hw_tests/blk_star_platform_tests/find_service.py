#!/usr/bin/env python3
import socket
import argparse
import time

import zeroconf as zeroconfig


class FindService:
    def find(service_name, type="kpms", return_service=False, timeout=10):
        ret_val = "0.0.0.0",0
        type_ = "_{}._tcp.local.".format(type)
        count = timeout * 10

        def on_change(zeroconf, service_type, name, state_change):
            nonlocal ret_val
            nonlocal count
            if state_change is zeroconfig.ServiceStateChange.Added:
                info = zeroconf.get_service_info(service_type, name)
                if info:
                    address = "{}".format(socket.inet_ntoa(info.addresses[0]))
                    server = str(info.server)
                    if service_name in server:
                        if return_service:
                            ret_val = server.rstrip(".")
                        else:
                            ret_val = address, info.port
                        count = 0

        zeroconf = zeroconfig.Zeroconf()
        browser = zeroconfig.ServiceBrowser(zeroconf, type_, handlers=[on_change])

        while count > 0:
            time.sleep(0.1)
            count = count - 1

        zeroconf.close()
        return ret_val

    def find_pms(return_service=False, ip_addr=None, timeout=10):
        service_name = "Power Meter Service"
        if ip_addr:
            service_name += " {}".format(ip_addr).replace(".", "-")
        return FindService.find(service_name, "kpms", return_service, timeout)

    def find_sgs(return_service=False, ip_addr=None, timeout=10):
        service_name = "Signal Generator Service"
        if ip_addr:
            service_name += " {}".format(ip_addr).replace(".", "-")
        return FindService.find(service_name, "kpms", return_service, timeout)

    def find_sas(return_service=False, ip_addr=None, timeout=10):
        service_name = "Spectrum Analyser Service"
        if ip_addr:
            service_name += " {}".format(ip_addr).replace(".", "-")
        return FindService.find(service_name, "kpms", return_service, timeout)
    
    def find_psus(return_service=False, ip_addr=None, timeout=10):
        service_name = "Power Supply Service"
        if ip_addr:
            service_name += " {}".format(ip_addr).replace(".", "-")
        return FindService.find(service_name, "kpms", return_service, timeout)

    def find_blk(return_service=False, timeout=10):
        return FindService.find("BLK-", "ssh", return_service, timeout)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Find zeroconf service")
    parser.add_argument("-n", "--name", help="Display service name (no IP address or port)", action="store_true")
    parser.add_argument("-p", "--pms", help="Find Power Meter Service", action="store_true")
    parser.add_argument("-s", "--sgs", help="Find Signal Generator Service", action="store_true")
    parser.add_argument("-a", "--sas", help="Find Signal Generator Service", action="store_true")
    parser.add_argument("-u", "--psus", help="Find Signal Generator Service", action="store_true")
    parser.add_argument("-b", "--blk", help="Find BLK", action="store_true")
    args = parser.parse_args()

    if args.pms:
        details = FindService.find_pms(args.name)
    elif args.sgs:
        details = FindService.find_sgs(args.name)
    elif args.sas:
        details = FindService.find_sas(args.name)
    elif args.psus:
        details = FindService.find_psus(args.name)
    elif args.blk:
        details = FindService.find_blk(args.name)

    if isinstance(details, list):
        print("{}:{}".format(details[0], details[1]))
    else:
        print(details)