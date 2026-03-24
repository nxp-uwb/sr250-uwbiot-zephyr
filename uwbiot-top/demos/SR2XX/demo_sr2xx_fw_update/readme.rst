..
    Copyright 2025-2026 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _sr2xx-demo-fw-update:


=======================================================================
 Demo FW update
=======================================================================
.. brief:start

Running SR2xx Firmware Download
===================================
This demo ensures that the UWB device firmware is up-to-date.
It checks the current firmware version on the chip against the version in the middleware binary.
If an update is required, it downloads the new firmware, initializes the MW stack, and retrieves device information.

.. brief:end

Firmware Update Scenarios
-------------------------
The firmware update logic handles different scenarios based on the ``forceFwUpdate`` flag and the presence of firmware context structure :cpp:type:`phUwbFWImageContext_t`.

1. **Force Firmware Enabled, Firmware Context Not Passed**

   - The API returns a failure.
   - **Error Message**: "Cannot enable force firmware download without passing the firmware context."

2. **Force Firmware Enabled, Firmware Context Passed**

   - Firmware is updated unconditionally.
   - No version check is performed.

3. **Force Firmware Disabled, Firmware Context Passed**

   - Firmware version is checked.
   - If the version mismatches, the firmware is updated.

4. **Force Firmware Disabled, Firmware Context Not Passed**

   - Firmware version is checked using hardcoded macros.
   - The API returns a failure due to missing firmware context.

.. note::

   When integrating the firmware, users must update the following version macros in the ``Mainline_Firmware.h`` header file located in the binaries path :file:`uwbiot-top/firmware_images/SR2XX/`

   - ``#define UWB_SR2XX_MAJOR_NUMBER    0xXX``
   - ``#define UWB_SR2XX_MINOR_NUMBER    0xXX``
   - ``#define UWB_SR2XX_PATCH_NUMBER    0xXX``

   These macros define the expected firmware version and are essential for ensuring compatibility between the middleware (MW) and the firmware (FW).
   If the firmware has already been updated and the user does **not** wish to re-download it every time, the firmware context can be skipped during
   initialization.However, in such cases, these macros **must** be correctly updated to reflect the actual firmware version present on the device.
   Failure to do so will result in a version mismatch error, as the system will detect incompatibility between MW and FW versions.

How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. only:: sr250 and (not zephyr_sr250) and ( not sr250_frdm_rw612)

    - For RTOS based platform refer :ref:`virgo-McuXpresso-project-McuXpresso-project`

.. only:: sr200s

    - For RTOS based platform refer :ref:`virgo-sn-McuXpresso-project`

.. only:: sr250_frdm_rw612

    - For Zephyr based platform, refer the **Build Demo Application** section from **Getting_started guide**

.. only:: zephyr_sr250

    - For Zephyr RTOS based platform refer :ref:`virgo-Zephyr-McuXpresso-project`.

.. only:: porting_guide

    .. code-block:: bash

        west build -b frdm_rw612 demos/SR2XX/demo_sr2xx_fw_update/zephyr -p

- Source:   ``demo_sr2xx_fw_update``


How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

.. only:: zephyr_sr250 or sr250_frdm_rw612

    - For Zephyr RTOS based device, flash the ``demo_sr2xx_fw_update.bin`` file.

.. only:: (not zephyr_sr250) and (not sr250_frdm_rw612)

    - For embedded RTOS based device, flash the ``demo_sr2xx_fw_update.bin`` file.
    - On linux platform run the built executable.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

   #################################################
   ## demo_sr2xx_fw_download : SR250
   ## UWBIOT_v05.08.00
   #################################################
   TMLUWB  :WARN :SPI Frequency = 16000000
   FWDNLD  :INFO :phNxpUciHal_fw_download enter.....
   FWDNLD  :INFO :RECV                :00048101 02007DC8
   FWDNLD  :INFO :RECV                :00048103 020013A8
   FWDNLD  :INFO :RECV                :00048101 02007DC8
   FWDNLD  :INFO :RECV                :00048103 020013A8
   FWDNLD  :INFO :=====================GET_INFO =======================
   FWDNLD  :INFO : "Session Control Open"
   FWDNLD  :INFO : "ROM Version A1V2"
   FWDNLD  :INFO : "N-1/N-2 Page is OK"
   FWDNLD  :INFO :FW Version: Major.Minor: 01.31
   FWDNLD  :INFO : "Lifecycle Customer"
   FWDNLD  :INFO :=====================GET_INFO_EXT =======================

   FWDNLD  :INFO :Fw Rc version value: 0xC5

   FWDNLD  :INFO :mw_fw_ver: 01.31 chip_fw_ver: 01.31
   FWDNLD  :INFO :mw_fw_patch_ver: FF chip_fw_patch_ver: C5
   FWDNLD  :INFO :Patch version Mismatch FW Update required
   FWDNLD  :INFO :phLoadFwBinary() status - 0
   FWDNLD  :INFO :phNxpUciHal_fw_download completed.....
   FWDNLD  :INFO :RECV                :00048101 02007DC8
   HALUCI  :RX < :RECV                :60010001 01
   TMLUWB  :TX > :SEND                :2E000002 0000
   HALUCI  :RX < :RECV                :60070001 0A
   TMLUWB  :TX > :SEND                :2E000002 0000
   HALUCI  :RX < :RECV                :4E000001 00
   HALUCI  :RX < :RECV                :60010001 01
   TMLUWB  :TX > :SEND                :20000001 00
   HALUCI  :RX < :RECV                :60070001 0A
   TMLUWB  :TX > :SEND                :20000001 00
   HALUCI  :RX < :RECV                :40000001 00
   HALUCI  :RX < :RECV                :60010001 01
   TMLUWB  :TX > :SEND                :20040012 03E40201 00E40402 F401E43A 0500E803 E803
   HALUCI  :RX < :RECV                :6E060001 02
   UWBAPI  :WARN :processProprietaryNtf: unhandled oid 0x6
   HALUCI  :RX < :RECV                :40040002 0000
   TMLUWB  :TX > :SEND                :2004003D 03E46019 04010103 00030002 02030003 00030303 00030004 04000000 00E4630D 02010100 00000002 02030003 00E4620D 02010102 00000002 00020300 00
   HALUCI  :RX < :RECV                :40040002 0000
   TMLUWB  :TX > :SEND                :20040004 01010101
   HALUCI  :RX < :RECV                :40040002 0000
   TMLUWB  :TX > :SEND                :20040006 01E40402 F401
   HALUCI  :RX < :RECV                :40040002 0000
   TMLUWB  :TX > :SEND                :20040005 01E43301 01
   HALUCI  :RX < :RECV                :40040002 0000
   TMLUWB  :TX > :SEND                :20030000
   HALUCI  :RX < :RECV                :40030093 00280002 F7070102 F7070204 01000200 03040100 02000401 0B0502AF 0006027F 0607011F 08010709 01020A01 070B0101 0C01010D 01010E01 090F010B 10010311 013F1205 AFAAAAAA 0113010F 14010015 01011601 0317010A 18010019 01001A01 001B020D 551C0101 A0017FA1 04000F00 00A2011F A30103A4 020001A5 04000001 00A60300 1122A701 01B10200 08B201AE B3010C
   TMLUWB  :TX > :SEND                :20020000
   HALUCI  :RX < :RECV                :4002007C 00030002 00020002 0072000F 53523235 305F4131 56325F50 524F4401 030131FF 02030200 23031054 4E334132 32303944 35A20127 00370004 01640502 00000604 A5A5A5A5 60030300 00610303 00006211 62383963 33346637 37323566 37643038 2D630101 A0020203 A108312E 302E3000 0000A208 302E372E 35000000
   APP     :INFO :Device Name                    : SR250_A1V2_PROD

   APP     :INFO :Firmware Version               : 01.31.FF

   APP     :INFO :Finished C:uwbiot-top/demos/SR2XX/demo_sr2xx_fw_update/demo_sr2xx_fw_update.c : Success!

If such a log is not seen, re-run the program.

.. .. addLineBreaks 27
