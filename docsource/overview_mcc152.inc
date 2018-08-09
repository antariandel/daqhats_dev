MCC 152
=======

The MCC 152 is 2-channel analog output / 8 digital I/O board with the following
features:

- 12-bit D/A converter
- 0 - 5 V output
- 5 V / 3.3 V selectable digital I/O
- Programmable pull-up/pull-down resistors
- 2 5 mA sink per output
- Interrupt on input change

.. only:: html

    .. image:: images/mcc_152.png
        :align: center
        :alt: MCC 152 circuit board
        :scale: 50%

.. only:: latex

    .. image:: images/mcc_152.png
        :align: center
        :alt: MCC 152 circuit board
        :scale: 50%


Board components
----------------
Screw terminals
^^^^^^^^^^^^^^^
- **AO0** to **AO1** (AOx): Analog output terminals.
- **DIO0** to **DIO7** (DIOx): Digital input/output terminals.
- **AGND** (GND): Common ground for the analog terminals.
- **DGND** (GND): Common ground for the digital terminals.

Address jumpers
^^^^^^^^^^^^^^^
- **A0** to **A2**: Used to identify each HAT when multiple boards are
  connected. The first HAT connected to the Raspberry Pi must be at address 0
  (no jumper). Install jumpers on each additional connected board to set the
  desired address. Refer to the :ref:`multiple` topic for more information about
  the recommended addressing method.

Power jumper
^^^^^^^^^^^^^^^
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

    .. image:: images/mcc152-diag.jpg
	:align: center
	:alt: MCC 152 block diagram
	:scale: 30%

.. only:: latex

    .. image:: images/mcc152-diag.jpg
	:align: center
	:alt: MCC 152 block diagram
	:scale: 90%


Specifications
--------------

.. only:: html

    `MCC 152 Electrical Specifications <_static/esmcc152.pdf>`_

.. raw:: latex

    \includepdf[pages=-]{../../specs/esmcc152.pdf}