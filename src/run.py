# Adapted from mOS's test.py

import subprocess
import time
import socket
import shutil
import os

PORT = 1111

LOCALHOST = "127.0.0.1"
DATA_DELAY = 0.1
SLICE_SIZE = 128

QEMU_ARGS = ["qemu-system-i386", "-boot", "c", "-no-reboot", "-no-shutdown"]

QEMU_SERIAL_DEV = "tcp:localhost:{port},server".format(port=PORT)
QEMU_DRIVE = "format=raw,file=./bin/mOS.bin"

QEMU_ARGS.append("-serial")
QEMU_ARGS.append(QEMU_SERIAL_DEV)
QEMU_ARGS.append("-drive")
QEMU_ARGS.append(QEMU_DRIVE)

BINARY = "./bin/game.bin"

if __name__ == "__main__":
    shutil.copyfile("./mOS/mOS.bin", "./bin/mOS.bin")

    instance = subprocess.Popen(QEMU_ARGS)
    time.sleep(1)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as bus:
        bus.connect((LOCALHOST, PORT))

        time.sleep(1)

        bus.send(b"test\0")

        data = bus.recv(1)

        while (len(data) < 10 or data[-10:].decode("utf-8") != "begin test"):
            data += bus.recv(1)

        with open(BINARY, "rb") as binary:
            bin = binary.read()

            bus.send(len(bin).to_bytes(4, "little"))

            chunks = int(len(bin) / SLICE_SIZE)

            for i in range(0, chunks):
                print("Sending chunk " + str(i) + " of " + str(chunks))
                chunk = bin[i * SLICE_SIZE: (i + 1) * SLICE_SIZE]
                bus.send(chunk)
                time.sleep(DATA_DELAY)

            last = bin[chunks * SLICE_SIZE:]
            if (len(last) > 0):
                bus.send(last)

            print("Binary has been sent!")

        #blocks until end
        instance.communicate()

    os.remove("./bin/mOS.bin")
