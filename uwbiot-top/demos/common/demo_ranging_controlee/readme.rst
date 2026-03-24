..
    Copyright 2020,2022,2025-2026 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _demo-ranging-controlee:

=======================================================================
 Demo Ranging Controlee
=======================================================================

.. brief:start

This demo showcases normal ranging with one device configured as a Controlee - Responder
and another device configured as a Controller - Initiator [Another demo].

.. brief:end

Following sequence of steps are handled.

- Initialize UWBD in Mainline Firmware.
- Set the application ranging parameters
- Perform normal ranging with static STS.


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. only:: sr150

    - For RTOS based platform refer :ref:`rhodesv4se-McuXpresso-project`
    - For embed linux platform refer :ref:`build-rpi-mk-shield`

.. only:: sr100_sn

    - For RTOS based platform refer :ref:`rhodesv4sn-McuXpresso-project`

.. only:: sr100_p71

    - For RTOS based platform refer :ref:`rhodesv4-p71-McuXpresso-project`

.. only:: sr040

    - For RTOS based platform refer :ref:`qn9090-McuXpresso-project`

.. only:: sr250 and (not zephyr_sr250) and ( not sr250_frdm_rw612)

    - For RTOS based platform refer :ref:`virgo-McuXpresso-project`.
    - For embed linux Raspberry-Pi with Virgo setup :ref:`build-pi-virgo`

.. only:: sr250_frdm_rw612

    - For Zephyr based platform, refer the **Build Demo Application** section from **Getting_started guide**

.. only:: zephyr_sr250

    - For Zephyr RTOS based platform refer :ref:`virgo-Zephyr-McuXpresso-project`.

.. only:: porting_guide

    .. code-block:: bash

        west build -b frdm_rw612 demos/common/demo_ranging_controlee/zephyr -p


- Source:   ``demo_ranging_controlee.c``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

.. only:: zephyr_sr250 or sr250_frdm_rw612

    - For Zephyr RTOS based device, flash the ``demo_ranging_controlee.bin`` file.

.. only:: (not zephyr_sr250) and (not sr250_frdm_rw612)

    - For embedded RTOS based device, flash the ``demo_ranging_controlee.bin`` file.
    - On linux platform run the built executable.

- Run :ref:`demo-ranging-controller` on counterpart for normal ranging.


Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :62000055 2B010000 01000000 00C80000 .. 308DD080 D0
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :60070001 0A
    TMLUWB  :RX < :RECV                :62000055 2C010000 01000000 00C80000 .. 3070D080 D0
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0300
    TMLUWB  :TX > :SEND                :21010004 01000000
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/common/demo_ranging_controlee/demo_ranging_controlee.c : Success!

If such a log is not seen, re-run the program.
