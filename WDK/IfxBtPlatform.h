/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

   IfxBtPlatform.h

Abstract:

    Placeholder platform configuration declarations for the Infineon CYW55571
    Bluetooth UART transport.

Environment:

   Kernel mode only

--*/

#ifndef __IFXBT_PLATFORM_H__
#define __IFXBT_PLATFORM_H__

#include <ntddk.h>

typedef enum _IFXBT_PLATFORM_UART_FLOW_CONTROL {
    IfxBtPlatformUartFlowControlPlaceholderUnknown = 0
} IFXBT_PLATFORM_UART_FLOW_CONTROL;

typedef enum _IFXBT_PLATFORM_FIRMWARE_SEQUENCE_STATE {
    IfxBtPlatformFirmwareSequencePlaceholderUnknown = 0
} IFXBT_PLATFORM_FIRMWARE_SEQUENCE_STATE;

typedef enum _IFXBT_PLATFORM_POWER_RESOURCE_STATE {
    IfxBtPlatformPowerResourcesPlaceholderOnly = 0
} IFXBT_PLATFORM_POWER_RESOURCE_STATE;

typedef struct _IFXBT_PLATFORM_CONFIG {
    ULONG Size;
    PCWSTR PlatformName;
    PCWSTR AcpiPlaceholderHardwareId;
    ULONG InitialUartBaudPlaceholder;
    IFXBT_PLATFORM_UART_FLOW_CONTROL UartFlowControlPlaceholder;
    PCWSTR ResetGpioPlaceholder;
    PCWSTR RegOnGpioPlaceholder;
    PCWSTR HostWakeGpioPlaceholder;
    PCWSTR DevWakeGpioPlaceholder;
    IFXBT_PLATFORM_POWER_RESOURCE_STATE PowerResourceState;
    IFXBT_PLATFORM_FIRMWARE_SEQUENCE_STATE FirmwareSequenceState;
    BOOLEAN PlaceholderPlatformValue;
} IFXBT_PLATFORM_CONFIG, *PIFXBT_PLATFORM_CONFIG;

const IFXBT_PLATFORM_CONFIG*
IfxBtPlatformGetConfig(VOID);

NTSTATUS
IfxBtPlatformValidateConfig(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    );

VOID
IfxBtPlatformLogConfig(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    );

NTSTATUS
IfxBtPlatformValidatePowerResources(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    );

VOID
IfxBtPlatformLogPowerPlaceholderConfig(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    );

#endif
