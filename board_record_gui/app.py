import os
# Force Bleak to use the .NET backend (must be set before importing Bleak)
os.environ["BLEAK_USE_WINRT"] = "0"

import tkinter as tk
import asyncio
import threading
from bleak import BleakScanner, BleakClient


class BLEGuiApp:
    def __init__(self, master):
        self.master = master
        self.master.title("BLE .NET Backend - Scan & Connect")

        # We'll store a list of (address, name) from the last scan
        self.discovered_devices = []

        # UI: Log text box
        self.log_box = tk.Text(master, width=60, height=15)
        self.log_box.pack(padx=10, pady=(10, 5))

        # UI: Buttons
        button_frame = tk.Frame(master)
        button_frame.pack(pady=(0, 10))

        self.scan_button = tk.Button(button_frame, text="Scan for Devices", command=self.start_scan)
        self.scan_button.pack(side=tk.LEFT, padx=5)

        self.connect_button = tk.Button(button_frame, text="Connect to Selected", command=self.start_connect)
        self.connect_button.pack(side=tk.LEFT, padx=5)

        # UI: Listbox to show devices
        self.listbox = tk.Listbox(master, width=50)
        self.listbox.pack(padx=10, pady=(0, 10))

        self.log("Ready.")

    def log(self, message: str):
        """Append a message to the text box and also print to console."""
        self.log_box.insert(tk.END, message + "\n")
        self.log_box.see(tk.END)
        print(message)

    # -------------------------------------------------------------------------
    # SCAN DEVICES
    # -------------------------------------------------------------------------
    def start_scan(self):
        """Spawn a background thread so we don't freeze the GUI while scanning."""
        self.log("Starting BLE scan (using .NET backend, 5s)...")
        self.discovered_devices = []  # reset device list
        self.listbox.delete(0, tk.END)
        thread = threading.Thread(target=self.scan_ble)
        thread.start()

    def scan_ble(self):
        """Worker thread function for scanning."""
        try:
            # Create an asyncio loop in this thread
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)
            devices = loop.run_until_complete(BleakScanner.discover(timeout=5.0))
            loop.close()

            # Store results, then update listbox in main thread
            new_list = [(d.address, d.name or "Unknown") for d in devices]
            self.master.after(0, self.update_device_list, new_list)
        except Exception as e:
            self.master.after(0, self.display_error, f"Scan error: {e}")

    def update_device_list(self, device_list):
        """Update the UI listbox with the scanned devices."""
        self.discovered_devices = device_list
        self.log(f"Scan complete, found {len(device_list)} devices:")
        for addr, name in device_list:
            self.log(f"  - {addr}: {name}")
            self.listbox.insert(tk.END, f"{name} [{addr}]")

    # -------------------------------------------------------------------------
    # CONNECT TO SELECTED DEVICE
    # -------------------------------------------------------------------------
    def start_connect(self):
        """Spawn a background thread to connect to the selected device."""
        selection = self.listbox.curselection()
        if not selection:
            self.log("No device selected.")
            return

        index = selection[0]
        address, name = self.discovered_devices[index]
        self.log(f"Attempting to connect to: {name} [{address}]")

        thread = threading.Thread(target=self.connect_device, args=(address, name))
        thread.start()

    def connect_device(self, address, name):
        """Worker thread function for connecting to a BLE device."""
        try:
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)
            loop.run_until_complete(self.async_connect(address, name))
            loop.close()
        except Exception as e:
            self.master.after(0, self.display_error, f"Connect error: {e}")

    async def async_connect(self, address, name):
        """Async function to connect, check if connected, then disconnect."""
        self.log(f"Connecting to {name} [{address}]...")
        try:
            async with BleakClient(address) as client:
                connected = await client.is_connected()
                if connected:
                    self.log(f"Connected to {name} [{address}]!")
                    # FIX: Convert the BleakGATTServiceCollection to a list
                    services = await client.get_services()
                    services_list = list(services)  # Now it's a standard list
                    self.log(f"Number of services: {len(services_list)}")

                    # You could iterate over services_list to see the actual service UUIDs
                    # for svc in services_list:
                    #     self.log(f"Service UUID: {svc.uuid}")
                else:
                    self.log(f"Could not connect to {name} [{address}].")
        except Exception as e:
            self.log(f"Error during connect: {e}")

    # -------------------------------------------------------------------------
    # ERROR DISPLAY
    # -------------------------------------------------------------------------
    def display_error(self, error_msg: str):
        self.log(error_msg)


def main():
    root = tk.Tk()
    app = BLEGuiApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
