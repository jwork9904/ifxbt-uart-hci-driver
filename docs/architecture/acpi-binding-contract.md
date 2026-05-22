# Phase 14P ACPI Binding Contract

This driver must bind to the Infineon CYW55571 Bluetooth client/function ACPI
node, not directly to the lower UART controller. Until Advantech/TL provide the
real firmware data, the active placeholder binding is:

```text
ACPI\PLACEHOLDER_CYW55571_BT_UART
```

Pending platform values are explicitly tracked as:

```text
TODO_REAL_BT_CLIENT_ACPI_HID
TODO_REAL_BT_CLIENT_ACPI_CID
TODO_REAL_BT_CLIENT_ACPI_CRS
```

Known board evidence from TL/Advantech inspection identifies `ACPI\QCOM0E16\7`
as the UAR7 lower Qualcomm UART controller. That node is controlled by `qcuart`
and must remain the lower UART controller unless the platform team provides a
different contract.

The eventual Bluetooth client node `_CRS` must expose a UART
SerialBus/ResourceHub connection to UAR7 / `ACPI\QCOM0E16`. IfxBtUartHci should
consume that Bluetooth client/function node contract and must not replace
`qcuart` or bind directly to `ACPI\QCOM0E16` on the main path.

Runtime behavior remains placeholder-only until real board data arrives: no UART
configuration, GPIO control, firmware/vendor command sequence, HCI Reset, BTHX
change, child PDO ID change, or parent transport Operational transition is
authorized by this contract scaffold.
