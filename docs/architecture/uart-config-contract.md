# Phase 15P UART Configuration Contract

IfxBtUartHci owns the Bluetooth UART transport configuration only after the
platform layer provides a real UART contract. Until then the driver must fail
closed with `STATUS_DEVICE_NOT_READY` and must not issue serial programming
IOCTLs with placeholder values.

## Runtime Ordering

`IfxBtConfigureUart` uses the following staged flow:

1. `IfxBtUartConfigStageValidateTarget`
2. `IfxBtUartConfigStageValidatePlatformConfig`
3. `IfxBtUartConfigStageApplyBaudRate`
4. `IfxBtUartConfigStageApplyLineControl`
5. `IfxBtUartConfigStageApplyFlowControl`
6. `IfxBtUartConfigStageApplyTimeouts`
7. `IfxBtUartConfigStagePurge`
8. `IfxBtUartConfigStageComplete`

The real-value IOCTL order is:

1. `IOCTL_SERIAL_SET_BAUD_RATE` with `InitialBaud`
2. `IOCTL_SERIAL_SET_LINE_CONTROL` with platform-provided data bits, parity,
   and stop bits
3. Convert the neutral platform handflow config to `SERIAL_HANDFLOW` inside
   the UART application layer, then call `IOCTL_SERIAL_SET_HANDFLOW`
4. Convert the neutral platform timeout config to `SERIAL_TIMEOUTS` inside
   the UART application layer, then call `IOCTL_SERIAL_SET_TIMEOUTS`
5. `IOCTL_SERIAL_PURGE` only when the platform purge mask is nonzero

`OperationalBaud` and `RequiresBaudSwitch` are validated and logged here, but
the operational baud switch belongs to a later firmware/vendor
baud-synchronization phase. This UART configuration path does not send HCI
Reset or vendor HCI commands.

## Required Real Values

The active placeholder contract is backed by these replacement points:

```text
TODO_REAL_UART_INITIAL_BAUD
TODO_REAL_UART_OPERATIONAL_BAUD
TODO_REAL_UART_DATA_BITS
TODO_REAL_UART_PARITY
TODO_REAL_UART_STOP_BITS
TODO_REAL_UART_FLOW_CONTROL
TODO_REAL_UART_TIMEOUTS
TODO_REAL_UART_PURGE_POLICY
TODO_REAL_UART_BAUD_SWITCH_REQUIRED
```

Real integration must provide nonzero initial and operational baud rates,
explicit line-control values, a platform-provided neutral handflow policy,
neutral timeout values, a purge policy, and the baud-switch requirement. The
shared platform header intentionally does not include `ntddser.h`; conversion
to WDK serial structures belongs in the UART application layer. The driver must
not infer common defaults such as 115200, 8N1, or RTS/CTS.

## Placeholder Behavior

`IFXBT_PLATFORM_UART_CONFIG.PlaceholderUartConfig` is `TRUE` in Phase 15P.
`IfxBtPlatformValidateUartConfig` therefore returns
`STATUS_DEVICE_NOT_READY`, logs every missing `TODO_REAL_UART_*` field, and
prevents `IfxBtConfigureUart` from entering the serial IOCTL stages.

The placeholder path emits:

```text
UART_CONFIG no_serial_ioctls_issued_placeholder=1
UART_CONFIG placeholder_blocking_initialization status=%!STATUS!
UART_CONFIG future_real_path_not_executed_placeholder=1
```

This preserves the current Code 10/no-BSOD behavior and keeps child PDO
exposure gated by parent transport readiness.

## Failure Mapping

Placeholder UART config maps to `STATUS_DEVICE_NOT_READY` and
`IfxBtFailureUartConfigPlaceholder`.

Invalid UART contract shape, including a null pointer or wrong structure size,
maps to `STATUS_INVALID_PARAMETER` and `IfxBtFailureUartConfigFailed`.

Future serial IOCTL failures return the underlying NTSTATUS from the lower UART
target. If a failure occurs after an earlier UART setting stage succeeded, the
partial-config rollback hook marks the parent uninitialized and fail-closed.

## Cleanup And Rollback

Partial UART configuration is treated as unsafe for child exposure. The current
rollback hook records the failed stage, clears `DeviceInitialized`, and leaves
the failed PrepareHardware/D0 path to tear down the target. If a later phase
captures previous serial state before applying changes, restoration should be
added inside that hook without changing the stage ordering above.

No GPIO control, firmware download, vendor command, HCI Reset, BTHX behavior,
ACPI binding, COM fallback, or child PDO ID behavior is part of this contract.
