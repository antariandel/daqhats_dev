#!/usr/bin/env python3
"""
    MCC 118 Control Panel 

    Purpose:
        Display the MCC 118 input voltages

    Description:
        This example reads and displays the input voltages
"""
from daqhats import hat_list, mcc118, HatIDs, HatError
from tkinter import *
from tkinter import messagebox
#import tkMessageBox
import tkinter.font

class ControlApp:
    
    def __init__(self, master):
        self.master = master
        master.title("MCC 18 Control Panel")
    
        # Initialize variables
        self.device_open = False
        self.open_address = 0
        self.board = None

        # GUI Setup

        self.BOLD_FONT = tkinter.font.Font(
            family=tkinter.font.nametofont("TkDefaultFont")["family"],
            size=tkinter.font.nametofont("TkDefaultFont")["size"],
            weight="bold")

        # Create and organize frames
        self.top_frame = LabelFrame(master, text="Select Device")
        self.top_frame.pack(side=TOP, expand=False, fill=X)

        self.bottom_frame = LabelFrame(master, text="Analog Inputs")
        self.bottom_frame.pack(side=BOTTOM, expand=True, fill=BOTH)
        
        # Create widgets

        self.dev_label = Label(self.top_frame, text="MCC 118 address:")
        self.dev_label.grid(row=0, column=0)

        self.open_button = Button(self.top_frame, text="Open", width=6, command=self.pressedOpenButton)

        # Get list of MCC 118 devices for the device list widget
        self.addr_list = self.listDevices()

        if len(self.addr_list) == 0:
            self.device_lister = Label(self.top_frame, text="None found")
            self.open_button.config(state=DISABLED)
        else:
            self.device_variable = StringVar(self.top_frame)
            self.device_variable.set(self.addr_list[0])
            self.device_lister = OptionMenu(self.top_frame, self.device_variable, *self.addr_list)
            
        self.device_lister.grid(row=0, column=1)
        self.open_button.grid(row=0, column=2)

        self.channel_labels = []
        self.voltages = []
        for index in range(mcc118.info().NUM_AI_CHANNELS):
            # Labels
            self.channel_labels.append(Label(self.bottom_frame, text="Ch {}".format(index), font=self.BOLD_FONT))
            self.channel_labels[index].grid(row=index, column=0)
            self.channel_labels[index].grid_configure(sticky="W")
            
            self.voltages.append(Label(self.bottom_frame, text="", font=self.BOLD_FONT))
            self.voltages[index].grid(row=index, column=1)
            self.voltages[index].grid_configure(sticky="E")

            self.bottom_frame.grid_rowconfigure(index, weight=1)
            
        #self.bottom_frame.grid_rowconfigure(0, weight=1)
        self.bottom_frame.grid_columnconfigure(0, weight=1)
        self.bottom_frame.grid_columnconfigure(1, weight=1)

        self.bottom_frame.bind("<Configure>", self.resizeText)

        # Disable widgets until a device is opened
        self.disableControls()
        
        master.protocol('WM_DELETE_WINDOW', self.close) # exit cleanup
        
    def resizeText(self, event):
        new_size = -max(12, int(event.height / 12))
        #label_height = (event.height - 18)/8 - 4
        #font_height = self.BOLD_FONT.metrics("linespace")
        #ratio = label_height/font_height
        #new_size = self.BOLD_FONT.cget("size")
        #new_size *= ratio
        self.BOLD_FONT.configure(size=new_size)
        
        
    def disableControls(self):
        # Enable the address selector
        self.device_lister.config(state=NORMAL)
        # Disable the board controls
        for child in self.bottom_frame.winfo_children():
            child.config(state=DISABLED)

    def enableControls(self):
        # Disable the address selector
        self.device_lister.config(state=DISABLED)
        # Enable the board controls
        for child in self.bottom_frame.winfo_children():
            child.config(state=NORMAL)

    def listDevices(self):
        self.dev_list = hat_list(HatIDs.MCC_118)
        addr_list = ["{}".format(dev.address) for dev in self.dev_list]
        return addr_list

    def openDevice(self, address):
        try:
            self.board = mcc118(address)
        except:
            return False
        else:
            return True

    def closeDevice(self):
        self.board = None
        
    def updateInputs(self):
        if self.device_open:
            for channel in range(mcc118.info().NUM_AI_CHANNELS):
                value = self.board.a_in_read(channel)
                self.voltages[channel].config(text="{:.3f}".format(value))
                    
            # schedule another update in 200 ms
            self.master.after(200, self.updateInputs)
                
    # Event handlers
    def pressedOpenButton(self):
        if self.open_button.cget('text') == "Open":
            # Open the selected device
            address = int(self.device_variable.get())
            if self.openDevice(address):
                self.device_open = True
                self.open_address = address

                self.enableControls()
                
                # Periodically read the inputs and update controls
                self.updateInputs()
                
                self.open_button.config(text="Close")
            else:
                messagebox.showerror("Error", "Could not open device.")
        else:
            if self.device_open:
                self.closeDevice()
                self.device_open = False
            self.open_button.config(text="Open")
            self.disableControls()

    def close(self):
        self.master.destroy()


root = Tk()
app = ControlApp(root)
root.mainloop()
