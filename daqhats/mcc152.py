"""
Wraps all of the methods from the MCC 152 library for use in Python.
"""
import os.path
from ctypes import *
from daqhats.hats import Hat, HatError
from array import array


# Load the library
libname = 'libdaqhats.so.0'
try:
    _libc = cdll.LoadLibrary(libname)
except:
    _libc = 0

class mcc152(Hat):
    """
    The class for an MCC 152 board.
    
    Args:
        address (int): board address, must be 0-7.
        
    Raises:
        HatError: the board did not respond or was of an incorrect type
    """

    _AOUT_NUM_CHANNELS      = 2         # Number of analog output channels
    _DIO_NUM_CHANNELS       = 8         # Number of digital I/O channels

    _OPTS_NOSCALEDATA       = 0x0001

    def __init__(self, address = 0):
        """
        Initialize the class.  
        """
        # call base class initializer
        Hat.__init__(self, address)

        if _libc == 0:
            self._initialized = false
            raise Exception("daqhats shared library is not installed.")
        
        # set up library argtypes and restypes
        _libc.mcc152_open.argtypes  = [c_ubyte]
        _libc.mcc152_open.restype   = c_int
        
        _libc.mcc152_close.argtypes  = [c_ubyte]
        _libc.mcc152_close.restype   = c_int
        
        _libc.mcc152_serial.argtypes    = [c_ubyte, c_char_p]
        _libc.mcc152_serial.restype     = c_int
        
        _libc.mcc152_a_out_write.argtypes = [c_ubyte, c_ubyte, c_ulong, c_double]
        _libc.mcc152_a_out_write.restype  = c_int
        
        _libc.mcc152_a_out_write_all.argtypes = [c_ubyte, c_ulong, POINTER(c_double)]
        _libc.mcc152_a_out_write_all.restype  = c_int
        
        _libc.mcc152_dio_reset.argtypes = [c_ubyte]
        _libc.mcc152_dio_reset.restype  = c_int
        
        _libc.mcc152_dio_input_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_input_read.restype  = c_int

        _libc.mcc152_dio_output_write.argtypes = [c_ubyte, c_ubyte, c_ubyte]
        _libc.mcc152_dio_output_write.restype  = c_int
        
        _libc.mcc152_dio_output_read.argtypes = [c_ubyte,c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_output_read.restype  = c_int

        _libc.mcc152_dio_direction_write.argtypes = [c_ubyte, c_ubyte, c_ubyte]
        _libc.mcc152_dio_direction_write.restype  = c_int
        
        _libc.mcc152_dio_direction_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_direction_read.restype  = c_int

        _libc.mcc152_dio_pull_config_write.argtypes = [c_ubyte, c_ubyte, c_ubyte]
        _libc.mcc152_dio_pull_config_write.restype  = c_int
        
        _libc.mcc152_dio_pull_config_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_pull_config_read.restype  = c_int

        _libc.mcc152_dio_pull_enable_write.argtypes = [c_ubyte, c_ubyte, c_ubyte]
        _libc.mcc152_dio_pull_enable_write.restype  = c_int
        
        _libc.mcc152_dio_pull_enable_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_pull_enable_read.restype  = c_int

        _libc.mcc152_dio_input_invert_write.argtypes = [c_ubyte, c_ubyte, c_ubyte]
        _libc.mcc152_dio_input_invert_write.restype  = c_int
        
        _libc.mcc152_dio_input_invert_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_input_invert_read.restype  = c_int

        _libc.mcc152_dio_input_latch_write.argtypes = [c_ubyte, c_ubyte, c_ubyte]
        _libc.mcc152_dio_input_latch_write.restype  = c_int
        
        _libc.mcc152_dio_input_latch_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_input_latch_read.restype  = c_int

        _libc.mcc152_dio_output_type_write.argtypes = [c_ubyte, c_ubyte]
        _libc.mcc152_dio_output_type_write.restype  = c_int
        
        _libc.mcc152_dio_output_type_read.argtypes = [c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_output_type_read.restype  = c_int

        _libc.mcc152_dio_interrupt_mask_write.argtypes = [c_ubyte, c_ubyte, c_ubyte]
        _libc.mcc152_dio_interrupt_mask_write.restype  = c_int
        
        _libc.mcc152_dio_interrupt_mask_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_interrupt_mask_read.restype  = c_int

        _libc.mcc152_dio_interrupt_status_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_ubyte)]
        _libc.mcc152_dio_interrupt_status_read.restype  = c_int

        if _libc.mcc152_open(self._address) != self._RESULT_SUCCESS:
            self._initialized = False
            raise HatError(self._address, "Board not responding.")
            
        #self._scanning = False
        return
            
    def __del__(self):
        if self._initialized:
            _libc.mcc152_close(self._address)
        return
    
    def serial(self):
        """
        Read the serial number.

        Returns:
            string: The serial number.
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        # create string to hold the result
        buffer = create_string_buffer(9)
        if _libc.mcc152_serial(self._address, buffer) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return buffer.value

    @staticmethod
    def a_out_num_channels():
        """
        Return the number of analog output channels.
        
        Returns:
            int: the number of channels.
        """
        return mcc152._AOUT_NUM_CHANNELS
        
    @staticmethod
    def dio_num_channels():
        """
        Return the number of digital I/O channels.
        
        Returns:
            int: the number of channels.
        """
        return mcc152._DIO_NUM_CHANNELS
        
    def a_out_write(self, channel, value, scaled=True):
        """
        Write a single analog output channel value.
        
        Updates the specified analog output channel in either volts or DAC code (determined by the scaled
        argument.)  The value must be 0.0 - 5.0 if scaled is True or 0.0 - 4095.0 if False.
        
        Args:
            channel (int): The analog input channel number, 0-1.
            value (float): The value to write.
            scaled (bool): True to write voltage, False to write DAC code.
            
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
            ValueError: the channel number or value is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        if channel not in range(self._AOUT_NUM_CHANNELS):
            raise ValueError("Invalid channel {0}. Must be 0-{1}.".format(channel, self._AOUT_NUM_CHANNELS-1))
            
        options = 0
        if not scaled:
            options |= self._OPTS_NOSCALEDATA
            
        result = _libc.mcc152_a_out_write(self._address, channel, options, value)
        
        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid value {}.".format(value))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return
        
    def a_out_write_all(self, values, scaled=True):
        """
        Write all analog output channel values simultaneously.
        
        Updates all analog output channels in either volts or DAC code (determined by the scaled
        argument.)  The values must be 0.0 - 5.0 if scaled is True or 0.0 - 4095.0 if False.  Only
        the first 2 values from the list will be used.
        
        Args:
            values (list of float): The values to write, in channel order.
            scaled (bool): True to write voltage, False to write DAC code.
            
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
            ValueError: the channel number or a data value is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        if len(values) < self._AOUT_NUM_CHANNELS:
            raise ValueError("Not enough elements in values. Must be at least {}".format(self._AOUT_NUM_CHANNELS))
            
        options = 0
        if not scaled:
            options |= self._OPTS_NOSCALEDATA
            
        data_array = (c_double * len(values))(*values)
        result = _libc.mcc152_a_out_write_all(self._address, options, data_array)
        
        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid value in {}.".format(values))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return
        
    def dio_reset(self):
        """
        Reset the DIO to the default configuration.
        
        Resets the DIO interface to the power on defaults:
        - All channels input
        - Output registers set to 1
        - Input inversion disabled
        - No input latching
        - Pull-up resistors enabled
        - All interrupts disabled
        - Push-pull output type        
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        if _libc.mcc152_dio_reset(self._address) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_input_read(self, channel):
        """
        Read a single DIO input.
        
        Read a single digital channel input value or all inputs at once.  Returns 0 or 1.

        If the specified channel is configured as an output this will return the value present at the 
        terminal.

        This method reads the entire input register even if a single channel is specified, so care must be taken when 
        latched inputs are enabled.  If a latched input changes between input reads then changes back to its original
        value, the next input read will report the change to the first value then the following read will show the 
        original value.  If another input is read then this input change could be missed so it is best to use dio_input_read_all()
        when using latched inputs.

        Args:
            channel (int): The dio channel number, 0-7.
        
        Returns:
            int: The input value.
            
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
           
        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))
            
        value = c_ubyte()
        result = _libc.mcc152_dio_input_read(self._address, channel, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_input_read_all(self):
        """
        Read all DIO inputs.
        
        Read all inputs at once.  Returns a list of all input values in channel order.

        If a channel is configured as an output this will return the value present at the 
        terminal.

        This method reads the entire input register even if a single channel is specified, so care must be taken when 
        latched inputs are enabled.  If a latched input changes between input reads then changes back to its original
        value, the next input read will report the change to the first value then the following read will show the 
        original value.  

        Returns
            list of int: The input values.
            
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        value = c_ubyte()
        result = _libc.mcc152_dio_input_read(self._address, self._DIO_CHANNEL_ALL, byref(value))
        
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS
        
        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist
        
    def dio_output_write(self, channel, value):
        """
        Write a single DIO output.
        
        Write a single digital channel output value.  If the specified channel is configured as an input
        this will not have any effect at the terminal, but allows the output register to be loaded before 
        configuring the channel as an output.
        
        Args:
            channel (int): The dio channel number, 0-7 or DIO_CHANNEL_ALL.
            value (int): The output value, 0 or 1.
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
            ValueError: the channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        if value not in (0, 1):
            raise ValueError("Invalid value {}, expecting 0 or 1.".format(value))
            
        result = _libc.mcc152_dio_output_write(self._address, channel, value)
        
        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid channel {}.".format(channel))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_output_write_multiple(self, value_dict):
        """
        Write multiple DIO outputs.
        
        Write multiple digital channel output values by passing a dictionary with channel / value pairs.  
        If a specified channel is configured as an input this will not have any effect at the terminal, 
        but allows the output register to be loaded before configuring the channel as an output.
        
        For example, to set channels 0 and 2 to 1 and channel 3 to 0 call:
        
        dio_output_write_multiple({0 : 1, 2 : 1, 3 : 0})
        
        Args:
            value_dict (dictionary): A dictionary containing channel : value pairs.
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
            ValueError: a channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
          
        channels = 0
        orig_val = c_ubyte()
        result = _libc.mcc152_dio_output_read(self._address, self._DIO_CHANNEL_ALL, byref(orig_val))
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        values = orig_val.value()
        
        for channel, value in value_dict.items():
            if channel not in range(self._DIO_NUM_CHANNELS):
                raise ValueError("Invalid channel {}.".format(channel))
            if value not in (0, 1):
                raise ValueError("Invalid value {}, expecting 0 or 1.".format(value))
            mask = 1 << channel
            channels = channels | mask
            if value == 0:
                values = values & ~mask
            else:
                values = values | mask
                
        if channels == 0:
            raise ValueError("No channels specified.")
            
        result = _libc.mcc152_dio_output_write(self._address, self._DIO_CHANNEL_ALL, value)
        
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_output_read(self, channel):
        """
        Read a single DIO output.
        
        Read a single digital channel output value.  Returns 0 or 1.

        This function returns the value stored in the output register.  It may not represent 
        the value at the terminal if the channel is configured as input or open-drain output.

        Args:
            channel (int): The DIO channel number, 0-7.
        
        Returns:
            int: The output value.
            
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))
            
        value = c_ubyte()
        result = _libc.mcc152_dio_input_read(self._address, channel, byref(value))
        
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_output_read_all(self):
        """
        Read all DIO outputs.
        
        Read all digital channel output values.  Returns a list of all output values in channel order.

        This function returns the values stored in the output register.  They may not represent 
        the value at the terminal if the channel is configured as input or open-drain output.

        Returns
            list of int: The output values.
            
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        value = c_ubyte()
        result = _libc.mcc152_dio_output_read(self._address, self._DIO_CHANNEL_ALL, byref(value))
        
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS
        
        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist
