"""
Wraps the global methods from the MCC Hat library for use in Python.
"""
import os.path
from ctypes import *
import sys

# Load the library
libname = 'libdaqhats.so.0'
try:
    _libc = cdll.LoadLibrary(libname)
except:
    _libc = 0

class HatIDs:
    """Known MCC HAT IDs."""
    ANY     = 0         #: Match any MCC ID in :py:func:`hat_list`
    MCC_118 = 0x0142    #: MCC 118 ID
    MCC_134 = 0x0143    #: MCC 134 ID

class TriggerModes:
    """Scan trigger input modes."""
    RISING_EDGE     = 0     #: Start the scan on a rising edge of TRIG.
    FALLING_EDGE    = 1     #: Start the scan on a falling edge of TRIG.
    ACTIVE_HIGH     = 2     #: Start the scan any time TRIG is high.
    ACTIVE_LOW      = 3     #: Start the scan any time TRIG is low.

# exception classes
class HatError(Exception):
    """
    Exceptions raised for MCC HAT specific errors.

    Args:
        address (int): the address of the board that caused the exception.
        value (str): the exception description.
    """
    def __init__(self, address, value):
        self.address = address
        self.value = value
    def __str__(self):
        return "Addr {}: ".format(self.address) + self.value

class _Info(Structure):
    _fields_ = [("address", c_ubyte),
                ("id", c_ushort),
                ("version", c_ushort),
                ("product_name", c_char * 256)]

def hat_list(filter_by_id = 0):
    """
    Return a list of detected DAQ HAT boards.

    Scans certain locations for information from the HAT EEPROMs.  Verifies the contents are
    valid HAT EEPROM contents and returns a list of dictionaries containing information on the
    HAT.  Info will only be returned for DAQ HATs.  The EEPROM contents are stored in
    /etc/mcc/hats when using the daqhats_read_eeproms tool, or in /proc/device-tree in the case of
    a single HAT at address 0.

    Args:
        filter_by_id (int): If this is :py:const:`HatIDs.ANY` return all DAQ HATs found.  Otherwise, return
            only DAQ HATs with ID matching this value.

    Returns:
        list: A list of dictionaries, the number of elements match the number of DAQ HATs found.
        Each dictionary will contain the following keys

        | **address** (int): device address
        | **id** (int): device product ID, identifies the type of DAQ HAT
        | **version** (int): device hardware version
        | **product_name** (str): device product name
    """
    if _libc == 0:
        return []

    _libc.hat_list.argtypes = [c_ushort, POINTER(_Info)]
    _libc.hat_list.restype  = c_int

    # find out how many structs we need
    count = _libc.hat_list(filter_by_id, None)
    if count == 0:
        return []

    # allocate array of Info structs
    my_info = (_Info * count)()

    # get the info
    count = _libc.hat_list(filter_by_id, my_info)

    # create the list of dictionaries to return
    my_list = []
    for item in my_info:
        name = cast(item.product_name, c_char_p)
        if sys.version_info > (3, 0):
            info = {
                'address'       : item.address,
                'id'            : item.id,
                'version'       : item.version,
                'product_name'  : name.value.decode()
            }
        else:
            info = {
                'address'       : item.address,
                'id'            : item.id,
                'version'       : item.version,
                'product_name'  : name.value
            }
        my_list.append(info)

    return my_list

class Hat(object):
    """
    DAQ HAT base class.

    Args:
        address (int): board address, must be 0-7.

    Raises:
        ValueError: the address is invalid.
    """
    _RESULT_SUCCESS             = 0
    _RESULT_BAD_PARAMETER       = -1
    _RESULT_BUSY                = -2
    _RESULT_TIMEOUT             = -3
    _RESULT_LOCK_TIMEOUT        = -4
    _RESULT_INVALID_DEVICE      = -5
    _RESULT_RESOURCE_UNAVAIL    = -6
    _RESULT_UNDEFINED           = -10

    def __init__(self, address = 0):
        """Initialize the class.  Address must be 0-7."""
        self._initialized = False
        self._address = 0

        # max address value is 7
        if address in range(8):
            self._address = address
        else:
            raise ValueError("Invalid address {}. Must be 0-7.".format(address))

        self._initialized = True
        return

    def address(self):
        """Return the device address."""
        return self._address

