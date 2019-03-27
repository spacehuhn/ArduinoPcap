# Original version made by @xdavidhu (github.com/xdavidhu, https://xdavidhu.me/)
# Modified to work on windows by L0laapk3 (github.com/L0laapk3)

import serial
import io
import os
import subprocess
import signal
import time
import win32pipe, win32file

import sys

import ctypes


def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

if not is_admin():
    ctypes.windll.shell32.ShellExecuteW(None, u"runas", unicode(sys.executable), unicode(__file__), None, 1)
    exit(0)

try:
    serialportInput = raw_input("[?] Select a serial port (default COM5'): ")
    
    if serialportInput == "":
        serialport = "COM5"
    else:
        serialport = serialportInput
except KeyboardInterrupt:
    print("\n[+] Exiting...")
    exit()

try:
    canBreak = False
    while not canBreak:
        boardRateInput = raw_input("[?] Select a baudrate (default '921600'): ")
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
    wiresharkCmdInput = raw_input("[?] Select wireshark location (default 'D:\Program Files\Wireshark\Wireshark.exe'): ")
    if wiresharkCmdInput == "":
        wiresharkCmd = ['D:\Program Files\Wireshark\Wireshark.exe', r'-i\\.\pipe\wireshark','-k']
    else:
        wiresharkCmd = [wiresharkCmdInput, r'-i\\.\pipe\wireshark','-k']
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

print("[?] Waiting for ESP boot..")


counter = 0

check = 0
while check == 0:
    line = ser.readline()
    if b"<<START>>" in line:
        check = 1
        print("[+] Stream started...")
    #else: print '"'+line+'"'

print("[+] Starting up wireshark...")
proc=subprocess.Popen(wiresharkCmd)

pipe = win32pipe.CreateNamedPipe(
    r'\\.\pipe\wireshark',
    win32pipe.PIPE_ACCESS_OUTBOUND,
    win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
    1, 65536, 65536,
    300,
    None)
win32pipe.ConnectNamedPipe(pipe, None)



try:
    while True:
        
        data = ser.read()
        win32file.WriteFile(pipe, data)
except KeyboardInterrupt:
    print("[+] Stopping...")
    subprocess.call(['taskkill', '/F', '/T', '/PID', str(proc.pid)])
except:
    print("[+] Stopping...")


ser.close()
print("[+] Done.")
