/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

   IfxBtFirmware.h

Abstract:

    Placeholder firmware/vendor initialization declarations for the Infineon
    CYW55571 Bluetooth UART transport.

Environment:

   Kernel mode only

--*/

#pragma once

#ifndef __IFXBT_FIRMWARE_H__
#define __IFXBT_FIRMWARE_H__

#include <ntddk.h>
#include "IfxBtPlatform.h"

typedef enum _IFXBT_FIRMWARE_STAGE {
    IfxBtFirmwareStageNotStarted = 0,
    IfxBtFirmwareStageValidatePlatformReadiness,
    IfxBtFirmwareStageValidateFirmwareContract,
    IfxBtFirmwareStageDetermineFirmwareRequirement,
    IfxBtFirmwareStageLoadFirmwareBlob,
    IfxBtFirmwareStageParseFirmwareImage,
    IfxBtFirmwareStageSendVendorCommand,
    IfxBtFirmwareStageWaitForCommandComplete,
    IfxBtFirmwareStageValidateResponse,
    IfxBtFirmwareStageApplyBaudSwitchCommand,
    IfxBtFirmwareStageReconfigureHostUartForOperationalBaud,
    IfxBtFirmwareStageValidateControllerReady,
    IfxBtFirmwareStageComplete,
    IfxBtFirmwareStageFailed
} IFXBT_FIRMWARE_STAGE;

typedef enum _IFXBT_FIRMWARE_REQUIREMENT {
    IfxBtFirmwareRequirementUnknown = 0,
    IfxBtFirmwareRequirementNotRequired,
    IfxBtFirmwareRequirementRequired
} IFXBT_FIRMWARE_REQUIREMENT;

typedef enum _IFXBT_FIRMWARE_FILE_TYPE {
    IfxBtFirmwareFileTypeUnknown = 0,
    IfxBtFirmwareFileTypeHcdPlaceholder,
    IfxBtFirmwareFileTypeVendorSpecificPlaceholder
} IFXBT_FIRMWARE_FILE_TYPE;

typedef enum _IFXBT_FIRMWARE_SOURCE {
    IfxBtFirmwareSourceUnknown = 0,
    IfxBtFirmwareSourceFilePath,
    IfxBtFirmwareSourceRegistryConfigured,
    IfxBtFirmwareSourcePackagedResourcePlaceholder
} IFXBT_FIRMWARE_SOURCE;

typedef enum _IFXBT_VENDOR_COMMAND_SEQUENCE {
    IfxBtVendorCommandSequenceUnknown = 0,
    IfxBtVendorCommandSequenceTableDrivenPlaceholder
} IFXBT_VENDOR_COMMAND_SEQUENCE;

typedef enum _IFXBT_VENDOR_RESPONSE_POLICY {
    IfxBtVendorResponsePolicyUnknown = 0,
    IfxBtVendorResponsePolicyCommandComplete,
    IfxBtVendorResponsePolicyCommandStatus,
    IfxBtVendorResponsePolicyVendorSpecific
} IFXBT_VENDOR_RESPONSE_POLICY;

typedef enum _IFXBT_FIRMWARE_BAUD_SWITCH_REQUIREMENT {
    IfxBtFirmwareBaudSwitchRequirementUnknown = 0,
    IfxBtFirmwareBaudSwitchNotRequired,
    IfxBtFirmwareBaudSwitchRequired
} IFXBT_FIRMWARE_BAUD_SWITCH_REQUIREMENT;

typedef enum _IFXBT_FIRMWARE_D3_RETENTION_POLICY {
    IfxBtFirmwareD3RetentionUnknown = 0,
    IfxBtFirmwareD3RetentionRetained,
    IfxBtFirmwareD3RetentionReloadRequired
} IFXBT_FIRMWARE_D3_RETENTION_POLICY;

typedef struct _IFXBT_FIRMWARE_CONTRACT {
    ULONG Size;
    IFXBT_FIRMWARE_REQUIREMENT FirmwareRequirement;
    IFXBT_FIRMWARE_FILE_TYPE FirmwareFileType;
    IFXBT_FIRMWARE_SOURCE FirmwareSource;
    IFXBT_VENDOR_COMMAND_SEQUENCE VendorCommandSequence;
    IFXBT_VENDOR_RESPONSE_POLICY ResponsePolicy;
    IFXBT_FIRMWARE_BAUD_SWITCH_REQUIREMENT BaudSwitchRequirement;
    IFXBT_FIRMWARE_D3_RETENTION_POLICY D3RetentionPolicy;
    PCWSTR FirmwareRequirementReplacement;
    PCWSTR FirmwareFileNameReplacement;
    PCWSTR FirmwareFileFormatReplacement;
    PCWSTR FirmwareSourceReplacement;
    PCWSTR VendorOpcodeSequenceReplacement;
    PCWSTR VendorResponsePolicyReplacement;
    PCWSTR BaudSwitchCommandReplacement;
    PCWSTR ControllerReadyValidationReplacement;
    PCWSTR D3FirmwareRetentionPolicyReplacement;
    BOOLEAN PlaceholderFirmwareContract;
} IFXBT_FIRMWARE_CONTRACT, *PIFXBT_FIRMWARE_CONTRACT;

const IFXBT_FIRMWARE_CONTRACT*
IfxBtFirmwareGetContract(VOID);

NTSTATUS
IfxBtFirmwareValidateContract(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    );

VOID
IfxBtFirmwareLogContract(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    );

NTSTATUS
IfxBtFirmwareEvaluateInitialization(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig,
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    );

NTSTATUS
IfxBtFirmwareValidateControllerReadyPlaceholder(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig,
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    );

VOID
IfxBtFirmwareLogControllerReadyPlaceholder(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    );

NTSTATUS
IfxBtFirmwareValidatePlaceholderState(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig
    );

VOID
IfxBtFirmwareLogPlaceholderState(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig
    );

#endif
