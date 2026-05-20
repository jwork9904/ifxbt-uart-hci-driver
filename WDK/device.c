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
    NTSTATUS Status = STATUS_SUCCESS;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceEnableWakeControl entry device=%p powerState=%d",
            _Device, _PowerState));

    IfxBtDeviceLogPowerPlaceholderState(L"DeviceEnableWakeControl");

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("WAKE_PLACEHOLDER DeviceEnableWakeControl no HOST_WAKE interrupt programmed no DEV_WAKE protocol applied"));
    
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

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("WAKE_PLACEHOLDER DeviceDisableWakeControl no wake GPIO or interrupt state changed"));

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceDisableWakeControl exit device=%p", _Device));
   
    return;
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

    DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_CONFIG IfxBtConfigureUart entry fdoExtension=%p platformConfig=%p isUartReset=%d",
            FdoExtension, PlatformConfig, IsUartReset));

    if (FdoExtension == NULL || PlatformConfig == NULL)
    {
        Status = STATUS_INVALID_PARAMETER;
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONFIG missing required context fdoExtension=%p platformConfig=%p status=%!STATUS!",
                FdoExtension, PlatformConfig, Status));
        goto Exit;
    }

    if (FdoExtension->IoTargetSerial == NULL || FdoExtension->RequestIoctlSync == NULL)
    {
        Status = STATUS_DEVICE_NOT_READY;
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONFIG missing UART target or sync request ioTarget=%p request=%p status=%!STATUS!",
                FdoExtension->IoTargetSerial, FdoExtension->RequestIoctlSync, Status));
        goto Exit;
    }

    if (PlatformConfig->PlaceholderPlatformValue ||
        PlatformConfig->InitialUartBaudPlaceholder == 0 ||
        PlatformConfig->UartFlowControlPlaceholder == IfxBtPlatformUartFlowControlPlaceholderUnknown)
    {
        Status = STATUS_DEVICE_NOT_READY;
        DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG placeholder config active placeholder=%d status=%!STATUS!",
                PlatformConfig->PlaceholderPlatformValue, Status));
        DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG uart baud unknown value=%lu real platform baud pending",
                PlatformConfig->InitialUartBaudPlaceholder));
        DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG uart flow control unknown value=%d real platform flow control pending",
                PlatformConfig->UartFlowControlPlaceholder));
        DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG line control pending real platform data"));
        DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG no serial IOCTLs issued"));
        goto Exit;
    }

    //
    // Real UART programming is intentionally deferred until platform baud,
    // line-control, and flow-control values are provided.
    //
    Status = STATUS_NOT_IMPLEMENTED;
    DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_CONFIG real UART values present but IOCTL programming is not implemented status=%!STATUS!",
            Status));
    DoTrace(LEVEL_WARNING, TFLAG_UART, ("UART_CONFIG no serial IOCTLs issued"));

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
    NTSTATUS PlatformStatus;
    NTSTATUS FirmwareStatus;
    NTSTATUS UartStatus;

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
        if (PlatformConfig->PlaceholderPlatformValue ||
            PlatformConfig->InitialUartBaudPlaceholder == 0 ||
            PlatformConfig->UartFlowControlPlaceholder == IfxBtPlatformUartFlowControlPlaceholderUnknown) {
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
    NTSTATUS Status = STATUS_SUCCESS;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceEnable entry device=%p enabled=%d", _Device, _IsEnabled));

    IfxBtDeviceLogPowerPlaceholderState(L"DeviceEnable");

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("GPIO_PLACEHOLDER DeviceEnable enabled=%d no BT_REG_ON or reset GPIO toggled",
            _IsEnabled));
    
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
    NTSTATUS Status = STATUS_SUCCESS;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DevicePowerOn entry device=%p", _Device));

    IfxBtDeviceLogPowerPlaceholderState(L"DevicePowerOn");

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_PLACEHOLDER DevicePowerOn no regulator control no GPIO toggles no reset timing no power sequencing"));
    
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
    NTSTATUS Status = STATUS_SUCCESS;
    
    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DevicePowerOff entry device=%p", _Device));

    IfxBtDeviceLogPowerPlaceholderState(L"DevicePowerOff");

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER_PLACEHOLDER DevicePowerOff no-op placeholder no regulator control no GPIO toggles"));

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
    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceDoPLDR entry fdo=%p", _Fdo));

    IfxBtDeviceLogPowerPlaceholderState(L"DeviceDoPLDR");

    DoTrace(LEVEL_WARNING, TFLAG_POWER, ("RESET_PLACEHOLDER DeviceDoPLDR no hardware reset asserted no reset timing applied"));

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceDoPLDR exit fdo=%p", _Fdo));
}
