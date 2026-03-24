..
    Copyright 2022,2026 NXP

    NXP Proprietary. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms.  By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.

.. _demo-radar:

=======================================================================
 SR250 Radar
=======================================================================

.. brief:start

This demo showcases normal Radar with SR250 device

- NOTE : Application/Demo needs to allocate the memory for CIR Data.

- NOTE : Make sure while configuration in ccmake UWBFTR_RADAR is ON

.. brief:end

Following sequence of steps are handled.

- Initialize UWBD in Mainline Firmware.
- Set the application Radar parameters


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Set Macro "USE_BARE_BOARD" to '0' or '1' depending upon the respective Host
  refer file `UWB_DeviceConfig.h` in ``<TOP>/boards/Host/${UWBIOT_Host}/UWB_DeviceConfig.h``

.. only:: sr150 or sr100

    - For RTOS based platform refer :ref:`rhodesv4se-McuXpresso-project`

.. only:: sr150 or sr040

    - For embed linux platform refer :ref:`build-rpi-mk-shield`

.. only:: sr250 and (not zephyr_sr250) and ( not sr250_frdm_rw612)

    - For RTOS based platform refer :ref:`virgo-McuXpresso-project`.
    - For embed linux Raspberry-Pi with Virgo setup :ref:`build-pi-virgo`

.. only:: sr250_frdm_rw612

    - For Zephyr based platform, refer the **Build Demo Application** section from **Getting_started guide**

.. only:: zephyr_sr250

    - For Zephyr RTOS based platform refer :ref:`virgo-Zephyr-McuXpresso-project`.

- Source:   ``demo_radar``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

.. only:: zephyr_sr250 or sr250_frdm_rw612

    - For Zephyr RTOS based device, flash the ``demo_radar.bin`` file.

.. only:: (not zephyr_sr250) and (not sr250_frdm_rw612)

    - For embedded RTOS based device, flash the ``demo_radar.bin`` file.
    - On linux platform run the built executable.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    TMLUWB  :RX < :RECV                :798A0008 44332211 00000400 80008504 0000E893
    TMLUWB  :RX < :RECV                :690A000A C8F8F7FB FBFB66F7 F5FB
    TMLUWB  :RX < :RECV                :798A0008 44332211 00000400 80008904 0000EC17
    TMLUWB  :RX < :RECV                :690A000A C5F8F7FB FBFB64F7 F5FB
    TMLUWB  :RX < :RECV                :798A0008 44332211 00000400 80008D04 0000E59B
    TMLUWB  :RX < :RECV                :690A000A C5F8F7FB FBFB62F7 F5FB
    TMLUWB  :TX > :SEND                :22010004 44332211
    TMLUWB  :RX < :RECV                :42010001 00
    TMLUWB  :RX < :RECV                :61020006 44332211 0300
    TMLUWB  :TX > :SEND                :21010004 44332211
    TMLUWB  :RX < :RECV                :60010001 01
    TMLUWB  :RX < :RECV                :41010001 00
    TMLUWB  :RX < :RECV                :61020006 44332211 0100

    APP     :INFO :Finished <PROJECT_PATH>/uwbiot-top/demos/radar/demo_radar/demo_radar.c : Success!

If such a log is not seen, re-run the program.
