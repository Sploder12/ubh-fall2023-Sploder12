# Adapted from mOS's test.py

import subprocess
import time
import socket

PORT = 11423

LOCALHOST = "127.0.0.1"
DATA_DELAY = 0.1
SLICE_SIZE = 128

QEMU_ARGS = ["qemu-system-i386", "-boot", "c", "-no-reboot", "-no-shutdown", "-nographic"]

QEMU_SERIAL_DEV = "tcp:localhost:{port},server".format(port=PORT)
QEMU_DRIVE = "format=raw,file=./mOS/mOS.bin"

QEMU_ARGS.append("-serial")
QEMU_ARGS.append(QEMU_SERIAL_DEV)
QEMU_ARGS.append("-drive")
QEMU_ARGS.append(QEMU_DRIVE)

BINARY = "bin/game.bin"

if __name__ == "__main__":
    instance = subprocess.Popen(QEMU_ARGS, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(1)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as bus:
        bus.connect((LOCALHOST, PORT))

    time.sleep(1)

    bus.send("test\0")

    data = bus.recv(1)

    while (len(data) < 10 or data[-10:].decode("utf-8") != "begin test"):
        data += bus.recv(1)

    with open(BINARY, "rb") as binary:
        bin = binary.read()

        bus.send(len(bin).to_bytes(4, "little"))

        chunks = int(len(bin) / SLICE_SIZE)

        for i in range(0, chunks):
            chunk = bin[i * SLICE_SIZE: (i + 1) * SLICE_SIZE]
            bus.send(chunk)
            time.sleep(DATA_DELAY)

        last = bin[chunks * SLICE_SIZE]
        if (len(last) > 0):
            bus.send(last)

    #blocks until end
    instance.communicate()