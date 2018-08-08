.. currentmodule:: daqhats

Global methods and data
=======================

Methods
-------

=================================  =======================================================
Method                             Description
---------------------------------  -------------------------------------------------------
:py:func:`hat_list`                Return a list of detected DAQ HAT boards.
:py:func:`hat_interrupt_state`     Read the current interrupt status.
:py:func:`hat_wait_for_interrupt`  Wait for an interrupt to occur.
=================================  =======================================================

.. autofunction:: hat_list
.. autofunction:: hat_interrupt_state
.. autofunction:: hat_wait_for_interrupt

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
