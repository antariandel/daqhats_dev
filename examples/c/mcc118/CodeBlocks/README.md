# CodeBlocks DataLogger Example

## About
CodeBlocks is an open-source, cross-platform IDE used to create, build, and run C and C++ projects. 

The CodeBlocks DataLogger example shows how to acquire data from the DAQ HAT board, display the data on a strip chart, 
and log the data to a CSV file. 

## Dependencies
- CodeBlocks open-source, cross-platform IDE used to create, build, and run C and C++ projects. 
- GTK+ cross-platform toolkit for creating graphical user interfaces.
- GtkDatabox widget used to display two-dimensional data.
- Monitor connected to the Raspberry Pi to configure acquisitin options and view acquired data 

## Install the Dependencies

### CodeBlocks

	```
	sudo apt-get install codeblocks
	```

### GTK+

	```
	sudo apt-get install libgtk-3-dev
	```
	
### GTKDatabox (required to run the DataLogger example)

	```
	git config --global http.postbuffer 524288080 (file size of something? nothin happens here - is that expected?)
	git clone git://git.code.sf.net/p/gtkdatabox/git gtkdatabox-git (gtkdatabox-git sets the installation folder)
	cd gtkdatabox-git
	scripts/myAutoconf.sh./configure
	sudo make
	sudo make install	
	```

## Run the CodeBlocks DataLogger example
1. Select the raspberry in the top left corner.
2. Select Programming > Code::Blocks.
3. Select File  > Open and select the *.cbp file.

### Uninstall the CodeBlocks IDE
If ou want to uninstall the CodeBlocks IDE and GTK+ toolkit, use the following commands:
```
sudo apt-get --purge remove codeblocks
sudo apt-get --purge remove libgtk-3-dev
sudo apt autoremove
```

## Support/Feedback
Contact technical support through our [support page](https://www.mccdaq.com/support/support_form.aspx). 

## More Information
- CodeBlocks: http://www.codeblocks.org/
- GTK+: https://www.gtk.org/ 
- GTKDataBox: https://sourceforge.net/projects/gtkdatabox/