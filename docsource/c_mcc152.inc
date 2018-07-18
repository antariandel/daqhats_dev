MCC 152 functions and data
==========================

Functions
---------

==========================================  =======================================================
Function                                    Description
------------------------------------------  -------------------------------------------------------
:c:func:`mcc152_open`                       Open an MCC 152 for use.
:c:func:`mcc152_is_open`                    Check if an MCC 152 is open.
:c:func:`mcc152_close`                      Close an MCC 152.
:c:func:`mcc152_serial`                     Read the serial number.
:c:func:`mcc152_a_out_num_channels`         Get the number of analog output channels.
:c:func:`mcc152_a_out_write`                Write an analog output channel value.
:c:func:`mcc152_a_out_write_all`            Write all analog output channels simultaneously.
:c:func:`mcc152_dio_num_channels`           Get the number of digital I/O channels.
:c:func:`mcc152_dio_reset`                  Reset the DIO to the default configuration.
:c:func:`mcc152_dio_input_read`             Read the DIO input(s).
:c:func:`mcc152_dio_output_write`           Write the DIO output(s).
:c:func:`mcc152_dio_output_read`            Read the DIO output state(s).
:c:func:`mcc152_dio_direction_write`        Set the DIO channel direction(s).
:c:func:`mcc152_dio_direction_read`         Read the DIO channel direction(s).
:c:func:`mcc152_dio_pull_config_write`      Configure the DIO pull-up / pull-down resistor(s).
:c:func:`mcc152_dio_pull_config_read`       Read the DIO pull-up / pull-down resistor configuration.
:c:func:`mcc152_dio_pull_enable_write`      Enable the DIO pull-up / pull-down resistor(s).
:c:func:`mcc152_dio_pull_enable_read`       Read the DIO pull-up / pull-down resistor enable value.
:c:func:`mcc152_dio_input_invert_write`     Configure the DIO input polarity inversion.
:c:func:`mcc152_dio_input_invert_read`      Read the DIO input polarity inversion configuration.
:c:func:`mcc152_dio_input_latch_write`      Configure the DIO input latching.
:c:func:`mcc152_dio_input_latch_read`       Read the DIO input latching configuration.
:c:func:`mcc152_dio_output_type_write`      Configure the DIO output type.
:c:func:`mcc152_dio_output_type_read`       Read the DIO output type configuration.
:c:func:`mcc152_dio_interrupt_mask_write`   Write the DIO interrupt mask.
:c:func:`mcc152_dio_interrupt_mask_read`    Read the DIO interrupt mask.
:c:func:`mcc152_dio_interrupt_status_read`  Read the DIO interrupt status.
==========================================  =======================================================
    
.. doxygenfunction:: mcc152_open
.. doxygenfunction:: mcc152_is_open
.. doxygenfunction:: mcc152_close
.. doxygenfunction:: mcc152_serial
.. doxygenfunction:: mcc152_a_out_num_channels
.. doxygenfunction:: mcc152_a_out_write
.. doxygenfunction:: mcc152_a_out_write_all
.. doxygenfunction:: mcc152_dio_num_channels
.. doxygenfunction:: mcc152_dio_reset
.. doxygenfunction:: mcc152_dio_input_read
.. doxygenfunction:: mcc152_dio_output_write
.. doxygenfunction:: mcc152_dio_output_read
.. doxygenfunction:: mcc152_dio_direction_write
.. doxygenfunction:: mcc152_dio_direction_read
.. doxygenfunction:: mcc152_dio_pull_config_write
.. doxygenfunction:: mcc152_dio_pull_config_read
.. doxygenfunction:: mcc152_dio_pull_enable_write
.. doxygenfunction:: mcc152_dio_pull_enable_read
.. doxygenfunction:: mcc152_dio_input_invert_write
.. doxygenfunction:: mcc152_dio_input_invert_read
.. doxygenfunction:: mcc152_dio_input_latch_write
.. doxygenfunction:: mcc152_dio_input_latch_read
.. doxygenfunction:: mcc152_dio_output_type_write
.. doxygenfunction:: mcc152_dio_output_type_read
.. doxygenfunction:: mcc152_dio_interrupt_mask_write
.. doxygenfunction:: mcc152_dio_interrupt_mask_read
.. doxygenfunction:: mcc152_dio_interrupt_status_read

Data definitions
----------------

.. doxygendefine:: DIO_CHANNEL_ALL
