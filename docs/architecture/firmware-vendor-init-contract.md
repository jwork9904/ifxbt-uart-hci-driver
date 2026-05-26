# Phase 17B Firmware/Vendor Init Contract

IfxBtUartHci must not report firmware or vendor readiness from placeholder
data. Until Advantech/TL/Infineon provide the real CYW55571 Bluetooth contract,
firmware/vendor initialization fails closed and sends no HCI command.

## Intended Runtime Ordering

The production-shaped firmware/vendor initialization sequence is:

1. `IfxBtFirmwareStageValidatePlatformReadiness`
2. `IfxBtFirmwareStageValidateFirmwareContract`
3. `IfxBtFirmwareStageDetermineFirmwareRequirement`
4. `IfxBtFirmwareStageLoadFirmwareBlob`
5. `IfxBtFirmwareStageParseFirmwareImage`
6. `IfxBtFirmwareStageSendVendorCommand`
7. `IfxBtFirmwareStageWaitForCommandComplete`
8. `IfxBtFirmwareStageValidateResponse`
9. `IfxBtFirmwareStageApplyBaudSwitchCommand`
10. `IfxBtFirmwareStageReconfigureHostUartForOperationalBaud`
11. `IfxBtFirmwareStageValidateControllerReady`
12. `IfxBtFirmwareStageComplete`

Any failed step transitions to `IfxBtFirmwareStageFailed`. In Phase 17B the
normal current path still fails earlier in UART placeholder validation. If UART
and power later become real, the firmware/vendor placeholder contract then
blocks initialization with `STATUS_DEVICE_NOT_READY`.

## Required Real Values

The active placeholder contract is backed by these replacement points:

```text
TODO_REAL_FIRMWARE_REQUIREMENT
TODO_REAL_FIRMWARE_FILE_NAME
TODO_REAL_FIRMWARE_FILE_FORMAT
TODO_REAL_FIRMWARE_SOURCE
TODO_REAL_VENDOR_OPCODE_SEQUENCE
TODO_REAL_VENDOR_RESPONSE_POLICY
TODO_REAL_BAUD_SWITCH_COMMAND
TODO_REAL_CONTROLLER_READY_VALIDATION
TODO_REAL_D3_FIRMWARE_RETENTION_POLICY
```

Real integration must define whether firmware is required, how it is located,
what format/parser applies, the vendor command sequence, expected response
policy, baud-switch command and timing, controller-ready validation, and D3
firmware retention behavior. Common defaults must not be inferred.

## Placeholder Behavior

`IFXBT_FIRMWARE_CONTRACT.PlaceholderFirmwareContract` is `TRUE` in Phase 17B.
`IfxBtFirmwareEvaluateInitialization` logs the contract and returns
`STATUS_DEVICE_NOT_READY` for the unknown firmware requirement and other unknown
vendor-init fields.

The placeholder path emits grep-friendly traces:

```text
FW_CONTRACT requirement=%d placeholder=%d
FW_CONTRACT file_type=%d placeholder=%d
FW_CONTRACT vendor_sequence=%d placeholder=%d
FW_CONTRACT response_policy=%d placeholder=%d
FW_CONTRACT baud_switch_required=%d placeholder=%d
FW_CONTRACT d3_retention_policy=%d placeholder=%d
FW_INIT stage=%d status=%!STATUS!
FW_INIT placeholder_blocking_initialization status=%!STATUS!
FW_INIT no_firmware_blob_loaded_placeholder=1
FW_INIT no_vendor_commands_sent_placeholder=1
FW_INIT no_baud_switch_placeholder=1
```

No firmware blob is loaded, no parser runs, no vendor command is sent, no HCI
Reset is sent, no response wait loop runs, no baud switch occurs, and no
controller validation packet is sent.

## Phase 18B Parent Readiness Scaffold

Parent transport readiness is intentionally stricter than UART-open readiness.
The final parent sequence is:

1. `IfxBtTransportStateResourcesParsed`
2. `IfxBtTransportStateUartOpened`
3. `IfxBtTransportStateUartConfigured`
4. `IfxBtTransportStatePlatformPowered`
5. `IfxBtTransportStateFirmwareReady`
6. `IfxBtTransportStateBaudSynchronized`
7. `IfxBtTransportStateControllerReady`
8. `IfxBtTransportStateOperational`

Child PDO exposure remains gated on `Operational`, `IfxBtFailureNone`,
`DeviceInitialized == TRUE`, and a non-null UART I/O target. `Operational` must
not be reached while any placeholder contract is active.

Controller-ready validation is a separate proof after firmware/vendor init and
baud synchronization. Firmware/vendor init changes or prepares controller state;
controller-ready validation proves the final parent transport can exchange a
known-good standard HCI transaction at the expected baud before the child stack
owns HCI traffic.

Phase 18B adds only a placeholder controller-ready gate. It returns
`STATUS_DEVICE_NOT_READY`, maps to
`IfxBtFailureControllerValidationPlaceholder`, and logs:

```text
CTRL_READY placeholder_blocking_validation status=%!STATUS!
CTRL_READY no_hci_commands_sent_placeholder=1
CTRL_READY no_response_wait_placeholder=1
IFXBT_READY operational_not_reached_placeholder=1
```

No HCI command is sent, no HCI Reset is sent, no response wait loop is started,
and no controller-ready success is reported from placeholders. Future timeout
and unexpected-event failures should map to `STATUS_IO_TIMEOUT` or
`STATUS_INVALID_DEVICE_RESPONSE` with
`IfxBtFailureControllerValidationFailed`.

## Command/Response Engine Principle

The eventual vendor init engine should be private to the parent transport and
must run before child PDO exposure. It should serialize one vendor command at a
time, match the expected opcode and event type, validate status bytes, and use
bounded per-command and sequence timeouts. It must not consume or reorder
BthPort-owned HCI traffic after the child stack is exposed.

Phase 17B deliberately does not implement this engine. The existing H4 RX path
continues to feed BTHX read queues only after normal transport startup.

## Baud Switch Principle

Baud switching is a synchronized two-sided operation. The controller command
must be sent at the current initial baud, the expected response and timing must
be known, and only then may the host UART be reconfigured to the operational
baud. If the controller changes baud but host reconfiguration fails, the driver
must assume host/controller desynchronization and fail closed until reset or
recovery repairs state.

Phase 17B does not implement a baud switch because the command, response, and
timing contract are unknown.

## D0/D3 Policy

Whether CYW55571 firmware survives D3 on this ARM64 platform is unknown. The
contract models this as `IfxBtFirmwareD3RetentionUnknown`, which is not a
runtime-ready policy. Future integration must state whether firmware is retained
across D3 or must be reloaded on resume.

## Cleanup And Rollback

If firmware download fails halfway in a future phase, the parent transport must
remain failed and uninitialized, and the child PDO must not be exposed. If baud
switch fails after the controller may have changed speed, recovery must reset or
otherwise resynchronize the controller and host UART before retrying.

Rollback is idempotent: release or undo only resources and state the driver
actually acquired or changed. Phase 17B acquires no firmware resources and sends
no HCI bytes, so rollback is limited to fail-closed status and trace logging.

## Reference Caution

Linux or cywbtserialbus-style flows may be useful for high-level architecture
ideas such as staged power, firmware download, command/response matching, and
H4 framing. Do not copy Broadcom/BCM assumptions, `.hcd` parser details,
firmware file names, local-name commands, HCI Reset behavior, opcodes, or
autobaud timing into this driver without confirmation for this CYW55571 Windows
platform.
