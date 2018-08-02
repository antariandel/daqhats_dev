# DataLogger Example

## About
The DataLogger example shows how to acquire data from the MCC 118 HAT, display the data on a strip chart, 
and log the data to a CSV file. 
This example can be run from a terminal window, or accessed with an IDE such as Geany or CodeBlocks. 


## Dependencies
- GTK+ cross-platform toolkit for creating graphical user interfaces.
- GtkDatabox widget used to display two-dimensional data.
- Monitor connected to the Raspberry Pi to configure acquisition options and view acquired data 
- IDE, if using. Source code is provided for Geany and CodeBlocks IDEs
 
 > Source code is supplied for Geany and CodeBlocks. 

## Install the Dependencies
Install GTK+ and GTKDatabox:
  ```
    git clone https://github.com/erikd/gtkdatabox.git
    cd gtkdatabox
    ./autogen.sh
    ./configure
    sudo make install
  ```
Install Geany or CodeBlocks (optional):
  ```
    sudo apt-get install geany
  ```

  ```
	sudo apt-get install codeblocks
  ```

## Running the example
To run the example from a terminal window, enter the following commands:
  ```
    cd ~daqhats/examples/c/mcc118/data_logger/logger
    make
    ./logger
  ```
When using the Geany or CodeBlocks IDE, the example is compiled when you load the 
project file located in examples/c/mcc118/data_logger (logger.geany or logger.cbp).

## Support/Feedback
Contact technical support through our [support page](https://www.mccdaq.com/support/support_form.aspx). 

## More Information
- GTK+: https://www.gtk.org/ 
- GTKDataBox: https://sourceforge.net/projects/gtkdatabox/
- CodeBlocks: http://www.codeblocks.org/
- Geany: https://www.geany.org/