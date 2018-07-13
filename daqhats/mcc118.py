"""
Wraps all of the methods from the MCC 118 library for use in Python.
"""
import sys
from collections import namedtuple
from ctypes import c_ubyte, c_int, c_ushort, c_ulong, c_long, c_double, \
    POINTER, c_char_p, byref, create_string_buffer
from daqhats.hats import Hat, HatError, OptionFlags

class mcc118(Hat): # pylint: disable=invalid-name
    """
    The class for an MCC 118 board.

    Args:
        address (int): board address, must be 0-7.

    Raises:
        HatError: the board did not respond or was of an incorrect type
    """

    _AIN_NUM_CHANNELS = 8         # Number of analog channels

    _STATUS_HW_OVERRUN = 0x0001
    _STATUS_BUFFER_OVERRUN = 0x0002
    _STATUS_TRIGGERED = 0x0004
    _STATUS_RUNNING = 0x0008

    _MAX_SAMPLE_RATE = 100000.0

    def __init__(self, address=0):
        """
        Initialize the class.
        """
        # call base class initializer
        Hat.__init__(self, address)

        # set up library argtypes and restypes
        self._lib.mcc118_open.argtypes = [c_ubyte]
        self._lib.mcc118_open.restype = c_int

        self._lib.mcc118_close.argtypes = [c_ubyte]
        self._lib.mcc118_close.restype = c_int

        self._lib.mcc118_close.argtypes = [c_ubyte]
        self._lib.mcc118_close.restype = c_int

        self._lib.mcc118_blink_led.argtypes = [c_ubyte, c_ubyte]
        self._lib.mcc118_blink_led.restype = c_int

        self._lib.mcc118_firmware_version.argtypes = [
            c_ubyte, POINTER(c_ushort), POINTER(c_ushort)]
        self._lib.mcc118_firmware_version.restype = c_int

        self._lib.mcc118_serial.argtypes = [c_ubyte, c_char_p]
        self._lib.mcc118_serial.restype = c_int

        self._lib.mcc118_calibration_date.argtypes = [c_ubyte, c_char_p]
        self._lib.mcc118_calibration_date.restype = c_int

        self._lib.mcc118_calibration_coefficient_read.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_double), POINTER(c_double)]
        self._lib.mcc118_calibration_coefficient_read.restype = c_int

        self._lib.mcc118_calibration_coefficient_write.argtypes = [
            c_ubyte, c_ubyte, c_double, c_double]
        self._lib.mcc118_calibration_coefficient_write.restype = c_int

        self._lib.mcc118_trigger_mode.argtypes = [c_ubyte, c_ubyte]
        self._lib.mcc118_trigger_mode.restype = c_int

        self._lib.mcc118_a_in_read.argtypes = [
            c_ubyte, c_ubyte, c_ulong, POINTER(c_double)]
        self._lib.mcc118_a_in_read.restype = c_int

        self._lib.mcc118_a_in_scan_actual_rate.argtypes = [
            c_ubyte, c_double, POINTER(c_double)]
        self._lib.mcc118_a_in_scan_actual_rate.restype = c_int

        self._lib.mcc118_a_in_scan_start.argtypes = [
            c_ubyte, c_ubyte, c_ulong, c_double, c_ulong]
        self._lib.mcc118_a_in_scan_start.restype = c_int

        self._lib.mcc118_a_in_scan_status.argtypes = [
            c_ubyte, POINTER(c_ushort), POINTER(c_ulong)]
        self._lib.mcc118_a_in_scan_status.restype = c_int

        self._lib.mcc118_a_in_scan_buffer_size.argtypes = [
            c_ubyte, POINTER(c_ulong)]
        self._lib.mcc118_a_in_scan_buffer_size.restype = c_int

        self._lib.mcc118_a_in_scan_read.restype = c_int

        self._lib.mcc118_a_in_scan_stop.argtypes = [c_ubyte]
        self._lib.mcc118_a_in_scan_stop.restype = c_int

        self._lib.mcc118_a_in_scan_cleanup.argtypes = [c_ubyte]
        self._lib.mcc118_a_in_scan_cleanup.restype = c_int

        self._lib.mcc118_a_in_scan_channel_count.argtypes = [c_ubyte]
        self._lib.mcc118_a_in_scan_channel_count.restype = c_ubyte

        self._lib.mcc118_test_clock.argtypes = [
            c_ubyte, c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc118_test_clock.restype = c_int

        self._lib.mcc118_test_trigger.argtypes = [c_ubyte, POINTER(c_ubyte)]
        self._lib.mcc118_test_trigger.restype = c_int

        if self._lib.mcc118_open(self._address) != self._RESULT_SUCCESS:
            self._initialized = False
            raise HatError(self._address, "Board not responding.")

        return

    def __del__(self):
        if self._initialized:
            self._lib.mcc118_a_in_scan_cleanup(self._address)
            self._lib.mcc118_close(self._address)
        return

    def firmware_version(self):
        """
        Read the board firmware and bootloader versions.

        Returns:
            namedtuple: a namedtuple containing the following field names

            * **version** (string): The firmware version, i.e "1.03".
            * **bootloader_version** (string): The bootloader version,
              i.e "1.01".

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        version = c_ushort()
        boot_version = c_ushort()
        if (self._lib.mcc118_firmware_version(
                self._address, byref(version), byref(boot_version))
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")
        version_str = "{0:X}.{1:02X}".format(
            version.value >> 8, version.value & 0x00FF)
        boot_str = "{0:X}.{1:02X}".format(
            boot_version.value >> 8, boot_version.value & 0x00FF)
        version_info = namedtuple(
            'MCC118VersionInfo', ['version', 'bootloader_version'])
        return version_info(
            version=version_str,
            bootloader_version=boot_str)

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
        if (self._lib.mcc118_serial(self._address, my_buffer)
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")
        my_serial = my_buffer.value.decode('ascii')
        return my_serial

    def blink_led(self, count):
        """
        Blink the MCC 118 LED.

        Setting count to 0 will cause the LED to blink continuously until
        blink_led() is called again with a non-zero count.

        Args:
            count (int): The number of times to blink (max 255).

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        if (self._lib.mcc118_blink_led(self._address, count)
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")
        return

    def calibration_date(self):
        """
        Read the calibration date.

        Returns:
            string: The calibration date in the format "YYYY-MM-DD".

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        # create string to hold the result
        my_buffer = create_string_buffer(11)
        if (self._lib.mcc118_calibration_date(self._address, my_buffer)
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")
        my_date = my_buffer.value.decode('ascii')
        return my_date

    def calibration_coefficient_read(self, channel):
        """
        Read the calibration coefficients for a single channel.

        The coefficients are applied in the library as: ::

            calibrated_ADC_code = (raw_ADC_code * slope) + offset

        Returns:
            namedtuple: a namedtuple containing the following field names

            * **slope** (float): The slope.
            * **offset** (float): The offset.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        slope = c_double()
        offset = c_double()
        if (self._lib.mcc118_calibration_coefficient_read(
                self._address, channel, byref(slope), byref(offset))
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")
        cal_info = namedtuple('MCC118CalInfo', ['slope', 'offset'])
        return cal_info(
            slope=slope.value,
            offset=offset.value)

    def calibration_coefficient_write(self, channel, slope, offset):
        """
        Temporarily write the calibration coefficients for a single channel.

        The user can apply their own calibration coefficients by writing to
        these values. The values will reset to the factory values from the
        EEPROM whenever the class is initialized.  This function will fail and
        raise a HatError exception if a scan is active when it is called.

        The coefficients are applied in the library as: ::

            calibrated_ADC_code = (raw_ADC_code * slope) + offset

        Args:
            slope (float): The new slope value.
            offset (float): The new offset value.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        if (self._lib.mcc118_calibration_coefficient_write(
                self._address, channel, slope, offset)
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")
        return

    def trigger_mode(self, mode):
        """
        Set the external trigger input mode.

        The available modes are:

        * :py:const:`TriggerModes.RISING_EDGE`: Start the scan when the TRIG
          input transitions from low to high.
        * :py:const:`TriggerModes.FALLING_EDGE`: Start the scan when the TRIG
          input transitions from high to low.
        * :py:const:`TriggerModes.ACTIVE_HIGH`: Start the scan when the TRIG
          input is high.
        * :py:const:`TriggerModes.ACTIVE_LOW`: Start the scan when the TRIG
          input is low.

        Args:
            mode (:py:class:`TriggerModes`): The trigger mode.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        if (self._lib.mcc118_trigger_mode(self._address, mode) !=
                self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")
        return

    @staticmethod
    def a_in_num_channels():
        """
        Return the number of analog input channels.

        Returns:
            int: the number of channels.
        """
        return mcc118._AIN_NUM_CHANNELS

    def a_in_read(self, channel, options=OptionFlags.DEFAULT):
        """
        Perform a single reading of an analog input channel and return the
        value.

        **options** is an ORed combination of OptionFlags. Valid flags for this
        method are:

        * :py:const:`OptionFlags.DEFAULT`: Return a calibrated voltage value.
          Any other flags will override DEFAULT behavior.
        * :py:const:`OptionFlags.NOSCALEDATA`: Return an ADC code (a value
          between 0 and 4095) rather than voltage.
        * :py:const:`OptionFlags.NOCALIBRATEDATA`: Return data without the
          calibration factors applied.

        Args:
            channel (int): The analog input channel number, 0-7.
            options (int): ORed combination of :py:class:`OptionFlags`,
                :py:const:`OptionFlags.DEFAULT` if unspecified.

        Returns:
            float: the read value

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the channel number is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if channel not in range(self._AIN_NUM_CHANNELS):
            raise ValueError("Invalid channel {0}. Must be 0-{1}.".format(
                channel, self._AIN_NUM_CHANNELS-1))

        data_value = c_double()

        if (self._lib.mcc118_a_in_read(
                self._address, channel, options, byref(data_value))
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")
        return data_value.value

    def a_in_scan_actual_rate(self, channel_count, sample_rate_per_channel):
        """
        Read the actual sample rate per channel for a requested sample rate.

        The internal scan clock is generated from a 16 MHz clock source so only
        discrete frequency steps can be achieved.  This function will return the
        actual rate for a requested channel count and rate setting.

        This function does not perform any actions with a board, it simply
        calculates the rate.

        Args:
            channel_count (int): The number of channels in the scan, 1-8.
            sample_rate_per_channel (float): The desired per-channel rate of the
                internal sampling clock, max 100,000.0.

        Returns:
            float: the actual sample rate

        Raises:
            ValueError: a scan argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        data_value = c_double()

        if (self._lib.mcc118_a_in_scan_actual_rate(
                channel_count, sample_rate_per_channel, byref(data_value))
                != self._RESULT_SUCCESS):
            raise ValueError(
                "The specified parameters are invalid or outside the device "
                "capabilities.")
        return data_value.value

    def a_in_scan_start(self, channel_mask, samples_per_channel,
                        sample_rate_per_channel, options):
        """
        Start a hardware-paced analog input channel scan.

        The scan runs as a separate thread from the user's code. This function
        will allocate a scan buffer and start the thread that reads data from
        the device into that buffer. The user reads the data from the scan
        buffer and the scan status using the :py:func:`a_in_scan_read` function.
        :py:func:`a_in_scan_stop` is used to stop a continuous scan, or to stop
        a finite scan before it completes. The user must call
        :py:func:`a_in_scan_cleanup` after the scan has finished and all desired
        data has been read; this frees all resources from the scan and allows
        additional scans to be performed.

        The scan state has defined terminology:

        * **Active**: :py:func:`a_in_scan_start` has been called and the device
          may be acquiring data or finished with the acquisition. The scan has
          not been cleaned up by calling :py:func:`a_in_scan_cleanup`, so
          another scan may not be started.
        * **Running**: The scan is active and the device is still acquiring
          data. Certain methods like :py:func:`a_in_read` will return an error
          because the device is busy.

        The scan options that may be used are:

        * :py:const:`OptionFlags.DEFAULT`: Return scaled and calibrated data,
          internal scan clock, no trigger, and finite operation. Any other flags
          will override DEFAULT behavior.
        * :py:const:`OptionFlags.NOSCALEDATA`: Return ADC codes (values between
          0 and 4095) rather than voltage.
        * :py:const:`OptionFlags.NOCALIBRATEDATA`: Return data without the
          calibration factors applied.
        * :py:const:`OptionFlags.EXTCLOCK`: Use an external 3.3V or 5V logic
          signal at the CLK input as the scan clock. Multiple devices can be
          synchronized by connecting the CLK pins together and using this flag
          on all but one device so they will be clocked by the single device
          using its internal clock. **sample_rate_per_channel** is only used for
          buffer sizing.
        * :py:const:`OptionFlags.EXTTRIGGER`: Hold off the scan (after calling
          :py:func:`a_in_scan_start`) until the trigger condition is met. The
          trigger is a 3.3V or 5V logic signal applied to the TRIG pin.
        * :py:const:`OptionFlags.CONTINUOUS`: Scans continuously until stopped
          by the user by calling :py:func:`a_in_scan_stop` and writes data to a
          circular buffer. The data must be read before being overwritten to
          avoid a buffer overrun error. **samples_per_channel** is only used for
          buffer sizing.

        The scan buffer size will be allocated as follows:

        **Finite mode:** Total number of samples in the scan.

        **Continuous mode:** Either **samples_per_channel** or the value in the
        table below, whichever is greater.

        ==============      =========================
        Sample Rate         Buffer Size (per channel)
        ==============      =========================
        Not specified       10 kS
        0-100 S/s           1 kS
        100-10k S/s         10 kS
        10k-100k S/s        100 kS
        ==============      =========================

        Specifying a very large value for samples_per_channel could use too much
        of the Raspberry Pi memory. If the memory allocation fails, the function
        will raise a HatError with this description. The allocation could
        succeed, but the lack of free memory could cause other problems in the
        Raspberry Pi. If you need to acquire a high number of samples then it is
        better to run the scan in continuous mode and stop it when you have
        acquired the desired amount of data. If a scan is active this method
        will raise a HatError.

        Args:
            channel_mask (int): A bit mask of the desired channels (0x01 -
                0xFF).
            samples_per_channel (int): The number of samples to acquire per
                channel (finite mode,) or or can be used to set a larger scan 
                buffer size than the default value (continuous mode.)
            sample_rate_per_channel (float): The per-channel rate of the
                internal scan clock, or the expected maximum rate of an external
                scan clock, max 100,000.0.
            options (int): An ORed combination of :py:class:`OptionFlags` flags
                that control the scan.

        Raises:
            HatError: a scan is active; memory could not be allocated; the board
                is not initialized, does not respond, or responds incorrectly.
            ValueError: a scan argument is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        # Perform some argument checking
        if channel_mask == 0:
            raise ValueError("channel_mask must be nonzero.")

        num_channels = 0
        for index in range(8):
            bit_mask = 1 << index
            if (channel_mask & bit_mask) != 0:
                num_channels += 1

        if num_channels * sample_rate_per_channel > self._MAX_SAMPLE_RATE:
            raise ValueError(
                "Invalid sample_rate_per_channel, exceeds maximum rate for "
                "device.")

        result = self._lib.mcc118_a_in_scan_start(
            self._address, channel_mask, samples_per_channel,
            sample_rate_per_channel, options)

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid scan parameter.")
        elif result == self._RESULT_BUSY:
            raise HatError(self._address, "A scan is already active.")
        elif result == self._RESULT_RESOURCE_UNAVAIL:
            raise HatError(self._address, "Memory could not be allocated.")
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response {}.".format(
                result))
        return

    def a_in_scan_buffer_size(self):
        """
        Read the internal scan data buffer size.

        An internal data buffer is allocated for the scan when
        :py:func:`a_in_scan_start` is called. This function returns the total
        size of that buffer in samples.

        Returns:
            int: the buffer size in samples

        Raises:
            HatError: the board is not initialized or no scan buffer is
                allocated (a scan is not active).
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")
        data_value = c_ulong()

        result = self._lib.mcc118_a_in_scan_buffer_size(
            self._address, byref(data_value))

        if result == self._RESULT_RESOURCE_UNAVAIL:
            raise HatError(self._address, "No scan is active.")
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response {}.".format(
                result))
        return data_value.value

    def a_in_scan_status(self):
        """
        Read scan status and number of available samples per channel.

        The analog input scan is started with :py:func:`a_in_scan_start` and
        runs in the background.  This function reads the status of that
        background scan and the number of samples per channel available in
        the scan thread buffer.

        Returns:
            namedtuple: a namedtuple containing the following field names:

            * **running** (bool): True if the scan is running, False if it has
              stopped or completed.
            * **hardware_overrun** (bool): True if the hardware could not
              acquire and unload samples fast enough and data was lost.
            * **buffer_overrun** (bool): True if the background scan buffer was
              not read fast enough and data was lost.
            * **triggered** (bool): True if the trigger conditions have been met
              and data acquisition started.
            * **samples_available** (int): The number of samples per channel
              currently in the scan buffer.

        Raises:
            HatError: A scan is not active, the board is not initialized, does
                not respond, or responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        status = c_ushort(0)
        samples_available = c_ulong(0)

        result = self._lib.mcc118_a_in_scan_status(
            self._address, byref(status), byref(samples_available))

        if result == self._RESULT_RESOURCE_UNAVAIL:
            raise HatError(self._address, "Scan not active.")
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response {}.".format(
                result))

        scan_status = namedtuple(
            'MCC118ScanStatus',
            ['running', 'hardware_overrun', 'buffer_overrun', 'triggered',
             'samples_available'])
        return scan_status(
            running=(status.value & self._STATUS_RUNNING) != 0,
            hardware_overrun=(status.value & self._STATUS_HW_OVERRUN) != 0,
            buffer_overrun=(status.value & self._STATUS_BUFFER_OVERRUN) != 0,
            triggered=(status.value & self._STATUS_TRIGGERED) != 0,
            samples_available=samples_available.value)

    def a_in_scan_read(self, samples_per_channel, timeout):
        # pylint: disable=too-many-locals
        """
        Read scan status and data (as a list).

        The analog input scan is started with :py:func:`a_in_scan_start` and
        runs in the background.  This function reads the status of that
        background scan and optionally reads sampled data from the scan buffer.

        Args:
            samples_per_channel (int): The number of samples per channel to read
                from the scan buffer. Specify a negative number to return all
                available samples immediately and ignore **timeout** or 0 to
                only read the scan status and return no data.
            timeout (float): The amount of time in seconds to wait for the
                samples to be read. Specify a negative number to wait
                indefinitely, or 0 to return immediately with the samples that
                are already in the scan buffer (up to **samples_per_channel**.)
                If the timeout is met and the specified number of samples have
                not been read, then the function will return all the available
                samples and the timeout status set.

        Returns:
            namedtuple: a namedtuple containing the following field names:

            * **running** (bool): True if the scan is running, False if it has
              stopped or completed.
            * **hardware_overrun** (bool): True if the hardware could not
              acquire and unload samples fast enough and data was lost.
            * **buffer_overrun** (bool): True if the background scan buffer was
              not read fast enough and data was lost.
            * **triggered** (bool): True if the trigger conditions have been met
              and data acquisition started.
            * **timeout** (bool): True if the timeout time expired before the
              specified number of samples were read.
            * **data** (list of float): The data that was read from the scan
              buffer.

        Raises:
            HatError: A scan is not active, the board is not initialized, does
                not respond, or responds incorrectly.
            ValueError: Incorrect argument.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        num_channels = self._lib.mcc118_a_in_scan_channel_count(self._address)

        self._lib.mcc118_a_in_scan_read.argtypes = [
            c_ubyte, POINTER(c_ushort), c_long, c_double, POINTER(c_double),
            c_ulong, POINTER(c_ulong)]

        samples_read_per_channel = c_ulong(0)
        samples_to_read = 0
        status = c_ushort(0)
        timed_out = False

        if samples_per_channel < 0:
            # read all available data

            # first, get the number of available samples
            samples_available = c_ulong(0)
            result = self._lib.mcc118_a_in_scan_status(
                self._address, byref(status), byref(samples_available))

            if result != self._RESULT_SUCCESS:
                raise HatError(self._address, "Incorrect response {}.".format(
                    result))

            # allocate a buffer large enough for all the data
            samples_to_read = samples_available.value
            buffer_size = samples_available.value * num_channels
            data_buffer = (c_double * buffer_size)()
        elif samples_per_channel == 0:
            # only read the status
            samples_to_read = 0
            buffer_size = 0
            data_buffer = None
        elif samples_per_channel > 0:
            # read the specified number of samples
            samples_to_read = samples_per_channel
            # create a C buffer for the read
            buffer_size = samples_per_channel * num_channels
            data_buffer = (c_double * buffer_size)()
        else:
            # invalid samples_per_channel
            raise ValueError("Invalid samples_per_channel {}.".format(
                samples_per_channel))

        result = self._lib.mcc118_a_in_scan_read(
            self._address, byref(status), samples_to_read, timeout, data_buffer,
            buffer_size, byref(samples_read_per_channel))

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid parameter.")
        elif result == self._RESULT_RESOURCE_UNAVAIL:
            raise HatError(self._address, "Scan not active.")
        elif result == self._RESULT_TIMEOUT:
            timed_out = True
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response {}.".format(
                result))

        total_read = samples_read_per_channel.value * num_channels

        # python 2 / 3 workaround for xrange
        if sys.version_info.major > 2:
            data_list = [data_buffer[i] for i in range(total_read)]
        else:
            data_list = [data_buffer[i] for i in xrange(total_read)]

        scan_status = namedtuple(
            'MCC118ScanRead',
            ['running', 'hardware_overrun', 'buffer_overrun', 'triggered',
             'timeout', 'data'])
        return scan_status(
            running=(status.value & self._STATUS_RUNNING) != 0,
            hardware_overrun=(status.value & self._STATUS_HW_OVERRUN) != 0,
            buffer_overrun=(status.value & self._STATUS_BUFFER_OVERRUN) != 0,
            triggered=(status.value & self._STATUS_TRIGGERED) != 0,
            timeout=timed_out,
            data=data_list)

    def a_in_scan_read_numpy(self, samples_per_channel, timeout):
        # pylint: disable=too-many-locals
        """
        Read scan status and data (as a NumPy array).

        This function is similar to :py:func:`a_in_scan_read` except that the
        *data* key in the returned namedtuple is a NumPy array of float64 values
        and may be used directly with NumPy functions.

        Args:
            samples_per_channel (int): The number of samples per channel to read
                from the scan buffer.  Specify a negative number to read all
                available samples or 0 to only read the scan status and return
                no data.
            timeout (float): The amount of time in seconds to wait for the
                samples to be read.  Specify a negative number to wait
                indefinitely, or 0 to return immediately with the samples that
                are already in the scan buffer.  If the timeout is met and the
                specified number of samples have not been read, then the
                function will return with the amount that has been read and the
                timeout status set.

        Returns:
            namedtuple: a namedtuple containing the following field names:

            * **running** (bool): True if the scan is running, False if it has
              stopped or completed.
            * **hardware_overrun** (bool): True if the hardware could not
              acquire and unload samples fast enough and data was lost.
            * **buffer_overrun** (bool): True if the background scan buffer was
              not read fast enough and data was lost.
            * **triggered** (bool): True if the trigger conditions have been met
              and data acquisition started.
            * **timeout** (bool): True if the timeout time expired before the
              specified number of samples were read.
            * **data** (NumPy array of float64): The data that was read from the
              scan buffer.

        Raises:
            HatError: A scan is not active, the board is not initialized, does
                not respond, or responds incorrectly.
            ValueError: Incorrect argument.
        """
        try:
            import numpy
            from numpy.ctypeslib import ndpointer
        except ImportError:
            raise

        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        self._lib.mcc118_a_in_scan_read.argtypes = [
            c_ubyte, POINTER(c_ushort), c_long, c_double,
            ndpointer(c_double, flags="C_CONTIGUOUS"), c_ulong,
            POINTER(c_ulong)]

        num_channels = self._lib.mcc118_a_in_scan_channel_count(self._address)
        samples_read_per_channel = c_ulong()
        status = c_ushort()
        timed_out = False
        samples_to_read = 0

        if samples_per_channel < 0:
            # read all available data

            # first, get the number of available samples
            samples_available = c_ulong(0)
            result = self._lib.mcc118_a_in_scan_status(
                self._address, byref(status), byref(samples_available))

            if result != self._RESULT_SUCCESS:
                raise HatError(self._address, "Incorrect response {}.".format(
                    result))

            # allocate a buffer large enough for all the data
            samples_to_read = samples_available.value
            buffer_size = samples_available.value * num_channels
            data_buffer = numpy.empty(buffer_size, dtype=numpy.float64)
        elif samples_per_channel == 0:
            # only read the status
            samples_to_read = 0
            buffer_size = 0
            data_buffer = None
        elif samples_per_channel > 0:
            # read the specified number of samples
            samples_to_read = samples_per_channel
            # create a C buffer for the read
            buffer_size = samples_per_channel * num_channels
            data_buffer = numpy.empty(buffer_size, dtype=numpy.float64)
        else:
            # invalid samples_per_channel
            raise ValueError("Invalid samples_per_channel {}.".format(
                samples_per_channel))

        result = self._lib.mcc118_a_in_scan_read(
            self._address, byref(status), samples_to_read, timeout, data_buffer,
            buffer_size, byref(samples_read_per_channel))

        if result == self._RESULT_BAD_PARAMETER:
            raise ValueError("Invalid parameter.")
        elif result == self._RESULT_RESOURCE_UNAVAIL:
            raise HatError(self._address, "Scan not active.")
        elif result == self._RESULT_TIMEOUT:
            timed_out = True
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response {}.".format(
                result))

        total_read = samples_read_per_channel.value * num_channels

        if total_read < buffer_size:
            data_buffer = numpy.resize(data_buffer, (total_read,))

        scan_status = namedtuple(
            'MCC118ScanRead',
            ['running', 'hardware_overrun', 'buffer_overrun', 'triggered',
             'timeout', 'data'])
        return scan_status(
            running=(status.value & self._STATUS_RUNNING) != 0,
            hardware_overrun=(status.value & self._STATUS_HW_OVERRUN) != 0,
            buffer_overrun=(status.value & self._STATUS_BUFFER_OVERRUN) != 0,
            triggered=(status.value & self._STATUS_TRIGGERED) != 0,
            timeout=timed_out,
            data=data_buffer)

    def a_in_scan_channel_count(self):
        """
        Read the number of channels in the current analog input scan.

        Returns:
            int: the number of channels (0 if no scan is active, 1-8 otherwise)

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        num_channels = self._lib.mcc118_a_in_scan_channel_count(self._address)
        return num_channels

    def a_in_scan_stop(self):
        """
        Stops an analog input scan.

        The device stops acquiring data immediately. The scan data that has been
        read into the scan buffer is available until
        :py:func:`a_in_scan_cleanup` is called.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if (self._lib.mcc118_a_in_scan_stop(self._address)
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")

        return

    def a_in_scan_cleanup(self):
        """
        Free analog input scan resources after the scan is complete.

        This will free the scan buffer and other resources used by the
        background scan and make it possible to start another scan with
        :py:func:`a_in_scan_start`.

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if (self._lib.mcc118_a_in_scan_cleanup(self._address)
                != self._RESULT_SUCCESS):
            raise HatError(self._address, "Incorrect response.")

        return

    def test_clock(self, mode):
        """
        Test the sample clock pin (CLK).

        This function exercises the CLK pin in output mode and returns the value
        read at the pin for input testing.  Return the mode to input after
        testing the pin.

        Args:
            mode (int): The CLK pin mode

                * 0 = input
                * 1 = output low
                * 2 = output high
                * 3 = output 1 kHz square wave

        Returns:
            int: the value read at the CLK pin after applying the mode (0 or 1).

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
            ValueError: the mode is invalid.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        if mode not in range(4):
            raise ValueError("Invalid mode. Must be 0-3.")

        data_value = c_ubyte()
        result = self._lib.mcc118_test_clock(self._address, mode,
                                             byref(data_value))
        if result == self._RESULT_BUSY:
            raise HatError(self._address,
                           "Cannot test the CLK pin while a scan is running.")
        elif result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return data_value.value

    def test_trigger(self):
        """
        Test the external trigger pin (TRIG).

        This value read at the pin for input testing.

        Returns:
            int: the value read at the TRIG pin (0 or 1).

        Raises:
            HatError: the board is not initialized, does not respond, or
                responds incorrectly.
        """
        if not self._initialized:
            raise HatError(self._address, "Not initialized.")

        data_value = c_ubyte()
        result = self._lib.mcc118_test_trigger(self._address, byref(data_value))
        if result != self._RESULT_SUCCESS:
            raise HatError(self._address, "Incorrect response.")
        return data_value.value
