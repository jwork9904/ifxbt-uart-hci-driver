/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

   IfxBtPlatform.c

Abstract:

    Centralized placeholder platform configuration for the Infineon CYW55571
    Bluetooth UART transport.

Environment:

    Kernel mode only

--*/

#include "driver.h"
#include "IfxBtPlatform.h"
#include "IfxBtPlatform.tmh"

#define IFXBT_PLATFORM_NAME \
    L"Infineon CYW55571 Bluetooth UART placeholder platform"

//
// TODO_REAL_BT_CLIENT_ACPI_HID PLACEHOLDER_PLATFORM_VALUE:
// Intentionally fake Bluetooth client/function ACPI _HID placeholder for
// platform bring-up only. IfxBtUartHci must not bind to the lower UART
// controller node.
//
#define IFXBT_PLATFORM_ACPI_PLACEHOLDER_HID \
    L"ACPI\\PLACEHOLDER_CYW55571_BT_UART"

//
// TODO_REAL_BT_CLIENT_ACPI_CID PLACEHOLDER_PLATFORM_VALUE:
// Real Bluetooth client/function _CID remains pending from Advantech/TL.
//
#define IFXBT_PLATFORM_BT_CLIENT_ACPI_CID_PLACEHOLDER \
    L"TODO_REAL_BT_CLIENT_ACPI_CID"

//
// TODO_REAL_BT_CLIENT_ACPI_CRS PLACEHOLDER_PLATFORM_VALUE:
// Real Bluetooth client/function _CRS must expose a UART SerialBus/ResourceHub
// reference to the lower UAR7/QCOM0E16 controller when platform data arrives.
//
#define IFXBT_PLATFORM_BT_CLIENT_ACPI_CRS_PLACEHOLDER \
    L"TODO_REAL_BT_CLIENT_ACPI_CRS"

//
// Phase 14P evidence from TL/Advantech inspection:
// QCOM0E16 instance 7 is the lower Qualcomm UART controller serviced by qcuart,
// not the CYW55571 Bluetooth client/function node for IfxBtUartHci binding.
//
#define IFXBT_PLATFORM_KNOWN_LOWER_UART_CONTROLLER_HID \
    L"ACPI\\QCOM0E16"
#define IFXBT_PLATFORM_KNOWN_LOWER_UART_CONTROLLER_INSTANCE_HINT \
    L"ACPI\\QCOM0E16\\7"

//
// UART contract placeholders. These values are intentionally not usable for
// serial programming; PlaceholderUartConfig blocks all IOCTL_SERIAL_* calls.
//
#define TODO_REAL_UART_INITIAL_BAUD 0UL
#define TODO_REAL_UART_OPERATIONAL_BAUD 0UL
#define TODO_REAL_UART_DATA_BITS ((UCHAR)0)
#define TODO_REAL_UART_PARITY ((UCHAR)0xff)
#define TODO_REAL_UART_STOP_BITS ((UCHAR)0xff)
#define TODO_REAL_UART_FLOW_CONTROL IfxBtPlatformUartFlowControlPlaceholderUnknown
#define TODO_REAL_UART_TIMEOUTS { 0UL, 0UL, 0UL, 0UL, 0UL }
#define TODO_REAL_UART_PURGE_POLICY 0UL
#define TODO_REAL_UART_BAUD_SWITCH_REQUIRED FALSE

//
// TODO_REAL_GPIO_RESOURCES PLACEHOLDER_PLATFORM_VALUE:
// Symbolic names only. These are not ACPI resource names and must not be used
// as real GPIO identifiers.
//
#define IFXBT_PLATFORM_GPIO_BT_RESET_PLACEHOLDER \
    L"TODO_REAL_GPIO_RESOURCES:BT_RESET"
#define IFXBT_PLATFORM_GPIO_BT_REG_ON_PLACEHOLDER \
    L"TODO_REAL_GPIO_RESOURCES:BT_REG_ON"
#define IFXBT_PLATFORM_GPIO_HOST_WAKE_PLACEHOLDER \
    L"TODO_REAL_GPIO_RESOURCES:HOST_WAKE"
#define IFXBT_PLATFORM_GPIO_DEV_WAKE_PLACEHOLDER \
    L"TODO_REAL_GPIO_RESOURCES:DEV_WAKE"

//
// Power/reset/wake contract placeholders. These are symbolic contract slots,
// not hardware resources. They must fail closed until the board contract
// provides ownership, resources, polarity, timing, and wake trigger details.
//
#define TODO_REAL_POWER_OWNERSHIP IfxBtPowerOwnershipPlaceholderUnknown
#define TODO_REAL_GPIO_BT_REG_ON IFXBT_PLATFORM_GPIO_BT_REG_ON_PLACEHOLDER
#define TODO_REAL_GPIO_RESET IFXBT_PLATFORM_GPIO_BT_RESET_PLACEHOLDER
#define TODO_REAL_GPIO_DEV_WAKE IFXBT_PLATFORM_GPIO_DEV_WAKE_PLACEHOLDER
#define TODO_REAL_GPIO_HOST_WAKE IFXBT_PLATFORM_GPIO_HOST_WAKE_PLACEHOLDER
#define TODO_REAL_GPIO_POLARITY FALSE
#define TODO_REAL_POWER_TIMING 0UL
#define TODO_REAL_WAKE_TRIGGER 0UL

static const IFXBT_PLATFORM_UART_CONFIG IfxBtPlatformUartConfig = {
    (ULONG)sizeof(IFXBT_PLATFORM_UART_CONFIG),
    TODO_REAL_UART_INITIAL_BAUD,
    TODO_REAL_UART_OPERATIONAL_BAUD,
    TODO_REAL_UART_DATA_BITS,
    TODO_REAL_UART_STOP_BITS,
    TODO_REAL_UART_PARITY,
    TODO_REAL_UART_FLOW_CONTROL,
    TODO_REAL_UART_BAUD_SWITCH_REQUIRED,
    TRUE,  // TODO_REAL_UART_TIMEOUTS PLACEHOLDER_PLATFORM_VALUE
    TRUE,  // TODO_REAL_UART_PURGE_POLICY PLACEHOLDER_PLATFORM_VALUE
    TODO_REAL_UART_TIMEOUTS,
    { 0UL, 0UL, 0, 0 }, // TODO_REAL_UART_FLOW_CONTROL handflow policy pending
    TODO_REAL_UART_PURGE_POLICY,
    TRUE
};

static const IFXBT_PLATFORM_POWER_CONFIG IfxBtPlatformPowerConfig = {
    (ULONG)sizeof(IFXBT_PLATFORM_POWER_CONFIG),
    TODO_REAL_POWER_OWNERSHIP, // TODO_REAL_POWER_OWNERSHIP
    TODO_REAL_POWER_OWNERSHIP, // TODO_REAL_POWER_OWNERSHIP reset ownership
    TODO_REAL_POWER_OWNERSHIP, // TODO_REAL_POWER_OWNERSHIP DEV_WAKE ownership
    TODO_REAL_POWER_OWNERSHIP, // TODO_REAL_POWER_OWNERSHIP HOST_WAKE ownership
    TODO_REAL_GPIO_BT_REG_ON,  // TODO_REAL_GPIO_BT_REG_ON
    TODO_REAL_GPIO_RESET,      // TODO_REAL_GPIO_RESET
    TODO_REAL_GPIO_DEV_WAKE,   // TODO_REAL_GPIO_DEV_WAKE
    TODO_REAL_GPIO_HOST_WAKE,  // TODO_REAL_GPIO_HOST_WAKE
    TODO_REAL_GPIO_POLARITY,   // TODO_REAL_GPIO_POLARITY
    TODO_REAL_GPIO_POLARITY,   // TODO_REAL_GPIO_POLARITY
    TODO_REAL_WAKE_TRIGGER,    // TODO_REAL_WAKE_TRIGGER
    TODO_REAL_GPIO_POLARITY,   // TODO_REAL_GPIO_POLARITY
    TODO_REAL_POWER_TIMING,    // TODO_REAL_POWER_TIMING
    TODO_REAL_POWER_TIMING,    // TODO_REAL_POWER_TIMING
    TODO_REAL_POWER_TIMING,    // TODO_REAL_POWER_TIMING
    TODO_REAL_POWER_TIMING,    // TODO_REAL_WAKE_TRIGGER debounce timing
    TRUE
};

static const IFXBT_PLATFORM_CONFIG IfxBtPlatformConfig = {
    (ULONG)sizeof(IFXBT_PLATFORM_CONFIG),
    IFXBT_PLATFORM_NAME,
    IFXBT_PLATFORM_ACPI_PLACEHOLDER_HID,
    IFXBT_PLATFORM_ACPI_PLACEHOLDER_HID, // TODO_REAL_BT_CLIENT_ACPI_HID PLACEHOLDER_PLATFORM_VALUE
    IFXBT_PLATFORM_BT_CLIENT_ACPI_CID_PLACEHOLDER,
    IFXBT_PLATFORM_BT_CLIENT_ACPI_CRS_PLACEHOLDER,
    IFXBT_PLATFORM_KNOWN_LOWER_UART_CONTROLLER_HID,
    IFXBT_PLATFORM_KNOWN_LOWER_UART_CONTROLLER_INSTANCE_HINT,
    IFXBT_PLATFORM_GPIO_BT_RESET_PLACEHOLDER,
    IFXBT_PLATFORM_GPIO_BT_REG_ON_PLACEHOLDER,
    IFXBT_PLATFORM_GPIO_HOST_WAKE_PLACEHOLDER,
    IFXBT_PLATFORM_GPIO_DEV_WAKE_PLACEHOLDER,
    IfxBtPlatformPowerResourcesPlaceholderOnly,
    IfxBtPlatformFirmwareSequencePlaceholderUnknown, // TODO_REAL_FIRMWARE_SEQUENCE PLACEHOLDER_PLATFORM_VALUE
    TRUE,
    TRUE,
    TRUE
};

static
VOID
IfxBtPlatformLogAcpiContract(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    );

static
VOID
IfxBtPlatformLogMissingUartPlaceholders(VOID);

const IFXBT_PLATFORM_CONFIG*
IfxBtPlatformGetConfig(VOID)
{
    return &IfxBtPlatformConfig;
}

const IFXBT_PLATFORM_UART_CONFIG*
IfxBtPlatformGetUartConfig(VOID)
{
    return &IfxBtPlatformUartConfig;
}

const IFXBT_PLATFORM_POWER_CONFIG*
IfxBtPlatformGetPowerConfig(VOID)
{
    return &IfxBtPlatformPowerConfig;
}

static
BOOLEAN
IfxBtPlatformIsValidPowerOwnership(
    _In_ IFXBT_POWER_OWNERSHIP Ownership
    )
{
    switch (Ownership) {
    case IfxBtPowerOwnershipPlaceholderUnknown:
    case IfxBtPowerOwnershipPlatformOwnedAlwaysOn:
    case IfxBtPowerOwnershipAcpiPowerResourceOwned:
    case IfxBtPowerOwnershipDriverControlledGpio:
    case IfxBtPowerOwnershipNotPresentOptional:
        return TRUE;

    default:
        return FALSE;
    }
}

NTSTATUS
IfxBtPlatformValidateConfig(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (Config == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Config->Size != (ULONG)sizeof(IFXBT_PLATFORM_CONFIG)) {
        Status = STATUS_INFO_LENGTH_MISMATCH;
        goto Exit;
    }

    if (Config->PlatformName == NULL ||
        Config->AcpiPlaceholderHardwareId == NULL ||
        Config->BluetoothClientAcpiHardwareIdPlaceholder == NULL ||
        Config->BluetoothClientAcpiCompatibleIdPlaceholder == NULL ||
        Config->BluetoothClientAcpiCrsPlaceholder == NULL ||
        Config->KnownLowerUartControllerHardwareId == NULL ||
        Config->KnownLowerUartControllerInstanceHint == NULL ||
        Config->ResetGpioPlaceholder == NULL ||
        Config->RegOnGpioPlaceholder == NULL ||
        Config->HostWakeGpioPlaceholder == NULL ||
        Config->DevWakeGpioPlaceholder == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Status = IfxBtPlatformValidatePowerResources(Config);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    if (Config->FirmwareSequenceState != IfxBtPlatformFirmwareSequencePlaceholderUnknown) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Config->RequiresBluetoothClientAcpiNode == FALSE) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Config->CandidateQcom0e16IsLowerControllerOnly == FALSE) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Config->PlaceholderPlatformValue == FALSE) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

Exit:

    return Status;
}

NTSTATUS
IfxBtPlatformValidateUartConfig(
    _In_ const IFXBT_PLATFORM_UART_CONFIG* UartConfig
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONTRACT validate entry uartConfig=%p",
            UartConfig));

    if (UartConfig == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (UartConfig->Size != (ULONG)sizeof(IFXBT_PLATFORM_UART_CONFIG)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (UartConfig->PlaceholderUartConfig) {
        Status = STATUS_DEVICE_NOT_READY;
        IfxBtPlatformLogMissingUartPlaceholders();
        goto Exit;
    }

    if (UartConfig->InitialBaud == 0 ||
        UartConfig->OperationalBaud == 0 ||
        UartConfig->DataBits < 5 ||
        UartConfig->DataBits > 8 ||
        UartConfig->Parity > 4 ||
        UartConfig->StopBits > 2 ||
        UartConfig->FlowControl != IfxBtPlatformUartFlowControlPlatformProvided ||
        UartConfig->TimeoutPolicyPlaceholder ||
        UartConfig->PurgePolicyPlaceholder) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((UartConfig->PurgeMask &
         ~(IfxBtPlatformUartPurgeTxAbort |
           IfxBtPlatformUartPurgeRxAbort |
           IfxBtPlatformUartPurgeTxClear |
           IfxBtPlatformUartPurgeRxClear)) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

Exit:

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONTRACT validate exit status=%!STATUS!",
                Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONTRACT validate exit status=%!STATUS!",
                Status));
    }

    return Status;
}

NTSTATUS
IfxBtPlatformValidatePowerResources(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER_PLACEHOLDER validate_power_resources entry config=%p",
            Config));

    if (Config == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Config->Size != (ULONG)sizeof(IFXBT_PLATFORM_CONFIG)) {
        Status = STATUS_INFO_LENGTH_MISMATCH;
        goto Exit;
    }

    if (Config->ResetGpioPlaceholder == NULL ||
        Config->RegOnGpioPlaceholder == NULL ||
        Config->HostWakeGpioPlaceholder == NULL ||
        Config->DevWakeGpioPlaceholder == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Config->PowerResourceState != IfxBtPlatformPowerResourcesPlaceholderOnly) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

Exit:

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("POWER_PLACEHOLDER validate_power_resources failed status=%!STATUS!",
                Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER_PLACEHOLDER validate_power_resources placeholder_only status=%!STATUS!",
                Status));
    }

    return Status;
}

NTSTATUS
IfxBtPlatformValidatePowerConfig(
    _In_ const IFXBT_PLATFORM_POWER_CONFIG* PowerConfig
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER_CONFIG validate entry powerConfig=%p",
            PowerConfig));

    if (PowerConfig == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (PowerConfig->Size != (ULONG)sizeof(IFXBT_PLATFORM_POWER_CONFIG)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (PowerConfig->RegOnGpioPlaceholderName == NULL ||
        PowerConfig->ResetGpioPlaceholderName == NULL ||
        PowerConfig->DevWakeGpioPlaceholderName == NULL ||
        PowerConfig->HostWakeGpioPlaceholderName == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (!IfxBtPlatformIsValidPowerOwnership(PowerConfig->PowerOwnership) ||
        !IfxBtPlatformIsValidPowerOwnership(PowerConfig->ResetOwnership) ||
        !IfxBtPlatformIsValidPowerOwnership(PowerConfig->DevWakeOwnership) ||
        !IfxBtPlatformIsValidPowerOwnership(PowerConfig->HostWakeOwnership)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (PowerConfig->PlaceholderPowerConfig) {
        Status = STATUS_DEVICE_NOT_READY;
        goto Exit;
    }

    if (PowerConfig->PowerOwnership == IfxBtPowerOwnershipPlaceholderUnknown ||
        PowerConfig->ResetOwnership == IfxBtPowerOwnershipPlaceholderUnknown ||
        PowerConfig->DevWakeOwnership == IfxBtPowerOwnershipPlaceholderUnknown ||
        PowerConfig->HostWakeOwnership == IfxBtPowerOwnershipPlaceholderUnknown) {
        Status = STATUS_DEVICE_CONFIGURATION_ERROR;
        goto Exit;
    }

Exit:

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("POWER_CONFIG validate exit status=%!STATUS!",
                Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER_CONFIG validate exit status=%!STATUS!",
                Status));
    }

    return Status;
}

static
VOID
IfxBtPlatformLogMissingUartPlaceholders(VOID)
{
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONTRACT missing TODO_REAL_UART_INITIAL_BAUD"));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONTRACT missing TODO_REAL_UART_OPERATIONAL_BAUD"));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONTRACT missing TODO_REAL_UART_DATA_BITS"));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONTRACT missing TODO_REAL_UART_PARITY"));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONTRACT missing TODO_REAL_UART_STOP_BITS"));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONTRACT missing TODO_REAL_UART_FLOW_CONTROL"));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONTRACT missing TODO_REAL_UART_TIMEOUTS"));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONTRACT missing TODO_REAL_UART_PURGE_POLICY"));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONTRACT missing TODO_REAL_UART_BAUD_SWITCH_REQUIRED"));
}

VOID
IfxBtPlatformLogUartConfig(
    _In_ const IFXBT_PLATFORM_UART_CONFIG* UartConfig
    )
{
    if (UartConfig == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONTRACT missing uart config"));
        return;
    }

    DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONTRACT initial_baud=%lu placeholder=%d",
            UartConfig->InitialBaud,
            UartConfig->PlaceholderUartConfig ? 1 : 0));

    DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONTRACT operational_baud=%lu placeholder=%d",
            UartConfig->OperationalBaud,
            UartConfig->PlaceholderUartConfig ? 1 : 0));

    DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONTRACT line_control data_bits=%d parity=%d stop_bits=%d placeholder=%d",
            UartConfig->DataBits,
            UartConfig->Parity,
            UartConfig->StopBits,
            UartConfig->PlaceholderUartConfig ? 1 : 0));

    DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONTRACT flow_control=%d placeholder=%d",
            UartConfig->FlowControl,
            UartConfig->PlaceholderUartConfig ? 1 : 0));

    DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONTRACT baud_switch_required=%d placeholder=%d",
            UartConfig->RequiresBaudSwitch ? 1 : 0,
            UartConfig->PlaceholderUartConfig ? 1 : 0));

    if (UartConfig->PlaceholderUartConfig) {
        IfxBtPlatformLogMissingUartPlaceholders();
    }
}

VOID
IfxBtPlatformLogPowerConfig(
    _In_ const IFXBT_PLATFORM_POWER_CONFIG* PowerConfig
    )
{
    PCWSTR RegOnGpioPlaceholder;
    PCWSTR ResetGpioPlaceholder;
    PCWSTR DevWakeGpioPlaceholder;
    PCWSTR HostWakeGpioPlaceholder;

    if (PowerConfig == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("POWER_CONTRACT missing power config"));
        return;
    }

    RegOnGpioPlaceholder =
        (PowerConfig->RegOnGpioPlaceholderName != NULL) ?
            PowerConfig->RegOnGpioPlaceholderName :
            L"<null>";
    ResetGpioPlaceholder =
        (PowerConfig->ResetGpioPlaceholderName != NULL) ?
            PowerConfig->ResetGpioPlaceholderName :
            L"<null>";
    DevWakeGpioPlaceholder =
        (PowerConfig->DevWakeGpioPlaceholderName != NULL) ?
            PowerConfig->DevWakeGpioPlaceholderName :
            L"<null>";
    HostWakeGpioPlaceholder =
        (PowerConfig->HostWakeGpioPlaceholderName != NULL) ?
            PowerConfig->HostWakeGpioPlaceholderName :
            L"<null>";

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONTRACT ownership=%d placeholder=%d",
            PowerConfig->PowerOwnership,
            PowerConfig->PlaceholderPowerConfig ? 1 : 0));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONTRACT reset_ownership=%d dev_wake_ownership=%d host_wake_ownership=%d",
            PowerConfig->ResetOwnership,
            PowerConfig->DevWakeOwnership,
            PowerConfig->HostWakeOwnership));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONTRACT reg_on_placeholder=%S",
            RegOnGpioPlaceholder));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONTRACT reset_placeholder=%S",
            ResetGpioPlaceholder));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONTRACT dev_wake_placeholder=%S",
            DevWakeGpioPlaceholder));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONTRACT host_wake_placeholder=%S",
            HostWakeGpioPlaceholder));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONTRACT polarity_placeholder reg_on=%d reset=%d dev_wake=%d host_wake_trigger=%lu",
            PowerConfig->RegOnActiveHighPlaceholder ? 1 : 0,
            PowerConfig->ResetActiveHighPlaceholder ? 1 : 0,
            PowerConfig->DevWakeActiveHighPlaceholder ? 1 : 0,
            PowerConfig->HostWakeInterruptPolarityPlaceholder));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONTRACT timing_placeholder power_stable_ms=%lu reset_assert_ms=%lu post_reset_ms=%lu host_wake_debounce_ms=%lu",
            PowerConfig->PowerStableDelayMsPlaceholder,
            PowerConfig->ResetAssertDelayMsPlaceholder,
            PowerConfig->PostResetDelayMsPlaceholder,
            PowerConfig->HostWakeDebounceMsPlaceholder));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONTRACT no_gpio_actions_placeholder=1"));
}

VOID
IfxBtPlatformLogPowerPlaceholderConfig(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    )
{
    PCWSTR ResetGpioPlaceholder;
    PCWSTR RegOnGpioPlaceholder;
    PCWSTR HostWakeGpioPlaceholder;
    PCWSTR DevWakeGpioPlaceholder;

    if (Config == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("POWER_PLACEHOLDER missing config"));
        return;
    }

    ResetGpioPlaceholder = (Config->ResetGpioPlaceholder != NULL) ? Config->ResetGpioPlaceholder : L"<null>";
    RegOnGpioPlaceholder = (Config->RegOnGpioPlaceholder != NULL) ? Config->RegOnGpioPlaceholder : L"<null>";
    HostWakeGpioPlaceholder = (Config->HostWakeGpioPlaceholder != NULL) ? Config->HostWakeGpioPlaceholder : L"<null>";
    DevWakeGpioPlaceholder = (Config->DevWakeGpioPlaceholder != NULL) ? Config->DevWakeGpioPlaceholder : L"<null>";

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_PLACEHOLDER power resources are placeholder-only state=%d no GPIO operations performed",
            Config->PowerResourceState));

    IfxBtPlatformLogPowerConfig(IfxBtPlatformGetPowerConfig());

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("GPIO_PLACEHOLDER symbolic_only reset=%S reg_on=%S host_wake=%S dev_wake=%S",
            ResetGpioPlaceholder,
            RegOnGpioPlaceholder,
            HostWakeGpioPlaceholder,
            DevWakeGpioPlaceholder));

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("GPIO_PLACEHOLDER TODO_REAL_GPIO_RESOURCES required before real control"));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("GPIO_PLACEHOLDER real data required polarity timing power ownership wake trigger"));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("RESET_PLACEHOLDER reset sequence unknown no reset asserted no reset timing applied"));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("WAKE_PLACEHOLDER wake resources unknown no HOST_WAKE interrupt programmed no DEV_WAKE protocol applied"));
}

static
VOID
IfxBtPlatformLogAcpiContract(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    )
{
    PCWSTR BluetoothClientAcpiHardwareIdPlaceholder;
    PCWSTR KnownLowerUartControllerHardwareId;
    PCWSTR KnownLowerUartControllerInstanceHint;

    if (Config == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("ACPI_CONTRACT missing config"));
        return;
    }

    BluetoothClientAcpiHardwareIdPlaceholder =
        (Config->BluetoothClientAcpiHardwareIdPlaceholder != NULL) ?
            Config->BluetoothClientAcpiHardwareIdPlaceholder :
            L"<null>";
    KnownLowerUartControllerHardwareId =
        (Config->KnownLowerUartControllerHardwareId != NULL) ?
            Config->KnownLowerUartControllerHardwareId :
            L"<null>";
    KnownLowerUartControllerInstanceHint =
        (Config->KnownLowerUartControllerInstanceHint != NULL) ?
            Config->KnownLowerUartControllerInstanceHint :
            L"<null>";

    DoTrace(LEVEL_WARNING, TFLAG_PNP, ("ACPI_CONTRACT bt_client_hid_placeholder=%S",
            BluetoothClientAcpiHardwareIdPlaceholder));

    DoTrace(LEVEL_WARNING, TFLAG_PNP, ("ACPI_CONTRACT lower_uart_controller_hid=%S",
            KnownLowerUartControllerHardwareId));

    DoTrace(LEVEL_WARNING, TFLAG_PNP, ("ACPI_CONTRACT lower_uart_controller_instance_hint=%S",
            KnownLowerUartControllerInstanceHint));

    DoTrace(LEVEL_WARNING, TFLAG_PNP, ("ACPI_CONTRACT qcom0e16_is_lower_controller_only=%d",
            Config->CandidateQcom0e16IsLowerControllerOnly ? 1 : 0));

    DoTrace(LEVEL_WARNING, TFLAG_PNP, ("ACPI_CONTRACT awaiting_real_bt_child_node=%d",
            Config->RequiresBluetoothClientAcpiNode ? 1 : 0));
}

VOID
IfxBtPlatformLogConfig(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    )
{
    if (Config == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("PLATFORM_CONFIG missing config"));
        return;
    }

    DoTrace(LEVEL_INFO, TFLAG_PNP, ("PLATFORM_CONFIG placeholder active name=%S placeholder=%d",
            Config->PlatformName,
            Config->PlaceholderPlatformValue));

    DoTrace(LEVEL_INFO, TFLAG_PNP, ("PLATFORM_CONFIG acpi_placeholder_hid=%S",
            Config->AcpiPlaceholderHardwareId));

    IfxBtPlatformLogAcpiContract(Config);
    IfxBtPlatformLogUartConfig(IfxBtPlatformGetUartConfig());

    DoTrace(LEVEL_INFO, TFLAG_UART, ("PLATFORM_CONFIG firmware_sequence_placeholder=%d",
            Config->FirmwareSequenceState));

    DoTrace(LEVEL_INFO, TFLAG_PNP, ("PLATFORM_CONFIG gpio_placeholders symbolic_only no_real_gpio_resources_configured"));

    IfxBtPlatformLogPowerPlaceholderConfig(Config);
}
