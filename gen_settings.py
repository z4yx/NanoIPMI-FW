#!/usr/bin/env python3

import struct
import tempfile
import binascii
import subprocess

magic = 0xdead4545
checksum = 0
upgrade_flags=0xffffffff
crc_new_fw=0
sz_new_fw=0

netconf_ip = bytes([192, 168, 1, 80])
netconf_sn = bytes([255, 255, 255, 0])
netconf_gw = bytes([192, 168, 1, 1])
netconf_dns = bytes([8, 8, 8, 8])
netconf_logserver = bytes([192, 168, 1, 1])
netconf_dhcp = 2

num = int(input("Start: "), 10)
while True:
    netconf_mac = bytes([0x78,0xCA,0x83,0x40,0x04,num])
    print("")
    input("MAC is {}, press enter to continue".format(
        ':'.join(['{:02X}'.format(b) for b in netconf_mac])
        )
    )

    for x in range(2):
        if x == 0:
            checksum = 0;
        else:
            checksum = sum(buf)

        buf = struct.pack('<5I6s4s4s4s4s4sBx',
            magic, checksum, upgrade_flags, crc_new_fw, sz_new_fw,
            netconf_mac, netconf_ip, netconf_sn, netconf_gw, netconf_dns,
            netconf_logserver, netconf_dhcp)
        # print(len(buf))

    try:
        subprocess.run(["st-flash", "write", 'build/BLdr.bin',     '0x8000000']).check_returncode()
        subprocess.run(["st-flash", "write", 'build/NanoIPMI.bin', '0x8003800']).check_returncode()

        with tempfile.NamedTemporaryFile() as fp:
            fp.write(buf)
            fp.flush()
            subprocess.run(["st-flash", "write", fp.name, '0x801f800']).check_returncode()
            # print (buf)
    except subprocess.CalledProcessError:
        print("Failed to write, please try again!")
        continue

    num += 1
