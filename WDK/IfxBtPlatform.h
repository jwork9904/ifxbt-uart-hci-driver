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
    IfxBtPlatformUartFlowControlPlaceholderUnknown = 0,
    IfxBtPlatformUartFlowControlPlatformProvided = 1
} IFXBT_PLATFORM_UART_FLOW_CONTROL;

typedef enum _IFXBT_PLATFORM_UART_PURGE_FLAGS {
    IfxBtPlatformUartPurgeTxAbort = 0x00000001,
    IfxBtPlatformUartPurgeRxAbort = 0x00000002,
    IfxBtPlatformUartPurgeTxClear = 0x00000004,
    IfxBtPlatformUartPurgeRxClear = 0x00000008
} IFXBT_PLATFORM_UART_PURGE_FLAGS;

typedef enum _IFXBT_UART_CONFIG_STAGE {
    IfxBtUartConfigStageNotStarted = 0,
    IfxBtUartConfigStageValidateTarget,
    IfxBtUartConfigStageValidatePlatformConfig,
    IfxBtUartConfigStageApplyBaudRate,
    IfxBtUartConfigStageApplyLineControl,
    IfxBtUartConfigStageApplyFlowControl,
    IfxBtUartConfigStageApplyTimeouts,
    IfxBtUartConfigStagePurge,
    IfxBtUartConfigStageComplete,
    IfxBtUartConfigStageFailed
} IFXBT_UART_CONFIG_STAGE;

typedef enum _IFXBT_PLATFORM_FIRMWARE_SEQUENCE_STATE {
    IfxBtPlatformFirmwareSequencePlaceholderUnknown = 0
} IFXBT_PLATFORM_FIRMWARE_SEQUENCE_STATE;

typedef enum _IFXBT_PLATFORM_POWER_RESOURCE_STATE {
    IfxBtPlatformPowerResourcesPlaceholderOnly = 0
} IFXBT_PLATFORM_POWER_RESOURCE_STATE;

typedef struct _IFXBT_PLATFORM_UART_TIMEOUTS_CONFIG {
    ULONG ReadIntervalTimeout;
    ULONG ReadTotalTimeoutMultiplier;
    ULONG ReadTotalTimeoutConstant;
    ULONG WriteTotalTimeoutMultiplier;
    ULONG WriteTotalTimeoutConstant;
} IFXBT_PLATFORM_UART_TIMEOUTS_CONFIG, *PIFXBT_PLATFORM_UART_TIMEOUTS_CONFIG;

typedef struct _IFXBT_PLATFORM_UART_HANDFLOW_CONFIG {
    ULONG ControlHandShake;
    ULONG FlowReplace;
    LONG XonLimit;
    LONG XoffLimit;
} IFXBT_PLATFORM_UART_HANDFLOW_CONFIG, *PIFXBT_PLATFORM_UART_HANDFLOW_CONFIG;

typedef struct _IFXBT_PLATFORM_UART_CONFIG {
    ULONG Size;
    ULONG InitialBaud;
    ULONG OperationalBaud;
    UCHAR DataBits;
    UCHAR StopBits;
    UCHAR Parity;
    IFXBT_PLATFORM_UART_FLOW_CONTROL FlowControl;
    BOOLEAN RequiresBaudSwitch;
    BOOLEAN TimeoutPolicyPlaceholder;
    BOOLEAN PurgePolicyPlaceholder;
    IFXBT_PLATFORM_UART_TIMEOUTS_CONFIG Timeouts;
    IFXBT_PLATFORM_UART_HANDFLOW_CONFIG Handflow;
    ULONG PurgeMask;
    BOOLEAN PlaceholderUartConfig;
} IFXBT_PLATFORM_UART_CONFIG, *PIFXBT_PLATFORM_UART_CONFIG;

typedef struct _IFXBT_PLATFORM_CONFIG {
    ULONG Size;
    PCWSTR PlatformName;
    PCWSTR AcpiPlaceholderHardwareId;
    PCWSTR BluetoothClientAcpiHardwareIdPlaceholder;
    PCWSTR BluetoothClientAcpiCompatibleIdPlaceholder;
    PCWSTR BluetoothClientAcpiCrsPlaceholder;
    PCWSTR KnownLowerUartControllerHardwareId;
    PCWSTR KnownLowerUartControllerInstanceHint;
    PCWSTR ResetGpioPlaceholder;
    PCWSTR RegOnGpioPlaceholder;
    PCWSTR HostWakeGpioPlaceholder;
    PCWSTR DevWakeGpioPlaceholder;
    IFXBT_PLATFORM_POWER_RESOURCE_STATE PowerResourceState;
    IFXBT_PLATFORM_FIRMWARE_SEQUENCE_STATE FirmwareSequenceState;
    BOOLEAN RequiresBluetoothClientAcpiNode;
    BOOLEAN CandidateQcom0e16IsLowerControllerOnly;
    BOOLEAN PlaceholderPlatformValue;
} IFXBT_PLATFORM_CONFIG, *PIFXBT_PLATFORM_CONFIG;

const IFXBT_PLATFORM_CONFIG*
IfxBtPlatformGetConfig(VOID);

const IFXBT_PLATFORM_UART_CONFIG*
IfxBtPlatformGetUartConfig(VOID);

NTSTATUS
IfxBtPlatformValidateConfig(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    );

NTSTATUS
IfxBtPlatformValidateUartConfig(
    _In_ const IFXBT_PLATFORM_UART_CONFIG* UartConfig
    );

VOID
IfxBtPlatformLogConfig(
    _In_ const IFXBT_PLATFORM_CONFIG* Config
    );

VOID
IfxBtPlatformLogUartConfig(
    _In_ const IFXBT_PLATFORM_UART_CONFIG* UartConfig
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
