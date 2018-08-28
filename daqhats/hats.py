"""
Wraps the global methods from the MCC Hat library for use in Python.
"""
from collections import namedtuple
from ctypes import cdll, Structure, c_ubyte, c_ushort, c_char, c_int, POINTER, \
    CFUNCTYPE, cast, py_object, c_void_p, pointer
from enum import IntEnum

class HatIDs(IntEnum):
    """Known MCC HAT IDs."""
    ANY = 0             #: Match any MCC ID in :py:func:`hat_list`
    MCC_118 = 0x0142    #: MCC 118 ID
    MCC_152 = 0x0144    #: MCC 152 ID

class TriggerModes(IntEnum):
    """Scan trigger input modes."""
    RISING_EDGE = 0     #: Start the scan on a rising edge of TRIG.
    FALLING_EDGE = 1    #: Start the scan on a falling edge of TRIG.
    ACTIVE_HIGH = 2     #: Start the scan any time TRIG is high.
    ACTIVE_LOW = 3      #: Start the scan any time TRIG is low.

class OptionFlags(IntEnum):
    """Scan / read option flags. See individual methods for detailed
    descriptions."""
    DEFAULT = 0x0000         #: Use default behavior.
    NOSCALEDATA = 0x0001     #: Read / write unscaled data.
    NOCALIBRATEDATA = 0x0002 #: Read / write uncalibrated data.
    EXTCLOCK = 0x0004        #: Use an external clock source.
    EXTTRIGGER = 0x0008      #: Use an external trigger source.
    CONTINUOUS = 0x0010      #: Run until explicitly stopped.

# exception class
class HatError(Exception):
    """
    Exceptions raised for MCC HAT specific errors.

    Args:
        address (int): the address of the board that caused the exception.
        value (str): the exception description.
    """
    def __init__(self, address, value):
        super(HatError, self).__init__(value)
        self.address = address
        self.value = value
    def __str__(self):
        return "Addr {}: ".format(self.address) + self.value

# HAT info structure class
class _Info(Structure): # pylint: disable=too-few-public-methods
    _fields_ = [("address", c_ubyte),
                ("id", c_ushort),
                ("version", c_ushort),
                ("product_name", c_char * 256)]

# Callback function class
class HatCallback(object):
    def __init__(self, function):
        self.function = function
        self.cbFUNCTYPE = None
        self.funclist = []
        
    def set_cbfunc(cbfunc):
        self.cbFUNCTYPE = cbfunc
        self.funclist.append(cbfunc)

def _load_daqhats_library():
    """
    Load the library
    """
    libname = 'libdaqhats.so.1'
    try:
        lib = cdll.LoadLibrary(libname)
    except: # pylint: disable=bare-except
        lib = 0
    return lib

def hat_list(filter_by_id=0):
    """
    Return a list of detected DAQ HAT boards.

    Scans certain locations for information from the HAT EEPROMs.  Verifies the
    contents are valid HAT EEPROM contents and returns a list of namedtuples
    containing information on the HAT.  Info will only be returned for DAQ HATs.
    The EEPROM contents are stored in /etc/mcc/hats when using the
    daqhats_read_eeproms tool, or in /proc/device-tree in the case of a single
    HAT at address 0.

    Args:
        filter_by_id (int): If this is :py:const:`HatIDs.ANY` return all DAQ
            HATs found.  Otherwise, return only DAQ HATs with ID matching this
            value.

    Returns:
        list: A list of namedtuples, the number of elements match the number of
        DAQ HATs found. Each namedtuple will contain the following field names

        * **address** (int): device address
        * **id** (int): device product ID, identifies the type of DAQ HAT
        * **version** (int): device hardware version
        * **product_name** (str): device product name
    """
    _libc = _load_daqhats_library()
    if _libc == 0:
        return []

    _libc.hat_list.argtypes = [c_ushort, POINTER(_Info)]
    _libc.hat_list.restype = c_int

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
    hat_info = namedtuple('HatInfo',
                          ['address', 'id', 'version', 'product_name'])
    for item in my_info:
        info = hat_info(
            address=item.address,
            id=item.id,
            version=item.version,
            product_name=item.product_name.decode('ascii'))
        my_list.append(info)

    return my_list

def interrupt_state():
    """
    Read the current DAQ HAT interrupt status

    Returns the status of the interrupt signal, True if active or False if
    inactive. The signal can be shared by multiple DAQ HATs so the status of
    each board that may generate an interrupt must be read and the interrupt
    source(s) cleared before the interrupt will become inactive.

    Returns:
        bool: The interrupt status.
    """
    _libc = _load_daqhats_library()
    if _libc == 0:
        return []

    _libc.hat_interrupt_state.argtypes = []
    _libc.hat_interrupt_state.restype = c_int

    # get the info
    state = _libc.hat_interrupt_state()

    return state == 1

def wait_for_interrupt(timeout):
    """
    Wait for an interrupt from a DAQ HAT to occur.

    Pass a timeout in seconds. Pass -1 to wait forever or 0 to return
    immediately. If the interrupt has not occurred before the timeout elapses
    the function will return 0.

    Returns:
        bool: The interrupt status - True = interrupt active, False = interrupt
        inactive.
    """
    _libc = _load_daqhats_library()
    if _libc == 0:
        return []

    _libc.hat_wait_for_interrupt.argtypes = [c_int]
    _libc.hat_wait_for_interrupt.restype = c_int

    if timeout == -1:
        timeout_ms = -1
    elif timeout == 0:
        timeout_ms = 0
    else:
        timeout_ms = timeout * 1000

    state = _libc.hat_wait_for_interrupt(timeout_ms)
    return state == 1

def interrupt_callback_enable(callback, user_data):
    """
    Configure an interrupt callback function.

    Set a function that will be called when a DAQ HAT interrupt occurs. The 
    function must have a void return type and void * argument, such as:
 
        void function(void* data)
 
    The function will be called when the DAQ HAT interrupt signal becomes
    active, and cannot be called again until the interrupt signal becomes
    inactive. Active sources become inactive when manually cleared (such as
    reading the digital I/O inputs or clearing the interrupt enable.) If not
    latched, an active source also becomes inactive when the value returns to
    the original value (the value at the source before the interrupt was
    generated.)
 
    There may only be one callback function at a time; if you call this
    when a function is already set as the callback function then it will be
    replaced with the new function and the old function will no longer be called
    if an interrupt occurs. The data argument to this function will be passed
    to the callback function when it is called.
 
    @param function     The callback function.
    @param data         The data to pass to the callback function.
    @return [RESULT_SUCCESS](@ref RESULT_SUCCESS) or 
        [RESULT_UNDEFINED](@ref RESULT_UNDEFINED).

    Returns:
        bool: The interrupt status.
    """
    _libc = _load_daqhats_library()
    if _libc == 0:
        return []

    cbFUNCTYPE = CFUNCTYPE(None, c_void_p)
    callback.set_cbfunc(cbFUNCTYPE(callback.function))
    
    _libc.hat_interrupt_callback_enable.argtypes = [cbFUNCTYPE, c_void_p]
    _libc.hat_interrupt_callback_enable.restype = c_int
    
    c_user_data = cast(pointer(py_object(user_data)), c_void_p)
    if (_libc.hat_interrupt_callback_enable(callback.cbFUNCTYPE, c_user_data) != 
            0):
        raise Exception("Could not enable callback function.")

def get_python_object(user_data):
    p = cast(user_data, POINTER(py_object))
    print(p.contents.value)
    return cast(user_data, POINTER(py_object)).contents.value
    
class Hat(object): # pylint: disable=too-few-public-methods
    """
    DAQ HAT base class.

    Args:
        address (int): board address, must be 0-7.

    Raises:
        ValueError: the address is invalid.
    """
    _RESULT_SUCCESS = 0
    _RESULT_BAD_PARAMETER = -1
    _RESULT_BUSY = -2
    _RESULT_TIMEOUT = -3
    _RESULT_LOCK_TIMEOUT = -4
    _RESULT_INVALID_DEVICE = -5
    _RESULT_RESOURCE_UNAVAIL = -6
    _RESULT_COMMS_FAILURE = -7
    _RESULT_UNDEFINED = -10

    def __init__(self, address=0):
        """Initialize the class.  Address must be 0-7."""
        self._initialized = False
        self._address = 0

        # max address value is 7
        if address in range(8):
            self._address = address
        else:
            raise ValueError("Invalid address {}. Must be 0-7.".format(address))

        self._lib = _load_daqhats_library()
        if self._lib == 0:
            raise Exception("daqhats shared library is not installed.")

        self._initialized = True
        return

    def address(self):
        """Return the device address."""
        return self._address
