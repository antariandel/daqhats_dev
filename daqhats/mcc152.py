# pylint: disable=too-many-lines
"""
Wraps all of the methods from the MCC 152 library for use in Python.
"""
from ctypes import c_ubyte, c_int, c_char_p, c_ulong, c_double, POINTER, \
    create_string_buffer, byref
from daqhats.hats import Hat, HatError, OptionFlags

class mcc152(Hat): # pylint: disable=invalid-name,too-many-public-methods
    """
    The class for an MCC 152 board.

    Args:
        address (int): board address, must be 0-7.

    Raises:
        HatError: the board did not respond or was of an incorrect type
    """

    _AOUT_NUM_CHANNELS = 2         # Number of analog output channels
    _DIO_NUM_CHANNELS = 8          # Number of digital I/O channels

    _DIO_CHANNEL_ALL = 0xFF

    def __init__(self, address=0): # pylint: disable=too-many-statements,similarities
        """
        Initialize the class.
        """
        # call base class initializer
        Hat.__init__(self, address)

        # set up library argtypes and restypes
        self._lib.mcc152_open.argtypes = [c_ubyte]
        self._lib.mcc152_open.restype = c_int

        self._lib.mcc152_close.argtypes = [c_ubyte]
        self._lib.mcc152_close.restype = c_int

        self._lib.mcc152_serial.argtypes = [c_ubyte, c_char_p]
        self._lib.mcc152_serial.restype = c_int

        self._lib.mcc152_a_out_write.argtypes = [
            c_ubyte, c_ubyte, c_ulong, c_double]
        self._lib.mcc152_a_out_write.restype = c_int

        self._lib.mcc152_a_out_write_all.argtypes = [
            c_ubyte, c_ulong, POINTER(c_double)]
        self._lib.mcc152_a_out_write_all.restype = c_int

        self._lib.mcc152_dio_reset.argtypes = [c_ubyte]
        self._lib.mcc152_dio_reset.restype = c_int

        self._lib.mcc152_dio_input_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_input_read.restype = c_int

        self._lib.mcc152_dio_output_write.argtypes = [
            c_ubyte, c_ubyte, c_ubyte]
        self._lib.mcc152_dio_output_write.restype = c_int

        self._lib.mcc152_dio_output_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_output_read.restype = c_int

        self._lib.mcc152_dio_direction_write.argtypes = [
            c_ubyte, c_ubyte, c_ubyte]
        self._lib.mcc152_dio_direction_write.restype = c_int

        self._lib.mcc152_dio_direction_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_direction_read.restype = c_int

        self._lib.mcc152_dio_pull_config_write.argtypes = [
            c_ubyte, c_ubyte, c_ubyte]
        self._lib.mcc152_dio_pull_config_write.restype = c_int

        self._lib.mcc152_dio_pull_config_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_pull_config_read.restype = c_int

        self._lib.mcc152_dio_pull_enable_write.argtypes = [
            c_ubyte, c_ubyte, c_ubyte]
        self._lib.mcc152_dio_pull_enable_write.restype = c_int

        self._lib.mcc152_dio_pull_enable_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_pull_enable_read.restype = c_int

        self._lib.mcc152_dio_input_invert_write.argtypes = [
            c_ubyte, c_ubyte, c_ubyte]
        self._lib.mcc152_dio_input_invert_write.restype = c_int

        self._lib.mcc152_dio_input_invert_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_input_invert_read.restype = c_int

        self._lib.mcc152_dio_input_latch_write.argtypes = [
            c_ubyte, c_ubyte, c_ubyte]
        self._lib.mcc152_dio_input_latch_write.restype = c_int

        self._lib.mcc152_dio_input_latch_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_input_latch_read.restype = c_int

        self._lib.mcc152_dio_output_type_write.argtypes = [c_ubyte, c_ubyte]
        self._lib.mcc152_dio_output_type_write.restype = c_int

        self._lib.mcc152_dio_output_type_read.argtypes = [
            c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_output_type_read.restype = c_int

        self._lib.mcc152_dio_interrupt_mask_write.argtypes = [
            c_ubyte, c_ubyte, c_ubyte]
        self._lib.mcc152_dio_interrupt_mask_write.restype = c_int

        self._lib.mcc152_dio_interrupt_mask_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_interrupt_mask_read.restype = c_int

        self._lib.mcc152_dio_interrupt_status_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc152_dio_interrupt_status_read.restype = c_int

        if self._lib.mcc152_open(self._address) != self._RESULT_SUCCESS:
            self._initialized = False
            raise HatError(self._address, "Board not responding.")

        return

    def __del__(self):
        if self._initialized:
            self._lib.mcc152_close(self._address)
        return

    def serial(self):
        """
        Read the serial number.

        Returns:
            string: The serial number.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        # create string to hold the result
        my_buffer = create_string_buffer(9)
        if (self._lib.mcc152_serial(self._address, my_buffer)
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")
        my_serial = my_buffer.value.decode('ascii')
        return my_serial

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

    def a_out_write(self, channel, value, options=OptionFlags.DEFAULT):
        """
        Write a single analog output channel value.

        **options** is an OptionFlags value. Valid flags for this
        method are:

        * :py:const:`OptionFlags.DEFAULT`: Write a voltage value (0 - 5).
        * :py:const:`OptionFlags.NOSCALEDATA`: Write a DAC code (a value
          between 0 and 4095) rather than voltage.

        Args:
            channel (int): The analog output channel number, 0-1.
            value (float): The value to write.
            options (int): An :py:class:`OptionFlags` value,
                :py:const:`OptionFlags.DEFAULT` if unspecified.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel number or value is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._AOUT_NUM_CHANNELS):
            raise ValueError("Invalid channel {0}. Must be 0-{1}.".
                             format(channel, self._AOUT_NUM_CHANNELS-1))

        result = self._lib.mcc152_a_out_write(
            self._address, channel, options, value)

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid value {}.".format(value))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return

    def a_out_write_all(self, values, options=OptionFlags.DEFAULT):
        """
        Write all analog output channels simultaneously.

        **options** is an OptionFlags value. Valid flags for this
        method are:

        * :py:const:`OptionFlags.DEFAULT`: Write voltage values (0 - 5).
        * :py:const:`OptionFlags.NOSCALEDATA`: Write DAC codes (values
          between 0 and 4095) rather than voltage.

        Args:
            values (list of float): The values to write, in channel order. There
                must be at least two values, but only the first two will be
                used.
            options (int): An :py:class:`OptionFlags` value,
                :py:const:`OptionFlags.DEFAULT` if unspecified.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel number or a data value is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if len(values) < self._AOUT_NUM_CHANNELS:
            raise ValueError(
                "Not enough elements in values. Must be at least {}".
                format(self._AOUT_NUM_CHANNELS))

        data_array = (c_double * len(values))(*values)
        result = self._lib.mcc152_a_out_write_all(
            self._address, options, data_array)

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
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if self._lib.mcc152_dio_reset(self._address) != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_input_read(self):
        """
        Read all DIO inputs at once.

        Returns a list of all input values in channel order.

        If a channel is configured as an output this will return the value
        present at the terminal.

        Care must be taken when latched inputs are enabled. If a latched input
        changes between input reads then changes back to its original value, the
        next input read will report the change to the first value then the
        following read will show the original value.

        Returns
            list of int: The input values.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_input_read(
            self._address, self._DIO_CHANNEL_ALL, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS

        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist

#    def dio_input_read(self, channel):
#        """
#        Read a single DIO input channel value.
#
#        Returns 0 if the input is low, 1 if it is high.
#
#        If the specified channel is configured as an output this will return the
#        value present at the terminal.
#
#        This method reads the entire input register even though a single channel
#        is specified, so care must be taken when latched inputs are enabled. If
#        a latched input changes between input reads then changes back to its
#        original value, the next input read will report the change to the first
#        value then the following read will show the original value. If another
#        input is read then this input change could be missed so it is best to
#        use dio_input_read_all() when using latched inputs.
#
#        Args:
#            channel (int): The DIO channel number, 0-7.
#
#        Returns:
#            int: The input value.
#
#        Raises:
#            HatError: the board is not initialized, does not respond, or
#                responds incorrectly.
#            ValueError: the channel argument is invalid.
#        """
#        if not self._initialized:
#            raise HatError(self._address, "Not initialized.")
#
#        if channel not in range(self._DIO_NUM_CHANNELS):
#            raise ValueError("Invalid channel {}.".format(channel))
#
#        value = c_ubyte()
#        result = self._lib.mcc152_dio_input_read(
#            self._address, channel, byref(value))
#
#        if result != self._RESULT_SUCCESS:
#            raise HatError(self._address, "Incorrect response.")
#
#        return value.value()
#
#    def dio_input_read_all(self):
#        """
#        Read all DIO inputs at once.
#
#        Returns a list of all input values in channel order.
#
#        If a channel is configured as an output this will return the value
#        present at the terminal.
#
#        Care must be taken when latched inputs are enabled. If a latched input
#        changes between input reads then changes back to its original value, the
#        next input read will report the change to the first value then the
#        following read will show the original value.
#
#        Returns
#            list of int: The input values.
#
#        Raises:
#            HatError: the board is not initialized, does not respond, or
#                responds incorrectly.
#        """
#        if not self._initialized:
#            raise HatError(self._address, "Not initialized.")
#
#        value = c_ubyte()
#        result = self._lib.mcc152_dio_input_read(
#            self._address, self._DIO_CHANNEL_ALL, byref(value))
#
#        if result != self._RESULT_SUCCESS:
#            raise HatError(self._address, "Incorrect response.")
#
#        # convert byte to list of bits
#        mylist = [0]*self._DIO_NUM_CHANNELS
#
#        reg = value.value()
#        for i in range(self._DIO_NUM_CHANNELS):
#            mylist[i] = (reg & (1<<i)) >> i
#
#        return mylist

    def dio_output_write(self, channel, value):
        """
        Write a single DIO output value.

        If the specified channel is configured as an input this will not have
        any effect at the terminal, but allows the output register to be loaded
        before configuring the channel as an output.

        Args:
            channel (int): The dio channel number, 0-7.
            value (int): The output value, 0 or 1.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if value not in (0, 1):
            raise ValueError(
                "Invalid value {}, expecting 0 or 1.".format(value))

        result = self._lib.mcc152_dio_output_write(
            self._address, channel, value)

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid channel {}.".format(channel))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_output_write_multiple(self, value_dict):
        """
        Write multiple DIO output channel values.

        Write multiple values by passing a dictionary with channel / value
        pairs. If a specified channel is configured as an input this will not
        have any effect at the terminal, but allows the output register to be
        loaded before configuring the channel as an output.

        For example, to set channels 0 and 2 to 1 and channel 3 to 0 call:

        dio_output_write_multiple({0 : 1, 2 : 1, 3 : 0})

        Args:
            value_dict (dictionary): A dictionary containing channel : value
                pairs.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: a channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        channels = 0
        orig_val = c_ubyte()
        result = self._lib.mcc152_dio_output_read(
            self._address, self._DIO_CHANNEL_ALL, byref(orig_val))
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        values = orig_val.value()

        for channel, value in value_dict.items():
            if channel not in range(self._DIO_NUM_CHANNELS):
                raise ValueError("Invalid channel {}.".format(channel))
            if value not in (0, 1):
                raise ValueError(
                    "Invalid value {}, expecting 0 or 1.".format(value))
            mask = 1 << channel
            channels = channels | mask
            if value == 0:
                values = values & ~mask
            else:
                values = values | mask

        if channels == 0:
            raise ValueError("No channels specified.")

        result = self._lib.mcc152_dio_output_write(
            self._address, self._DIO_CHANNEL_ALL, values)

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_output_read(self, channel):
        """
        Read a single DIO output value.

        This function returns the value stored in the output register. It may
        not represent the value at the terminal if the channel is configured as
        input or open-drain output.

        Args:
            channel (int): The DIO channel number, 0-7.

        Returns:
            int: The output value.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))

        value = c_ubyte()
        result = self._lib.mcc152_dio_output_read(
            self._address, channel, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_output_read_all(self):
        """
        Read all DIO output values at once.

        Returns a list of all output values in channel order.

        This function returns the values stored in the output register. They may
        not represent the value at the terminal if the channel is configured as
        input or open-drain output.

        Returns
            list of int: The output values.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_output_read(
            self._address, self._DIO_CHANNEL_ALL, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS

        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist

#===============================================================================

    def dio_direction_write(self, channel, value):
        """
        Set a single DIO channel direction.

        Set value to 0 for output, 1 for input.

        Args:
            channel (int): The dio channel number, 0-7.
            value (int): The direction, 0 or 1.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if value not in (0, 1):
            raise ValueError(
                "Invalid value {}, expecting 0 or 1.".format(value))

        result = self._lib.mcc152_dio_direction_write(
            self._address, channel, value)

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid channel {}.".format(channel))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_direction_write_multiple(self, value_dict):
        """
        Set multiple DIO channel directions.

        Set multiple directions by passing a dictionary with channel / direction
        pairs.

        For example, to set channels 0 and 2 to output and channel 3 to input
        call:

        dio_direction_write_multiple({0 : 0, 2 : 0, 3 : 1})

        Args:
            value_dict (dictionary): A dictionary containing channel : direction
                pairs.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: a channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        channels = 0
        orig_val = c_ubyte()
        result = self._lib.mcc152_dio_direction_read(
            self._address, self._DIO_CHANNEL_ALL, byref(orig_val))
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        values = orig_val.value()

        for channel, value in value_dict.items():
            if channel not in range(self._DIO_NUM_CHANNELS):
                raise ValueError("Invalid channel {}.".format(channel))
            if value not in (0, 1):
                raise ValueError(
                    "Invalid value {}, expecting 0 or 1.".format(value))
            mask = 1 << channel
            channels = channels | mask
            if value == 0:
                values = values & ~mask
            else:
                values = values | mask

        if channels == 0:
            raise ValueError("No channels specified.")

        result = self._lib.mcc152_dio_direction_write(
            self._address, self._DIO_CHANNEL_ALL, values)

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_direction_read(self, channel):
        """
        Read a single DIO channel direction.

        This function returns a 0 if the channel is output and 1 if input.

        Args:
            channel (int): The DIO channel number, 0-7.

        Returns:
            int: The output value.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))

        value = c_ubyte()
        result = self._lib.mcc152_dio_direction_read(
            self._address, channel, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_direction_read_all(self):
        """
        Read all DIO channel directions at once.

        Returns a list of all directions in channel order. The direction will be
        0 for output and 1 for input.


        Returns
            list of int: The direction values.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_direction_read(
            self._address, self._DIO_CHANNEL_ALL, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS

        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist

#===============================================================================

    def dio_pull_config_write(self, channel, value):
        """
        Configure a single DIO channel pull-up/down resistor.

        Set value to 0 for pull-down, 1 for pull-up. The resistor is enabled
        with dio_pull_enable_write() or dio_pull_enable_write_multiple().

        Args:
            channel (int): The dio channel number, 0-7.
            value (int): The pull configuration, 0 or 1.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if value not in (0, 1):
            raise ValueError(
                "Invalid value {}, expecting 0 or 1.".format(value))

        result = self._lib.mcc152_dio_pull_config_write(
            self._address, channel, value)

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid channel {}.".format(channel))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_pull_config_write_multiple(self, value_dict):
        """
        Configure multiple DIO channel pull-up/down resistors.

        Set multiple configurations by passing a dictionary with channel /
        config pairs.

        For example, to set channels 0 and 2 to pull-up and channel 3 to
        pull-down call:

        dio_pull_config_write_multiple({0 : 1, 2 : 1, 3 : 0})

        Note that the pull resistors are enabled separately with
        dio_pull_enable_write() or dio_pull_enable_write_multiple().

        Args:
            value_dict (dictionary): A dictionary containing channel : config
                pairs.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: a channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        channels = 0
        orig_val = c_ubyte()
        result = self._lib.mcc152_dio_pull_config_read(
            self._address, self._DIO_CHANNEL_ALL, byref(orig_val))
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        values = orig_val.value()

        for channel, value in value_dict.items():
            if channel not in range(self._DIO_NUM_CHANNELS):
                raise ValueError("Invalid channel {}.".format(channel))
            if value not in (0, 1):
                raise ValueError(
                    "Invalid value {}, expecting 0 or 1.".format(value))
            mask = 1 << channel
            channels = channels | mask
            if value == 0:
                values = values & ~mask
            else:
                values = values | mask

        if channels == 0:
            raise ValueError("No channels specified.")

        result = self._lib.mcc152_dio_pull_config_write(
            self._address, self._DIO_CHANNEL_ALL, values)

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_pull_config_read(self, channel):
        """
        Read a single DIO channel pull-up/down resistor configuration.

        This function returns a 0 for pull-down and 1 for pull-up.

        Args:
            channel (int): The DIO channel number, 0-7.

        Returns:
            int: The pull configuration.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))

        value = c_ubyte()
        result = self._lib.mcc152_dio_pull_config_read(
            self._address, channel, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_pull_config_read_all(self):
        """
        Read all DIO channel pull-up/down resistor configurations at once.

        Returns a list of all configurations in channel order. The configuration
        will be 0 for pull-down and 1 for pull-up.


        Returns
            list of int: The configuration values.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_pull_config_read(
            self._address, self._DIO_CHANNEL_ALL, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS

        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist

#===============================================================================

    def dio_pull_enable_write(self, channel, value):
        """
        Enable or disable a single DIO channel pull-up/down resistor.

        Set value to 0 to disable the resistor, 1 to enable it. The resistor is
        configured for pull-up/down with dio_pull_config_write() or
        dio_pull_config_write_multiple().

        Args:
            channel (int): The dio channel number, 0-7.
            value (int): The pull enable value, 0 or 1.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if value not in (0, 1):
            raise ValueError(
                "Invalid value {}, expecting 0 or 1.".format(value))

        result = self._lib.mcc152_dio_pull_enable_write(
            self._address, channel, value)

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid channel {}.".format(channel))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_pull_enable_write_multiple(self, value_dict):
        """
        Enable or disable multiple DIO channel pull-up/down resistors.

        Set multiple configurations by passing a dictionary with channel /
        enable pairs.

        For example, to set channels 0 and 2 to pull enabled and channel 3 to
        pull disabled call:

        dio_pull_enable_write_multiple({0 : 1, 2 : 1, 3 : 0})

        Note that the pull resistors are configured as pull-up or pull-down
        separately with dio_pull_config_write() or
        dio_pull_config_write_multiple().

        Args:
            value_dict (dictionary): A dictionary containing channel : enable
                pairs.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: a channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        channels = 0
        orig_val = c_ubyte()
        result = self._lib.mcc152_dio_pull_enable_read(
            self._address, self._DIO_CHANNEL_ALL, byref(orig_val))
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        values = orig_val.value()

        for channel, value in value_dict.items():
            if channel not in range(self._DIO_NUM_CHANNELS):
                raise ValueError("Invalid channel {}.".format(channel))
            if value not in (0, 1):
                raise ValueError(
                    "Invalid value {}, expecting 0 or 1.".format(value))
            mask = 1 << channel
            channels = channels | mask
            if value == 0:
                values = values & ~mask
            else:
                values = values | mask

        if channels == 0:
            raise ValueError("No channels specified.")

        result = self._lib.mcc152_dio_pull_enable_write(
            self._address, self._DIO_CHANNEL_ALL, values)

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_pull_enable_read(self, channel):
        """
        Read a single DIO channel pull-up/down resistor enable setting.

        This function returns a 0 for disabled and 1 for pull-up.

        Args:
            channel (int): The DIO channel number, 0-7.

        Returns:
            int: The pull enable setting.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))

        value = c_ubyte()
        result = self._lib.mcc152_dio_pull_enable_read(
            self._address, channel, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_pull_enable_read_all(self):
        """
        Read all DIO channel pull-up/down resistor enable settings at once.

        Returns a list of all enable settings in channel order. The setting will
        be 0 for disabled and 1 for enabled.


        Returns
            list of int: The enable settings.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_pull_enable_read(
            self._address, self._DIO_CHANNEL_ALL, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS

        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist

#===============================================================================

    def dio_input_invert_write(self, channel, value):
        """
        Configure a single DIO input channel polarity inversion.

        Set value to 0 for normal polarity, 1 to invert the input.

        Args:
            channel (int): The dio channel number, 0-7.
            value (int): The invert value, 0 or 1.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if value not in (0, 1):
            raise ValueError(
                "Invalid value {}, expecting 0 or 1.".format(value))

        result = self._lib.mcc152_dio_input_invert_write(
            self._address, channel, value)

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid channel {}.".format(channel))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_input_invert_write_multiple(self, value_dict):
        """
        Configure input inversion for multiple DIO channels.

        Set multiple configurations by passing a dictionary with channel /
        invert pairs.

        For example, to set channels 0 and 2 to inverted and channel 3 to
        non-inverted call:

        dio_input_invert_write_multiple({0 : 1, 2 : 1, 3 : 0})

        Args:
            value_dict (dictionary): A dictionary containing channel : invert
                pairs.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: a channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        channels = 0
        orig_val = c_ubyte()
        result = self._lib.mcc152_dio_input_invert_read(
            self._address, self._DIO_CHANNEL_ALL, byref(orig_val))
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        values = orig_val.value()

        for channel, value in value_dict.items():
            if channel not in range(self._DIO_NUM_CHANNELS):
                raise ValueError("Invalid channel {}.".format(channel))
            if value not in (0, 1):
                raise ValueError(
                    "Invalid value {}, expecting 0 or 1.".format(value))
            mask = 1 << channel
            channels = channels | mask
            if value == 0:
                values = values & ~mask
            else:
                values = values | mask

        if channels == 0:
            raise ValueError("No channels specified.")

        result = self._lib.mcc152_dio_input_invert_write(
            self._address, self._DIO_CHANNEL_ALL, values)

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_input_invert_read(self, channel):
        """
        Read a single DIO channel input inversion setting.

        This function returns 0 for normal inputs and 1 for inverted.

        Args:
            channel (int): The DIO channel number, 0-7.

        Returns:
            int: The pull enable setting.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))

        value = c_ubyte()
        result = self._lib.mcc152_dio_input_invert_read(
            self._address, channel, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_input_invert_read_all(self):
        """
        Read all DIO channel input inversion settings at once.

        Returns a list of all invert settings in channel order. The setting will
        be 0 for normal input and 1 for inverted.


        Returns
            list of int: The invert settings.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_input_invert_read(
            self._address, self._DIO_CHANNEL_ALL, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS

        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist

#===============================================================================

    def dio_input_latch_write(self, channel, value):
        """
        Configure a single DIO input channel latching.

        Input latching is used to detect a change in an input between reads of
        the input. It is disabled by setting value to 0 or enabled by setting it
        to 1. When disabled, reads of the input will show the current status of
        the input.  A state change in that input will generate an interrupt if
        it is not masked, and a read of the input will clear the interrupt. If
        the input goes back to its initial logic state before the input is read
        then the interrupt is cleared.

        When latching is enabled, the corresponding input state is latched. A
        change of state of the input generates and interrupt if not masked and
        the input logic value is loaded into the input port register. A read of
        the input will clear the interrupt. If the input returns to its initial
        logic state before the input is read then the interrupt is not cleared
        and the input register keeps the logic value that initiated the
        interrupt. The next read of the input will show the initial state.

        If the input is changed from latched to non-latched then a read from the
        input reflects the current terminal logic level. If the input is changed
        from non-latched to latched then a read from the input represents the
        latched logic level.

        Args:
            channel (int): The dio channel number, 0-7.
            value (int): The latch enable, 0 or 1.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if value not in (0, 1):
            raise ValueError(
                "Invalid value {}, expecting 0 or 1.".format(value))

        result = self._lib.mcc152_dio_input_latch_write(
            self._address, channel, value)

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid channel {}.".format(channel))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_input_latch_write_multiple(self, value_dict):
        """
        Configure input latching for multiple DIO channels.

        Set multiple configurations by passing a dictionary with channel /
        latch pairs. See the description for dio_input_latch_write() for a
        description on latch operation.

        For example, to set channels 0 and 2 to latched and channel 3 to
        non-latched call:

        dio_input_latch_write_multiple({0 : 1, 2 : 1, 3 : 0})

        Args:
            value_dict (dictionary): A dictionary containing channel : latch
                pairs.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: a channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        channels = 0
        orig_val = c_ubyte()
        result = self._lib.mcc152_dio_input_latch_read(
            self._address, self._DIO_CHANNEL_ALL, byref(orig_val))
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        values = orig_val.value()

        for channel, value in value_dict.items():
            if channel not in range(self._DIO_NUM_CHANNELS):
                raise ValueError("Invalid channel {}.".format(channel))
            if value not in (0, 1):
                raise ValueError(
                    "Invalid value {}, expecting 0 or 1.".format(value))
            mask = 1 << channel
            channels = channels | mask
            if value == 0:
                values = values & ~mask
            else:
                values = values | mask

        if channels == 0:
            raise ValueError("No channels specified.")

        result = self._lib.mcc152_dio_input_latch_write(
            self._address, self._DIO_CHANNEL_ALL, values)

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_input_latch_read(self, channel):
        """
        Read a single DIO channel input latch setting.

        This function returns 0 for non-latched and 1 for latched.

        Args:
            channel (int): The DIO channel number, 0-7.

        Returns:
            int: The latch setting.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))

        value = c_ubyte()
        result = self._lib.mcc152_dio_input_latch_read(
            self._address, channel, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_input_latch_read_all(self):
        """
        Read all DIO channel input latch settings at once.

        Returns a list of all latch settings in channel order. The setting will
        be 0 for non-latche and 1 for latched.


        Returns
            list of int: The latch settings.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_input_latch_read(
            self._address, self._DIO_CHANNEL_ALL, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS

        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist

#===============================================================================

    def dio_output_type_write(self, value):
        """
        Configure the DIO output type as push-pull or open-drain.

        This setting configures all of the outputs, they cannot be set
        separately. Set value to 0 for push-pull or 1 for open-drain.

        Args:
            value (int): The output type, 0 or 1.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if value not in (0, 1):
            raise ValueError(
                "Invalid value {}, expecting 0 or 1.".format(value))

        result = self._lib.mcc152_dio_output_type_write(
            self._address, value)

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_output_type_read(self):
        """
        Read the DIO output type.

        This function returns 0 for push-pull and 1 for open-drain.

        Returns:
            int: The output type, 0 or 1.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_output_type_read(
            self._address, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

#===============================================================================

    def dio_interrupt_mask_write(self, channel, value):
        """
        Configure a single DIO input interrupt mask.

        Set a 1 to disable (mask) the interrupt for the specified channel or 0
        to enable the interrupt. When the interrupt is enabled the device will
        generate an interrupt if the input changes state from the last read
        value.

        All attached devices share a single interrupt signal so the interrupt
        source must be determined when multiple interrupts are enabled. The
        current interrupt state may be read with hat_interrupt_status(). A
        program can wait for the interrupt to become active with
        hat_wait_for_interrupt(). The interrupt is cleared by reading the
        input(s) with dio_input_read() or dio_input_read_multiple(). When
        multiple devices are configured to generate an interrupt then the source
        of the interrupt must be determined by reading the status of each device
        with dio_interrupt_status_read() and all active interrupt sources must
        be cleared before the interrupt will become inactive.

        Args:
            channel (int): The dio channel number, 0-7.
            value (int): The mask value, 0 or 1.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if value not in (0, 1):
            raise ValueError(
                "Invalid value {}, expecting 0 or 1.".format(value))

        result = self._lib.mcc152_dio_interrupt_mask_write(
            self._address, channel, value)

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid channel {}.".format(channel))
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_interrupt_mask_write_multiple(self, value_dict):
        """
        Configure multiple DIO input interrupt mask values.

        Set multiple masks by passing a dictionary with channel /
        mask pairs. See the description for dio_interrupt_mask_write() for a
        description on interrupt mask operation.

        For example, to enable interrupts on channels 0 and 2 and disable them
        on channel 3 call:

        dio_interrupt_mask_write_multiple({0 : 0, 2 : 0, 3 : 1})

        Args:
            value_dict (dictionary): A dictionary containing channel : mask
                pairs.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: a channel or value argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        channels = 0
        orig_val = c_ubyte()
        result = self._lib.mcc152_dio_interrupt_mask_read(
            self._address, self._DIO_CHANNEL_ALL, byref(orig_val))
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        values = orig_val.value()

        for channel, value in value_dict.items():
            if channel not in range(self._DIO_NUM_CHANNELS):
                raise ValueError("Invalid channel {}.".format(channel))
            if value not in (0, 1):
                raise ValueError(
                    "Invalid value {}, expecting 0 or 1.".format(value))
            mask = 1 << channel
            channels = channels | mask
            if value == 0:
                values = values & ~mask
            else:
                values = values | mask

        if channels == 0:
            raise ValueError("No channels specified.")

        result = self._lib.mcc152_dio_interrupt_mask_write(
            self._address, self._DIO_CHANNEL_ALL, values)

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return

    def dio_interrupt_mask_read(self, channel):
        """
        Read a single DIO channel interrupt mask setting.

        This function returns 0 for un-masked (enabled) and 1 for masked
        (disabled.)

        Args:
            channel (int): The DIO channel number, 0-7.

        Returns:
            int: The mask setting.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))

        value = c_ubyte()
        result = self._lib.mcc152_dio_interrupt_mask_read(
            self._address, channel, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_interrupt_mask_read_all(self):
        """
        Read all DIO channel interrupt mask settings at once.

        Returns a list of all mask settings in channel order. The setting will
        be 0 for un-masked (enabled) and 1 for masked (disabled.)


        Returns
            list of int: The mask settings.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_interrupt_mask_read(
            self._address, self._DIO_CHANNEL_ALL, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS

        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist

#===============================================================================

    def dio_interrupt_status_read(self, channel):
        """
        Read a single DIO channel interrupt status.

        This function returns 0 if the channel is not generating an interrupt or
        1 if it is generating an interrupt.

        Args:
            channel (int): The DIO channel number, 0-7.

        Returns:
            int: The interrupt status.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._DIO_NUM_CHANNELS):
            raise ValueError("Invalid channel {}.".format(channel))

        value = c_ubyte()
        result = self._lib.mcc152_dio_interrupt_status_read(
            self._address, channel, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        return value.value()

    def dio_interrupt_status_read_all(self):
        """
        Read all DIO channel interrupt status settings at once.

        Returns a list of all status values in channel order. The status will
        be 0 for if the associated channel is not generating an interrupt or 1
        if it is generating an interrupt.


        Returns
            list of int: The interrupt status values.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        value = c_ubyte()
        result = self._lib.mcc152_dio_interrupt_status_read(
            self._address, self._DIO_CHANNEL_ALL, byref(value))

        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")

        # convert byte to list of bits
        mylist = [0]*self._DIO_NUM_CHANNELS

        reg = value.value()
        for i in range(self._DIO_NUM_CHANNELS):
            mylist[i] = (reg & (1<<i)) >> i

        return mylist
