.. currentmodule:: daqhats

Global methods and data
=======================

Methods
-------

=================================  =============================================
Method                             Description
---------------------------------  ---------------------------------------------
:py:func:`hat_list`                Return a list of detected DAQ HAT boards.
:py:func:`interrupt_state`         Read the current DAQ HAT interrupt status.
:py:func:`wait_for_interrupt`      Wait for a DAQ HAT  interrupt to occur.
=================================  =============================================

.. autofunction:: hat_list
.. autofunction:: interrupt_state
.. autofunction:: wait_for_interrupt

Data
----

Hat IDs
~~~~~~~

.. autoclass:: HatIDs
    :members:

Trigger modes
~~~~~~~~~~~~~

.. autoclass:: TriggerModes
    :members:

Scan / read option flags
~~~~~~~~~~~~~~~~~~~~~~~~

.. autoclass:: OptionFlags
    :members:

HatError class
--------------

.. autoexception:: HatError
