# Phase 16B Power/Reset/Wake Contract

IfxBtUartHci must not report Bluetooth controller power, reset, or wake
readiness from symbolic placeholder data. Until Advantech/TL/Infineon provide
the real board contract, power/reset/wake readiness paths fail closed and no
GPIO, regulator, Resource Hub, firmware, or HCI operation is performed.

## Intended Runtime Sequence

The production-shaped power-on sequence is:

1. `IfxBtPowerStageValidatePlatformConfig`
2. `IfxBtPowerStageValidatePowerResources`
3. `IfxBtPowerStageDetermineOwnership`
4. `IfxBtPowerStageAssertReset`
5. `IfxBtPowerStageEnableRegOn`
6. `IfxBtPowerStageWaitPowerStable`
7. `IfxBtPowerStageDeassertReset`
8. `IfxBtPowerStageWaitControllerBoot`
9. `IfxBtPowerStageConfigureDevWake`
10. `IfxBtPowerStageConfigureHostWake`
11. `IfxBtPowerStageComplete`

Any failed step transitions to `IfxBtPowerStageFailed`. Cleanup is reverse-order
and idempotent: only resources actually acquired by the driver may be released.
In Phase 16B no resources are acquired, so rollback logs cleanup only.

## Ownership Modes

`IfxBtPowerOwnershipPlaceholderUnknown` means the board contract is missing and
must not be treated as hardware readiness.

`IfxBtPowerOwnershipPlatformOwnedAlwaysOn` means firmware or platform wiring
keeps the controller powered and no driver GPIO action is required. This is only
valid when `PlaceholderPowerConfig` is false.

`IfxBtPowerOwnershipAcpiPowerResourceOwned` means ACPI power resources own the
transition and the driver should not open GPIOs directly. This is only valid
when the real ACPI contract documents the resource ownership.

`IfxBtPowerOwnershipDriverControlledGpio` means the driver will eventually own
GPIO open/write sequencing. Phase 16B deliberately does not implement this.

`IfxBtPowerOwnershipNotPresentOptional` means an optional reset or wake line is
absent by contract, not merely unknown.

## Required Real Values

The replacement points are:

```text
TODO_REAL_POWER_OWNERSHIP
TODO_REAL_GPIO_BT_REG_ON
TODO_REAL_GPIO_RESET
TODO_REAL_GPIO_DEV_WAKE
TODO_REAL_GPIO_HOST_WAKE
TODO_REAL_GPIO_POLARITY
TODO_REAL_POWER_TIMING
TODO_REAL_WAKE_TRIGGER
```

Real integration must provide resource identity, ownership, polarity, reset
timing, power-stable and post-reset delays, wake trigger polarity, wake debounce,
and D0/D3 retention policy. Common defaults must not be inferred.

## Placeholder Behavior

`IFXBT_PLATFORM_POWER_CONFIG.PlaceholderPowerConfig` is `TRUE` in Phase 16B.
`IfxBtPlatformValidatePowerConfig` therefore returns `STATUS_DEVICE_NOT_READY`.

Readiness paths fail closed:

```text
DeviceEnable(TRUE)
DevicePowerOn
DeviceEnableWakeControl
FdoEvtDeviceArmWake through DeviceEnableWakeControl
DeviceDoPLDR
DeviceInitialize power-readiness validation after UART success
```

Cleanup paths may no-op success because the driver acquired nothing:

```text
DevicePowerOff
DeviceEnable(FALSE)
DeviceDisableWakeControl
rollback after failed staged power-on
```

Phase 18B invalidates parent readiness on D0 exit, hardware release, and
parent-side PLDR/reset entry. Invalidation clears `DeviceInitialized`, moves the
transport out of `Operational`, and logs:

```text
IFXBT_READY_INVALIDATE reason=...
```

Normal teardown invalidation does not invent a new failure reason. Failure paths
preserve the root failure reason and move the transport to
`IfxBtTransportStateFailed`. Because CYW55571 D3 firmware retention is still
unknown, D0 entry must rebuild readiness instead of treating pre-D3
`Operational` as still valid.

The placeholder path emits `POWER_CONTRACT no_gpio_actions_placeholder=1` and
the operation-specific blocking trace, such as
`POWER_CONFIG placeholder_blocking_power_on`,
`WAKE_CONFIG placeholder_blocking_wake_arm`, or
`RESET_CONFIG placeholder_reset_unavailable`.

## Failure Mapping

Placeholder power config maps to `STATUS_DEVICE_NOT_READY` and
`IfxBtFailurePowerResourcesPlaceholder`.

Future missing required real power resources map to
`STATUS_DEVICE_CONFIGURATION_ERROR` and `IfxBtFailurePowerSequenceFailed`.

Future GPIO open/write failures must return the underlying NTSTATUS and map to
`IfxBtFailurePowerSequenceFailed`.

Future reset timeout maps to `STATUS_IO_TIMEOUT` and
`IfxBtFailurePowerSequenceFailed`.

Future wake configuration failure returns the underlying NTSTATUS. Placeholder
wake-arm returns `STATUS_DEVICE_NOT_READY`.

Explicit platform-owned always-on power may return `STATUS_SUCCESS` only after
the contract is no longer placeholder-backed.

## Safety Boundaries

Phase 16B does not open Resource Hub GPIO targets, write GPIOs, assert reset,
toggle BT_REG_ON, program wake interrupts, send HCI Reset, download firmware, or
change UART/BTHX/ACPI/child PDO behavior.

The parent transport must not become `Operational` from placeholder power. Child
PDO exposure remains gated by parent transport readiness.
