..
    Copyright 2024,2026 NXP

    NXP Proprietary. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms.  By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.

.. _demo-ocpd:

=======================================================================
 SR250 ON Chip Presence Detection
=======================================================================

.. brief:start

#) This demo showcases the Presence detection with SR250.
#) Radar Mode configured as Medium distance used for Presence detection.
#) Radar distance Range for Presnece detecion is 3m to 8m.

Default Configs
------------------

    #) By default settings for the different conifurations for the OCPD .
    #) To change any settings update the below macros in :file:`demo_ocpd.c`, at location ``demos/radar/demo_ocpd`` .

        - ``#define PRESENCE_DETECTION_MODE_ENABLE             0x03 (Enabled both Presence and Radar Detection Mode)``

        - ``#define PRESENCE_DETECTION_PERIODIC_DATA_REPORT    0x04 (Disabled to send CIRs to Host with PD Distance and presence reporting every 400 msec)``

        - ``#define PRESENCE_DETECTION_ENABLE_IRQ              0x00 (Enable/Disable the IRQ and RADAR_RX_NTF)``

        - ``#define PRESENCE_DETECTION_SENSITIVITY_VALUE       3.75 (Default Sensitivity)``

        - ``#define PRESENCE_DETECTION_DISTANCE_MIN            30 (30cm Distance supported)``

        - ``#define MIN_PRESENCE_DETECTION_DISTANCE_MAX        200 (2m Distance supported)``

        - ``#define MIN_PRESENCE_DETECTION_HOLD_DELAY          1600 (Default Hold Sensitivity)``

        - ``#define MIN_PRESENCE_DETECTION_ANGLE               -90 (Minimum AoA supports)``

        - ``#define MAX_PRESENCE_DETECTION_ANGLE               +90 (Maximum AoA supports)``

.. note:: Application/Demo needs to allocate the memory for CIR Data.
          Make sure while configuration in cmake UWBFTR_RADAR is ON.

.. brief:end

Following sequence of steps are handled.

- Initialize UWBD in Mainline Firmware.
- Set the application Radar parameters


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Set Macro "USE_BARE_BOARD" to '0' or '1' depending upon the respective Host
  refer file `UWB_DeviceConfig.h` in ``<TOP>/boards/Host/${UWBIOT_Host}/UWB_DeviceConfig.h``

.. only:: sr250 and (not zephyr_sr250) and ( not sr250_frdm_rw612)

    - For RTOS based platform refer :ref:`virgo-McuXpresso-project`.
    - For embed linux Raspberry-Pi with Virgo setup :ref:`build-pi-virgo`

.. only:: sr250_frdm_rw612

    - For Zephyr based platform, refer the **Build Demo Application** section from **Getting_started guide**

.. only:: zephyr_sr250

    - For Zephyr RTOS based platform refer :ref:`virgo-Zephyr-McuXpresso-project`.

- Source:   ``demo_ocpd.c``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

.. only:: zephyr_sr250 or sr250_frdm_rw612

    - For Zephyr RTOS based device, flash the ``demo_ocpd.bin`` file.

.. only:: (not zephyr_sr250) and (not sr250_frdm_rw612)

    - For embedded RTOS based device, flash the ``demo_ocpd.bin`` file.
    - On linux platform run the built executable.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    HALUCI  :RX < :RECV                :798A0008 44332211 00000400 80008504 0000E893
    HALUCI  :RX < :RECV                :690A000A C8F8F7FB FBFB66F7 F5FB
    HALUCI  :RX < :RECV                :798A0008 44332211 00000400 80008904 0000EC17
    HALUCI  :RX < :RECV                :690A000A C5F8F7FB FBFB64F7 F5FB
    HALUCI  :RX < :RECV                :798A0008 44332211 00000400 80008D04 0000E59B
    HALUCI  :RX < :RECV                :690A000A C5F8F7FB FBFB62F7 F5FB
    HALUCI  :RX < :RECV                :690A0012 010000F0 00010100 00002D00 00004C2E 0000
    APP     :INFO :presence_detected          : 0x1
    APP     :INFO :presence_distance          : 45
    APP     :INFO :presence_detection_value   : 0x2e4
    HALUCI  :TX > :SEND                :22010004 44332211
    HALUCI  :RX < :RECV                :42010001 00
    HALUCI  :RX < :RECV                :61020006 44332211 0300
    HALUCI  :TX > :SEND                :21010004 44332211
    HALUCI  :RX < :RECV                :60010001 01
    HALUCI  :RX < :RECV                :41010001 00
    HALUCI  :RX < :RECV                :61020006 44332211 0100

    APP     :INFO :Finished <PROJECT_PATH>/uwbiot-top/demos/radar/demo_radar/demo_ocpd.c : Success!

If such a log is not seen, re-run the program.
