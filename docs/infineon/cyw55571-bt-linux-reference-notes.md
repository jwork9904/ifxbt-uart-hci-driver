\# CYW55571 Bluetooth Linux Reference Notes



Source:

infineon-getting-started-with-wi-fi-and-bluetooth-combo-chip-on-imx8-nano-developers-kit-v3-in-linux-usermanual-en.pdf



Relevant sections:

\- 9.1 Bring up CYW5557x for Bluetooth

\- 9.2.3 BTSTACK porting layer

\- 9.3.1 Bluetooth firmware download sequence

\- 9.3.2 Programming Bluetooth firmware using MBT

\- 9.3.3 Antenna configuration

\- 9.4 BlueZ support



Important extracted facts:

1\. BT\_REG\_ON GPIO status must be verified.

2\. VDDIO and VBAT are toggled.

3\. CYW5557x UART CTS is pulled low.

4\. BT\_REG\_ON is toggled LOW to HIGH.

5\. This puts the device into auto-baud mode.

6\. HCI Reset is sent to autodetect baud.

7\. Firmware download must begin within 3 seconds after the reset/autobaud step.

8\. Firmware file is .hcd.

9\. Linux uses MBT and bt\_load.sh to download firmware.

10\. Firmware file is renamed FW.hcd in the Linux flow.

11\. BlueZ later uses:

&#x20;   hciattach -s 115200 /dev/ttymxc0 bcm43xx

12\. HCI H4 packet type indicator is a 1-byte prefix before HCI packet.



Windows translation:

\- GPIO handling must come from ARM64 QCA board ACPI resources.

\- UART path must come from ACPI UART resource.

\- .hcd firmware parser/download logic is Infineon-specific and must not be invented.

\- MBT source/scripts are required before implementing firmware download.

\- HCI Reset milestone must pass before firmware download implementation.

