..
    Copyright 2024-2026 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _sr2xx-demo-device-calibration:

=======================================================================
 Demo Device Calibration
=======================================================================

.. brief:start

This demo showcases setting of device calibration into the flash.

This demo is added to support during Customer or before life cycle.

.. warning:: This demo is not applicable in "Protected" life cycle.

.. brief:end

The demo sets a predefined calibration parameters using UwbApi_SetCalibration and validates if the set values by calling the UwbApi_GetCalibration(). Following sequence
of steps are handled.

- Initialize UWBD in Mainline Firmware
- Sets the predefined calibration values by calling UwbApi_SetCalibration().
- Gets the calibration values by calling UwbApi_GetCalibration().
- Validates both the calibration values.

.. note:: During the MW Init, Default Calibration will not execute.

Host-Specific Calibration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The calibration values are host-specific and loaded from host-scoped files:

.. only:: sr250_virgo

    - Calibration File: :file:`uwbiot-top/boards/Host/Virgo/include/UWB_DeviceCalib_values.h`
    - Applies calibration to **Channel 5 and Channel 9**

.. only:: zephyr_sr250

    - Calibration File: :file:`uwbiot-top/boards/Host/nxp/virgo/include/UWB_DeviceCalib_values.h`
    - Applies calibration to **Channel 5 and Channel 9**

.. only:: sr250_frdm_rw612

    - Calibration File::file:`uwbiot-top/boards/Host/nxp/frdm_rw612/include/UWB_DeviceCalib_values.h`
    - Applies calibration to **Channel 9 only**

.. only:: nxp

    Configuring Host Type for PC Windows
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    For PC Windows builds, you must manually select the host type before running the demo.

    Edit the file: ``boards/Host/PCWindows/include/UWB_DeviceConfig.h``

    Enable **ONLY ONE** of the following options:

    .. only:: sr250_virgo

        .. code-block:: c

            #define UWBIOT_PC_WIN_HOST_VIRGO 1
            #define UWBIOT_PC_WIN_HOST_FRDM 0

    .. only:: sr250_frdm_rw612

        .. code-block:: c

            #define UWBIOT_PC_WIN_HOST_VIRGO 0
            #define UWBIOT_PC_WIN_HOST_FRDM 1

    .. warning::
        Ensure only one host type is enabled (set to 1) at a time.
        The other must be set to 0.

Updating Calibration Values
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To update or customize calibration values:

1. **Identify your host type**

2. **Locate the calibration file:**

    .. only:: sr250_virgo

        - Calibration File: :file:`uwbiot-top/boards/Host/Virgo/include/UWB_DeviceCalib_values.h`
        - Applies calibration to **Channel 5 and Channel 9**

    .. only:: zephyr_sr250

        - Calibration File: :file:`uwbiot-top/boards/Host/nxp/virgo/include/UWB_DeviceCalib_values.h`
        - Applies calibration to **Channel 5 and Channel 9**

    .. only:: sr250_frdm_rw612

        - Calibration File::file:`uwbiot-top/boards/Host/nxp/frdm_rw612/include/UWB_DeviceCalib_values.h`
        - Applies calibration to **Channel 9 only**

3. **Edit the calibration parameters** in the respective file

4. **Rebuild the demo** after making changes


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. only:: sr250 and (not zephyr_sr250) and ( not sr250_frdm_rw612)

    - For RTOS based platform refer :ref:`virgo-McuXpresso-project`.
    - For embed linux Raspberry-Pi with Virgo setup :ref:`build-pi-virgo`

.. only:: sr250_frdm_rw612

    - For Zephyr based platform, refer the **Build Demo Application** section from **Getting_started guide**

.. only:: zephyr_sr250

    - For Zephyr RTOS based platform refer :ref:`virgo-Zephyr-McuXpresso-project`.

.. only:: sr200s

    - For RTOS based platform refer :ref:`virgo-sn-McuXpresso-project`

- Source:   ``demo_device_calibration``

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Steps to be followed to run:

.. only:: zephyr_sr250 or sr250_frdm_rw612

    - For Zephyr RTOS based device, flash the ``demo_device_calibration.bin`` file.

.. only:: (not zephyr_sr250) and (not sr250_frdm_rw612)

    - For embedded RTOS based device, flash the ``demo_device_calibration.bin`` file.
    - On linux platform run the built executable.

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    #################################################
    ## Demo Device Calibration : SR250
    ## UWBIOT_v05.08.00
    #################################################
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
    FWDNLD  :INFO :mw_fw_patch_ver: C5 chip_fw_patch_ver: C5
    FWDNLD  :INFO :FW Update not required
    FWDNLD  :INFO :Same FW version found skipping FW download
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
    TMLUWB  :TX > :SEND                :20040012 03E40201 00E40402 F401E43A 0500E803 E800
    HALUCI  :RX < :RECV                :6E060001 02
    UWBAPI  :WARN :processProprietaryNtf: unhandled oid 0x6
    HALUCI  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :2004003D 03E46019 04010103 00030002 02030003 00030303 00030004 04000000 00E4630D 02010100 00000002 02030003 00E4620D 02010102 00000002 00020300 00
    HALUCI  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20040004 01010100
    HALUCI  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20040006 01E40402 F400
    HALUCI  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20040005 01E43301 00
    HALUCI  :RX < :RECV                :40040002 0000
    TMLUWB  :TX > :SEND                :20030000
    HALUCI  :RX < :RECV                :40030093 00280002 F7070102 F7070204 01000200 03040100 02000401 0B0502AF 0006027F 0607011F 08010709 01020A01 070B0101 0C01010D 01010E01 090F010B 10010311 013F1205 AFAAAAAA 0113010F 14010015 01011601 0317010A 18010019 01001A01 001B020D 551C0101 A0017FA1 04000F00 00A2011F A30103A4 020001A5 04000001 00A60300 1122A701 01B10200 08B201AE B3010C
    UWBAPI  :WARN :parseCapabilityInfo: unknown param Id 0xA0
    UWBAPI  :WARN :parseCapabilityInfo: unknown param Id 0xA1
    UWBAPI  :WARN :parseCapabilityInfo: unknown param Id 0xA2
    UWBAPI  :WARN :parseCapabilityInfo: unknown param Id 0xA3
    UWBAPI  :WARN :parseCapabilityInfo: unknown param Id 0xA4
    UWBAPI  :WARN :parseCapabilityInfo: unknown param Id 0xA5
    UWBAPI  :WARN :parseCapabilityInfo: unknown param Id 0xA6
    UWBAPI  :WARN :parseCapabilityInfo: unknown param Id 0xA7
    TMLUWB  :TX > :SEND                :20020000
    HALUCI  :RX < :RECV                :4002007C 00030002 00020002 0072000F 53523235 305F4131 56325F50 524F4401 030131C5 02030200 23031054 4E334132 32303944 35A2011D 003B0004 01640502 00000604 A5A5A5A5 60030300 00610303 00006211 63343861 65373263 34353033 61303234 2D630101 A0020203 A108312E 302E3000 0000A208 302E372E 35000000
    UWBAPI  :WARN :parseDeviceInfo: unknown param Id 0xA0
    UWBAPI  :WARN :parseDeviceInfo: unknown param Id 0xA1
    UWBAPI  :WARN :parseDeviceInfo: unknown param Id 0xA2
    APP     :INFO :Device Name                    : SR250_A1V2_PROD

    APP     :INFO :Firmware Version               : 01.31.C5

    APP     :WARN :LifeCycle :0xA5A5A5A5

    APP     :WARN :Setting : DEMO_RF_CLK_ACCURACY_CALIB
    TMLUWB  :TX > :SEND                :2F21000A 00010703 25002500 0400
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_RF_CLK_ACCURACY_CALIB
    TMLUWB  :TX > :SEND                :2F220002 0000
    HALUCI  :RX < :RECV                :4F22000B 00010107 03250025 000400
    APP     :WARN :Setting : DEMO_RX_ANT_DELAY_CALIB_CH5
    TMLUWB  :TX > :SEND                :2F210010 05020D04 01784802 72480376 48046900
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_RX_ANT_DELAY_CALIB_CH5
    TMLUWB  :TX > :SEND                :2F220002 0500
    HALUCI  :RX < :RECV                :4F220011 0003020D 04017848 02724803 76480469 48
    APP     :WARN :Setting : DEMO_RX_ANT_DELAY_CALIB_CH9
    TMLUWB  :TX > :SEND                :2F210010 09020D04 01454802 4148034E 48045400
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_RX_ANT_DELAY_CALIB_CH9
    TMLUWB  :TX > :SEND                :2F220002 0900
    HALUCI  :RX < :RECV                :4F220011 0003020D 04014548 02414803 4E480454 48
    APP     :WARN :Setting : DEMO_PDOA_OFFSET_CALIB_CH5
    TMLUWB  :TX > :SEND                :2F21000A 05030702 01D23A02 4600
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_PDOA_OFFSET_CALIB_CH5
    TMLUWB  :TX > :SEND                :2F220002 0500
    HALUCI  :RX < :RECV                :4F22000B 00010307 0201D23A 0246C2
    APP     :WARN :Setting : DEMO_PDOA_OFFSET_CALIB_CH9
    TMLUWB  :TX > :SEND                :2F21000A 09030702 019A4F02 6D00
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_PDOA_OFFSET_CALIB_CH9
    TMLUWB  :TX > :SEND                :2F220002 0900
    HALUCI  :RX < :RECV                :4F22000B 00010307 02019A4F 026D4B
    APP     :WARN :Setting : DEMO_TX_POWER_PER_ANTENNA_CH5
    TMLUWB  :TX > :SEND                :2F21000E 05040B02 0103002A 00020300 2C00
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_TX_POWER_PER_ANTENNA_CH5
    TMLUWB  :TX > :SEND                :2F220002 0500
    HALUCI  :RX < :RECV                :4F22000F 0001040B 02010300 2A000203 002C00
    APP     :WARN :Setting : DEMO_TX_POWER_PER_ANTENNA_CH9
    TMLUWB  :TX > :SEND                :2F21000E 09040B02 0103002F 00020300 3200
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_TX_POWER_PER_ANTENNA_CH9
    TMLUWB  :TX > :SEND                :2F220002 0900
    HALUCI  :RX < :RECV                :4F22000F 0001040B 02010300 2F000203 003200
    APP     :WARN :Setting : DEMO_AOA_ANTENNAS_PDOA_CALIB_AP1_CH5
    TMLUWB  :TX > :SEND                :2F2100F7 0562F401 01493BC3 2FFE2588 1B720F1D 00C4F168 E5B1DC39 D235C1CA 38F12ED3 248F1961 0EF6FFD8 F147E59B DB28D13D C13C37EB 2E0E2519 1A690E00 0038F194 E571DB80 D0B9C1B5 37952E16 26A21A54 0D96FF90 F0A3E5BF DACFCF8C C5123B84 2F552851 1B2E0C3A FF7CF0B7 E552D947 CFCBC7C3 41E23406 2E991F84 10BF004B F249E741 D925D367 CE5546CD 34C02CAF 22481392 02BEF404 E9CADB2C D96BD6E9 43152DA7 1FC61BD8 0E4701FA F4B0E9A8 DF01DD5A DAB2405D 2E1821EF 15070C38 FD07F1B3 E59BDE5F D9EED654 3E3236EF 2A031706 0DB7FC21 ED14DDA3 D763CFA4 C9C9375F 40983652 1C310E29 036FF3BC E03AD5EE C5AB00
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_AOA_ANTENNAS_PDOA_CALIB_AP1_CH5
    TMLUWB  :TX > :SEND                :2F220003 056200
    HALUCI  :RX < :RECV                :4F2200F8 000162F4 0101493B C32FFE25 881B720F 1D00C4F1 68E5B1DC 39D235C1 CA38F12E D3248F19 610EF6FF D8F147E5 9BDB28D1 3DC13C37 EB2E0E25 191A690E 000038F1 94E571DB 80D0B9C1 B537952E 1626A21A 540D96FF 90F0A3E5 BFDACFCF 8CC5123B 842F5528 511B2E0C 3AFF7CF0 B7E552D9 47CFCBC7 C341E234 062E991F 8410BF00 4BF249E7 41D925D3 67CE5546 CD34C02C AF224813 9202BEF4 04E9CADB 2CD96BD6 E943152D A71FC61B D80E4701 FAF4B0E9 A8DF01DD 5ADAB240 5D2E1821 EF15070C 38FD07F1 B3E59BDE 5FD9EED6 543E3236 EF2A0317 060DB7FC 21ED14DD A3D763CF A4C9C937 5F409836 521C310E 29036FF3 BCE03AD5 EEC5ABB9
    APP     :WARN :Setting : DEMO_AOA_ANTENNAS_PDOA_CALIB_AP1_CH9
    TMLUWB  :TX > :SEND                :2F2100F7 0962F401 016749E3 380B2E3B 1E54148C 0BEFF42F DE5CCE7B BF18B75A 45513AF4 2F1C23CE 15A40B30 F7A9E4AE D2CDC2D9 B9BC4349 3B7C2FEB 22D415A5 0850F6CB E45ED23B C4B6B819 43523AA7 2E0522DE 132C03D0 F26DE23C D1B4C495 B9DC429F 3A5D2F08 232E11F0 00C4F2D1 E088CF18 C38EB8CA 41D938B4 2C28203C 103A028F F299DD24 CD83C29E B6853FE7 36A22A61 1EBB11FB 03A1F23A DDF4CC72 C199B659 3D163712 2B4C1D90 10260600 F48BDE0F CE28C249 B76D3CD7 37EE29F2 1CA6104D 06E4F450 DE30CF5A C2CFB976 3B223676 2A9F1E84 12B8046C F617DCDA CED7C179 BC07385E 33132DA1 1B5E1503 099DF3E7 D89ECF40 C20500
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_AOA_ANTENNAS_PDOA_CALIB_AP1_CH9
    TMLUWB  :TX > :SEND                :2F220003 096200
    HALUCI  :RX < :RECV                :4F2200F8 000162F4 01016749 E3380B2E 3B1E5414 8C0BEFF4 2FDE5CCE 7BBF18B7 5A45513A F42F1C23 CE15A40B 30F7A9E4 AED2CDC2 D9B9BC43 493B7C2F EB22D415 A50850F6 CBE45ED2 3BC4B6B8 1943523A A72E0522 DE132C03 D0F26DE2 3CD1B4C4 95B9DC42 9F3A5D2F 08232E11 F000C4F2 D1E088CF 18C38EB8 CA41D938 B42C2820 3C103A02 8FF299DD 24CD83C2 9EB6853F E736A22A 611EBB11 FB03A1F2 3ADDF4CC 72C199B6 593D1637 122B4C1D 90102606 00F48BDE 0FCE28C2 49B76D3C D737EE29 F21CA610 4D06E4F4 50DE30CF 5AC2CFB9 763B2236 762A9F1E 8412B804 6CF617DC DACED7C1 79BC0738 5E33132D A11B5E15 03099DF3 E7D89ECF 40C205C0
    APP     :WARN :Setting : DEMO_AOA_ANTENNAS_PDOA_CALIB_AP2_CH5
    TMLUWB  :TX > :SEND                :2F2100F7 0562F401 0202CE24 D072D454 D9AADFBD E79BEF2F F7D3FEEA 09971BB1 C656C8DE CBF0D2A9 DCCAE637 F272FFE6 06510AFE 0A88C9F4 CCA3CF31 D660E0FE EAA2F7CB 0E9C1A2E 1F071F15 C7F2CB9A CE82D4A8 E088EDC9 FB181460 2AC033C6 2FF2BFC4 C41CC9E2 D07FDF5D EDF200E0 18902A1C 31B6305B BE81C292 C9C9D3CB DF44EF43 05591B96 28DE3044 3395BECA C49BCE81 D875E194 EDD20344 18F12202 2FD83B8B C139CB7F D433DB93 DFD8EADE 01021346 1A442DA5 41BEC007 CECED797 DE7EE42E F1C60214 0FA413CF 26D53E75 C90FD448 DD19E44D EA49F2D4 FB6D038C 09031C81 306CE663 EBDCF0D6 EFD3F19A F416F9F1 FB16FE45 0BE300
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_AOA_ANTENNAS_PDOA_CALIB_AP2_CH5
    TMLUWB  :TX > :SEND                :2F220003 056200
    HALUCI  :RX < :RECV                :4F2200F8 000162F4 010202CE 24D072D4 54D9AADF BDE79BEF 2FF7D3FE EA09971B B1C656C8 DECBF0D2 A9DCCAE6 37F272FF E606510A FE0A88C9 F4CCA3CF 31D660E0 FEEAA2F7 CB0E9C1A 2E1F071F 15C7F2CB 9ACE82D4 A8E088ED C9FB1814 602AC033 C62FF2BF C4C41CC9 E2D07FDF 5DEDF200 E018902A 1C31B630 5BBE81C2 92C9C9D3 CBDF44EF 4305591B 9628DE30 443395BE CAC49BCE 81D875E1 94EDD203 4418F122 022FD83B 8BC139CB 7FD433DB 93DFD8EA DE010213 461A442D A541BEC0 07CECED7 97DE7EE4 2EF1C602 140FA413 CF26D53E 75C90FD4 48DD19E4 4DEA49F2 D4FB6D03 8C09031C 81306CE6 63EBDCF0 D6EFD3F1 9AF416F9 F1FB16FE 450BE332
    APP     :WARN :Setting : DEMO_AOA_ANTENNAS_PDOA_CALIB_AP2_CH9
    TMLUWB  :TX > :SEND                :2F2100F7 0962F401 0235DFA7 E812F273 FA2B0103 07C00FAB 1BEB2672 2DD72F1A D719E2A2 EBE1F6F7 FEE3079F 154D214C 27C52E63 38AAD0B8 DA6EE5D0 F374FC3D 08571A51 27392DB1 33993C41 C85FCF29 DFC6F04A FB7809A2 1D722894 2E07397E 42D6C594 CE61D84F EAC2FAB3 08E819C6 267A30E1 38AE3EBF C369CE73 D9F1ED66 FD9E08E0 18ED2398 30B33996 3C82BD19 C931D94D EC2DF84A 05A41763 250B3123 37953E17 C4BEC865 DB06EDD3 F64A06BB 17572474 2F7D362D 3852D13B D6C3E4EE F031FB6F 08CB1540 1EB123D8 2B3E3C2B DF12E632 ED71F6F0 FF3809B0 13FF1C7B 251A2DFC 2E8FEA2B EDE8F566 FC6D0338 0B8A12DA 161A1A35 1CCF00
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_AOA_ANTENNAS_PDOA_CALIB_AP2_CH9
    TMLUWB  :TX > :SEND                :2F220003 096200
    HALUCI  :RX < :RECV                :4F2200F8 000162F4 010235DF A7E812F2 73FA2B01 0307C00F AB1BEB26 722DD72F 1AD719E2 A2EBE1F6 F7FEE307 9F154D21 4C27C52E 6338AAD0 B8DA6EE5 D0F374FC 3D08571A 5127392D B133993C 41C85FCF 29DFC6F0 4AFB7809 A21D7228 942E0739 7E42D6C5 94CE61D8 4FEAC2FA B308E819 C6267A30 E138AE3E BFC369CE 73D9F1ED 66FD9E08 E018ED23 9830B339 963C82BD 19C931D9 4DEC2DF8 4A05A417 63250B31 2337953E 17C4BEC8 65DB06ED D3F64A06 BB175724 742F7D36 2D3852D1 3BD6C3E4 EEF031FB 6F08CB15 401EB123 D82B3E3C 2BDF12E6 32ED71F6 F0FF3809 B013FF1C 7B251A2D FC2E8FEA 2BEDE8F5 66FC6D03 380B8A12 DA161A1A 351CCF1D
    APP     :WARN :Setting : DEMO_TX_ANT_DELAY_CALIB_CH5
    TMLUWB  :TX > :SEND                :2F210007 05640401 01E800
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_TX_ANT_DELAY_CALIB_CH5
    TMLUWB  :TX > :SEND                :2F220002 0500
    HALUCI  :RX < :RECV                :4F220008 00016404 0101E8FF
    APP     :WARN :Setting : DEMO_TX_ANT_DELAY_CALIB_CH9
    TMLUWB  :TX > :SEND                :2F210007 09640401 01F800
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_TX_ANT_DELAY_CALIB_CH9
    TMLUWB  :TX > :SEND                :2F220002 0900
    HALUCI  :RX < :RECV                :4F220008 00016404 0101F8FF
    APP     :WARN :Setting : DEMO_PDOA_MANUFACT_ZERO_OFFSET_CALIB_CH5
    TMLUWB  :TX > :SEND                :2F21000A 05650702 01000002 0000
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_PDOA_MANUFACT_ZERO_OFFSET_CALIB_CH5
    TMLUWB  :TX > :SEND                :2F220002 0500
    HALUCI  :RX < :RECV                :4F22000B 00016507 02010000 020000
    APP     :WARN :Setting : DEMO_PDOA_MANUFACT_ZERO_OFFSET_CALIB_CH9
    TMLUWB  :TX > :SEND                :2F21000A 09650702 01000002 0000
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_PDOA_MANUFACT_ZERO_OFFSET_CALIB_CH9
    TMLUWB  :TX > :SEND                :2F220002 0900
    HALUCI  :RX < :RECV                :4F22000B 00016507 02010000 020000
    APP     :WARN :Setting : DEMO_AOA_THRESHOLD_PDOA_CH5
    TMLUWB  :TX > :SEND                :2F21000A 05660702 01D3E002 4500
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_AOA_THRESHOLD_PDOA_CH5
    TMLUWB  :TX > :SEND                :2F220002 0500
    HALUCI  :RX < :RECV                :4F22000B 00016607 0201D3E0 02451C
    APP     :WARN :Setting : DEMO_AOA_THRESHOLD_PDOA_CH9
    TMLUWB  :TX > :SEND                :2F21000A 09660702 019BF502 6E00
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_AOA_THRESHOLD_PDOA_CH9
    TMLUWB  :TX > :SEND                :2F220002 0900
    HALUCI  :RX < :RECV                :4F22000B 00016607 02019BF5 026EF1
    APP     :WARN :Setting : DEMO_TX_TEMPERATURE_COMP_PER_ANTENNA_CH5
    TMLUWB  :TX > :SEND                :2F210016 05671302 01800080 00800080 00028000 80008000 8000
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_TX_TEMPERATURE_COMP_PER_ANTENNA_CH5
    TMLUWB  :TX > :SEND                :2F220002 0500
    HALUCI  :RX < :RECV                :4F220017 00016713 02018000 80008000 80000280 00800080 008000
    APP     :WARN :Setting : DEMO_TX_TEMPERATURE_COMP_PER_ANTENNA_CH9
    TMLUWB  :TX > :SEND                :2F210016 09671302 01800080 00800080 00028000 80008000 8000
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_TX_TEMPERATURE_COMP_PER_ANTENNA_CH9
    TMLUWB  :TX > :SEND                :2F220002 0900
    HALUCI  :RX < :RECV                :4F220017 00016713 02018000 80008000 80000280 00800080 008000
    APP     :WARN :Setting : DEMO_RSSI_CALIB_CONSTANT_PER_ANTENNA_CH5
    TMLUWB  :TX > :SEND                :2F210008 05690502 01000200
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_RSSI_CALIB_CONSTANT_PER_ANTENNA_CH5
    TMLUWB  :TX > :SEND                :2F220002 0500
    HALUCI  :RX < :RECV                :4F220009 00016905 02010002 00
    APP     :WARN :Setting : DEMO_RSSI_CALIB_CONSTANT_PER_ANTENNA_CH9
    TMLUWB  :TX > :SEND                :2F210008 09690502 01000200
    HALUCI  :RX < :RECV                :4F210001 00
    APP     :WARN :Getting : DEMO_RSSI_CALIB_CONSTANT_PER_ANTENNA_CH9
    TMLUWB  :TX > :SEND                :2F220002 0900
    HALUCI  :RX < :RECV                :4F220009 00016905 02010002 00
    APP     :INFO :Finished /opt/samba/nxf88846/uwb/_ddm/repo1/uwbiot-top/demos/SR2XX/demo_device_calibration/demo_device_calibration.c : Success!
