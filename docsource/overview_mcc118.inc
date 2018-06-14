MCC 118
=======

The MCC 118 is an 8-channel analog voltage input board with the following features:

- 12-bit, 100 kS/s A/D converter
- ± 10 V single-ended analog inputs
- Factory calibration with ± 20.8 mV input accuracy
- Hardware sample I/O clock
- Onboard sample buffers
- Digital trigger input

.. only:: html

    .. image:: images/mcc_118.jpg
        :align: center
        :alt: MCC 118 circuit board
	:scale: 50%

.. only:: latex

    .. image:: images/mcc_118.jpg
        :align: center
        :alt: MCC 118 circuit board
	:scale: 50%


Board components
----------------
Screw terminals
^^^^^^^^^^^^^^^
- **CH 0 In** to **CH 7 In**: Single-ended analog input terminals.
- **Clock**: Bidirectional terminal for pacer I/O. Set the direction with software. Set for input to pace operations 
  with an external clock signal, or output to pace operations with the internal sample clock.
- **Trigger**: External digital trigger input terminal. The trigger mode is software configurable for edge or level sensitive, 
  rising or falling edge, high or low level.
- **AGND**: Common ground for the analog input terminals.
- **DGND**: Common ground for the clock and trigger terminals.

Address jumpers
^^^^^^^^^^^^^^^
- **A0** to **A2**: Used to identify each HAT when multiple boards are connected. The first HAT connected to the 
  Raspberry Pi must be at address 0 (no jumper). Install a jumper on each additional connected board.
  Refer to the :ref:`multiple` discussion for more information about the recommended addressing method.

Status LED
^^^^^^^^^^
The LED turns on when the board is connected to a Raspberry Pi with external power applied. You can flash the LED with 
software.

Header connector
^^^^^^^^^^^^^^^^
The board header is used to connect with the Raspberry Pi. Refer to :ref:`install` for more information about the  
header connector.


Functional block diagram
------------------------

.. only:: html

    .. image:: images/mcc118-diag.jpg
	:align: center
	:alt: MCC 118 block diagram
	:scale: 30%

.. only:: latex

    .. image:: images/mcc118-diag.jpg
	:align: center
	:alt: MCC 118 block diagram
	:scale: 90%


Functional details
------------------

Clock mode
^^^^^^^^^^

The clock input / output on the MCC 118 is used to output the internal sample clock or apply an external sample clock to the device.  Each pulse on the clock initiates a series of conversions of the selected channels in the scan.  For example, when scanning channels 0, 1, and 2 the conversion activity will be:

.. only:: html

    .. image:: images/mcc118_clock.png
	:align: center
	:alt: MCC 118 clock mode
	:scale: 40%

.. only:: latex

    .. image:: images/mcc118_clock.png
	:align: center
	:alt: MCC 118 clock mode


Specifications
--------------

.. only:: html

    `MCC 118 Electrical Specifications <_static/esmcc118.pdf>`_

.. raw:: latex

    \includepdf[pages=-]{../../specs/esmcc118.pdf}
