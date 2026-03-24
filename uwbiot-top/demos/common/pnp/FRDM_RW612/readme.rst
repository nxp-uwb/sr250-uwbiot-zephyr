..
    Copyright 2025-2026 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _pnp-fw-frdm-rw612:

=======================================================================
 FRDM_RW612 Plug-n-Play FW
=======================================================================

.. brief:start

This firmware enables PC applications to run over the FRDM_RW612 board. The FRDM_RW612
board runs the PnP firmware, which acts as a bridge between PC applications
and the SR250 chip. PC applications send commands over UART to the PnP firmware,
which forwards them to the SR250 and relays responses and notifications back
to the PC.

.. brief:end

How to Use
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The PnP FW supports internal FW download and also from application,
by default internal firmware download is disabled.
to enable the internal firmware download, enable following macro.

.. literalinclude:: /_source/libs/halimpl/inc/phUwb_BuildConfig.h
    :language: c
    :start-after: /* doc:start:enable-int-fw-download */
    :end-before: /* doc:end:enable-int-fw-download */

1. Flash the FW on to FRDM_RW612 board.

2. Pre-built firmware is present in :file:`binaries/FRDM_RW612` directory.

3. On PC, set environment variable ``UWBIOT_ENV_COM`` to the FRDM_RW612 UART *COMPORT* and execute the application.
