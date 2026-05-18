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
// TODO_REAL_ACPI_HID PLACEHOLDER_PLATFORM_VALUE:
// Intentionally fake parent ACPI _HID placeholder for platform bring-up only.
//
#define IFXBT_PLATFORM_ACPI_PLACEHOLDER_HID \
    L"ACPI\\PLACEHOLDER_CYW55571_BT_UART"

//
// TODO_REAL_UART_BAUD PLACEHOLDER_PLATFORM_VALUE:
// Zero means unknown/not configured in this phase.
//
#define IFXBT_PLATFORM_INITIAL_UART_BAUD_PLACEHOLDER 0UL

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

static const IFXBT_PLATFORM_CONFIG IfxBtPlatformConfig = {
    (ULONG)sizeof(IFXBT_PLATFORM_CONFIG),
    IFXBT_PLATFORM_NAME,
    IFXBT_PLATFORM_ACPI_PLACEHOLDER_HID,
    IFXBT_PLATFORM_INITIAL_UART_BAUD_PLACEHOLDER,
    IfxBtPlatformUartFlowControlPlaceholderUnknown, // TODO_REAL_UART_FLOW_CONTROL PLACEHOLDER_PLATFORM_VALUE
    IFXBT_PLATFORM_GPIO_BT_RESET_PLACEHOLDER,
    IFXBT_PLATFORM_GPIO_BT_REG_ON_PLACEHOLDER,
    IFXBT_PLATFORM_GPIO_HOST_WAKE_PLACEHOLDER,
    IFXBT_PLATFORM_GPIO_DEV_WAKE_PLACEHOLDER,
    IfxBtPlatformFirmwareSequencePlaceholderUnknown, // TODO_REAL_FIRMWARE_SEQUENCE PLACEHOLDER_PLATFORM_VALUE
    TRUE
};

const IFXBT_PLATFORM_CONFIG*
IfxBtPlatformGetConfig(VOID)
{
    return &IfxBtPlatformConfig;
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
        Config->ResetGpioPlaceholder == NULL ||
        Config->RegOnGpioPlaceholder == NULL ||
        Config->HostWakeGpioPlaceholder == NULL ||
        Config->DevWakeGpioPlaceholder == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Config->UartFlowControlPlaceholder != IfxBtPlatformUartFlowControlPlaceholderUnknown) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Config->FirmwareSequenceState != IfxBtPlatformFirmwareSequencePlaceholderUnknown) {
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

    DoTrace(LEVEL_INFO, TFLAG_UART, ("PLATFORM_CONFIG uart_baud_placeholder=%lu flow_control_placeholder=%d",
            Config->InitialUartBaudPlaceholder,
            Config->UartFlowControlPlaceholder));

    DoTrace(LEVEL_INFO, TFLAG_UART, ("PLATFORM_CONFIG firmware_sequence_placeholder=%d",
            Config->FirmwareSequenceState));

    DoTrace(LEVEL_INFO, TFLAG_PNP, ("PLATFORM_CONFIG gpio_placeholders symbolic_only no_real_gpio_resources_configured"));
}
