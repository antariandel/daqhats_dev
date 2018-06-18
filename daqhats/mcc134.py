"""
Wraps all of the methods from the MCC 134 library for use in Python.
"""
import os.path
from ctypes import *
from daqhats.hats import Hat, HatError
from array import array
import sys

# Load the library
libname = 'libdaqhats.so.0'
try:
    _libc = cdll.LoadLibrary(libname)
except:
    _libc = 0

class tcTypes:
    """Thermocouple types."""
    TYPE_J  = 0     #: Type J
    TYPE_K  = 1     #: Type K
    TYPE_T  = 2     #: Type T
    TYPE_E  = 3     #: Type E
    TYPE_R  = 4     #: Type R
    TYPE_S  = 5     #: Type S
    TYPE_B  = 6     #: Type B
    TYPE_N  = 7     #: Type N

class mcc134(Hat):
    """
    The class for an MCC 134 board.
    
    Args:
        address (int): board address, must be 0-7.
        
    Raises:
        HatError: the board did not respond or was of an incorrect type
    """

    OPEN_TC_VALUE           = -9999.0   #: Return value for an open thermocouple.
    OVERRANGE_TC_VALUE      = -8888.0   #: Return value for thermocouple voltage outside the valid range.
    
    _AIN_NUM_CHANNELS       = 4         # Number of analog channels

    _OPTS_NOSCALEDATA       = 0x0001
    _OPTS_NOCALIBRATEDATA   = 0x0002

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
        _libc.mcc134_open.argtypes  = [c_ubyte]
        _libc.mcc134_open.restype   = c_int
        
        _libc.mcc134_close.argtypes  = [c_ubyte]
        _libc.mcc134_close.restype   = c_int
        
        _libc.mcc134_serial.argtypes    = [c_ubyte, c_char_p]
        _libc.mcc134_serial.restype     = c_int
        
        _libc.mcc134_calibration_date.argtypes    = [c_ubyte, c_char_p]
        _libc.mcc134_calibration_date.restype     = c_int
        
        _libc.mcc134_calibration_coefficient_read.argtypes  = [c_ubyte, c_ubyte, POINTER(c_double), POINTER(c_double)]
        _libc.mcc134_calibration_coefficient_read.restype   = c_int
        
        _libc.mcc134_calibration_coefficient_write.argtypes  = [c_ubyte, c_ubyte, c_double, c_double]
        _libc.mcc134_calibration_coefficient_write.restype   = c_int
        
        _libc.mcc134_a_in_read.argtypes = [c_ubyte, c_ubyte, c_ubyte, POINTER(c_double)]
        _libc.mcc134_a_in_read.restype  = c_int
        
        _libc.mcc134_self_offset_correction.argtypes    = [c_ubyte]
        _libc.mcc134_self_offset_correction.restype     = c_int
        
        _libc.mcc134_tc_type_write.argtypes = [c_ubyte, c_ubyte, c_ubyte]
        _libc.mcc134_tc_type_write.restype  = c_int
        
        _libc.mcc134_tc_type_read.argtypes  = [c_ubyte, c_ubyte, POINTER(c_ubyte)]
        _libc.mcc134_tc_type_read.restype   = c_int
        
        _libc.mcc134_t_in_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_double)]
        _libc.mcc134_t_in_read.restype  = c_int
        
        _libc.mcc134_cjc_read.argtypes = [c_ubyte, c_ubyte, POINTER(c_double)]
        _libc.mcc134_cjc_read.restype  = c_int
        
        if _libc.mcc134_open(self._address) != self._RESULT_SUCCESS:
            self._initialized = False
            raise HatError(self._address, "Board not responding.")
            
        return
            
    def __del__(self):
        if self._initialized:
            _libc.mcc134_close(self._address)
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
        if _libc.mcc134_serial(self._address, buffer) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        if sys.version_info > (3, 0):
            my_serial = buffer.value.decode()
        else:
            my_serial = buffer.value
        return my_serial

    def calibration_date(self):
        """
        Read the calibration date.

        Returns:
            string: The calibration date in the format "YYYY-MM-DD".
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        # create string to hold the result
        buffer = create_string_buffer(11)
        if _libc.mcc134_calibration_date(self._address, buffer) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        if sys.version_info > (3, 0):
            my_date = buffer.value.decode()
        else:
            my_date = buffer.value
        return my_date

    def calibration_coefficient_read(self, channel):
        """
        Read the calibration coefficients for a single channel.

        The coefficients are applied in the library as: ::
        
            calibrated_ADC_code = (raw_ADC_code * slope) + offset
        
        Returns:
            dictionary: coefficients containing the following keys
            
            | **slope** (float): The slope.
            | **offset** (float): The offset.
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        slope = c_double()
        offset = c_double()
        if _libc.mcc134_calibration_coefficient_read(self._address, channel, byref(slope), byref(offset)) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return {'slope': slope.value, 'offset': offset.value}
    
    def calibration_coefficient_write(self, channel, slope, offset):
        """
        Temporarily write the calibration coefficients for a single channel.

        The user can apply their own calibration coefficients by writing to these values. 
        The values will reset to the factory values from the EEPROM whenever the class is 
        initialized.
        
        The coefficients are applied in the library as: ::
        
            calibrated_ADC_code = (raw_ADC_code * slope) + offset
        
        Args:
            slope (float): The new slope value.
            offset (float): The new offset value.
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        if _libc.mcc134_calibration_coefficient_write(self._address, channel, slope, offset) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return
        
    @staticmethod
    def a_in_num_channels():
        """
        Return the number of analog input channels.
        
        Returns:
            int: the number of channels.
        """
        return mcc134._AIN_NUM_CHANNELS
        
    def self_offset_correction(self):
        """
        Clear any internal ADC offset error.

        Performs an ADC self offset correction. This is automatically called when the board is opened.
        For absolute accuracy this should be performed whenever the board temperature changes by more 
        than 5 degrees C from the last time the offset error was cleared.  The board temperature can be 
        monitored with the :py:func:`cjc_read` method. The calibration takes approximately 800ms to complete.
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        if _libc.mcc134_self_offset_correction(self._address) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return
        
    def a_in_read(self, channel, scaled=True, calibrated=True):
        """
        Read an analog input channel and return the value.
        
        Args:
            channel (int): The analog input channel number, 0-3.
            scaled (bool): True to return voltage, False to return ADC code.
            calibrated (bool): True to apply calibration to the value, False to return uncalibrated value.
            
        Returns:
            float: the read value
            
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
            ValueError: the channel number is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        if channel not in range(self._AIN_NUM_CHANNELS):
            raise ValueError("Invalid channel {0}. Must be 0-{1}.".format(channel, self._AIN_NUM_CHANNELS-1))
            
        data_value = c_double()
        
        options = 0
        if not scaled:
            options |= self._OPTS_NOSCALEDATA
        if not calibrated:
            options |= self._OPTS_NOCALIBRATEDATA
            
        if _libc.mcc134_a_in_read(self._address, channel, options, byref(data_value)) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return data_value.value
        
    def tc_type_read(self, channel):
        """
        Read the thermocouple type for a channel.

        Reads the current thermocouple type for the specified channel. The type is one of 
        :py:class:`tcTypes` and the board will default to all channels set to 
        :py:const:`tcTypes.TYPE_J` when it is first opened.
        
        Args:
            channel (int): The analog input channel number, 0-3.
            
        Returns
            int: The thermocouple type.
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        type_value = c_int()
        if _libc.mcc134_tc_type_read(self._address, channel, byref(type_value)) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return type_value.value

    def tc_type_write(self, channel, type):
        """
        Write the thermocouple type for a channel.

        Tells the MCC 134 library what thermocouple type is connected to the specified channel. This is needed 
        for correct temperature calculations. The type is one of :py:class:`tcTypes` and the board will 
        default to all channels set to :py:const:`tcTypes.TYPE_J` when it is first opened.
        
        Args:
            channel (int): The analog input channel number, 0-3.
            type (:py:class:`tcTypes`): The thermocouple type.
        
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        if _libc.mcc134_tc_type_write(self._address, channel, type) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return

    def t_in_read(self, channel):
        """
        Read a thermocouple input channel.
        
        Returns the value as degrees Celsius. The temperature value can have two special values:
            - :py:const:`mcc134.OPEN_TC_VALUE` if an open thermocouple is detected
            - :py:const:`mcc134.OVERRANGE_TC_VALUE` if a value outside valid thermocouple voltage is detected
        
        Args:
            channel (int): The analog input channel number, 0-3.
            
        Returns:
            float: The thermocouple temperature.
            
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
            ValueError: the channel number is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        if channel not in range(self._AIN_NUM_CHANNELS):
            raise ValueError("Invalid channel {0}. Must be 0-{1}.".format(channel, self._AIN_NUM_CHANNELS-1))
            
        temp = c_double()
        
        if _libc.mcc134_t_in_read(self._address, channel, byref(temp)) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return temp.value
        
    def cjc_read(self, channel):
        """
        Read a cold junction compensation temperature.
        
        Returns the temperature of the specified terminal in degrees Celsius.
        
        Args:
            channel (int): The analog input channel number, 0-3.
            
        Returns:
            float: The CJC temperature.
            
        Raises:
            HatError: the board is not initialized, does not respond, or responds incorrectly.
            ValueError: the channel number is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
            
        if channel not in range(self._AIN_NUM_CHANNELS):
            raise ValueError("Invalid channel {0}. Must be 0-{1}.".format(channel, self._AIN_NUM_CHANNELS-1))
            
        temp = c_double()
        
        if _libc.mcc134_cjc_read(self._address, channel, byref(temp)) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return temp.value
