# Python IFTTT Example

## About
IFTTT (If This Than That) is a free web-based service that interacts with apps and hardware to 
automate various functions.

The Python IFTTT example reads DAQ HAT channels 0 and 1 every 5 minutes, and logs the time and data 
to a Google Sheets spreadsheet. Users can remotely monitor the spreadsheet from Google Drive. 

## Prerequisites

- IFTTT account
- Google Drive account

## Create the IFTTT applet
1. At IFTTT.com go to My Applets, then click **New Applet**.
2. Click "this" then search for Webhooks. Click Webhooks when found.
3. Click "Receive a web request" on the Choose trigger screen.
4. Enter your event name in the "Event Name" field (**voltage_data** for this example), then click "Create trigger".
5. Click "that" then search for Google Sheets.  Click Google Sheets when found.
6. Click "Add row to spreadsheet" on the Choose action screen.
7. Enter your desired spreadsheet name, then change the formatted row to only contain "{{OccurredAt}} ||| {{Value1}} |||{{Value2}}".
   Change the Drive folder path, if desired, then click "Create action".
8. Review the details, then click "Finish".

Enter your key from IFTTT Webhooks documentation for the variable "key".  You can find this key by:
1. At IFTTT.com go to My Applets, then click the "Services" heading.
2. Enter Webhooks in the Filter services field, then click on "Webhooks".
3. Click on "Documentation" and copy the key listed.  
4. Open the "ifttt_log.py" file and look for the "IFTTT values" section. 
5. In the key = "<my_key>" line, replace "<my_key>" with the value copied in step 3, taking care to retain the quotes.

## Start the IFTTT web service
1. Open ifttt_log.py from an IDE, or execute in a terminal window:  

   ```
   $ CD ~/daqhats/examples/python/mcc118/ifttt
   $ ./ifttt_log.py
   ```   
2. Launch Google Drive on the web; log into your account if you are not already signed in.
3. Open the Google Sheets file named **voltage_data** in the path specified when you created the applet. 
The spreadsheet dynamically updates with data as it is acquired.

Stop the web server example
- Press Ctrl+C in the terminal window where the server was started.

## Support/Feedback
Contact technical support through our [support page](https://www.mccdaq.com/support/support_form.aspx). 

## More Information
- IFTTT: https://ifttt.com/discover