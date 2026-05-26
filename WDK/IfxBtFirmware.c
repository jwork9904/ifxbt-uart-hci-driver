/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

   IfxBtFirmware.c

Abstract:

    Placeholder firmware/vendor initialization scaffold for the Infineon
    CYW55571 Bluetooth UART transport.

Environment:

    Kernel mode only

--*/

#include "driver.h"
#include "IfxBtPlatform.h"
#include "IfxBtFirmware.h"
#include "IfxBtFirmware.tmh"

#define TODO_REAL_FIRMWARE_REQUIREMENT \
    L"TODO_REAL_FIRMWARE_REQUIREMENT"
#define TODO_REAL_FIRMWARE_FILE_NAME \
    L"TODO_REAL_FIRMWARE_FILE_NAME"
#define TODO_REAL_FIRMWARE_FILE_FORMAT \
    L"TODO_REAL_FIRMWARE_FILE_FORMAT"
#define TODO_REAL_FIRMWARE_SOURCE \
    L"TODO_REAL_FIRMWARE_SOURCE"
#define TODO_REAL_VENDOR_OPCODE_SEQUENCE \
    L"TODO_REAL_VENDOR_OPCODE_SEQUENCE"
#define TODO_REAL_VENDOR_RESPONSE_POLICY \
    L"TODO_REAL_VENDOR_RESPONSE_POLICY"
#define TODO_REAL_BAUD_SWITCH_COMMAND \
    L"TODO_REAL_BAUD_SWITCH_COMMAND"
#define TODO_REAL_CONTROLLER_READY_VALIDATION \
    L"TODO_REAL_CONTROLLER_READY_VALIDATION"
#define TODO_REAL_D3_FIRMWARE_RETENTION_POLICY \
    L"TODO_REAL_D3_FIRMWARE_RETENTION_POLICY"

static const IFXBT_FIRMWARE_CONTRACT IfxBtFirmwareContract = {
    (ULONG)sizeof(IFXBT_FIRMWARE_CONTRACT),
    IfxBtFirmwareRequirementUnknown,
    IfxBtFirmwareFileTypeUnknown,
    IfxBtFirmwareSourceUnknown,
    IfxBtVendorCommandSequenceUnknown,
    IfxBtVendorResponsePolicyUnknown,
    IfxBtFirmwareBaudSwitchRequirementUnknown,
    IfxBtFirmwareD3RetentionUnknown,
    TODO_REAL_FIRMWARE_REQUIREMENT,
    TODO_REAL_FIRMWARE_FILE_NAME,
    TODO_REAL_FIRMWARE_FILE_FORMAT,
    TODO_REAL_FIRMWARE_SOURCE,
    TODO_REAL_VENDOR_OPCODE_SEQUENCE,
    TODO_REAL_VENDOR_RESPONSE_POLICY,
    TODO_REAL_BAUD_SWITCH_COMMAND,
    TODO_REAL_CONTROLLER_READY_VALIDATION,
    TODO_REAL_D3_FIRMWARE_RETENTION_POLICY,
    TRUE
};

static
VOID
IfxBtFirmwareLogInitStage(
    _In_ IFXBT_FIRMWARE_STAGE Stage,
    _In_ NTSTATUS Status
    );

static
VOID
IfxBtFirmwareLogMissingContractPlaceholders(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    );

static
NTSTATUS
IfxBtFirmwareValidateContractShape(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IfxBtFirmwareGetContract)
#pragma alloc_text(PAGE, IfxBtFirmwareValidateContract)
#pragma alloc_text(PAGE, IfxBtFirmwareLogContract)
#pragma alloc_text(PAGE, IfxBtFirmwareEvaluateInitialization)
#pragma alloc_text(PAGE, IfxBtFirmwareValidateControllerReadyPlaceholder)
#pragma alloc_text(PAGE, IfxBtFirmwareLogControllerReadyPlaceholder)
#pragma alloc_text(PAGE, IfxBtFirmwareValidatePlaceholderState)
#pragma alloc_text(PAGE, IfxBtFirmwareLogPlaceholderState)
#endif

static
BOOLEAN
IfxBtFirmwareIsValidRequirement(
    _In_ IFXBT_FIRMWARE_REQUIREMENT Requirement
    )
{
    switch (Requirement) {
    case IfxBtFirmwareRequirementUnknown:
    case IfxBtFirmwareRequirementNotRequired:
    case IfxBtFirmwareRequirementRequired:
        return TRUE;

    default:
        return FALSE;
    }
}

static
BOOLEAN
IfxBtFirmwareIsValidFileType(
    _In_ IFXBT_FIRMWARE_FILE_TYPE FileType
    )
{
    switch (FileType) {
    case IfxBtFirmwareFileTypeUnknown:
    case IfxBtFirmwareFileTypeHcdPlaceholder:
    case IfxBtFirmwareFileTypeVendorSpecificPlaceholder:
        return TRUE;

    default:
        return FALSE;
    }
}

static
BOOLEAN
IfxBtFirmwareIsValidSource(
    _In_ IFXBT_FIRMWARE_SOURCE Source
    )
{
    switch (Source) {
    case IfxBtFirmwareSourceUnknown:
    case IfxBtFirmwareSourceFilePath:
    case IfxBtFirmwareSourceRegistryConfigured:
    case IfxBtFirmwareSourcePackagedResourcePlaceholder:
        return TRUE;

    default:
        return FALSE;
    }
}

static
BOOLEAN
IfxBtFirmwareIsValidVendorSequence(
    _In_ IFXBT_VENDOR_COMMAND_SEQUENCE VendorSequence
    )
{
    switch (VendorSequence) {
    case IfxBtVendorCommandSequenceUnknown:
    case IfxBtVendorCommandSequenceTableDrivenPlaceholder:
        return TRUE;

    default:
        return FALSE;
    }
}

static
BOOLEAN
IfxBtFirmwareIsValidResponsePolicy(
    _In_ IFXBT_VENDOR_RESPONSE_POLICY ResponsePolicy
    )
{
    switch (ResponsePolicy) {
    case IfxBtVendorResponsePolicyUnknown:
    case IfxBtVendorResponsePolicyCommandComplete:
    case IfxBtVendorResponsePolicyCommandStatus:
    case IfxBtVendorResponsePolicyVendorSpecific:
        return TRUE;

    default:
        return FALSE;
    }
}

static
BOOLEAN
IfxBtFirmwareIsValidBaudSwitchRequirement(
    _In_ IFXBT_FIRMWARE_BAUD_SWITCH_REQUIREMENT BaudSwitchRequirement
    )
{
    switch (BaudSwitchRequirement) {
    case IfxBtFirmwareBaudSwitchRequirementUnknown:
    case IfxBtFirmwareBaudSwitchNotRequired:
    case IfxBtFirmwareBaudSwitchRequired:
        return TRUE;

    default:
        return FALSE;
    }
}

static
BOOLEAN
IfxBtFirmwareIsValidD3RetentionPolicy(
    _In_ IFXBT_FIRMWARE_D3_RETENTION_POLICY D3RetentionPolicy
    )
{
    switch (D3RetentionPolicy) {
    case IfxBtFirmwareD3RetentionUnknown:
    case IfxBtFirmwareD3RetentionRetained:
    case IfxBtFirmwareD3RetentionReloadRequired:
        return TRUE;

    default:
        return FALSE;
    }
}

static
VOID
IfxBtFirmwareLogInitStage(
    _In_ IFXBT_FIRMWARE_STAGE Stage,
    _In_ NTSTATUS Status
    )
{
    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("FW_INIT stage=%d status=%!STATUS!",
                Stage, Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_HCI, ("FW_INIT stage=%d status=%!STATUS!",
                Stage, Status));
    }
}

static
VOID
IfxBtFirmwareLogMissingContractPlaceholders(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    )
{
    UNREFERENCED_PARAMETER(FirmwareContract);

    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT missing TODO_REAL_FIRMWARE_REQUIREMENT"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT missing TODO_REAL_FIRMWARE_FILE_NAME"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT missing TODO_REAL_FIRMWARE_FILE_FORMAT"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT missing TODO_REAL_FIRMWARE_SOURCE"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT missing TODO_REAL_VENDOR_OPCODE_SEQUENCE"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT missing TODO_REAL_VENDOR_RESPONSE_POLICY"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT missing TODO_REAL_BAUD_SWITCH_COMMAND"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT missing TODO_REAL_CONTROLLER_READY_VALIDATION"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT missing TODO_REAL_D3_FIRMWARE_RETENTION_POLICY"));
}

static
NTSTATUS
IfxBtFirmwareValidateContractShape(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (FirmwareContract == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (FirmwareContract->Size != (ULONG)sizeof(IFXBT_FIRMWARE_CONTRACT)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (!IfxBtFirmwareIsValidRequirement(FirmwareContract->FirmwareRequirement) ||
        !IfxBtFirmwareIsValidFileType(FirmwareContract->FirmwareFileType) ||
        !IfxBtFirmwareIsValidSource(FirmwareContract->FirmwareSource) ||
        !IfxBtFirmwareIsValidVendorSequence(FirmwareContract->VendorCommandSequence) ||
        !IfxBtFirmwareIsValidResponsePolicy(FirmwareContract->ResponsePolicy) ||
        !IfxBtFirmwareIsValidBaudSwitchRequirement(FirmwareContract->BaudSwitchRequirement) ||
        !IfxBtFirmwareIsValidD3RetentionPolicy(FirmwareContract->D3RetentionPolicy)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (FirmwareContract->FirmwareRequirementReplacement == NULL ||
        FirmwareContract->FirmwareFileNameReplacement == NULL ||
        FirmwareContract->FirmwareFileFormatReplacement == NULL ||
        FirmwareContract->FirmwareSourceReplacement == NULL ||
        FirmwareContract->VendorOpcodeSequenceReplacement == NULL ||
        FirmwareContract->VendorResponsePolicyReplacement == NULL ||
        FirmwareContract->BaudSwitchCommandReplacement == NULL ||
        FirmwareContract->ControllerReadyValidationReplacement == NULL ||
        FirmwareContract->D3FirmwareRetentionPolicyReplacement == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

Exit:

    return Status;
}

const IFXBT_FIRMWARE_CONTRACT*
IfxBtFirmwareGetContract(VOID)
{
    PAGED_CODE();

    return &IfxBtFirmwareContract;
}

NTSTATUS
IfxBtFirmwareValidateContract(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_HCI, ("FW_CONTRACT validate entry firmwareContract=%p",
            FirmwareContract));

    Status = IfxBtFirmwareValidateContractShape(FirmwareContract);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    if (FirmwareContract->PlaceholderFirmwareContract ||
        FirmwareContract->FirmwareRequirement == IfxBtFirmwareRequirementUnknown ||
        FirmwareContract->FirmwareFileType == IfxBtFirmwareFileTypeUnknown ||
        FirmwareContract->FirmwareFileType == IfxBtFirmwareFileTypeHcdPlaceholder ||
        FirmwareContract->FirmwareFileType == IfxBtFirmwareFileTypeVendorSpecificPlaceholder ||
        FirmwareContract->FirmwareSource == IfxBtFirmwareSourceUnknown ||
        FirmwareContract->FirmwareSource == IfxBtFirmwareSourcePackagedResourcePlaceholder ||
        FirmwareContract->VendorCommandSequence == IfxBtVendorCommandSequenceUnknown ||
        FirmwareContract->VendorCommandSequence == IfxBtVendorCommandSequenceTableDrivenPlaceholder ||
        FirmwareContract->ResponsePolicy == IfxBtVendorResponsePolicyUnknown ||
        FirmwareContract->BaudSwitchRequirement == IfxBtFirmwareBaudSwitchRequirementUnknown ||
        FirmwareContract->D3RetentionPolicy == IfxBtFirmwareD3RetentionUnknown) {
        Status = STATUS_DEVICE_NOT_READY;
        IfxBtFirmwareLogMissingContractPlaceholders(FirmwareContract);
        goto Exit;
    }

Exit:

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("FW_CONTRACT validate exit status=%!STATUS!",
                Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_HCI, ("FW_CONTRACT validate exit status=%!STATUS!",
                Status));
    }

    return Status;
}

VOID
IfxBtFirmwareLogContract(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    )
{
    PAGED_CODE();

    if (FirmwareContract == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("FW_CONTRACT missing firmware contract"));
        return;
    }

    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT requirement=%d placeholder=%d",
            FirmwareContract->FirmwareRequirement,
            FirmwareContract->PlaceholderFirmwareContract ? 1 : 0));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT file_type=%d placeholder=%d",
            FirmwareContract->FirmwareFileType,
            FirmwareContract->PlaceholderFirmwareContract ? 1 : 0));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT firmware_source=%d placeholder=%d",
            FirmwareContract->FirmwareSource,
            FirmwareContract->PlaceholderFirmwareContract ? 1 : 0));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT vendor_sequence=%d placeholder=%d",
            FirmwareContract->VendorCommandSequence,
            FirmwareContract->PlaceholderFirmwareContract ? 1 : 0));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT response_policy=%d placeholder=%d",
            FirmwareContract->ResponsePolicy,
            FirmwareContract->PlaceholderFirmwareContract ? 1 : 0));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT baud_switch_required=%d placeholder=%d",
            FirmwareContract->BaudSwitchRequirement,
            FirmwareContract->PlaceholderFirmwareContract ? 1 : 0));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT d3_retention_policy=%d placeholder=%d",
            FirmwareContract->D3RetentionPolicy,
            FirmwareContract->PlaceholderFirmwareContract ? 1 : 0));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT replacement=%S",
            FirmwareContract->FirmwareRequirementReplacement));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT replacement=%S",
            FirmwareContract->FirmwareFileNameReplacement));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT replacement=%S",
            FirmwareContract->FirmwareFileFormatReplacement));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT replacement=%S",
            FirmwareContract->FirmwareSourceReplacement));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT replacement=%S",
            FirmwareContract->VendorOpcodeSequenceReplacement));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT replacement=%S",
            FirmwareContract->VendorResponsePolicyReplacement));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT replacement=%S",
            FirmwareContract->BaudSwitchCommandReplacement));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT replacement=%S",
            FirmwareContract->ControllerReadyValidationReplacement));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_CONTRACT replacement=%S",
            FirmwareContract->D3FirmwareRetentionPolicyReplacement));
}

NTSTATUS
IfxBtFirmwareEvaluateInitialization(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig,
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    IFXBT_FIRMWARE_STAGE Stage = IfxBtFirmwareStageNotStarted;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_HCI, ("FW_INIT evaluate entry platformConfig=%p firmwareContract=%p",
            PlatformConfig, FirmwareContract));

    IfxBtFirmwareLogInitStage(Stage, STATUS_SUCCESS);

    Stage = IfxBtFirmwareStageValidatePlatformReadiness;
    Status = IfxBtPlatformValidateConfig(PlatformConfig);
    IfxBtFirmwareLogInitStage(Stage, Status);
    if (!NT_SUCCESS(Status)) {
        goto Failed;
    }

    Stage = IfxBtFirmwareStageValidateFirmwareContract;
    Status = IfxBtFirmwareValidateContractShape(FirmwareContract);
    IfxBtFirmwareLogInitStage(Stage, Status);
    if (!NT_SUCCESS(Status)) {
        goto Failed;
    }

    IfxBtFirmwareLogContract(FirmwareContract);

    Stage = IfxBtFirmwareStageDetermineFirmwareRequirement;
    Status = IfxBtFirmwareValidateContract(FirmwareContract);
    IfxBtFirmwareLogInitStage(Stage, Status);
    if (!NT_SUCCESS(Status)) {
        goto Failed;
    }

    Stage = IfxBtFirmwareStageLoadFirmwareBlob;
    if (FirmwareContract->FirmwareRequirement == IfxBtFirmwareRequirementRequired) {
        Status = STATUS_NOT_IMPLEMENTED;
        IfxBtFirmwareLogInitStage(Stage, Status);
        goto Failed;
    }
    IfxBtFirmwareLogInitStage(Stage, STATUS_SUCCESS);

    Stage = IfxBtFirmwareStageParseFirmwareImage;
    if (FirmwareContract->FirmwareRequirement == IfxBtFirmwareRequirementRequired) {
        Status = STATUS_NOT_IMPLEMENTED;
        IfxBtFirmwareLogInitStage(Stage, Status);
        goto Failed;
    }
    IfxBtFirmwareLogInitStage(Stage, STATUS_SUCCESS);

    Stage = IfxBtFirmwareStageSendVendorCommand;
    Status = STATUS_NOT_IMPLEMENTED;
    IfxBtFirmwareLogInitStage(Stage, Status);

Failed:

    if (Status == STATUS_DEVICE_NOT_READY) {
        DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_INIT placeholder_blocking_initialization status=%!STATUS!",
                Status));
    }

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_INIT no_firmware_blob_loaded_placeholder=1"));
        DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_INIT no_vendor_commands_sent_placeholder=1"));
        DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FW_INIT no_baud_switch_placeholder=1"));
        Stage = IfxBtFirmwareStageFailed;
        IfxBtFirmwareLogInitStage(Stage, Status);
    }
    else {
        Stage = IfxBtFirmwareStageComplete;
        IfxBtFirmwareLogInitStage(Stage, Status);
    }

    DoTrace(LEVEL_INFO, TFLAG_HCI, ("FW_INIT evaluate exit status=%!STATUS!",
            Status));

    return Status;
}

VOID
IfxBtFirmwareLogControllerReadyPlaceholder(
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    )
{
    PAGED_CODE();

    if (FirmwareContract == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("CTRL_READY missing firmware contract"));
        DoTrace(LEVEL_WARNING, TFLAG_HCI, ("CTRL_READY no_hci_commands_sent_placeholder=1"));
        DoTrace(LEVEL_WARNING, TFLAG_HCI, ("CTRL_READY no_response_wait_placeholder=1"));
        return;
    }

    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("CTRL_READY validation_replacement=%S",
            FirmwareContract->ControllerReadyValidationReplacement));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("CTRL_READY no_hci_commands_sent_placeholder=1"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("CTRL_READY no_response_wait_placeholder=1"));
}

NTSTATUS
IfxBtFirmwareValidateControllerReadyPlaceholder(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig,
    _In_ const IFXBT_FIRMWARE_CONTRACT* FirmwareContract
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_HCI, ("CTRL_READY validate_placeholder entry platformConfig=%p firmwareContract=%p",
            PlatformConfig, FirmwareContract));

    Status = IfxBtPlatformValidateConfig(PlatformConfig);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    Status = IfxBtFirmwareValidateContractShape(FirmwareContract);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    IfxBtFirmwareLogControllerReadyPlaceholder(FirmwareContract);

    Status = STATUS_DEVICE_NOT_READY;
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("CTRL_READY placeholder_blocking_validation status=%!STATUS!",
            Status));

Exit:

    IfxBtFirmwareLogInitStage(IfxBtFirmwareStageValidateControllerReady,
                              Status);

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("CTRL_READY validate_placeholder exit status=%!STATUS!",
                Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_HCI, ("CTRL_READY validate_placeholder exit status=%!STATUS!",
                Status));
    }

    return Status;
}

NTSTATUS
IfxBtFirmwareValidatePlaceholderState(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER validate entry platformConfig=%p",
            PlatformConfig));

    if (PlatformConfig == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER validate missing platform config status=%!STATUS!",
                Status));
        goto Exit;
    }

    if (PlatformConfig->FirmwareSequenceState != IfxBtPlatformFirmwareSequencePlaceholderUnknown) {
        Status = STATUS_INVALID_PARAMETER;
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER validate unsupported firmware sequence state=%d status=%!STATUS!",
                PlatformConfig->FirmwareSequenceState,
                Status));
        goto Exit;
    }

Exit:

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER validate exit status=%!STATUS!",
                Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER validate placeholder_unknown accepted state=%d status=%!STATUS!",
                PlatformConfig->FirmwareSequenceState,
                Status));
    }

    return Status;
}

VOID
IfxBtFirmwareLogPlaceholderState(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig
    )
{
    PAGED_CODE();

    if (PlatformConfig == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER missing platform config"));
        return;
    }

    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER active state=%d",
            PlatformConfig->FirmwareSequenceState));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER TODO_REAL_FIRMWARE_SEQUENCE pending"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER firmware file type name and path unknown"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER firmware binary format unknown no parser selected"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER vendor command sequence unknown"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER expected vendor responses unknown"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER baud-switch sequence unknown"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER no firmware download performed"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER no vendor HCI commands sent"));
    DoTrace(LEVEL_WARNING, TFLAG_HCI, ("FIRMWARE_PLACEHOLDER no HCI Reset sent no baud switch performed"));
}
