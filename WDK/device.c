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
#include "Device.tmh"
    
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DeviceQueryDeviceParameters)
#endif

#define STR_BAUDRATE    L"BaudRateIndex"  

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
    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceDisableWakeControl exit device=%p", _Device));
   
    return;
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

    DoTrace(LEVEL_INFO, TFLAG_UART, ("DEVICE_STUB DeviceInitialize entry fdoExtension=%p ioTarget=%p request=%p isUartReset=%d",
            _FdoExtension, _IoTargetSerial, _RequestSync, _IsUartReset));

    //
    // Vendor specifc operation;
    // 

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
    DoTrace(LEVEL_INFO, TFLAG_POWER, ("DEVICE_STUB DeviceDoPLDR exit fdo=%p", _Fdo));
}
