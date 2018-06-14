#!/usr/bin/env python
#
# MCC 134 example program
#
# Read channels 0 and 1 every 5 minutes and send an IFTTT event for logging to a Google sheets spreadsheet.
# This uses the IFTTT Webhooks channel for inputs.  You create your own event and enter
# the name here for event_name (we used temperature_data for the name), then choose an activity
# to occur when the event happens.  We used the Google Sheets activity and selected "add a row to 
# a spreadsheet", then set it up to log the time and temperatures (Value0 and Value1).
#
# 1. At IFTTT.com go to My Applets, then click New Applet.
# 2. Click "this" then search for Webhooks.  Click Webhooks when found.
# 3. Click "Receive a web request" on the Choose trigger screen.
# 4. Enter your event name in the Event Name field (temperature_data for this example) then click "Create trigger"
# 5. Click "that" then search for Google Sheets.  Click Google Sheets when found.
# 6. Click "Add row to spreadsheet" on the Choose action screen.
# 7. Enter your desired spreadsheet name, then change the formatted row to only contain "{{OccurredAt}} ||| {{Value1}} |||{{Value2}}".
#    Change the Drive folder path if desired, then click "Create action"
# 8. Review the details, then click Finish.
#
# Enter your key from IFTTT Webhooks documentation for the variable "key".  You can find this key by:
#
# 1. At IFTTT.com go to My Applets, then click the Services heading.
# 2. Enter Webhooks in the Filter services field, then click on Webhooks.
# 3. Click on Documentation.  Your key will be listed at the top; copy that key and paste it inside the quotes below, replacing <my_key>.
#
import time
import daqhats as hats
import requests

# IFTTT values
event_name = "temperature_data"
key = "<my_key>"


def send_trigger(event, value1 = "", value2 = "", value3 = ""):
    report = {}
    report['value1'] = str(value1)
    report['value2'] = str(value2)
    report['value3'] = str(value3)
    requests.post("https://maker.ifttt.com/trigger/" + event + "/with/key/" + key, data=report)

above_threshold = False

# Find the first MCC 134
list = hats.hat_list(filter_by_id = hats.HatIDs.MCC_134)
if not list:
    print("No MCC 134 boards found")
    sys.exit()

board = hats.mcc134(list[0]['address'])

# Configure for type T thermocouple on channels 0 and 1
board.tc_type_write(0, hats.tcTypes.TYPE_T)
board.tc_type_write(1, hats.tcTypes.TYPE_T)

while True:
    # read the temperature
    temperature0 = board.t_in_read(0)
    temperature1 = board.t_in_read(1)
    
    send_trigger(event_name, "{:.2f}".format(temperature0), "{:.2f}".format(temperature1))
    time.sleep(5*60)