.. _mcc134:

MCC 134
=======

4-channel thermocouple input board

- 24-bit A/D converter
- Onboard 16-bit temperature sensor for cold junction compensation
- Linearization for J, K, R, S, T, N, E, B type thermocouples
- Open thermocouple detection
- Thermocouple inputs are electrically isolated from the Raspberry Pi for use in harsh environments

.. image:: images/mcc_134.jpg
    :align: center
    :alt: MCC 118 circuit board


Board components
----------------
Screw terminals
^^^^^^^^^^^^^^^
- **CH 0 In** to **CH 3 In**: Differential thermocouple input terminals.

Status LED
^^^^^^^^^^
The LED turns on when the board is connected to a Raspberry Pi with external power applied. 

Header connector
^^^^^^^^^^^^^^^^
The board header is used to connect with the Raspberry Pi. Refer to :ref:`install` for more information about the  
header connector.
