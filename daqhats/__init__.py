"""
MCC DAQ HATs module.
"""
from daqhats.hats import HatError, hat_list, HatIDs, TriggerModes, \
OptionFlags, wait_for_interrupt, interrupt_state
from daqhats.mcc118 import mcc118
from daqhats.mcc152 import mcc152, DIOConfigItem
