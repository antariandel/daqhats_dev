# Python Web Server Example

## About
The Python Web Server example demonstrates a simple web server that displasy data acquired from a 
MCC 118 DAQ HAT device. This example is intended to run on a single client.  

## Dependencies

- Dash: Python framework for building Web-based applications
- Plotly: Python graphing library (An interactive, browser-based graphing library for Python)

## Install the Dependencies
Install pip first if it is not already installed, then install Dash and Plotly.

   ```
   & sudo apt-get install python-pip
   $ sudo pip install dash dash-renderer dash-html-components dash-core-components
   $ pip install plotly
   ```
   
## Start the web server
1. Open a terminal window and enter the following commands: 

   ```
   $ CD ~/daqhats/examples/python/mcc118/web_server
   $ ./web_server.py
   ```   
2. Open a web browser on a device on the same network as the host device and
   enter http://<host>:8080 in the address bar, replacing <host> with either the 
   IP Address or the hostname of the host device.

Stop the web server example
- Press Ctrl+C in the terminal window where the server was started.

## Support/Feedback
Contact technical support through our [support page](https://www.mccdaq.com/support/support_form.aspx). 

## More Information
- Dash: https://dash.plot.ly/ 
- Plotly: https://plot.ly/python/
