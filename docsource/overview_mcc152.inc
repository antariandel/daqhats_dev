MCC 152
=======

The MCC 152 is an analog output / digital I/O board with the following features:

- 2 analog outputs

  - 12-bit D/A converter
  - 0 - 5 V outputs
  
- 8 digital I/O

  - 5 V / 3.3 V selectable
  - Programmable pull-up/pull-down resistors
  - Interrupt on input change

.. only:: html

    .. image:: images/mcc152_low.jpg
        :align: center
        :alt: MCC 152 circuit board

.. only:: latex

    .. image:: images/mcc152.jpg
        :align: center
        :alt: MCC 152 circuit board

Board components
----------------
Screw terminals
^^^^^^^^^^^^^^^
- **Analog output 0** to **Analog output 1** (AOx): Analog output terminals.
- **Digital I/O 0** to **Digital I/O 7** (DIOx): Digital input/output terminals.
- **AGND** (AGND): Common ground for the analog output terminals.
- **VIO** (VIO): Digital I/O supply voltage (3.3V / 5V, depending on W3 position.)
- **DGND** (DGND): Common ground for the digital I/O terminals.

Address jumpers
^^^^^^^^^^^^^^^
- **A0** to **A2**: Used to identify each DAQ HAT when multiple boards are
  connected. The first DAQ HAT connected to the Raspberry Pi must be at address 0
  (no jumper). Install jumpers on each additional connected board to set the
  desired address. Refer to the :ref:`multiple` topic for more information about
  the recommended addressing method.

DIO Power jumper
^^^^^^^^^^^^^^^^
- **5V** and **3.3V**: Selects the DIO voltage.

Status LED
^^^^^^^^^^
The LED turns on when the board is connected to a Raspberry Pi with external power 
applied. 

Header connector
^^^^^^^^^^^^^^^^
The board header is used to connect with the Raspberry Pi. Refer to :ref:`install`
for more information about the header connector.

Functional block diagram
------------------------

.. only:: html

    .. image:: images/mcc152-diag_low.jpg
	:align: center
	:alt: MCC 152 block diagram

.. only:: latex

    .. image:: images/mcc152-diag.jpg
	:align: center
	:alt: MCC 152 block diagram

Functional details
------------------

Mixing 3.3V and 5V digital inputs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The MCC 152 digital inputs are tolerant of 5V signals when the DIO is set to 3.3V
operation with jumper W3.  However, current can flow into the MCC 152 from the 5V
signal so the user must limit this current to avoid raising the voltage of the 
digital power supply rail (VIO) and possibly damaging components.  MCC recommends
using a series resistor of 700 ohms or larger.

Example:

.. only:: html

    .. image:: images/mcc152_resistor_example_low.png
	:align: center
	:alt: MCC 152 resistor example

.. only:: latex

    .. image:: images/mcc152_resistor_example.png
	:align: center
	:alt: MCC 152 block diagram

Specifications
--------------

.. only:: html

    `MCC 152 Electrical Specifications <_static/esmcc152.pdf>`_

.. raw:: latex

    \includepdf[pages=-]{../../specs/esmcc152.pdf}