..
    Copyright 2024-2026 NXP

    NXP Proprietary. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms.  By
    expressly accepting such terms or by downloading, installing, activating
    and/or otherwise using the software, you are agreeing that you have read,
    and that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you may
    not retain, install, activate or otherwise use the software.

.. include:: <isonum.txt>

.. _sr2xx-demo-nearby-interaction:

=======================================================================
 Demo nearby interaction
=======================================================================

.. brief:start

This demo showcases ranging via Bluetooth LE |trade| with background(pairing) feature with SR250 configured as
a either Controller - Initiator or Controlee - Responder.

.. brief:end

For details on Peer-to-Peer ranging sequence and configuration details
for SR250 refer to :ref:`p2p-ranging`.

IOS
=========

1) By default the UWB spec version is set to v1.1. To enable v1.0, we shall set the ``UWB_IOS_SPEC_VERSION_MINOR`` to ``0x00, 0x00`` in :file:`UwbApi_types.h`.

#) By default the Bonding and pairing capability is disabled, To enable them we shall set the below macros to ``1`` in :file:`app_preinclude.h`, at location

    .. only:: sr250 and (not sr250_frdm_rw612)

        ``boards/Host/Virgo``

    .. only:: sr250_frdm_rw612

        ``boards/Host/FRDM_RW612``

    ``#define CONFIG_BT_PAIRING 1``

    .. note:: In case of pairing With an IOS device in debug mode, it has been observed that the after pairing we don't receive any BLE message. If the device is already paired, then there is no issue observed.

Android
=========

1) By default the UWB spec version is set to v1.0 in :file:`UwbApi_types.h`.


.. note:: For multiple connection using ble, ``CONFIG_BT_MAX_CONN`` define needs to updated to max number of connections allowed in :file:`app_preinclude.h`, at location ``boards/Host/Virgo``  by default it's defined to 5

App Ranging with Bluetooth LE |trade| on Virgo-SR250/ FRDM_RW612-SR250
=======================================================================

In this demo, a smartphone first asks to pair the UWB device, once paired it sends commands over Bluetooth LE |trade| to Virgo/FRDM_RW612
to initialize and configure session and start ranging.

By Default the Low Power Mode is Enabled, To Disable the Low Power Mode we need to set the below macros as ``0``
inside

.. only:: sr250 and (not sr250_frdm_rw612)

    :file:`uwbiot-top/boards/Host/Virgo/app_preinclude.h`

.. only:: sr250_frdm_rw612

    :file:`uwbiot-top/boards/Host/FRDM_RW612/app_preinclude.h`

``#define APP_LOWPOWER_ENABLED 0``

For UWB and App developer specific changes refer
``/*Define for App developer*/`` & ``/*Define for UWB developer*/`` section in
``TLV_Mng.c`` file.

How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. only:: sr250 and ( not sr250_frdm_rw612)

    Build the Bluetooth LE |trade| based Ranging project for SR250 Using Virgo
    MCUXpresso Project:

.. only:: (sr250_frdm_rw612)

    Build the Bluetooth LE |trade| based Ranging application for SR250 Using FRDM_RW612

- Source:   ``demo_nearby_interaction``

.. only:: sr250 and (not zephyr_sr250) and ( not sr250_frdm_rw612)

    - For RTOS based platform refer :ref:`virgo-McuXpresso-project`.
    - For embed linux Raspberry-Pi with Virgo setup :ref:`build-pi-virgo`

.. only:: sr250_frdm_rw612

    - For Zephyr based platform, refer the **Build Demo Application** section from **Getting_started guide**

.. only:: zephyr_sr250

    - For Zephyr RTOS based platform refer :ref:`virgo-Zephyr-McuXpresso-project`.

.. only:: sr200s

    - For RTOS based platform refer :ref:`virgo-sn-McuXpresso-project`

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

.. only:: zephyr_sr250 or sr250_frdm_rw612

    - For Zephyr RTOS based device, flash the ``demo_nearby_interaction.bin`` file.

.. only:: (not zephyr_sr250) and (not sr250_frdm_rw612)

    - For embedded RTOS based device, flash the ``demo_nearby_interaction.bin`` file.

Flash BLE Firmware
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This step needs to be done only once. This is used for flashing the BLE firmware to the module so that it can run any BLE applications.

- In MCUXpresso Click on the GUI Flash Tool icon and select the BLE firmware ``uwbiot-top/ext/boards/RW612/components/conn_fwloader/fw_bin/rw61x_sb_ble_a2.bin``.
- Change the base address from “0x8000000” to “0x8540000”.
- click on ``Run``

SR250 configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

According to specification here are the configuration that can be changed:

- ``TLV_Types_i.h``
  -   DEVICE_ROLE::

        #define DEMO_DEVICE_TYPE kUWB_DeviceType_Controller     // Configure accessory as Controller
        #define DEMO_DEVICE_ROLE kUWB_DeviceRole_Initiator      // Configure accessory as Initiator
        // #define DEMO_DEVICE_TYPE kUWB_DeviceType_Controlee   // Configure accessory as Controlee
        // #define DEMO_DEVICE_ROLE kUWB_DeviceRole_Responder   // Configure accessory as Responder


- ``TLV_Mng.c``::

    uint8_t SpecMajorVersion[2] = {0x01, 0x00};                 // Spec major version
    uint8_t SpecMinorVersion[2] = {0x01, 0x00};                 // Spec minor version
    configData.PreferedUpdateRate                               // Prefered Update data rate (use PreferedUpdateRate_t definition)

.. addLineBreaks 24

Sequence Diagram
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: /architecture/arch/mwarch_files/nearbyInteraction.png
    :scale: 50%

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

After program execution, log message like this must be seen::

    #################################################
    ## Demo Nearby Interaction (BLE tracker) : SR250
    ## UWBIOT_v05.00.01
    #################################################
    APP     :WARN :device init
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
    FWDNLD  :INFO :FW Version: Major.Minor: 01.28
    FWDNLD  :INFO : "Lifecycle Customer"
    FWDNLD  :INFO :=====================GET_INFO_EXT =======================

    FWDNLD  :INFO :Fw Rc version value: 0xFE

    FWDNLD  :INFO :mw_fw_ver: 01.28 chip_fw_ver: 01.28
    FWDNLD  :INFO :mw_fw_patch_ver: FE chip_fw_patch_ver: FE
    FWDNLD  :INFO :FW Update not required
    FWDNLD  :INFO :Same FW version found skipping FW download
    FWDNLD  :INFO :RECV                :00048101 02007DC8
    HALUCI  :RX < :RECV                :60010001 01
    TMLUWB  :TX > :SEND                :2E000002 0000
    HALUCI  :WARN :hal_extns_write_rsp_timeout_cb - write timeout!!!
    TMLUWB  :TX > :SEND                :2E000002 0000
    HALUCI  :RX < :RECV                :60070001 0A
    TMLUWB  :TX > :SEND                :2E000002 0000
    HALUCI  :RX < :RECV                :4E000001 00
    HALUCI  :RX < :RECV                :60010001 01
    TMLUWB  :TX > :SEND                :20000001 00
    HALUCI  :WARN :hal_extns_write_rsp_timeout_cb - write timeout!!!
    TMLUWB  :TX > :SEND                :20000001 00
    HALUCI  :RX < :RECV                :60070001 0A
    TMLUWB  :TX > :SEND                :20000001 00
    HALUCI  :RX < :RECV                :40000001 00
    HALUCI  :RX < :RECV                :60010001 01
    TMLUWB  :TX > :SEND                :20040058 07E40201 00E40301 B4E40402 F401E460 1F050104 00000000 02030000 00000302 00000000 04010200 00000501 02000200 E4620D02 01000302 00000204 00020000 E4630D02 01010000 00000202 00000000 E43A0500 E803E803
    HALUCI  :RX < :RECV                :6E060001 02
    UWBAPI  :WARN :handle_binding_status_ntf: Binding Status : 2
    HALUCI  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :2F210013 05021005 01EE4702 0E4803FD 47040748 052648
    HALUCI  :RX < :RECV                :4F210001 00
    TMLUWB  :TX > :SEND                :2F210013 09021005 01F24702 DA470310 48041F48 051348
    HALUCI  :RX < :RECV                :4F210001 00
    TMLUWB  :TX > :SEND                :20040004 01010101
    HALUCI  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20040006 01E40402 F401
    HALUCI  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20040005 01E43301 01
    HALUCI  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20030000
    HALUCI  :RX < :RECV                :4003008A 00250002 F7070102 F7070204 01000200 03040100 02000401 030502AF 0006027F 0007011F 08010309 01020A01 070B0101 0C01010D 01010E01 090F010B 10010311 013F1205 FFFFFF7F 0013010F 14010115 01011601 0317010A 18010019 01001A01 00A0017F A104000F 0000A201 F8A30103 A4020001 A5020100 A6030011 22E30002 0008E301 01AEE302 010C
    APP     :INFO :BLE Connected to peer: 52:39:11:AA:39:51 (random), 1 peers connected
    APP     :INFO :Security changed: 52:39:11:AA:39:51 (random) level 2 (error 0)
    TMLUWB  :TX > :SEND                :2F020000
    HALUCI  :RX < :RECV                :60070001 0A
    UCICORE :WARN :Retrying last sent command as FW asked for retry (CORE_GENERIC_ERROR_NTF -> STATUS_MESSAGE_RETRY)
    TMLUWB  :TX > :SEND                :2F020000
    HALUCI  :RX < :RECV                :4F020002 0000
    TMLUWB  :TX > :SEND                :20020000
    HALUCI  :RX < :RECV                :40020072 00020002 00020002 0068000F 53523235 305F4131 56325F50 524F4401 030128FE 02030200 11031054 4E334132 32303944 35A2011D 00300004 01640502 00000604 A5A5A5A5 60030200 00610302 00006211 61376338 32333132 33323932 39393932 2D630101 A0020203 A108312E 302E3000 0000
    TMLUWB  :TX > :SEND                :2A020001 02
    HALUCI  :RX < :RECV                :4A020003 009E36
    APP     :INFO :mac addr :          :9E36
    TMLUWB  :TX > :SEND                :21000005 4AB1D7BF 00
    HALUCI  :RX < :RECV                :41000005 00010000 00
    HALUCI  :RX < :RECV                :61020006 01000000 0000
    TMLUWB  :TX > :SEND                :2103002B 01000000 090904F0 00000008 0260091B 01062806 01020304 05062702 08071401 0A040109 05010107 021122
    HALUCI  :RX < :RECV                :41030002 0000
    TMLUWB  :TX > :SEND                :2103001B 01000000 07110100 03010026 01002201 0106029E 36010102 000100
    HALUCI  :RX < :RECV                :41030002 0000
    HALUCI  :RX < :RECV                :61020006 01000000 0300
    TMLUWB  :TX > :SEND                :22000004 01000000
    HALUCI  :RX < :RECV                :42000001 00
    HALUCI  :RX < :RECV                :60010001 02
    HALUCI  :RX < :RECV                :61020006 01000000 0200
    HALUCI  :RX < :RECV                :62000048 00000000 01000000 00F00000 00010000 00000000 00000000 01112221 00FFFF00 00000000 00000000 00000000 00000000 00000000 00000000 0E000000 00010100 01010000 00000000
    HALUCI  :RX < :RECV                :62000048 01000000 01000000 00F00000 00010000 00000000 00000000 01112221 00FFFF00 00000000 00000000 00000000 00000000 00000000 00000000 0E000000 00010100 01010000 00000000
    HALUCI  :RX < :RECV                :62000048 02000000 01000000 00F00000 00010000 00000000 00000000 01112221 00FFFF00 00000000 00000000 00000000 00000000 00000000 00000000 0E000000 00010100 01010000 00000000
    .
    .
    HALUCI  :RX < :RECV                :62000048 03000000 01000000 00F00000 00010000 00000000 00000000 01112221 00FFFF00 00000000 00000000 00000000 00000000 00000000 00000000 0E000000 00010100 01010000 00000000
    TMLUWB  :TX > :SEND                :22010004 01000000
    HALUCI  :RX < :RECV                :42010001 00
    HALUCI  :RX < :RECV                :61020006 01000000 0300
    TMLUWB  :TX > :SEND                :21010004 01000000
    HALUCI  :RX < :RECV                :60010001 01
    HALUCI  :RX < :RECV                :41010001 00
    HALUCI  :RX < :RECV                :61020006 01000000 0100
    APP     :INFO :BLE Disconnected peer 52:39:11:AA:39:51 (random), 0 peers connected

    APP     :WARN :device deinit
    APP     :INFO :BLE Disconnected (reason 19)

If such a log is not seen, re-run the program.
