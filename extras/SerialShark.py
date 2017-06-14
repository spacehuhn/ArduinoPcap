# Made by @xdavidhu (github.com/xdavidhu, https://xdavidhu.me/)

import serial
import io
import os
import subprocess
import signal
import time

try:
    serialportInput = input("[?] Select a serial port (default '/dev/ttyUSB0'): ")
    if serialportInput == "":
        serialport = "/dev/ttyUSB0"
    else:
        serialport = serialportInput
except KeyboardInterrupt:
    print("\n[+] Exiting...")
    exit()

try:
    canBreak = False
    while not canBreak:
        boardRateInput = input("[?] Select a baudrate (default '921600'): ")
        if boardRateInput == "":
            boardRate = 921600
            canBreak = True
        else:
            try:
                boardRate = int(boardRateInput)
            except KeyboardInterrupt:
                print("\n[+] Exiting...")
                exit()
            except Exception as e:
                print("[!] Please enter a number!")
                continue
            canBreak = True
except KeyboardInterrupt:
    print("\n[+] Exiting...")
    exit()

try:
    filenameInput = input("[?] Select a filename (default 'capture.pcap'): ")
    if filenameInput == "":
        filename = "capture.pcap"
    else:
        filename = filenameInput
except KeyboardInterrupt:
    print("\n[+] Exiting...")
    exit()

canBreak = False
while not canBreak:
    try:
        ser = serial.Serial(serialport, boardRate)
        canBreak = True
    except KeyboardInterrupt:
        print("\n[+] Exiting...")
        exit()
    except:
        print("[!] Serial connection failed... Retrying...")
        time.sleep(2)
        continue

print("[+] Serial connected. Name: " + ser.name)
counter = 0
f = open(filename,'wb')

check = 0
while check == 0:
    line = ser.readline()
    if b"<<START>>" in line:
        check = 1
        print("[+] Stream started...")
    #else: print '"'+line+'"'

print("[+] Starting up wireshark...")
cmd = "tail -f -c +0 " + filename + " | wireshark -k -i -"
p = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                       shell=True, preexec_fn=os.setsid)

try:
    while True:
        ch = ser.read()
        f.write(ch)
        f.flush()
except KeyboardInterrupt:
    print("[+] Stopping...")
    os.killpg(os.getpgid(p.pid), signal.SIGTERM)

f.close()
ser.close()
print("[+] Done.")
