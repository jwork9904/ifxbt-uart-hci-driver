/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

   Device.c

Abstract:

    This file handles device specific operations.

Environment:

    Kernel mode only

--*/

#include "driver.h"
#include "IfxBtPlatform.h"
#include "IfxBtFirmware.h"
#include "Device.tmh"
    
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DeviceQueryDeviceParameters)
#endif

#define STR_BAUDRATE    L"BaudRateIndex"  

static
VOID
IfxBtDeviceLogPowerPlaceholderState(
    _In_ PCWSTR Operation
    )
{
    const IFXBT_PLATFORM_CONFIG* PlatformConfig;
    NTSTATUS PowerResourceStatus;

    PlatformConfig = IfxBtPlatformGetConfig();
    PowerResourceStatus = IfxBtPlatformValidatePowerResources(PlatformConfig);

    if (!NT_SUCCESS(PowerResourceStatus)) {
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("POWER_PLACEHOLDER operation=%S power resource validation failed status=%!STATUS!",
                Operation, PowerResourceStatus));
    }

    IfxBtPlatformLogPowerPlaceholderConfig(PlatformConfig);

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_PLACEHOLDER operation=%S scaffold only no GPIO targets opened no hardware operation performed",
            Operation));
}

static
VOID
IfxBtLogPowerConfigStage(
    _In_ IFXBT_POWER_STAGE Stage,
    _In_ NTSTATUS Status
    )
{
    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("POWER_CONFIG stage=%d status=%!STATUS!",
                Stage, Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER_CONFIG stage=%d status=%!STATUS!",
                Stage, Status));
    }
}

static
IFXBT_FAILURE_REASON
IfxBtMapPowerFailureReason(
    _In_opt_ const IFXBT_PLATFORM_POWER_CONFIG* PowerConfig,
    _In_ NTSTATUS Status
    )
{
    if (Status == STATUS_DEVICE_NOT_READY &&
        PowerConfig != NULL &&
        PowerConfig->PlaceholderPowerConfig) {
        return IfxBtFailurePowerResourcesPlaceholder;
    }

    return IfxBtFailurePowerSequenceFailed;
}

static
VOID
IfxBtSetPowerFailure(
    _Inout_opt_ PFDO_EXTENSION FdoExtension,
    _In_opt_ const IFXBT_PLATFORM_POWER_CONFIG* PowerConfig,
    _In_ NTSTATUS Status
    )
{
    IFXBT_FAILURE_REASON FailureReason;

    if (FdoExtension == NULL) {
        return;
    }

    FdoExtension->DeviceInitialized = FALSE;
    FailureReason = IfxBtMapPowerFailureReason(PowerConfig, Status);
    IfxBtSetTransportFailure(FdoExtension, FailureReason, Status);
}

static
VOID
IfxBtRollbackPowerSequence(
    _Inout_opt_ PFDO_EXTENSION FdoExtension,
    _In_ IFXBT_POWER_STAGE FailedStage,
    _In_ NTSTATUS FailureStatus
    )
{
    UNREFERENCED_PARAMETER(FdoExtension);

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONFIG rollback_no_owned_resources failed_stage=%d status=%!STATUS!",
            FailedStage, FailureStatus));
    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONFIG cleanup_no_owned_resources status=%!STATUS!",
            STATUS_SUCCESS));
}

static
NTSTATUS
IfxBtPowerOnController(
    _In_ WDFDEVICE Device,
    _In_ PCWSTR Operation
    )
{
    PFDO_EXTENSION FdoExtension;
    const IFXBT_PLATFORM_CONFIG* PlatformConfig;
    const IFXBT_PLATFORM_POWER_CONFIG* PowerConfig;
    NTSTATUS Status = STATUS_SUCCESS;
    IFXBT_POWER_STAGE Stage = IfxBtPowerStageNotStarted;

    if (Device == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        IfxBtLogPowerConfigStage(IfxBtPowerStageFailed, Status);
        return Status;
    }

    FdoExtension = FdoGetExtension(Device);
    PlatformConfig = IfxBtPlatformGetConfig();
    PowerConfig = IfxBtPlatformGetPowerConfig();

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER_CONFIG power_on entry operation=%S device=%p",
            Operation, Device));

    IfxBtLogPowerConfigStage(Stage, STATUS_SUCCESS);

    Stage = IfxBtPowerStageValidatePlatformConfig;
    Status = IfxBtPlatformValidateConfig(PlatformConfig);
    IfxBtLogPowerConfigStage(Stage, Status);
    if (!NT_SUCCESS(Status)) {
        goto Failed;
    }

    IfxBtPlatformLogPowerConfig(PowerConfig);

    Stage = IfxBtPowerStageValidatePowerResources;
    Status = IfxBtPlatformValidatePowerConfig(PowerConfig);
    IfxBtLogPowerConfigStage(Stage, Status);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_DEVICE_NOT_READY &&
            PowerConfig != NULL &&
            PowerConfig->PlaceholderPowerConfig) {
            DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONFIG placeholder_blocking_power_on status=%!STATUS!",
                    Status));
        }
        goto Failed;
    }

    Stage = IfxBtPowerStageDetermineOwnership;
    IfxBtLogPowerConfigStage(Stage, STATUS_SUCCESS);

    if (PowerConfig->PowerOwnership == IfxBtPowerOwnershipPlatformOwnedAlwaysOn ||
        PowerConfig->PowerOwnership == IfxBtPowerOwnershipAcpiPowerResourceOwned) {
        Stage = IfxBtPowerStageComplete;
        IfxBtLogPowerConfigStage(Stage, STATUS_SUCCESS);
        return STATUS_SUCCESS;
    }

    //
    // Driver-controlled GPIO sequencing is intentionally not implemented until
    // the real Resource Hub target, polarity, timing, and ownership contract
    // are provided.
    //
    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
    DoTrace(LEVEL_ERROR, TFLAG_POWER, ("POWER_CONFIG driver_controlled_power_requires_real_gpio_contract status=%!STATUS!",
            Status));

Failed:

    IfxBtRollbackPowerSequence(FdoExtension, Stage, Status);
    Stage = IfxBtPowerStageFailed;
    IfxBtLogPowerConfigStage(Stage, Status);
    IfxBtSetPowerFailure(FdoExtension, PowerConfig, Status);

    return Status;
}

static
NTSTATUS
IfxBtPowerOffController(
    _In_ WDFDEVICE Device,
    _In_ PCWSTR Operation
    )
{
    const IFXBT_PLATFORM_POWER_CONFIG* PowerConfig;
    NTSTATUS Status;
    IFXBT_POWER_STAGE Stage = IfxBtPowerStageNotStarted;

    if (Device == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        IfxBtLogPowerConfigStage(IfxBtPowerStageFailed, Status);
        return Status;
    }

    PowerConfig = IfxBtPlatformGetPowerConfig();

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER_CONFIG power_off entry operation=%S device=%p",
            Operation, Device));

    IfxBtLogPowerConfigStage(Stage, STATUS_SUCCESS);
    IfxBtPlatformLogPowerConfig(PowerConfig);

    Stage = IfxBtPowerStageValidatePowerResources;
    Status = IfxBtPlatformValidatePowerConfig(PowerConfig);
    IfxBtLogPowerConfigStage(Stage, Status);

    if (Status == STATUS_DEVICE_NOT_READY &&
        PowerConfig != NULL &&
        PowerConfig->PlaceholderPowerConfig) {
        Status = STATUS_SUCCESS;
        DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONFIG cleanup_no_owned_resources status=%!STATUS!",
                Status));
        Stage = IfxBtPowerStageComplete;
        IfxBtLogPowerConfigStage(Stage, Status);
        return Status;
    }

    if (!NT_SUCCESS(Status)) {
        Stage = IfxBtPowerStageFailed;
        IfxBtLogPowerConfigStage(Stage, Status);
        return Status;
    }

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER_CONFIG power_off_no_gpio_actions status=%!STATUS!",
            STATUS_SUCCESS));
    Stage = IfxBtPowerStageComplete;
    IfxBtLogPowerConfigStage(Stage, STATUS_SUCCESS);

    return STATUS_SUCCESS;
}

static
NTSTATUS
IfxBtConfigureWake(
    _In_ WDFDEVICE Device,
    _In_ SYSTEM_POWER_STATE PowerState,
    _In_ BOOLEAN ArmWake
    )
{
    const IFXBT_PLATFORM_POWER_CONFIG* PowerConfig;
    NTSTATUS Status;
    IFXBT_POWER_STAGE Stage;

    if (Device == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        IfxBtLogPowerConfigStage(IfxBtPowerStageFailed, Status);
        return Status;
    }

    PowerConfig = IfxBtPlatformGetPowerConfig();
    Stage = ArmWake ? IfxBtPowerStageConfigureHostWake : IfxBtPowerStageConfigureDevWake;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("WAKE_CONFIG entry device=%p powerState=%d arm=%d",
            Device, PowerState, ArmWake ? 1 : 0));

    IfxBtPlatformLogPowerConfig(PowerConfig);
    Status = IfxBtPlatformValidatePowerConfig(PowerConfig);
    IfxBtLogPowerConfigStage(Stage, Status);

    if (Status == STATUS_DEVICE_NOT_READY &&
        PowerConfig != NULL &&
        PowerConfig->PlaceholderPowerConfig) {
        if (ArmWake) {
            DoTrace(LEVEL_WARNING, TFLAG_POWER, ("WAKE_CONFIG placeholder_blocking_wake_arm status=%!STATUS!",
                    Status));
            return Status;
        }

        Status = STATUS_SUCCESS;
        DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_CONFIG cleanup_no_owned_resources status=%!STATUS!",
                Status));
        return Status;
    }

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (PowerConfig->HostWakeOwnership == IfxBtPowerOwnershipDriverControlledGpio ||
        PowerConfig->DevWakeOwnership == IfxBtPowerOwnershipDriverControlledGpio) {
        Status = STATUS_DEVICE_CONFIGURATION_ERROR;
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("WAKE_CONFIG driver_controlled_wake_requires_real_gpio_contract status=%!STATUS!",
                Status));
        return Status;
    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
IfxBtDoControllerReset(
    _In_ WDFDEVICE Fdo
    )
{
    PFDO_EXTENSION FdoExtension;
    const IFXBT_PLATFORM_POWER_CONFIG* PowerConfig;
    NTSTATUS Status;
    IFXBT_POWER_STAGE Stage = IfxBtPowerStageAssertReset;

    if (Fdo == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        IfxBtLogPowerConfigStage(IfxBtPowerStageFailed, Status);
        return Status;
    }

    FdoExtension = FdoGetExtension(Fdo);
    PowerConfig = IfxBtPlatformGetPowerConfig();

    IfxBtPlatformLogPowerConfig(PowerConfig);
    Status = IfxBtPlatformValidatePowerConfig(PowerConfig);
    IfxBtLogPowerConfigStage(Stage, Status);

    if (Status == STATUS_DEVICE_NOT_READY &&
        PowerConfig != NULL &&
        PowerConfig->PlaceholderPowerConfig) {
        DoTrace(LEVEL_WARNING, TFLAG_POWER, ("RESET_CONFIG placeholder_reset_unavailable status=%!STATUS!",
                Status));
        IfxBtSetPowerFailure(FdoExtension, PowerConfig, Status);
        return Status;
    }

    if (!NT_SUCCESS(Status)) {
        IfxBtSetPowerFailure(FdoExtension, PowerConfig, Status);
        return Status;
    }

    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
    DoTrace(LEVEL_ERROR, TFLAG_POWER, ("RESET_CONFIG real_reset_sequence_not_implemented status=%!STATUS!",
            Status));
    IfxBtSetPowerFailure(FdoExtension, PowerConfig, Status);

    return Status;
}

VOID
DeviceQueryDeviceParameters(
    _In_ WDFDRIVER  _Driver
)
/*++
Routine Description:

    Query driver's registry location for device specific parameter, such as baudrate.

        HLM\system\CCS\Services\IfxBtUartHci\Parameters\
        
            KeyName/Type/value

Arguments:

    _Driver - WDF Driver object

Return Value: 

    None
    
--*/   
{
    WDFKEY Key;
    NTSTATUS Status;
    UNICODE_STRING ValueName;
    ULONG Value = 0;

    PAGED_CODE();    

    Status = WdfDriverOpenParametersRegistryKey(_Driver,
                                                GENERIC_READ,
                                                WDF_NO_OBJECT_ATTRIBUTES,
                                                &Key
                                                );
    if (NT_SUCCESS(Status)) {        

        RtlInitUnicodeString(&ValueName, STR_BAUDRATE);
        Status = WdfRegistryQueryULong(Key, &ValueName, &Value);
        
        if (NT_SUCCESS(Status)) {
            // Vendor: can cache and use this values.
        }
       
        WdfRegistryClose(Key);       
    }
    
}


NTSTATUS
DeviceEnableWakeControl(
    _In_  WDFDEVICE          _Device,
    _In_  SYSTEM_POWER_STATE _PowerState    
    )
/*++

Routine Description:

    Vendor: This is a device specific function, and it arms the wake mechanism
    for this driver to receive the wake signal.  This could be using an 
    HOST_WAKE GPIO interrupt, or inband CTS/RTS mechanism.  

Arguments:

    _Device - WDF Device object
    _PowerState - Context used for reading data from target UART device

Return Value:

    NTSTATUS
    
--*/      
{
    NTSTATUS Status;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceEnableWakeControl entry device=%p powerState=%d",
            _Device, _PowerState));

    IfxBtDeviceLogPowerPlaceholderState(L"DeviceEnableWakeControl");

    Status = IfxBtConfigureWake(_Device, _PowerState, TRUE);
    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("WAKE_PLACEHOLDER DeviceEnableWakeControl wake arm unavailable status=%!STATUS!",
                Status));
    }
    
    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceEnableWakeControl exit status=%!STATUS!", Status));

    return Status;
}

VOID
DeviceDisableWakeControl(
    WDFDEVICE _Device
    )
/*++

Routine Description:

    Vendor: This is a device specific function, and it disarms the wake mechanism
    for this driver to receive the wake signal.  

Arguments:

    _Device - WDF Device object

Return Value:

    VOID
    
--*/     
{
    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceDisableWakeControl entry device=%p", _Device));

    IfxBtDeviceLogPowerPlaceholderState(L"DeviceDisableWakeControl");

    (VOID)IfxBtConfigureWake(_Device, PowerSystemWorking, FALSE);

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("WAKE_PLACEHOLDER DeviceDisableWakeControl no wake GPIO or interrupt state changed"));

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceDisableWakeControl exit device=%p", _Device));
   
    return;
}

static
NTSTATUS
IfxBtSendUartConfigIoctl(
    _In_ PFDO_EXTENSION FdoExtension,
    _In_ IFXBT_UART_CONFIG_STAGE Stage,
    _In_ ULONG IoControlCode,
    _In_ ULONG InBufferSize,
    _In_reads_bytes_(InBufferSize) PVOID InBuffer
    )
{
    NTSTATUS Status;
    ULONG_PTR BytesWritten = 0;

    Status = FdoWriteToDeviceSync(FdoExtension->IoTargetSerial,
                                  FdoExtension->RequestIoctlSync,
                                  IoControlCode,
                                  InBufferSize,
                                  InBuffer,
                                  &BytesWritten);

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONFIG serial_ioctl stage=%d code=0x%x bytes=%d status=%!STATUS!",
                Stage, IoControlCode, (ULONG)BytesWritten, Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONFIG serial_ioctl stage=%d code=0x%x bytes=%d status=%!STATUS!",
                Stage, IoControlCode, (ULONG)BytesWritten, Status));
    }

    return Status;
}

static
VOID
IfxBtLogUartConfigStage(
    _In_ IFXBT_UART_CONFIG_STAGE Stage,
    _In_ NTSTATUS Status
    )
{
    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONFIG stage=%d status=%!STATUS!",
                Stage, Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONFIG stage=%d status=%!STATUS!",
                Stage, Status));
    }
}

static
VOID
IfxBtSetUartConfigFailure(
    _Inout_opt_ PFDO_EXTENSION FdoExtension,
    _In_ IFXBT_FAILURE_REASON FailureReason,
    _In_ NTSTATUS Status
    )
{
    if (FdoExtension == NULL) {
        return;
    }

    FdoExtension->LastFailureReason = FailureReason;
    FdoExtension->LastFailureStatus = Status;

    DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONFIG failure reason=%d status=%!STATUS!",
            FailureReason, Status));
}

static
VOID
IfxBtRollbackPartialUartConfig(
    _Inout_opt_ PFDO_EXTENSION FdoExtension,
    _In_ IFXBT_UART_CONFIG_STAGE FailedStage,
    _In_ NTSTATUS FailureStatus
    )
{
    if (FdoExtension != NULL) {
        FdoExtension->DeviceInitialized = FALSE;
    }

    //
    // Future rollback policy: once real board values are active, a failure after
    // any serial-setting stage must leave the parent transport closed to child
    // exposure. If a later phase captures previous serial state, restore it here;
    // otherwise the failed PrepareHardware/D0 path will tear down the target.
    //
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG partial_config_rollback_hook stage=%d status=%!STATUS!",
            FailedStage, FailureStatus));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG partial_config_fail_closed=1"));
}

static
VOID
IfxBtConvertUartTimeoutsConfig(
    _In_ const IFXBT_PLATFORM_UART_TIMEOUTS_CONFIG* PlatformTimeouts,
    _Out_ SERIAL_TIMEOUTS* SerialTimeouts
    )
{
    RtlZeroMemory(SerialTimeouts, sizeof(*SerialTimeouts));

    SerialTimeouts->ReadIntervalTimeout =
        PlatformTimeouts->ReadIntervalTimeout;
    SerialTimeouts->ReadTotalTimeoutMultiplier =
        PlatformTimeouts->ReadTotalTimeoutMultiplier;
    SerialTimeouts->ReadTotalTimeoutConstant =
        PlatformTimeouts->ReadTotalTimeoutConstant;
    SerialTimeouts->WriteTotalTimeoutMultiplier =
        PlatformTimeouts->WriteTotalTimeoutMultiplier;
    SerialTimeouts->WriteTotalTimeoutConstant =
        PlatformTimeouts->WriteTotalTimeoutConstant;
}

static
VOID
IfxBtConvertUartHandflowConfig(
    _In_ const IFXBT_PLATFORM_UART_HANDFLOW_CONFIG* PlatformHandflow,
    _Out_ SERIAL_HANDFLOW* SerialHandflow
    )
{
    RtlZeroMemory(SerialHandflow, sizeof(*SerialHandflow));

    SerialHandflow->ControlHandShake = PlatformHandflow->ControlHandShake;
    SerialHandflow->FlowReplace = PlatformHandflow->FlowReplace;
    SerialHandflow->XonLimit = PlatformHandflow->XonLimit;
    SerialHandflow->XoffLimit = PlatformHandflow->XoffLimit;
}

static
ULONG
IfxBtConvertUartPurgeMask(
    _In_ ULONG PlatformPurgeMask
    )
{
    ULONG SerialPurgeMask = 0;

    if ((PlatformPurgeMask & IfxBtPlatformUartPurgeTxAbort) != 0) {
        SerialPurgeMask |= SERIAL_PURGE_TXABORT;
    }

    if ((PlatformPurgeMask & IfxBtPlatformUartPurgeRxAbort) != 0) {
        SerialPurgeMask |= SERIAL_PURGE_RXABORT;
    }

    if ((PlatformPurgeMask & IfxBtPlatformUartPurgeTxClear) != 0) {
        SerialPurgeMask |= SERIAL_PURGE_TXCLEAR;
    }

    if ((PlatformPurgeMask & IfxBtPlatformUartPurgeRxClear) != 0) {
        SerialPurgeMask |= SERIAL_PURGE_RXCLEAR;
    }

    return SerialPurgeMask;
}

static
NTSTATUS
IfxBtConfigureUart(
    _In_ PFDO_EXTENSION FdoExtension,
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig,
    _In_ BOOLEAN IsUartReset
    )
{
    NTSTATUS Status = STATUS_DEVICE_NOT_READY;
    IFXBT_UART_CONFIG_STAGE Stage = IfxBtUartConfigStageNotStarted;
    const IFXBT_PLATFORM_UART_CONFIG* UartConfig;
    BOOLEAN PartialConfigApplied = FALSE;
    SERIAL_BAUD_RATE BaudRate;
    SERIAL_LINE_CONTROL LineControl;
    SERIAL_TIMEOUTS Timeouts;
    SERIAL_HANDFLOW Handflow;
    ULONG PurgeMask;

    DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONFIG IfxBtConfigureUart entry fdoExtension=%p platformConfig=%p isUartReset=%d",
            FdoExtension, PlatformConfig, IsUartReset));

    IfxBtLogUartConfigStage(Stage, STATUS_SUCCESS);

    Stage = IfxBtUartConfigStageValidateTarget;
    IfxBtLogUartConfigStage(Stage, STATUS_SUCCESS);

    if (FdoExtension == NULL || PlatformConfig == NULL)
    {
        Status = STATUS_INVALID_PARAMETER;
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONFIG missing required context fdoExtension=%p platformConfig=%p status=%!STATUS!",
                FdoExtension, PlatformConfig, Status));
        Stage = IfxBtUartConfigStageFailed;
        IfxBtLogUartConfigStage(Stage, Status);
        goto Exit;
    }

    if (FdoExtension->IoTargetSerial == NULL || FdoExtension->RequestIoctlSync == NULL)
    {
        Status = STATUS_DEVICE_NOT_READY;
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONFIG missing UART target or sync request ioTarget=%p request=%p status=%!STATUS!",
                FdoExtension->IoTargetSerial, FdoExtension->RequestIoctlSync, Status));
        IfxBtSetUartConfigFailure(FdoExtension,
                                  IfxBtFailureUartConfigFailed,
                                  Status);
        Stage = IfxBtUartConfigStageFailed;
        IfxBtLogUartConfigStage(Stage, Status);
        goto Exit;
    }

    Stage = IfxBtUartConfigStageValidatePlatformConfig;
    IfxBtLogUartConfigStage(Stage, STATUS_SUCCESS);

    UartConfig = IfxBtPlatformGetUartConfig();
    IfxBtPlatformLogUartConfig(UartConfig);

    Status = IfxBtPlatformValidateUartConfig(UartConfig);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_DEVICE_NOT_READY &&
            UartConfig != NULL &&
            UartConfig->PlaceholderUartConfig) {
            IfxBtSetUartConfigFailure(FdoExtension,
                                      IfxBtFailureUartConfigPlaceholder,
                                      Status);
            DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG no_serial_ioctls_issued_placeholder=1"));
            DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG placeholder_blocking_initialization status=%!STATUS!",
                    Status));
            DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG future_real_path_not_executed_placeholder=1"));
        }
        else {
            IfxBtSetUartConfigFailure(FdoExtension,
                                      IfxBtFailureUartConfigFailed,
                                      Status);
        }
        Stage = IfxBtUartConfigStageFailed;
        IfxBtLogUartConfigStage(Stage, Status);
        goto Exit;
    }

    //
    // Real-value execution order:
    // 1. Program the initial baud for host/controller synchronization.
    // 2. Program line control from platform-provided data bits/parity/stop bits.
    // 3. Convert neutral platform handflow config to SERIAL_HANDFLOW locally,
    //    then program handflow.
    // 4. Convert neutral platform timeout config to SERIAL_TIMEOUTS locally,
    //    then program timeouts.
    // 5. Purge only when the platform purge mask explicitly requires it.
    //
    // The WDK serial structs stay in this UART application layer so the shared
    // platform contract header does not include ntddser.h.
    //
    // OperationalBaud is intentionally logged and validated here, but the actual
    // baud switch belongs to the later firmware/vendor baud-synchronization
    // phase. This function does not send HCI Reset or vendor commands.
    //
    Stage = IfxBtUartConfigStageApplyBaudRate;
    IfxBtLogUartConfigStage(Stage, STATUS_SUCCESS);
    RtlZeroMemory(&BaudRate, sizeof(BaudRate));
    BaudRate.BaudRate = UartConfig->InitialBaud;
    Status = IfxBtSendUartConfigIoctl(FdoExtension,
                                      Stage,
                                      IOCTL_SERIAL_SET_BAUD_RATE,
                                      (ULONG)sizeof(BaudRate),
                                      &BaudRate);
    if (!NT_SUCCESS(Status)) {
        goto ApplyFailed;
    }
    PartialConfigApplied = TRUE;

    Stage = IfxBtUartConfigStageApplyLineControl;
    IfxBtLogUartConfigStage(Stage, STATUS_SUCCESS);
    RtlZeroMemory(&LineControl, sizeof(LineControl));
    LineControl.WordLength = UartConfig->DataBits;
    LineControl.Parity = UartConfig->Parity;
    LineControl.StopBits = UartConfig->StopBits;
    Status = IfxBtSendUartConfigIoctl(FdoExtension,
                                      Stage,
                                      IOCTL_SERIAL_SET_LINE_CONTROL,
                                      (ULONG)sizeof(LineControl),
                                      &LineControl);
    if (!NT_SUCCESS(Status)) {
        goto ApplyFailed;
    }

    Stage = IfxBtUartConfigStageApplyFlowControl;
    IfxBtLogUartConfigStage(Stage, STATUS_SUCCESS);
    IfxBtConvertUartHandflowConfig(&UartConfig->Handflow,
                                   &Handflow);
    Status = IfxBtSendUartConfigIoctl(FdoExtension,
                                      Stage,
                                      IOCTL_SERIAL_SET_HANDFLOW,
                                      (ULONG)sizeof(Handflow),
                                      &Handflow);
    if (!NT_SUCCESS(Status)) {
        goto ApplyFailed;
    }

    Stage = IfxBtUartConfigStageApplyTimeouts;
    IfxBtLogUartConfigStage(Stage, STATUS_SUCCESS);
    IfxBtConvertUartTimeoutsConfig(&UartConfig->Timeouts,
                                   &Timeouts);
    Status = IfxBtSendUartConfigIoctl(FdoExtension,
                                      Stage,
                                      IOCTL_SERIAL_SET_TIMEOUTS,
                                      (ULONG)sizeof(Timeouts),
                                      &Timeouts);
    if (!NT_SUCCESS(Status)) {
        goto ApplyFailed;
    }

    Stage = IfxBtUartConfigStagePurge;
    IfxBtLogUartConfigStage(Stage, STATUS_SUCCESS);
    PurgeMask = IfxBtConvertUartPurgeMask(UartConfig->PurgeMask);
    if (PurgeMask != 0) {
        Status = IfxBtSendUartConfigIoctl(FdoExtension,
                                          Stage,
                                          IOCTL_SERIAL_PURGE,
                                          (ULONG)sizeof(PurgeMask),
                                          &PurgeMask);
        if (!NT_SUCCESS(Status)) {
            goto ApplyFailed;
        }
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONFIG purge_not_required stage=%d status=%!STATUS!",
                Stage, STATUS_SUCCESS));
    }

    Stage = IfxBtUartConfigStageComplete;
    Status = STATUS_SUCCESS;
    IfxBtLogUartConfigStage(Stage, Status);
    goto Exit;

ApplyFailed:

    IfxBtSetUartConfigFailure(FdoExtension,
                              IfxBtFailureUartConfigFailed,
                              Status);

    if (PartialConfigApplied) {
        IfxBtRollbackPartialUartConfig(FdoExtension,
                                       Stage,
                                       Status);
    }

    Stage = IfxBtUartConfigStageFailed;
    IfxBtLogUartConfigStage(Stage, Status);

Exit:

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONFIG IfxBtConfigureUart exit status=%!STATUS!", Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONFIG IfxBtConfigureUart exit status=%!STATUS!", Status));
    }

    return Status;
}

BOOLEAN
DeviceInitialize(
    _In_  PFDO_EXTENSION _FdoExtension,
    _In_  WDFIOTARGET    _IoTargetSerial,
    _In_  WDFREQUEST     _RequestSync,
    _In_  BOOLEAN        _IsUartReset
    )
/*++
Routine Description:

    This function perform device specific operations to intialize in order
    to bring the device to operational state.

Arguments:

    _FdoExtension - Function device object extension    

    _IoTargetSerial - IO Target to issue request to serial port

    _RequestSync - A reuseable WDF Request to issue serial control

    -IsUartReset - Is UART Reset is required

Return Value:

    TRUE if initialization is completed and successful; FALSE otherwise.

--*/    
{
    BOOLEAN Initialized = TRUE;
    const IFXBT_PLATFORM_CONFIG* PlatformConfig;
    const IFXBT_PLATFORM_UART_CONFIG* UartConfig;
    NTSTATUS PlatformStatus;
    NTSTATUS FirmwareStatus;
    NTSTATUS UartStatus;
    NTSTATUS PowerStatus;

    DoTrace(LEVEL_INFO, TFLAG_UART, ("DEVICE_STUB DeviceInitialize entry fdoExtension=%p ioTarget=%p request=%p isUartReset=%d",
            _FdoExtension, _IoTargetSerial, _RequestSync, _IsUartReset));

    if (_FdoExtension == NULL || _IoTargetSerial == NULL || _RequestSync == NULL)
    {
        Initialized = FALSE;
        if (_FdoExtension != NULL) {
            _FdoExtension->LastFailureReason = IfxBtFailureUartConfigFailed;
            _FdoExtension->LastFailureStatus = STATUS_INVALID_PARAMETER;
        }
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("DEVICE_STUB DeviceInitialize missing required context fdoExtension=%p ioTarget=%p request=%p",
                _FdoExtension, _IoTargetSerial, _RequestSync));
        goto Exit;
    }

    PlatformConfig = IfxBtPlatformGetConfig();
    PlatformStatus = IfxBtPlatformValidateConfig(PlatformConfig);
    if (!NT_SUCCESS(PlatformStatus))
    {
        Initialized = FALSE;
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("DEVICE_STUB DeviceInitialize platform config validation failed status=%!STATUS!",
                PlatformStatus));
        goto Exit;
    }

    IfxBtPlatformLogConfig(PlatformConfig);
    UartConfig = IfxBtPlatformGetUartConfig();

    FirmwareStatus = IfxBtFirmwareValidatePlaceholderState(PlatformConfig);
    if (!NT_SUCCESS(FirmwareStatus))
    {
        Initialized = FALSE;
        _FdoExtension->LastFailureReason = IfxBtFailureFirmwareSequencePlaceholder;
        _FdoExtension->LastFailureStatus = FirmwareStatus;
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("DEVICE_STUB DeviceInitialize firmware placeholder validation failed status=%!STATUS!",
                FirmwareStatus));
        goto Exit;
    }

    IfxBtFirmwareLogPlaceholderState(PlatformConfig);

    UartStatus = IfxBtConfigureUart(_FdoExtension,
                                    PlatformConfig,
                                    _IsUartReset);
    if (!NT_SUCCESS(UartStatus))
    {
        Initialized = FALSE;
        if (UartConfig != NULL &&
            UartConfig->PlaceholderUartConfig &&
            UartStatus == STATUS_DEVICE_NOT_READY) {
            _FdoExtension->LastFailureReason = IfxBtFailureUartConfigPlaceholder;
        }
        else {
            _FdoExtension->LastFailureReason = IfxBtFailureUartConfigFailed;
        }
        _FdoExtension->LastFailureStatus = UartStatus;
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("DEVICE_STUB DeviceInitialize UART configuration pending or failed status=%!STATUS!",
                UartStatus));
        goto Exit;
    }

    PowerStatus = IfxBtPowerOnController(_FdoExtension->WdfDevice,
                                         L"DeviceInitializePowerReadiness");
    if (!NT_SUCCESS(PowerStatus))
    {
        Initialized = FALSE;
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("DEVICE_STUB DeviceInitialize power readiness failed status=%!STATUS!",
                PowerStatus));
        goto Exit;
    }

Exit:

    DoTrace(LEVEL_INFO, TFLAG_UART, ("DEVICE_STUB DeviceInitialize exit initialized=%d", Initialized));

    return Initialized;
}

NTSTATUS
DeviceEnable(
    _In_ WDFDEVICE _Device,
    _In_ BOOLEAN   _IsEnabled
    )

/*++

Routine Description:

    This function enable/wake serial bus device.

Arguments:

    _Device - Supplies a handle to the framework device object.

    _IsEnabled - Boolean to enable or disable the BT device.
    

Return Value:

    NTSTATUS code. 

--*/

{
    NTSTATUS Status;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceEnable entry device=%p enabled=%d", _Device, _IsEnabled));

    IfxBtDeviceLogPowerPlaceholderState(L"DeviceEnable");

    if (_IsEnabled) {
        Status = IfxBtPowerOnController(_Device, L"DeviceEnable");
    }
    else {
        Status = IfxBtPowerOffController(_Device, L"DeviceEnableDisable");
    }

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("GPIO_PLACEHOLDER DeviceEnable enabled=%d no BT_REG_ON or reset GPIO toggled status=%!STATUS!",
            _IsEnabled, Status));
    
    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceEnable exit status=%!STATUS!", Status));

    return Status;
}


NTSTATUS 
DevicePowerOn(
    _In_  WDFDEVICE _Device
)
/*++

Routine Description:

    This routine powers on the serial bus device

Arguments:

    _Device - Supplies a handle to the framework device object.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DevicePowerOn entry device=%p", _Device));

    IfxBtDeviceLogPowerPlaceholderState(L"DevicePowerOn");

    Status = IfxBtPowerOnController(_Device, L"DevicePowerOn");

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_PLACEHOLDER DevicePowerOn no regulator control no GPIO toggles no reset timing no power sequencing status=%!STATUS!",
            Status));
    
    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DevicePowerOn exit status=%!STATUS!", Status));

    return Status;
}

NTSTATUS 
DevicePowerOff(
    _In_  WDFDEVICE _Device
)
/*++

Routine Description:

    This routine powers off the serial bus device

Arguments:

    _Device - Supplies a handle to the framework device object.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    
    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DevicePowerOff entry device=%p", _Device));

    IfxBtDeviceLogPowerPlaceholderState(L"DevicePowerOff");

    Status = IfxBtPowerOffController(_Device, L"DevicePowerOff");

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_PLACEHOLDER DevicePowerOff no-op placeholder no regulator control no GPIO toggles status=%!STATUS!",
            Status));

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DevicePowerOff exit status=%!STATUS!", Status));

    return Status;
}

_Use_decl_annotations_
VOID
DeviceDoPLDR(
    WDFDEVICE _Fdo
    )
/*++

Routine Description:

    This vendor-specific routine takes appropriate actions necessary to fully reset the device.

Arguments:

    _Fdo - Framework device object representing the FDO.

Return Value:

    VOID.

--*/
{
    NTSTATUS Status;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceDoPLDR entry fdo=%p", _Fdo));

    IfxBtDeviceLogPowerPlaceholderState(L"DeviceDoPLDR");

    Status = IfxBtDoControllerReset(_Fdo);

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("RESET_PLACEHOLDER DeviceDoPLDR no hardware reset asserted no reset timing applied status=%!STATUS!",
            Status));

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceDoPLDR exit fdo=%p status=%!STATUS!", _Fdo, Status));
}
