#!/usr/bin/env python
#
# MCC 134 example program
#
# Continuously read channel 0 and send an IFTTT trigger if the temperature exceeds a threshold.
# This uses the IFTTT Webhooks maker channel for inputs.  You create your own event and enter
# the name here for event_name (I used temperature_alarm for the name), then choose an activity
# to occur when the event happens.  I chose the SMS activity and used the messate:
#
# Temperature alarm {{Value3}} at {{OccurredAt}}. Temp: {{Value1}}, Threshold: {{Value2}}
# 
# This lets me indicate whether the alarm is setting or clearing using Value3, and include the
# current temperature and threshold temperature.
#
# This uses the IFTTT Webhooks channel for inputs.  You create your own event and enter
# the name here for event_name (we used temperature_alarm for the name), then choose an activity
# to occur when the event happens.  We used the SMS activity and selected "Send me an SMS" 
# then set it up to send the time, state, temperature, and threshold in a message.
#
# 1. At IFTTT.com go to My Applets, then click New Applet.
# 2. Click "this" then search for Webhooks.  Click Webhooks when found.
# 3. Click "Receive a web request" on the Choose trigger screen.
# 4. Enter your event name in the Event Name field (temperature_alarm for this example) then click "Create trigger"
# 5. Click "that" then search for SMS.  Click SMS when found.
# 6. Click "Send me an SMS" on the Choose action screen.
# 7. Change the message field to contain "Temperature alarm {{Value3}} at {{OccurredAt}}. Temp: {{Value1}}, Threshold: {{Value2}}"
#    then click "Create action"
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

# temperature thresholds - use a bit of hysteresis so you don't get a lot of events if the temperature
# is hovering right around the threshold
temperature_threshold = 27.0
clear_threshold = 26.5

channel = 1

# IFTTT values
event_name = "temperature_alarm"
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

# Configure for type T thermocouple
board.tc_type_write(channel, hats.tcTypes.TYPE_T)

while True:
    # read the temperature
    temperature = board.t_in_read(channel)
    print(temperature)
    
    # compare against the thresholds
    if above_threshold:
        if temperature < clear_threshold:
            # we dropped below the falling threshold, send a trigger
            above_threshold = False
            send_trigger(event_name, "{:.2f}".format(temperature), "{:.2f}".format(clear_threshold), "cleared")
    else:
        if temperature > temperature_threshold:
            # we hit the rising threshold, send a trigger
            above_threshold = True
            send_trigger(event_name, "{:.2f}".format(temperature), "{:.2f}".format(temperature_threshold), "set")
    
    time.sleep(1)
