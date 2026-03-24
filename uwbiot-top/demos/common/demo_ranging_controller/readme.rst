..
    Copyright 2020,2022,2026 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _demo-ranging-controller:

=======================================================================
 Demo Ranging Controller
=======================================================================

.. brief:start

This demo showcases normal ranging with one device configured as a Controller - Initiator
and another device configured as a Controlee - Responder [Another demo].

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

        west build -b frdm_rw612 demos/common/demo_ranging_controller/zephyr -p

- Source:   ``demo_ranging_controller.c``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

.. only:: zephyr_sr250 or sr250_frdm_rw612

    - For Zephyr RTOS based device, flash the ``demo_ranging_controller.bin`` file.

.. only:: (not zephyr_sr250) and (not sr250_frdm_rw612)

    - For embedded RTOS based device, flash the ``demo_ranging_controller.bin`` file.
    - On linux platform run the built executable.

- Run :ref:`demo-ranging-controlee` on counterpart for normal ranging.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :62000055 2E010000 01000000 .. 309BD080 D0
    TMLUWB  :RX < :RECV                :62000055 2F010000 01000000 .. 00000000 00
    UCICORE :WARN :Retrying last failed command
    TMLUWB  :TX > :SEND                :22010004 01000000
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :62000055 30010000 01000000 .. 00000000 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0300
    TMLUWB  :TX > :SEND                :21010004 01000000
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 01000000 0100
    APP     :INFO :Finished <Project_Path>/uwbiot-top/demos/common/demo_ranging_controller/demo_ranging_controller.c : Success!

If such a log is not seen, re-run the program.

.. .. addLineBreaks 5
