/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

    Fdo.c

Abstract:

    This module contains routines to handle the function driver
    aspect of the bus driver.

Environment:

    kernel mode only

--*/

#include "driver.h"
#include <reshub.h>
#include "fdo.tmh"

#define BTHX_VALID_WRITE_PACKET_TYPE(type) (type == HciPacketCommand || type == HciPacketAclData)
#define BTHX_VALID_READ_PACKET_TYPE(type)  (type == HciPacketEvent   || type == HciPacketAclData)
#define HCI_TRACE_BYTE_LIMIT 16

static
VOID
TraceHciTxSummary(
    _In_ PBTHX_HCI_READ_WRITE_CONTEXT _HCIContext,
    _In_ ULONG _UartLength,
    _In_ ULONG _PayloadAvailable
    )
{
    UCHAR TraceBytes[HCI_TRACE_BYTE_LIMIT] = {0};
    ULONG BytesToTrace = 0;
    ULONG Index;
    ULONG Opcode = 0;
    const ULONG HciCommandHeaderLength = (ULONG) HCI_COMMAND_HEADER_LEN;

    if (_HCIContext->DataLen && _PayloadAvailable) {
        BytesToTrace = MinToPrint(MinToPrint(_HCIContext->DataLen, _PayloadAvailable), HCI_TRACE_BYTE_LIMIT);
        for (Index = 0; Index < BytesToTrace; Index++) {
            TraceBytes[Index] = _HCIContext->Data[Index];
        }
    }

    if (_HCIContext->Type == (UCHAR) HciPacketCommand) {
        if (_HCIContext->DataLen >= HciCommandHeaderLength &&
            _PayloadAvailable >= HciCommandHeaderLength) {
            Opcode = ((ULONG)_HCIContext->Data[1] << 8) | _HCIContext->Data[0];
            DoTrace(LEVEL_INFO, TFLAG_HCI, ("HCI_TX summary type=%d hciLen=%d uartLen=%d opcode=0x%x",
                    _HCIContext->Type, _HCIContext->DataLen, _UartLength, Opcode));
        }
        else {
            DoTrace(LEVEL_WARNING, TFLAG_HCI, ("HCI_TX summary type=%d hciLen=%d uartLen=%d payloadAvailable=%d opcode=short",
                    _HCIContext->Type, _HCIContext->DataLen, _UartLength, _PayloadAvailable));
        }
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_HCI, ("HCI_TX summary type=%d hciLen=%d uartLen=%d opcode=none",
                _HCIContext->Type, _HCIContext->DataLen, _UartLength));
    }

    DoTrace(LEVEL_VERBOSE, TFLAG_HCI, ("HCI_TX bytes type=%d hciLen=%d bytesLogged=%d data=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            _HCIContext->Type,
            _HCIContext->DataLen,
            BytesToTrace,
            TraceBytes[0],
            TraceBytes[1],
            TraceBytes[2],
            TraceBytes[3],
            TraceBytes[4],
            TraceBytes[5],
            TraceBytes[6],
            TraceBytes[7],
            TraceBytes[8],
            TraceBytes[9],
            TraceBytes[10],
            TraceBytes[11],
            TraceBytes[12],
            TraceBytes[13],
            TraceBytes[14],
            TraceBytes[15]));
}

static
NTSTATUS
ValidateHciTxWriteContext(
    _In_ PBTHX_HCI_READ_WRITE_CONTEXT HciContext,
    _In_ size_t PayloadAvailable
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG DataLen = HciContext->DataLen;
    PUCHAR Data = HciContext->Data;
    ULONG ExpectedLen;
    ULONG Opcode;
    ULONG ParamsCount;
    ULONG AclPayloadLen;
    const ULONG CommandHeaderLength = (ULONG) HCI_COMMAND_HEADER_LEN;
    const ULONG AclHeaderLength = (ULONG) HCI_ACL_HEADER_SIZE;

    if ((size_t) DataLen > PayloadAvailable) {
        Status = STATUS_INVALID_PARAMETER;
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("HCI_TX_VALIDATE invalid_datalen type=%d hciLen=%d payloadAvailable=%d status=%!STATUS!",
                HciContext->Type, DataLen, (ULONG) PayloadAvailable, Status));
        goto Done;
    }

    switch ((BTHX_HCI_PACKET_TYPE) HciContext->Type) {
    case HciPacketCommand:
        if (DataLen < CommandHeaderLength || DataLen > MAX_HCI_CMD_SIZE) {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_HCI, ("HCI_TX_VALIDATE command_invalid_length hciLen=%d minLen=%d maxLen=%d status=%!STATUS!",
                    DataLen, CommandHeaderLength, MAX_HCI_CMD_SIZE, Status));
            goto Done;
        }

        Opcode = ((ULONG) Data[1] << 8) | Data[0];
        ParamsCount = Data[2];
        ExpectedLen = CommandHeaderLength + ParamsCount;

        if (ExpectedLen != DataLen) {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_HCI, ("HCI_TX_VALIDATE command_length_mismatch opcode=0x%x hciLen=%d paramsCount=%d expectedLen=%d status=%!STATUS!",
                    Opcode, DataLen, ParamsCount, ExpectedLen, Status));
            goto Done;
        }
        break;

    case HciPacketAclData:
        if (DataLen < AclHeaderLength || DataLen > MAX_HCI_ACLDATA_SIZE) {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_HCI, ("HCI_TX_VALIDATE acl_invalid_length hciLen=%d minLen=%d maxLen=%d status=%!STATUS!",
                    DataLen, AclHeaderLength, MAX_HCI_ACLDATA_SIZE, Status));
            goto Done;
        }

        AclPayloadLen = ((ULONG) Data[3] << 8) | Data[2];
        if (AclPayloadLen > HCI_MAX_ACL_PAYLOAD_SIZE) {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_HCI, ("HCI_TX_VALIDATE acl_payload_too_large hciLen=%d aclPayloadLen=%d maxPayloadLen=%d status=%!STATUS!",
                    DataLen, AclPayloadLen, HCI_MAX_ACL_PAYLOAD_SIZE, Status));
            goto Done;
        }

        ExpectedLen = AclHeaderLength + AclPayloadLen;
        if (ExpectedLen != DataLen) {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_HCI, ("HCI_TX_VALIDATE acl_length_mismatch hciLen=%d aclPayloadLen=%d expectedLen=%d status=%!STATUS!",
                    DataLen, AclPayloadLen, ExpectedLen, Status));
            goto Done;
        }
        break;

    default:
        Status = STATUS_INVALID_PARAMETER;
        DoTrace(LEVEL_ERROR, TFLAG_HCI, ("HCI_TX_VALIDATE invalid_type type=%d status=%!STATUS!",
                HciContext->Type, Status));
        goto Done;
    }

Done:
    return Status;
}

VOID
IfxBtSetTransportState(
    _Inout_opt_ PFDO_EXTENSION FdoExtension,
    _In_ IFXBT_TRANSPORT_STATE NewState,
    _In_ NTSTATUS Status
    )
{
    IFXBT_TRANSPORT_STATE OldState;

    if (FdoExtension == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("IFXBT_STATE missing_context new=%d status=%!STATUS!",
                NewState, Status));
        return;
    }

    OldState = FdoExtension->TransportState;
    FdoExtension->TransportState = NewState;

    if (NT_SUCCESS(Status)) {
        DoTrace(LEVEL_INFO, TFLAG_PNP, ("IFXBT_STATE old=%d new=%d status=%!STATUS!",
                OldState, NewState, Status));
    }
    else {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("IFXBT_STATE old=%d new=%d status=%!STATUS!",
                OldState, NewState, Status));
    }
}

VOID
IfxBtInvalidateParentReadiness(
    _Inout_opt_ PFDO_EXTENSION FdoExtension,
    _In_ IFXBT_TRANSPORT_STATE NewState,
    _In_ NTSTATUS Status,
    _In_ PCWSTR Reason
    )
{
    IFXBT_TRANSPORT_STATE OldState;
    PCWSTR ReasonText;

    if (FdoExtension == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("IFXBT_READY_INVALIDATE missing_context reason=%S status=%!STATUS!",
                (Reason != NULL) ? Reason : L"<unspecified>",
                Status));
        return;
    }

    if (NewState == IfxBtTransportStateOperational) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("IFXBT_READY_INVALIDATE refused_operational_target status=%!STATUS!",
                Status));
        NewState = IfxBtTransportStateStopping;
    }

    OldState = FdoExtension->TransportState;
    ReasonText = (Reason != NULL) ? Reason : L"<unspecified>";

    FdoExtension->DeviceInitialized = FALSE;

    DoTrace(LEVEL_WARNING, TFLAG_PNP, ("IFXBT_READY_INVALIDATE reason=%S old_state=%d new_state=%d last_reason=%d status=%!STATUS!",
            ReasonText,
            OldState,
            NewState,
            FdoExtension->LastFailureReason,
            Status));

    IfxBtSetTransportState(FdoExtension,
                           NewState,
                           Status);
}

static
VOID
IfxBtSetFailureReason(
    _Inout_opt_ PFDO_EXTENSION FdoExtension,
    _In_ IFXBT_FAILURE_REASON Reason,
    _In_ NTSTATUS Status
    )
{
    if (FdoExtension == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("IFXBT_FAIL missing_context reason=%d status=%!STATUS!",
                Reason, Status));
        return;
    }

    if (Reason != IfxBtFailureNone &&
        NT_SUCCESS(Status)) {
        Status = STATUS_DEVICE_NOT_READY;
    }

    FdoExtension->LastFailureReason = Reason;
    FdoExtension->LastFailureStatus = Status;

    if (Reason == IfxBtFailureNone) {
        DoTrace(LEVEL_INFO, TFLAG_PNP, ("IFXBT_FAIL reason=%d status=%!STATUS!",
                Reason, Status));
    }
    else {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("IFXBT_FAIL reason=%d status=%!STATUS!",
                Reason, Status));
    }

    if (Reason != IfxBtFailureNone) {
        IfxBtInvalidateParentReadiness(FdoExtension,
                                       IfxBtTransportStateFailed,
                                       Status,
                                       L"failure");
    }
}

VOID
IfxBtSetTransportFailure(
    _Inout_opt_ PFDO_EXTENSION FdoExtension,
    _In_ IFXBT_FAILURE_REASON Reason,
    _In_ NTSTATUS Status
    )
{
    IfxBtSetFailureReason(FdoExtension, Reason, Status);
}

static
NTSTATUS
IfxBtPreserveLatchedFailureStatus(
    _In_opt_ PFDO_EXTENSION FdoExtension,
    _In_ NTSTATUS DefaultStatus
    )
{
    NTSTATUS Status = DefaultStatus;

    if (FdoExtension != NULL &&
        FdoExtension->LastFailureReason != IfxBtFailureNone &&
        !NT_SUCCESS(FdoExtension->LastFailureStatus)) {
        Status = FdoExtension->LastFailureStatus;
        DoTrace(LEVEL_WARNING, TFLAG_PNP, ("ORCH preserve_latched_failure reason=%d status=%!STATUS!",
                FdoExtension->LastFailureReason,
                Status));
    }

    return Status;
}

static
BOOLEAN
IfxBtIsParentTransportReady(
    _In_opt_ PFDO_EXTENSION FdoExtension
    )
{
    BOOLEAN Ready;
    NTSTATUS Status;

    if (FdoExtension == NULL) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("IFXBT_READY ready=0 state=%d reason=%d status=%!STATUS!",
                -1, -1, STATUS_INVALID_PARAMETER));
        return FALSE;
    }

    Ready = (FdoExtension->TransportState == IfxBtTransportStateOperational &&
             FdoExtension->LastFailureReason == IfxBtFailureNone &&
             IsDeviceInitialized(FdoExtension) &&
             FdoExtension->IoTargetSerial != NULL);

    Status = Ready ? STATUS_SUCCESS : FdoExtension->LastFailureStatus;
    if (Status == STATUS_SUCCESS && Ready == FALSE) {
        Status = STATUS_DEVICE_NOT_READY;
    }

    if (Ready) {
        DoTrace(LEVEL_INFO, TFLAG_PNP, ("IFXBT_READY ready=%d state=%d reason=%d status=%!STATUS!",
                Ready,
                FdoExtension->TransportState,
                FdoExtension->LastFailureReason,
                Status));
    }
    else {
        DoTrace(LEVEL_WARNING, TFLAG_PNP, ("IFXBT_READY ready=%d state=%d reason=%d status=%!STATUS!",
                Ready,
                FdoExtension->TransportState,
                FdoExtension->LastFailureReason,
                Status));
    }

    return Ready;
}

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, FdoCreateOneChildDevice)
#pragma alloc_text (PAGE, FdoRemoveOneChildDevice)
#pragma alloc_text (PAGE, FdoCreateAllChildren)
#pragma alloc_text (PAGE, FdoFindConnectResources)
#pragma alloc_text (PAGE, FdoDevPrepareHardware)
#pragma alloc_text (PAGE, FdoDevReleaseHardware)
#pragma alloc_text (PAGE, FdoDevSelfManagedIoInit)
#pragma alloc_text (PAGE, FdoDevSelfManagedIoCleanup)
#pragma alloc_text (PAGE, FdoDevD0Exit)
#pragma alloc_text (PAGE, HlpInitializeFdoExtension)
#pragma alloc_text (PAGE, FdoWriteToDeviceSync)
#endif

//
// Child device node, PDO(s), could be enumerated statically if number of PDOs are known
// at driver start, or dynamic enuermation mechanism is used.  Both methods are presented
// in this code, but only one can be chosen using the define macro (see sources file).
//
#ifdef DYNAMIC_ENUM

typedef struct _ENABLE_PDO_CONTEXT {
    WDFDEVICE     Fdo;
} ENABLE_PDO_CONTEXT, *PENABLE_PDO_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(ENABLE_PDO_CONTEXT, GetEnablePdoWorkItemContext)

// Timeout used to delay dev node enuemeration
ULONG g_WaitToEnablePDO = 20000;  // MSec

VOID
DeviceEnablePDOWorker(
    _In_  WDFWORKITEM  _WorkItem
    )
/*++
Routine Description:

    A work item function to dynamically enuermate a PDO.

Arguments:

    _pWorkItem - work item that contains a context to help carrying out its task

Return Value:
--*/
{
    PENABLE_PDO_CONTEXT Context;
    LARGE_INTEGER    RemoteWakeTimeout;

    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_PNP, ("+DeviceEnablePDOWorker"));
    Context = GetEnablePdoWorkItemContext(_WorkItem);

    RemoteWakeTimeout.QuadPart = WDF_REL_TIMEOUT_IN_MS(g_WaitToEnablePDO);
    KeDelayExecutionThread(KernelMode, FALSE, &RemoteWakeTimeout);

    DoTrace(LEVEL_INFO, TFLAG_PNP, ("+Complete the wait"));

    Status = FdoCreateOneChildDeviceDynamic(Context->Fdo,
                                            BT_PDO_HARDWARE_IDS,
                                            sizeof(BT_PDO_HARDWARE_IDS)/sizeof(WCHAR),
                                            BLUETOOTH_FUNC_IDS );

    WdfObjectDelete(_WorkItem);

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("-DeviceEnablePDOWorker %!STATUS!", Status));

}

NTSTATUS
FdoEvtDeviceListCreatePdo(
    WDFCHILDLIST DeviceList,
    PWDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER IdentificationDescription,
    PWDFDEVICE_INIT ChildInit
    )
/*++

Routine Description:

    Called by the framework in response to Query-Device relation when
    a new PDO for a child device needs to be created.

Arguments:

    DeviceList - Handle to the default WDFCHILDLIST created by the framework as part
                 of FDO.

    IdentificationDescription - Decription of the new child device.

    ChildInit - It's a opaque structure used in collecting device settings
                and passed in as a parameter to CreateDevice.

Return Value:

    NT Status code.

--*/
{
    PPDO_IDENTIFICATION_DESCRIPTION pDesc;
    PFDO_EXTENSION FdoExtension;
    WDFDEVICE Fdo;
    NTSTATUS Status;

    PAGED_CODE();

    pDesc = CONTAINING_RECORD(IdentificationDescription,
                              PDO_IDENTIFICATION_DESCRIPTION,
                              Header);

    Fdo = WdfChildListGetDevice(DeviceList);
    FdoExtension = FdoGetExtension(Fdo);

    if (!IfxBtIsParentTransportReady(FdoExtension)) {
        Status = FdoExtension->LastFailureStatus;
        if (Status == STATUS_SUCCESS) {
            Status = STATUS_DEVICE_NOT_READY;
        }
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_child_dynamic callback blocked parent transport not ready serialNo=%d status=%!STATUS!",
                pDesc->SerialNo, Status));
        return Status;
    }

    return PdoCreateDynamic(Fdo,
                            ChildInit,
                            pDesc->HardwareIds,
                            pDesc->SerialNo);
}

NTSTATUS
FdoCreateOneChildDeviceDynamic(
    _In_ WDFDEVICE  _Device,
    _In_ PWCHAR     _HardwareIds,
    _In_ size_t     _CchHardwareIds,
    _In_ ULONG      _SerialNo
    )

/*++

Routine Description:

    The trigger event has been signalled that a new device on the bus has arrived.

    We therefore create a description structure in stack, fill in information about
    the child device and call WdfChildListAddOrUpdateChildDescriptionAsPresent
    to add the device.

--*/

{
    PDO_IDENTIFICATION_DESCRIPTION Description;
    PFDO_EXTENSION  FdoExtension;
    NTSTATUS         Status;

    PAGED_CODE ();

    FdoExtension = FdoGetExtension(_Device);
    if (!IfxBtIsParentTransportReady(FdoExtension)) {
        Status = FdoExtension->LastFailureStatus;
        if (Status == STATUS_SUCCESS) {
            Status = STATUS_DEVICE_NOT_READY;
        }
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_child_dynamic blocked parent transport not ready serialNo=%d status=%!STATUS!",
                _SerialNo, Status));
        return Status;
    }

    //
    // Initialize the description with the information about the newly
    // plugged in device.
    //
    WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(&Description.Header,
                                                     sizeof(Description));

    Description.SerialNo = _SerialNo;
    Description.CchHardwareIds = _CchHardwareIds;
    Description.HardwareIds = _HardwareIds;

    //
    // Call the framework to add this child to the childlist. This call
    // will internaly call our DescriptionCompare callback to check
    // whether this device is a new device or existing device. If
    // it's a new device, the framework will call DescriptionDuplicate to create
    // a copy of this description in nonpaged pool.
    // The actual creation of the child device will happen when the framework
    // receives QUERY_DEVICE_RELATION request from the PNP manager in
    // response to InvalidateDeviceRelations call made as part of adding
    // a new child.
    //
    Status = WdfChildListAddOrUpdateChildDescriptionAsPresent(WdfFdoGetDefaultChildList(_Device),
                                                              &Description.Header,
                                                               NULL); // AddressDescription

    if (Status == STATUS_OBJECT_NAME_EXISTS) {
        //
        // The description is already present in the list, the serial number is
        // not unique, return error.
        //
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

#endif  // ifdef DYNAMIC_ENUM

NTSTATUS
FdoCreateOneChildDevice(
    _In_ WDFDEVICE  _Device,
    _In_ PWSTR      _HardwareIds,
    _In_ ULONG      _SerialNo
    )
/*++

Routine Description:

    Create a new PDO, initialize it, add it to the list of PDOs for this
    FDO bus.

Arguments:

    _Device - WDF device object

    _HardwareIDs - hardware Id for a device

    _SerialNo - Unique ID for a child DO

Returns:

    Status

--*/
{
    NTSTATUS         Status = STATUS_SUCCESS;
    BOOLEAN          IsUnique = TRUE;
    WDFDEVICE        ChildDevice;
    PPDO_EXTENSION   PdoExtension;
    PFDO_EXTENSION   FdoExtension;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_PNP, ("FDO create_child FdoCreateOneChildDevice entry hwid=%S serialNo=%d", _HardwareIds, _SerialNo));

    //
    // First make sure that we don't already have another device with the
    // same serial number.
    // Framework creates a collection of all the child devices we have
    // created so far. So acquire the handle to the collection and lock
    // it before walking the item.
    //
    FdoExtension = FdoGetExtension(_Device);
    ChildDevice = NULL;

    if (!IfxBtIsParentTransportReady(FdoExtension)) {
        Status = FdoExtension->LastFailureStatus;
        if (Status == STATUS_SUCCESS) {
            Status = STATUS_DEVICE_NOT_READY;
        }
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_child blocked parent transport not ready serialNo=%d status=%!STATUS!",
                _SerialNo, Status));
        return Status;
    }

    //
    // We need an additional lock to synchronize addition because
    // WdfFdoLockStaticChildListForIteration locks against anyone immediately
    // updating the static child list (the changes are put on a queue until the
    // list has been unlocked).  This type of lock does not enforce our concept
    // of unique IDs on the bus (ie SerialNo).
    //
    // Without our additional lock, 2 threads could execute this function, both
    // find that the requested SerialNo is not in the list and attempt to add
    // it.  If that were to occur, 2 PDOs would have the same unique SerialNo,
    // which is incorrect.
    //
    // We must use a passive level lock because you can only call WdfDeviceCreate
    // at PASSIVE_LEVEL.
    //
    WdfWaitLockAcquire(FdoExtension->ChildLock, NULL);
    WdfFdoLockStaticChildListForIteration(_Device);

    while ((ChildDevice = WdfFdoRetrieveNextStaticChild(_Device,
                                                        ChildDevice,
                                                        WdfRetrieveAddedChildren)) != NULL) {
        //
        // WdfFdoRetrieveNextStaticChild returns reported and to be reported
        // children (ie children who have been added but not yet reported to PNP).
        //
        // A surprise removed child will not be returned in this list.
        //
        PdoExtension = PdoGetExtension(ChildDevice);

        //
        // It's okay to plug in another device with the same serial number
        // as long as the previous one is in a surprise-removed state. The
        // previous one would be in that state after the device has been
        // physically removed, if somebody has an handle open to it.
        //
        if (_SerialNo == PdoExtension->SerialNo) {
            IsUnique = FALSE;
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_child duplicate serialNo=%d status=%!STATUS!", _SerialNo, Status));
            break;
        }
    }

    if (IsUnique) {
        //
        // Create a new child device.  It is OK to create and add a child while
        // the list locked for enumeration.  The enumeration lock applies only
        // to enumeration, not addition or removal.
        //
        Status = PdoCreate(_Device, _HardwareIds, _SerialNo);
    }

    WdfFdoUnlockStaticChildListFromIteration(_Device);
    WdfWaitLockRelease(FdoExtension->ChildLock);

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_child FdoCreateOneChildDevice failed serialNo=%d status=%!STATUS!", _SerialNo, Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_PNP, ("FDO create_child FdoCreateOneChildDevice exit serialNo=%d status=%!STATUS!", _SerialNo, Status));
    }

    return Status;
}

NTSTATUS
FdoRemoveOneChildDevice(
    WDFDEVICE   _Device,
    ULONG       _SerialNo
    )
/*++

Routine Description:

    The application has told us a device has departed from the bus.

    We therefore need to flag the PDO as no longer present
    and then tell Plug and Play about it.

Arguments:

    _Device - WDF device object

    _SerialNo - Unique ID for a child DO

Returns:

    Status

--*/

{
    PPDO_EXTENSION  PdoExtension;
    BOOLEAN         Found = FALSE;
    BOOLEAN         PlugOutAll;
    WDFDEVICE       ChildDevice;
    NTSTATUS        Status = STATUS_INVALID_PARAMETER;

    PAGED_CODE();

    PlugOutAll = (0 == _SerialNo) ? TRUE : FALSE;

    ChildDevice = NULL;

    WdfFdoLockStaticChildListForIteration(_Device);

    while ((ChildDevice = WdfFdoRetrieveNextStaticChild(_Device,
                                                        ChildDevice,
                                                        WdfRetrieveAddedChildren)) != NULL) {
        if (PlugOutAll) {

            Status = WdfPdoMarkMissing(ChildDevice);
            if(!NT_SUCCESS(Status)) {
                DoTrace(LEVEL_INFO, TFLAG_PNP, ("WdfPdoMarkMissing failed 0x%x\n", Status));
                break;
            }

            Found = TRUE;
        }
        else {
            PdoExtension = PdoGetExtension(ChildDevice);

            if (_SerialNo == PdoExtension->SerialNo) {

                Status = WdfPdoMarkMissing(ChildDevice);
                if(!NT_SUCCESS(Status)) {
                    DoTrace(LEVEL_INFO, TFLAG_PNP, ("WdfPdoMarkMissing failed 0x%x\n", Status));
                    break;
                }

                Found = TRUE;
                break;
            }
        }
    }

    WdfFdoUnlockStaticChildListFromIteration(_Device);

    if (Found) {
        Status = STATUS_SUCCESS;
    }

    return Status;
}

NTSTATUS
FdoCreateAllChildren(
    _In_  WDFDEVICE _Device
    )
/*++
Routine Description:

    The routine enables you to statically enumerate child device functions
    during start.

Arguments:

    _Device - WDF device object

Returns:

    Status

--*/
{
    NTSTATUS       Status;
    PFDO_EXTENSION FdoExtension;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_PNP, ("FDO create_children FdoCreateAllChildren entry"));

    //
    // Bus driver enumerates all child devnode in this function.
    // Vendor Specific: retrieve all statically saved devnode info
    //     HWID, COMPATID, etc.
    //

    //
    // This sample code only enuemrate the Bluetooth function as the only
    // child device.
    //
    FdoExtension = FdoGetExtension(_Device);
    if (!IfxBtIsParentTransportReady(FdoExtension)) {
        Status = FdoExtension->LastFailureStatus;
        if (Status == STATUS_SUCCESS) {
            Status = STATUS_DEVICE_NOT_READY;
        }
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_children blocked parent transport not ready status=%!STATUS!",
                Status));
        return Status;
    }

    Status = FdoCreateOneChildDevice(_Device,
                                     BT_PDO_HARDWARE_IDS,
                                     BLUETOOTH_FUNC_IDS);

    if (NT_SUCCESS(Status)) {
        FdoExtension->IsRadioEnabled = TRUE;
    }

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_children FdoCreateAllChildren failed status=%!STATUS!", Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_PNP, ("FDO create_children FdoCreateAllChildren exit status=%!STATUS!", Status));
    }

    return Status;
}


NTSTATUS
HlpInitializeFdoExtension(
    WDFDEVICE _Device
    )
/*++
Routine Description:

    This helper function initialize the device context.

Arguments:

    _Device - WDF Device object

Return Value:

    Status

--*/
{
    PFDO_EXTENSION        FdoExtension;
    WDF_OBJECT_ATTRIBUTES Attributes;
    NTSTATUS              Status;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_PNP,("+HlpInitializeFdoExtension"));

    FdoExtension = FdoGetExtension(_Device);
    FdoExtension->WdfDevice = _Device;

    //
    // Set Bluetooth (PDO) capabilities
    //    MaxAclTransferInSize - is used by the host to notify the Bluetooth controller
    //        in HCI_Host_Buffer_Size command to set the maximum  size of the data portion
    //        of an HCI ACL packet that will be sent from the controller to the host.
    //        BthMini will only send down an HCI read request with this data buffer size.
    //
    FdoExtension->BthXCaps.MaxAclTransferInSize = MAX_HCI_ACLDATA_SIZE;
    FdoExtension->BthXCaps.ScoSupport = ScoSupportHCIBypass;  // Only option
    FdoExtension->BthXCaps.MaxScoChannels = 1;           // Limit to 1 HCIBypass channel
    FdoExtension->BthXCaps.IsDeviceIdleCapable = TRUE;   // Disable Idle to S0 and wake
    FdoExtension->BthXCaps.IsDeviceWakeCapable = FALSE;  // Wake from Sx

    //
    // Preallocate Request
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&Attributes);
    Attributes.ParentObject = _Device;

    Status = WdfRequestCreate(&Attributes, FdoExtension->IoTargetSerial, &FdoExtension->RequestIoctlSync);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, (" WdfRequestCreate failed %!STATUS!", Status));
        goto Exit;
    }

    FdoExtension->HardwareErrorDetected = FALSE;

    Status = WdfRequestCreate(&Attributes, FdoExtension->IoTargetSerial, &FdoExtension->RequestWaitOnError);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, (" WdfRequestCreate failed %!STATUS!", Status));
        goto Exit;
    }

    Status = WdfMemoryCreatePreallocated(&Attributes,
                                         &FdoExtension->SerErrorMask,
                                         sizeof(FdoExtension->SerErrorMask),
                                         &FdoExtension->WaitMaskMemory);

    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, (" WdfMemoryCreatePreallocated failed %!STATUS!", Status));
        goto Exit;
    }

    Status = WdfSpinLockCreate(&Attributes, &FdoExtension->QueueAccessLock);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, (" WdfSpinLockCreate failed %!STATUS!", Status));
        goto Exit;
    }

Exit:

    return Status;

}

VOID
FdoEvtDeviceDisarmWake(
    _In_  WDFDEVICE  _Device
    )
/*++
Routine Description:

    This function is invoked by the framework after the bus driver determines
    that an event has awakened the device, and after the bus driver subsequently
    completes the wait/wake IRP.

    This function perform any hardware operations that are needed to disable
    the device's ability to trigger a wake signal after the power has been lowered.

Arguments:

    _Device - WDF Device object

Return Value:

    VOID

--*/
{
    DoTrace(LEVEL_INFO, TFLAG_POWER,("POWER FdoEvtDeviceDisarmWake entry device=%p", _Device));
    DeviceDisableWakeControl(_Device);
    DoTrace(LEVEL_INFO, TFLAG_POWER,("POWER FdoEvtDeviceDisarmWake exit device=%p", _Device));
}

NTSTATUS
FdoEvtDeviceArmWake(
    _In_  WDFDEVICE  _Device
    )
/*++
Routine Description:

    This function is invoked while the device is still in the D0 device power state,
    before the bus driver lowers the device's power state but after the framework
    has sent a wait/wake IRP on behalf of the driver.

Arguments:

    _Device - WDF Device object

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS Status;

    DoTrace(LEVEL_INFO, TFLAG_POWER,("POWER FdoEvtDeviceArmWake entry device=%p", _Device));
    Status = DeviceEnableWakeControl(_Device, PowerSystemWorking);
    DoTrace(LEVEL_INFO, TFLAG_POWER,("POWER FdoEvtDeviceArmWake exit device=%p status=%!STATUS!", _Device, Status));

    return Status;
}

NTSTATUS
FdoFindConnectResources(
    _In_ WDFDEVICE    _Device,
    _In_ WDFCMRESLIST _ResourcesRaw,
    _In_ WDFCMRESLIST _ResourcesTranslated
    )
/*++

Routine Description:

    This routine enuermates and finds specific connection resources and cache them.

Arguments:

    _Device - Supplies a handle to a framework device object.

    _ResourcesRaw - Supplies a handle to a collection of framework resource
        objects. This collection identifies the raw (bus-relative) hardware
        resources that have been assigned to the device.

    _ResourcesTranslated - Supplies a handle to a collection of framework
        resource objects. This collection identifies the translated
        (system-physical) hardware resources that have been assigned to the
        device. The resources appear from the CPU's point of view.

Return Value:

    NT Status code.

--*/

{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor;
    PFDO_EXTENSION FdoExtension;
    ULONG Index;
    ULONG ResourceCount = 0;
    NTSTATUS Status;
    BOOLEAN UartConnectionIdIsFound = FALSE;

    UNREFERENCED_PARAMETER(_ResourcesRaw);

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP resource_discovery FdoFindConnectResources entry device=%p", _Device));

    FdoExtension = FdoGetExtension(_Device);

    FdoExtension->UARTConnectionId.QuadPart = 0;
    FdoExtension->I2CConnectionId.QuadPart = 0;
    FdoExtension->GPIOConnectionId.QuadPart = 0;

    Status = STATUS_SUCCESS;

    //
    // Walk through the resource list and find and cache expected resources.
    //

    ResourceCount = WdfCmResourceListGetCount(_ResourcesTranslated);
    DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP resource_discovery translatedCount=%d", ResourceCount));

    for (Index = 0; Index < ResourceCount; Index++)
    {
        Descriptor = WdfCmResourceListGetDescriptor(_ResourcesTranslated, Index);
        if (Descriptor == NULL)
        {
            DoTrace(LEVEL_WARNING, TFLAG_PNP,("PNP resource_discovery null descriptor index=%d", Index));
            continue;
        }

        switch(Descriptor->Type)
        {
        case CmResourceTypeConnection:

            //
            // Cache connetion ID that this BT Peripheral device is connected to
            //     - UART (must exist)
            //     - GPIO (optional)
            //

            if ((Descriptor->u.Connection.Class == CM_RESOURCE_CONNECTION_CLASS_SERIAL) &&
                (Descriptor->u.Connection.Type  == CM_RESOURCE_CONNECTION_TYPE_SERIAL_UART))
            {
                NT_ASSERT(UartConnectionIdIsFound == FALSE && L"More than one set of UART connection");

                UartConnectionIdIsFound = TRUE;

                FdoExtension->UARTConnectionId.LowPart  = Descriptor->u.Connection.IdLowPart;
                FdoExtension->UARTConnectionId.HighPart = Descriptor->u.Connection.IdHighPart;

                DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP resource_discovery UART ConnectionID high=0x%x low=0x%x",
                    FdoExtension->UARTConnectionId.HighPart, FdoExtension->UARTConnectionId.LowPart));
            }
            else if ((Descriptor->u.Connection.Class == CM_RESOURCE_CONNECTION_CLASS_SERIAL) &&
                (Descriptor->u.Connection.Type  == CM_RESOURCE_CONNECTION_TYPE_SERIAL_I2C))
            {

                FdoExtension->I2CConnectionId.LowPart  = Descriptor->u.Connection.IdLowPart;
                FdoExtension->I2CConnectionId.HighPart = Descriptor->u.Connection.IdHighPart;

                DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP resource_discovery I2C ConnectionID high=0x%x low=0x%x",
                    FdoExtension->I2CConnectionId.HighPart, FdoExtension->I2CConnectionId.LowPart));
            }
            else if ((Descriptor->u.Connection.Class == CM_RESOURCE_CONNECTION_CLASS_GPIO) &&
                (Descriptor->u.Connection.Type  == CM_RESOURCE_CONNECTION_TYPE_GPIO_IO))
            {

                FdoExtension->GPIOConnectionId.LowPart  = Descriptor->u.Connection.IdLowPart;
                FdoExtension->GPIOConnectionId.HighPart = Descriptor->u.Connection.IdHighPart;

                DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP resource_discovery GPIO ConnectionID high=0x%x low=0x%x",
                    FdoExtension->GPIOConnectionId.HighPart, FdoExtension->GPIOConnectionId.LowPart));
            }
            break;

        case CmResourceTypeInterrupt:

            //
            // NT Interrupt to support HOST_WAKE for remote wake (TBD)
            //

        default:
            DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP resource_discovery unused type=%d", Descriptor->Type));
            break;
        }

    }

    //
    // Expect to find UART controller
    //
    if (!UartConnectionIdIsFound)
    {
        Status = STATUS_NOT_FOUND;
        DoTrace(LEVEL_ERROR, TFLAG_PNP,("PNP resource_discovery missing required UART connection resource status=%!STATUS!", Status));
        IfxBtSetFailureReason(FdoExtension,
                              IfxBtFailureMissingUartResource,
                              Status);
    }
    else {
        IfxBtSetTransportState(FdoExtension,
                               IfxBtTransportStateResourcesParsed,
                               Status);
    }

    DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP resource_discovery FdoFindConnectResources exit resourceCount=%d status=%!STATUS!", ResourceCount, Status));

    return Status;
}


NTSTATUS
FdoOpenDevice(
    _In_  WDFDEVICE   _Device,
    _Out_ WDFIOTARGET *_pIoTarget
    )
/*++
Routine Description:

    This function search for a serial port and create a remote IO Target object,
    which will be used to send control and data.

Arguments:

    _Device - WDF Device object

    _pIoTarget - IO Target object to be created in this function

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS Status = STATUS_SUCCESS;
    WDFIOTARGET IoTargetSerial = NULL;
    PFDO_EXTENSION FdoExtension = NULL;
    WCHAR TargetDeviceNameBuffer[100];
    UNICODE_STRING TargetDeviceName;

    WDF_IO_TARGET_OPEN_PARAMS   OpenParams;

    DoTrace(LEVEL_INFO, TFLAG_UART,("UART_OPEN FdoOpenDevice entry device=%p", _Device));

    if (_pIoTarget == NULL)
    {
        Status = STATUS_INVALID_PARAMETER;
        DoTrace(LEVEL_ERROR, TFLAG_UART,("UART_OPEN invalid output target pointer status=%!STATUS!", Status));
        goto Exit;
    }

    *_pIoTarget = NULL;

    FdoExtension = FdoGetExtension(_Device);

    //
    // On this ACPI UART transport path, the UART connection ID is required.
    //

    if (!ValidConnectionID(FdoExtension->UARTConnectionId))
    {
        Status = STATUS_NOT_FOUND;
        DoTrace(LEVEL_ERROR, TFLAG_UART,("UART_OPEN missing ACPI UART connection; COM fallback disabled for placeholder/platform path status=%!STATUS!", Status));
        goto Exit;
    }

    DoTrace(LEVEL_INFO, TFLAG_UART,("UART_OPEN using ConnectionID high=0x%x low=0x%x",
            FdoExtension->UARTConnectionId.HighPart,
            FdoExtension->UARTConnectionId.LowPart));

    RtlInitEmptyUnicodeString(&TargetDeviceName,
                              TargetDeviceNameBuffer,
                              sizeof(TargetDeviceNameBuffer));

    Status = RESOURCE_HUB_CREATE_PATH_FROM_ID(&TargetDeviceName,
                                              FdoExtension->UARTConnectionId.LowPart,
                                              FdoExtension->UARTConnectionId.HighPart);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_UART,("UART_OPEN RESOURCE_HUB_CREATE_PATH_FROM_ID failed status=%!STATUS!", Status));
        goto Exit;
    }

    Status = WdfIoTargetCreate(_Device,
                               WDF_NO_OBJECT_ATTRIBUTES,
                               &IoTargetSerial);

    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_UART,("UART_OPEN WdfIoTargetCreate failed status=%!STATUS!", Status));
        goto Exit;
    }

    DoTrace(LEVEL_INFO, TFLAG_UART, ("UART_OPEN targetPath=%S", TargetDeviceName.Buffer));

    //
    // Open the "remote" IO Target (device) using its symbolic link.
    //
    WDF_IO_TARGET_OPEN_PARAMS_INIT_OPEN_BY_NAME(&OpenParams,
                                                &TargetDeviceName,
                                                STANDARD_RIGHTS_ALL);
    OpenParams.ShareAccess = 0; // Explicite: Exclusive access

    //
    // Open this serial device (Io Target) in order to send IOCTL_SERIAL_* control to it.
    //
    Status = WdfIoTargetOpen(IoTargetSerial,
                             &OpenParams);

    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_OPEN WdfIoTargetOpen failed status=%!STATUS!", Status));
        WdfObjectDelete(IoTargetSerial);
        IoTargetSerial = NULL;
        goto Exit;
    }

    *_pIoTarget = IoTargetSerial;

Exit:

    if (!NT_SUCCESS(Status)) {
        if (FdoExtension != NULL) {
            IfxBtSetFailureReason(FdoExtension,
                                  IfxBtFailureUartOpenFailed,
                                  Status);
        }
        DoTrace(LEVEL_ERROR, TFLAG_UART,("UART_OPEN FdoOpenDevice exit status=%!STATUS!", Status));
    }
    else {
        IfxBtSetTransportState(FdoExtension,
                               IfxBtTransportStateUartOpened,
                               Status);
        DoTrace(LEVEL_INFO, TFLAG_UART,("UART_OPEN FdoOpenDevice exit ioTarget=%p status=%!STATUS!", *_pIoTarget, Status));
    }

    return Status;
}

NTSTATUS
FdoSetIdleSettings(
    _In_  WDFDEVICE _Device,
    _In_  IDLE_CAP_STATE   _IdleCapState
    )
/*++
Routine Description:

    This function defines how device idle (Dx) is support while system is in
    (S0) for the Serial Hci device (not its child node, which is supported
    in the PDO).

    If its Enuemrator is "ROOT" (in the case of using a Bluetooth dev board),
    its Idle support is IdleCannotWakeFromS0.  Its power capabilities are
    limited to D0 and D3; it is basically on or off, and there is no Idle
    while in S0.

    Vendor: If its Enumerator is ACPI, then it might be possible to support
    idle while in S0. This is vendor specific.

Arguments:

    _Device - WDF Device object

    IDLE_CAP_STATE - The idle capability state to enter

Return Value:

    NTSTATUS

--*/
{
    WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS  IdleSettings;
    NTSTATUS  Status = STATUS_SUCCESS;
    BOOLEAN AssignS0IdleSettings = TRUE;

    DoTrace(LEVEL_INFO, TFLAG_PNP,("+FdoSetIdleSettings"));

    switch (_IdleCapState)
    {
    case IdleCapActiveOnly:

        //
        // By default ACPI supports D0 active, and idle to D3 without remote wake.
        // While in D3, only host (e.g. IO request) can wake the device to D0.
        //
        WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&IdleSettings,
                                                   IdleCannotWakeFromS0);

        // Low Dx state to enter after IdleTimeout has expired and Idle is enabled.
        IdleSettings.DxState         = PowerDeviceD3;
        IdleSettings.IdleTimeout     = IdleTimeoutDefaultValue;     // Use default (~5 seconds)
        IdleSettings.IdleTimeoutType = DriverManagedIdleTimeout;    // Driver is in control (typically for out of SoC).

        // Idle to DxState is not initially disable, and do not allow user control to enable it (as this is active only).
        IdleSettings.UserControlOfIdleSettings = IdleDoNotAllowUserControl;
        IdleSettings.Enabled         = WdfFalse;

        // Do not wake from D3 to D0 due to system wake (Sx to S0); ie only host app can wake.
        IdleSettings.PowerUpIdleDeviceOnSystemWake = WdfFalse;
        break;

    case IdleCapCanWake:

        //
        // If it has a child PDO and there is a controller (GPIO) being configured to support wake,
        // this state can be supported.
        //
        // Vendor: in order to support idle in S0 for this ACPI enumerated device, specify that the device
        // can wake in S0.  For example, if it can wake from D2 in S0, this should be set in its device section:
        //
        //    Name(_S0W, 0x2)
        //
        // Additionally, the wake interrupt, e.g. HOST_WAKE, will need to be known by ACPI (instead of exposing
        // it directly to this driver as system resource); so that, ACPI will do the arming and wake on this
        // driver's behalf  with Dx state transition.
        //

        WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&IdleSettings,
                                                   IdleCanWakeFromS0);

        // Low Dx state to enter after IdleTimeout has expired and Idle is enabled.
        IdleSettings.DxState         = PowerDeviceD2;
        IdleSettings.IdleTimeout     = 0;                           // May want to enter D2 immediately and invoke arm wake callback.
        IdleSettings.IdleTimeoutType = DriverManagedIdleTimeout;    // Driver is in control (typically for out of SoC).

        // Idle to DxState is initially enable, but allow user control as well (e.g to turn off idle support).
        IdleSettings.UserControlOfIdleSettings = IdleAllowUserControl;
        IdleSettings.Enabled         = WdfTrue;

        //
        // Note: wiil invoke EvtDeviceArmWakeFromS0 callback  before entering DxState;
        // Driver can arm for HOST_WAKE interrrupt in the callback.
        //
        break;

    case IdleCapCanTurnOff:

        //
        // If there is no child PDO (e.g. in Radio off mode), in effect the BT radio can be turned off
        // to enter D3 state.  All unused controllers (e.g. GPIO) can be turned off, also
        // the Bluetooth function block.  While in D3 state, only host can wake the device.
        //
        // Here is one approach to prevent the FDO from entering DxState while its PDO is in Dx and there is no pending IO:
        //
        // The PDO can hold a reference on its parent to prevent the parent from going into DxState. This is done in
        // PrepareHardware with WdfDeviceStopIdle() and releasing that reference
        // in the PDO's ReleaseHardware with WdfDeviceResumeIdle(). This applies to the case when the PDO is disabled.
        // In the resource rebalancing case, the FDO may enter D3 shortly and then resume to D0.
        //

        WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&IdleSettings,
                                                   IdleCannotWakeFromS0);

        // Low Dx state to enter after IdleTimeout has expired and Idle is enabled.
        IdleSettings.DxState         = PowerDeviceD3;
        IdleSettings.IdleTimeout     = IdleTimeoutDefaultValue;
        IdleSettings.IdleTimeoutType = DriverManagedIdleTimeout;    // Driver is in control (typically for out of SoC).

        // Idle to DxState is initially enabled, but allow user control as well (e.g. do not turn off).
        IdleSettings.UserControlOfIdleSettings = IdleAllowUserControl;
        IdleSettings.Enabled         = WdfTrue;

         // Do not wake from D3 to D0 due to system wake (Sx to S0); ie only host app can wake.
        IdleSettings.PowerUpIdleDeviceOnSystemWake = WdfFalse;
        break;

    default:
        AssignS0IdleSettings = FALSE;
        break;
    }

    if (AssignS0IdleSettings)
    {
        Status = WdfDeviceAssignS0IdleSettings(_Device,
                                               &IdleSettings);
    }

    DoTrace(LEVEL_INFO, TFLAG_PNP,("-FdoSetIdleSettings %!STATUS!", Status));
    return Status;
}

NTSTATUS
FdoDevPrepareHardware(
    _In_  WDFDEVICE     _Device,
    _In_  WDFCMRESLIST  _ResourcesRaw,
    _In_  WDFCMRESLIST  _ResourcesTranslated
    )
/*++
Routine Description:

    This PnP CB function allocate hardware related resource allocation and
    perform device initialization.

Arguments:

    _Device - WDF Device object

    _ResourcesRaw - (Not referenced)

    _ResourcesTranslated - (Not referenced)

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS       Status;
    PFDO_EXTENSION FdoExtension;
    IFXBT_FAILURE_REASON FailureReason;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP FdoDevPrepareHardware entry device=%p", _Device));

    FdoExtension = FdoGetExtension(_Device);
    FdoExtension->LastFailureReason = IfxBtFailureNone;
    FdoExtension->LastFailureStatus = STATUS_SUCCESS;
    IfxBtSetTransportState(FdoExtension,
                           IfxBtTransportStateCreated,
                           STATUS_SUCCESS);

    //
    // Acquire connection ID of connected controllers (UART and GPIO)
    //
    Status = FdoFindConnectResources(_Device,
                                     _ResourcesRaw,
                                     _ResourcesTranslated);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("PNP FdoDevPrepareHardware FdoFindConnectResources failed status=%!STATUS!", Status));
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("PNP missing required UART resource; failing PrepareHardware"));
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_children skipped because parent transport is not ready status=%!STATUS!", Status));
        goto Exit;
    }

    //
    // Open Bluetooth UART device as a remote IO Target
    //
    Status = FdoOpenDevice(_Device, &FdoExtension->IoTargetSerial);

    if (!NT_SUCCESS(Status) || FdoExtension->IoTargetSerial == NULL)
    {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("PNP FdoDevPrepareHardware FdoOpenDevice failed status=%!STATUS!", Status));
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_children skipped because UART open failed status=%!STATUS!", Status));

        // Log(Error): Failed to open UART controller
        goto Exit;
    }

    //
    // Initialize content of this device extension
    //
    Status = HlpInitializeFdoExtension(_Device);

    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("PNP FdoDevPrepareHardware HlpInitializeFdoExtension failed status=%!STATUS!", Status));
        IfxBtSetTransportFailure(FdoExtension,
                                 IfxBtFailureUartConfigFailed,
                                 Status);
        goto Exit;
    }

    //
    // Set device's idle configuration if it is capable
    //
    Status = FdoSetIdleSettings(_Device,
                                IdleCapCanTurnOff);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("PNP FdoDevPrepareHardware FdoSetIdleSettings failed status=%!STATUS!", Status));
        // goto Exit;
    }

    // Enable serial bus device
    if (ValidConnectionID(FdoExtension->GPIOConnectionId)) {
        Status = DeviceEnable(_Device, TRUE);
        if (!NT_SUCCESS(Status)) {
            DoTrace(LEVEL_ERROR, TFLAG_POWER,("POWER FdoDevPrepareHardware DeviceEnable failed status=%!STATUS!", Status));
            goto Exit;
        }
    }

    // Power On serial bus device
    if (ValidConnectionID(FdoExtension->I2CConnectionId)) {
        Status = DevicePowerOn(_Device);
        if (!NT_SUCCESS(Status)) {
            DoTrace(LEVEL_ERROR, TFLAG_POWER,("POWER FdoDevPrepareHardware DevicePowerOn failed status=%!STATUS!", Status));
            goto Exit;
        }
    }

    //
    // Configure local UART controller
    //
    FdoExtension->DeviceInitialized = DeviceInitialize(FdoExtension,
                                                       FdoExtension->IoTargetSerial,
                                                       FdoExtension->RequestIoctlSync,
                                                       TRUE);
    if (!IsDeviceInitialized(FdoExtension))
    {
        // Can have issue if this UART device cannot be initalized
        Status = STATUS_DEVICE_NOT_READY;
        FailureReason = FdoExtension->LastFailureReason;
        if (FailureReason == IfxBtFailureNone) {
            FailureReason = IfxBtFailureUartConfigFailed;
        }
        else {
            Status = IfxBtPreserveLatchedFailureStatus(FdoExtension,
                                                       Status);
        }
        IfxBtSetFailureReason(FdoExtension,
                              FailureReason,
                              Status);
        DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_OPEN FdoDevPrepareHardware DeviceInitialize failed status=%!STATUS!", Status));
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("FDO create_children skipped because parent transport initialization failed status=%!STATUS!", Status));

        // Log(Error): Failed to intialize/configure the device
        goto Exit;
    }

#ifdef DYNAMIC_ENUM
    //
    // This code segment is for testing: spawn a work item to do dynamic enuermation
    // of a Bluetooth dev node (PDO); the actual implementation could be to query
    // the peripheral device for what function blocks that it can support, or
    // to listen for a published interface of its dependent controller driver
    // to start the enuermation after driver has started.
    //
    {
        WDF_OBJECT_ATTRIBUTES   ObjAttributes;
        WDF_WORKITEM_CONFIG WorkitemConfig;
        WDFWORKITEM WorkItem;
        PENABLE_PDO_CONTEXT Context;

        PAGED_CODE();

        DoTrace(LEVEL_INFO, TFLAG_PNP, ("FDO create_child dynamic workitem create entry"));

        WDF_OBJECT_ATTRIBUTES_INIT(&ObjAttributes);

        WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&ObjAttributes,
                                               ENABLE_PDO_CONTEXT);
        ObjAttributes.ParentObject = _Device;

        WDF_WORKITEM_CONFIG_INIT(&WorkitemConfig, DeviceEnablePDOWorker);
        Status = WdfWorkItemCreate(&WorkitemConfig, &ObjAttributes, &WorkItem);

        if (NT_SUCCESS(Status))
        {
            // Get and initialize the context
            Context = GetEnablePdoWorkItemContext(WorkItem);
            Context->Fdo = _Device;

            // Initialize work item context
            WdfWorkItemEnqueue(WorkItem);
        }
    }
#else
    //
    // Perform static PDO enumertion by reading child device info saved in the registry.
    // But the info needs to be populated first by acquired supported device for supported
    // child devices.
    //
    Status = FdoCreateAllChildren(_Device);
#endif

Exit:

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP, ("PNP FdoDevPrepareHardware exit status=%!STATUS!", Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_PNP, ("PNP FdoDevPrepareHardware exit status=%!STATUS!", Status));
    }

    return Status;
}

NTSTATUS
FdoDevReleaseHardware(
    _In_  WDFDEVICE     _Device,
    _In_  WDFCMRESLIST  _ResourcesTranslated
    )
/*++
Routine Description:

    This PnP CB function free resource allocated in FdoDevPrepareHardware.

Arguments:

    _Device - WDF Device object

    _ResourcesTranslated - (Not referenced)

Return Value:

    NTSTATUS

--*/
{
    PFDO_EXTENSION FdoExtension;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(_ResourcesTranslated);

    DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP FdoDevReleaseHardware entry device=%p", _Device));

    FdoExtension = FdoGetExtension(_Device);

    IfxBtInvalidateParentReadiness(FdoExtension,
                                   IfxBtTransportStateCreated,
                                   STATUS_SUCCESS,
                                   L"ReleaseHardware");

    if (FdoExtension->IoTargetSerial)
    {
        DoTrace(LEVEL_INFO, TFLAG_UART,("UART_OPEN FdoDevReleaseHardware delete ioTarget=%p", FdoExtension->IoTargetSerial));
        WdfObjectDelete(FdoExtension->IoTargetSerial);
        FdoExtension->IoTargetSerial = NULL;
    }

    DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP FdoDevReleaseHardware exit status=%!STATUS!", STATUS_SUCCESS));

    return STATUS_SUCCESS;
}


NTSTATUS
FdoDevSelfManagedIoInit(
    _In_  WDFDEVICE  _Device
)
/*++
Routine Description:

    This PnP CB function is invoked once and will perform IO related resource allocation
    and start the read pump.

Arguments:

    _Device - WDF Device object

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS       Status;
    PFDO_EXTENSION FdoExtension;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP FdoDevSelfManagedIoInit entry device=%p", _Device));

    FdoExtension = FdoGetExtension(_Device);

    //
    // Preallocate resources needed to perform read opeations
    //
    Status = ReadResourcesAllocate(_Device);

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_READ FdoDevSelfManagedIoInit ReadResourcesAllocate failed status=%!STATUS!", Status));
        DoTrace(LEVEL_ERROR, TFLAG_IO, ("ORCH read_pump_start_failed status=%!STATUS!", Status));
        IfxBtSetTransportFailure(FdoExtension,
                                 IfxBtFailureReadPumpFailed,
                                 Status);
        goto Exit;
    }

    // Issue pending IO request to prefetch HCI event and data
    FdoExtension->ReadContext.RequestState = REQUEST_COMPLETE;

    // Start the read pump
    FdoExtension->ReadPumpRunning = TRUE;
    Status = ReadH4Packet(&FdoExtension->ReadContext,
                           FdoExtension->ReadRequest,
                           FdoExtension->ReadMemory,
                           FdoExtension->ReadBuffer,
                           INITIAL_H4_READ_SIZE);

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_READ FdoDevSelfManagedIoInit ReadH4Packet failed status=%!STATUS!", Status));
        FdoExtension->ReadPumpRunning = FALSE;
        DoTrace(LEVEL_ERROR, TFLAG_IO, ("ORCH read_pump_start_failed status=%!STATUS!", Status));
        IfxBtSetTransportFailure(FdoExtension,
                                 IfxBtFailureReadPumpFailed,
                                 Status);
        goto Exit;
    }

Exit:

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_PNP,("PNP FdoDevSelfManagedIoInit exit status=%!STATUS!", Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP FdoDevSelfManagedIoInit exit status=%!STATUS!", Status));
    }

    return Status;
}

VOID
FdoDevSelfManagedIoCleanup(
    _In_  WDFDEVICE  _Device
    )
/*++
Routine Description:

    This PnP CB function is invoked once and will be used here to free resource
    that was alocated in its corresponding SelfMagedInit fucntion.

Arguments:

    _Device - WDF Device object

Return Value:

    none

--*/
{
    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP FdoDevSelfManagedIoCleanup entry device=%p", _Device));

    //
    // Cancel and free resources
    //
    ReadResourcesFree(_Device);

    DoTrace(LEVEL_INFO, TFLAG_PNP,("PNP FdoDevSelfManagedIoCleanup exit device=%p", _Device));

    return;
}

NTSTATUS
FdoDevD0Entry(
    _In_   WDFDEVICE              _Device,
    _In_   WDF_POWER_DEVICE_STATE _PreviousState
    )
/*++
Routine Description:

    This PnP CB function is invoked after device has enter D0 (working) state. Most
    of initilization of hardware is already performed in PrepareHardware CB but will
    be performed again if the device was resume from non-D0 state.

Arguments:

    _Device - WDF Device object

    PreviousState - Next power state it is entering from D0

Return Value:

    NTSTATUS

--*/
{
    PFDO_EXTENSION FdoExtension = FdoGetExtension(_Device);
    NTSTATUS       Status = STATUS_SUCCESS;
    IFXBT_FAILURE_REASON FailureReason;

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER FdoDevD0Entry entry device=%p previousState=%d initialized=%d",
            _Device, _PreviousState, IsDeviceInitialized(FdoExtension)));

    // Reset error count upon resume to D0
    FdoExtension->OutOfSyncErrorCount = 0;

    // Initialize serial port settings if re-enter D0
    if (!IsDeviceInitialized(FdoExtension)) {

        // Enable serial bus device
        if (ValidConnectionID(FdoExtension->GPIOConnectionId)) {
            Status = DeviceEnable(_Device, TRUE);
            if (!NT_SUCCESS(Status)) {
                DoTrace(LEVEL_ERROR, TFLAG_POWER,("POWER FdoDevD0Entry DeviceEnable failed status=%!STATUS!", Status));
                goto Done;
            }
        }

        // Power On serial bus device
        if (ValidConnectionID(FdoExtension->I2CConnectionId)) {
            Status = DevicePowerOn(_Device);
            if (!NT_SUCCESS(Status)) {
                DoTrace(LEVEL_ERROR, TFLAG_POWER,("POWER FdoDevD0Entry DevicePowerOn failed status=%!STATUS!", Status));
                goto Done;
            }
        }

        //
        // The local UART may need to be re-initialized to match the remote UART if its context
        // was lost, but the assumption here is that the UART controller driver does save and
        // restore its context.
        //
#ifndef REQUIRE_REINITIALIZE
        DoTrace(LEVEL_WARNING, TFLAG_POWER, ("POWER FdoDevD0Entry firmware retention unknown rebuilding parent readiness"));
#endif

        // Reinitialize serial bus device
        FdoExtension->DeviceInitialized = DeviceInitialize(FdoExtension,
                                                         FdoExtension->IoTargetSerial,
                                                         FdoExtension->RequestIoctlSync,
                                                         FALSE);
        if (!IsDeviceInitialized(FdoExtension)) {
            Status = STATUS_DEVICE_NOT_READY;
            FailureReason = FdoExtension->LastFailureReason;
            if (FailureReason == IfxBtFailureNone) {
                FailureReason = IfxBtFailureUartConfigFailed;
            }
            else {
                Status = IfxBtPreserveLatchedFailureStatus(FdoExtension,
                                                           Status);
            }
            IfxBtSetFailureReason(FdoExtension,
                                  FailureReason,
                                  Status);
            DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_OPEN FdoDevD0Entry DeviceInitialize failed status=%!STATUS!", Status));
            goto Done;
        }

        // Restart the IOTarget to receiving request
        Status = WdfIoTargetStart(FdoExtension->IoTargetSerial);
        if (!NT_SUCCESS(Status)) {
            DoTrace(LEVEL_ERROR, TFLAG_UART, ("UART_OPEN FdoDevD0Entry WdfIoTargetStart failed status=%!STATUS!", Status));
            IfxBtSetTransportFailure(FdoExtension,
                                     IfxBtFailureSerialError,
                                     Status);
            goto Done;
        }

        // Restart read pump
        DoTrace(LEVEL_INFO, TFLAG_IO, ("UART_READ FdoDevD0Entry restarting read pump"));
        FdoExtension->ReadPumpRunning = TRUE;
        Status = ReadH4Packet(&FdoExtension->ReadContext,
                              FdoExtension->ReadRequest,
                              FdoExtension->ReadMemory,
                              FdoExtension->ReadBuffer,
                              INITIAL_H4_READ_SIZE);
        if (!NT_SUCCESS(Status)) {
            DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_READ FdoDevD0Entry ReadH4Packet failed status=%!STATUS!", Status));
            FdoExtension->ReadPumpRunning = FALSE;
            DoTrace(LEVEL_ERROR, TFLAG_IO, ("ORCH read_pump_restart_failed status=%!STATUS!", Status));
            IfxBtSetTransportFailure(FdoExtension,
                                     IfxBtFailureReadPumpFailed,
                                     Status);
            goto Done;
        }
    }

Done:

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_POWER, ("POWER FdoDevD0Entry exit status=%!STATUS!", Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER FdoDevD0Entry exit status=%!STATUS!", Status));
    }

    return Status;
}

NTSTATUS
FdoDevD0Exit(
    _In_   WDFDEVICE              _Device,
    _In_   WDF_POWER_DEVICE_STATE _TargetState
    )
/*++
Routine Description:

    This PnP CB function is invoked when device has exited D0 (working) state.
    It stops the queue and can be restarted later, and mark the device uninitialize
    and will be initialized again when resumes to D0.

Arguments:

    _Device - WDF Device object

    _TargetState - Next power state it is entering from D0

Return Value:

    NTSTATUS

--*/
{
    PFDO_EXTENSION FdoExtension = FdoGetExtension(_Device);

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER FdoDevD0Exit entry device=%p targetState=%d", _Device, _TargetState));

    IfxBtInvalidateParentReadiness(FdoExtension,
                                   IfxBtTransportStateStopping,
                                   STATUS_SUCCESS,
                                   L"D0Exit");

    // Cancel IO requests that are already in the IO queue,
    // wait for their completion before this function is returned.
    // Can restart this queue at later time.
    WdfIoTargetStop(FdoExtension->IoTargetSerial, WdfIoTargetCancelSentIo);

    // Delete GPIO IoTarget to disable the device and this will
    // require device to be re-initialized when it re-enters D0.
    if (FdoExtension->IoTargetGPIO)
    {
        WdfObjectDelete(FdoExtension->IoTargetGPIO);
        FdoExtension->IoTargetGPIO = NULL;
    }

    //
    // Note: Do not delete the UART's IoTarget.
    //

    DoTrace(LEVEL_INFO, TFLAG_POWER, ("POWER FdoDevD0Exit exit status=%!STATUS!", STATUS_SUCCESS));

    return STATUS_SUCCESS;
}

NTSTATUS
HCIContextValidate(
    ULONG _Index,
    PBTHX_HCI_READ_WRITE_CONTEXT _HCIContext
    )
/*++
Routine Description:

    This function validate the incoming data context and print out (WPP) trace.

Arguments:

    _Index - count number of HCI command/event/data that has been completed (0 based).
    _HCIContext - Context to be valdiated

Return Value:

    NTSTATUS - STATUS_SUCCESS or STATUS_INVALID_PARAMETER

--*/
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index;

    DoTrace(LEVEL_INFO, TFLAG_HCI,("+HCIContextValidate"));

    switch ((BTHX_HCI_PACKET_TYPE) _HCIContext->Type)
    {
    case HciPacketCommand:
        {
            PHCI_COMMAND_PACKET HciCommand = (PHCI_COMMAND_PACKET) _HCIContext->Data;
            DoTrace(LEVEL_INFO, TFLAG_HCI, (" -> HCI Command [%d] OpCode: 0x%x, nParams: %d ---->",
                _Index,
                HciCommand->Opcode,
                HciCommand->ParamsCount));

            for (Index = 0; Index < MinToPrint((ULONG) HciCommand->ParamsCount, MAX_COMMAND_PARAMS_TO_DISPLAY); Index++)
            {
                DoTrace(LEVEL_INFO, TFLAG_HCI, ("    [%d] 0x%.2x",
                    Index, HciCommand->Params[Index]));
            }

            if (!WithinRange(MIN_HCI_CMD_SIZE, _HCIContext->DataLen, MAX_HCI_CMD_SIZE))
            {
                Status = STATUS_INVALID_PARAMETER;
                DoTrace(LEVEL_ERROR, TFLAG_HCI,(" HciPacketCommand %!STATUS!", Status));
                break;
            }
        }
        break;

    case HciPacketEvent:
        {
            PHCI_EVENT_PACKET HciEvent = (PHCI_EVENT_PACKET) _HCIContext->Data;
            DoTrace(LEVEL_INFO, TFLAG_HCI, (" <- HCI Event [%d] EventCode: 0x%x (%S), nParams: %d",
                    _Index,
                    HciEvent->EventCode,
                    HciEvent->EventCode == CommandComplete ? L"Complete" :
                    HciEvent->EventCode == CommandStatus ? L"Status(Async)!!" : L"??",
                    HciEvent->ParamsCount));

            // Note if CommandStatus is returned, there will be another event to complete this command.

            for (Index = 0; Index < MinToPrint((ULONG) HciEvent->ParamsCount, MAX_EVENT_PARAMS_TO_DISPLAY); Index++)
            {
                DoTrace(LEVEL_VERBOSE, TFLAG_HCI, ("    [%d] 0x%.2x",
                        Index, HciEvent->Params[Index]));
            }

            if (!WithinRange(MIN_HCI_EVENT_SIZE, _HCIContext->DataLen, MAX_HCI_EVENT_SIZE))
            {
                Status = STATUS_INVALID_PARAMETER;
                DoTrace(LEVEL_ERROR, TFLAG_HCI,(" HciPacketEvent %!STATUS!", Status));
                break;
            }
        }
        break;

    case HciPacketAclData:
        {
            PHCI_ACLDATA_PACKET AclData = (PHCI_ACLDATA_PACKET) _HCIContext->Data;
            DoTrace(LEVEL_INFO, TFLAG_HCI, (" HCI Data [%d] (Handle:0x%x, PB:%x, BC:%x, Length:%d)",
                _Index,
                AclData->ConnectionHandle,
                AclData->PBFlag,
                AclData->BCFlag,
                AclData->DataLength));

            for (Index = 0; Index < (ULONG) (AclData->DataLength > 8 ? 8 : AclData->DataLength); Index++)
            {
                DoTrace(LEVEL_VERBOSE, TFLAG_HCI, ("    [%d] 0x%.2x",
                        Index, AclData->Data[Index]));
            }

            if (!WithinRange(1, AclData->DataLength, MAX_HCI_ACLDATA_SIZE))
            {
                Status = STATUS_INVALID_PARAMETER;
                DoTrace(LEVEL_ERROR, TFLAG_HCI,(" HciPacketAclData data (%d) exceeds its max %d, %!STATUS!",
                        AclData->DataLength, MAX_HCI_ACLDATA_SIZE, Status));
                break;
            }
        }
        break;

    default:
        DoTrace(LEVEL_ERROR, TFLAG_HCI, (" Packet type %d unexpected!", _HCIContext->Type));
        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    NT_ASSERT(NT_SUCCESS(Status) && L"Invlaid data is detected!");

    DoTrace(LEVEL_INFO, TFLAG_HCI,("-HCIContextValidate %!STATUS!", Status));

    return Status;
}


NTSTATUS
FdoWriteDeviceIO(
    _In_  WDFREQUEST      _RequestFromBthport,
    _In_  WDFDEVICE       _Device,
    _In_  PFDO_EXTENSION  _FdoExtension,
    _In_  PBTHX_HCI_READ_WRITE_CONTEXT _HCIContext
    )
/*++

Routine Description:

    This function send an HCI packet to target device.

Arguments:

    _RequestFromBthport - Request from upper layer that initiate this transfer
    _Device - WDF Device Object
    _FdoExtension - Device's context
    _HCIContext - Context used to process this HCI

Return Value:

    NTSTATUS

--*/
{
    WDF_OBJECT_ATTRIBUTES ObjAttributes;
    NTSTATUS Status;
    WDFREQUEST RequestToUART = NULL;
    PUART_WRITE_CONTEXT   TransferContext = NULL;
    ULONG  DataLength;
    PVOID   Data = NULL;

    DoTrace(LEVEL_INFO, TFLAG_IO,("UART_WRITE FdoWriteDeviceIO entry request=%p type=%d hciLen=%d",
            _RequestFromBthport, _HCIContext->Type, _HCIContext->DataLen));

    if (!IsDeviceInitialized(_FdoExtension))
    {
        Status = STATUS_DEVICE_NOT_READY;
        DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_WRITE FdoWriteDeviceIO device not initialized status=%!STATUS!", Status));
        goto Done;
    }

    //
    // Add a context to this existing WDFREQUEST for cancellation purpose
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&ObjAttributes,
                                            UART_WRITE_CONTEXT);

    Status = WdfObjectAllocateContext(_RequestFromBthport,
                                      &ObjAttributes,
                                      &TransferContext);
    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_WRITE FdoWriteDeviceIO WdfObjectAllocateContext failed status=%!STATUS!", Status));
        goto Done;
    }

    Status = HLP_AllocateResourceForWrite(
                    _Device,
                    _FdoExtension->IoTargetSerial,
                    &RequestToUART);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_IO,("UART_WRITE FdoWriteDeviceIO HLP_AllocateResourceForWrite failed status=%!STATUS!", Status));
        goto Done;
    }

    WDF_OBJECT_ATTRIBUTES_INIT(&ObjAttributes);
    ObjAttributes.ParentObject = _Device;

    // Reuse the data buffer coming from upper layer;  UART's HCI packet starts with
    // packet type, and then follows by the actual HCI packet.
    Data = (PVOID) &_HCIContext->Type;
    DataLength = (ULONG) sizeof(_HCIContext->Type) + _HCIContext->DataLen;

    _Analysis_assume_(DataLength > 0);
    Status = WdfMemoryCreatePreallocated(&ObjAttributes,
                                         Data,
                                         DataLength,
                                         &TransferContext->Memory);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_WRITE FdoWriteDeviceIO WdfMemoryCreatePreallocated failed status=%!STATUS!", Status));
        goto Done;
    }

    Status = WdfIoTargetFormatRequestForWrite(_FdoExtension->IoTargetSerial,
                                              RequestToUART,
                                              TransferContext->Memory,
                                              NULL,
                                              NULL);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_WRITE FdoWriteDeviceIO WdfIoTargetFormatRequestForWrite failed status=%!STATUS!", Status));
        goto Done;
    }

    // Setup transfer context
    TransferContext->FdoExtension       = _FdoExtension;
    TransferContext->HCIContext         = _HCIContext;
    TransferContext->RequestFromBthport = _RequestFromBthport;
    TransferContext->RequestCompletePath = REQUEST_PATH_NONE;
    TransferContext->RequestToUART      = RequestToUART;
    TransferContext->HCIPacket          = Data;
    TransferContext->HCIPacketLen       = DataLength;

    //
    // Both Requests are typically accessed by the completion routine, and in rare case also
    // by the cancellation callback.  Take a reference on them to ensure they stays valid in both cases.
    //
    WdfObjectReference(RequestToUART);
    WdfObjectReference(_RequestFromBthport);

    // Mark cancellable of the Request in our possession from upper layer
    // Cannot mark the request that we will forward to lower driver cancellable.
    // Only if the Request from upper layer is cancelled, we will then cancel the
    // Request that is sent to lower driver.
    Status = WdfRequestMarkCancelableEx(_RequestFromBthport, CB_RequestFromBthportCancel);
    DoTrace(LEVEL_INFO, TFLAG_IO, ("TX_CANCEL mark_cancelable_ex status=%!STATUS!", Status));
    if (!NT_SUCCESS(Status))
    {
        if (Status == STATUS_CANCELLED)
        {
            DoTrace(LEVEL_WARNING, TFLAG_IO, ("TX_CANCEL upper request already canceled before lower send"));
        }
        else
        {
            DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_WRITE FdoWriteDeviceIO WdfRequestMarkCancelableEx failed status=%!STATUS!", Status));
        }

        // The upper request was not marked cancelable, and the lower request was
        // not sent.  Balance the references taken for the normal send path; the
        // common failure cleanup below deletes the lower request and memory object.
        WdfObjectDereference(RequestToUART);
        WdfObjectDereference(_RequestFromBthport);
        goto Done;
    }

    WdfRequestSetCompletionRoutine(RequestToUART, CR_WriteDeviceIO, TransferContext);

    // This request will be delivered to its IoTarget asynchronously (the default option).  It should return
    // STATUS_PENDING unless there is an error in its delivery to its IoTarget.  After it has been delivered
    // successfully, its completion function will be called for any outcome - success, failure, or cancellation.
    if (!WdfRequestSend(RequestToUART, _FdoExtension->IoTargetSerial, WDF_NO_SEND_OPTIONS))
    {
        NTSTATUS StatusTemp;

        // Get failure status, and this request will be completed by its caller of this function with this status.
        Status = WdfRequestGetStatus(RequestToUART);

        // Unmark cancellable before it is completed.
        StatusTemp = WdfRequestUnmarkCancelable(_RequestFromBthport);

        // Balance the reference count for both Requests due to failure.
        WdfObjectDereference(RequestToUART);
        WdfObjectDereference(_RequestFromBthport);

        if (StatusTemp == STATUS_CANCELLED)
        {
            DoTrace(LEVEL_WARNING, TFLAG_IO, ("TX_CANCEL send_failed_unmark_cancelled cancel owns completion"));

            // The upper request cancel callback has been invoked or is about to
            // be invoked.  Return STATUS_PENDING so FdoIoQuDeviceControl does
            // not complete the upper request a second time.
            Status = STATUS_PENDING;
            goto Done;
        }

        DoTrace(LEVEL_INFO, TFLAG_IO, ("TX_CANCEL send_failed_unmark_ok caller owns completion"));

        DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_WRITE FdoWriteDeviceIO WdfRequestSend failed status=%!STATUS! unmarkStatus=%!STATUS!", Status, StatusTemp));
        goto Done;
    }
    else
    {
        // Request has been delivered to UART driver asychronously.  It will be completed in its completion function
        // after IoTarget (UART driver) completes its delivery to the BT controller.
    }

Done:

    if (!NT_SUCCESS(Status))
    {
        HLP_FreeResourceForWrite(TransferContext);
    }

    if (!NT_SUCCESS(Status)) {
        DoTrace(LEVEL_ERROR, TFLAG_IO, ("UART_WRITE FdoWriteDeviceIO exit status=%!STATUS!", Status));
    }
    else {
        DoTrace(LEVEL_INFO, TFLAG_IO, ("UART_WRITE FdoWriteDeviceIO exit status=%!STATUS!", Status));
    }

    return Status;
}

NTSTATUS
FdoWriteToDeviceSync(
    _In_  WDFIOTARGET  _IoTargetSerial,
    _In_  WDFREQUEST   _RequestWriteSync,
    _In_  ULONG        _IoControlCode,
    _In_opt_ ULONG     _InBufferSize,
    _In_opt_ PVOID     _InBuffer,
    _Out_  PULONG_PTR  _BytesWritten
)
/*++
Routine Description:

    This helper function send a synchronous write or Ioctl Request to device with
    timeout (to prevent hang).

Arguments:

    _IoTargetSerial - Serial port IO Target where to issue this request to
    _RequestWriteSync - caller allocated WDF Request
    _IoControlCode - IOCTL control code; if 0, it is a Write request.
    _InBufferSize - Input buffer size
    _InBuffer - (optional) Input buffer
    _BytesWritten - Bytes written to device; this is driver dependent; a write
                     could be successfully (and fully) written with 0 BytesWritten.

Return Value:

    NTSTATUS - STATUS_SUCCESS or Status from issuing this request

--*/
{
    NTSTATUS Status = STATUS_SUCCESS;
    WDF_REQUEST_REUSE_PARAMS RequestReuseParams;
    WDF_REQUEST_SEND_OPTIONS  Options;
    WDF_MEMORY_DESCRIPTOR MemoryDescriptor;
    ULONG_PTR BytesWritten = 0;
    BOOLEAN HasInputParam = FALSE;

    PAGED_CODE();

    DoTrace(LEVEL_INFO, TFLAG_IO,("+FdoWriteToDeviceSync"));

    WDF_REQUEST_REUSE_PARAMS_INIT(&RequestReuseParams, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
    Status = WdfRequestReuse(_RequestWriteSync, &RequestReuseParams);
    if (!NT_SUCCESS(Status))
    {
        DoTrace(LEVEL_ERROR, TFLAG_IO, (" WdfRequestReuse failed %!STATUS!", Status));
        goto Done;
    }

    if (_InBuffer && _InBufferSize) {
        HasInputParam = TRUE;
        WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&MemoryDescriptor,
                                          _InBuffer,
                                          _InBufferSize);
    }

    WDF_REQUEST_SEND_OPTIONS_INIT(&Options, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS);
    WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&Options, WDF_REL_TIMEOUT_IN_SEC(MAX_WRITE_TIMEOUT_IN_SEC));

    if (_IoControlCode)
    {
        Status = WdfIoTargetSendIoctlSynchronously(_IoTargetSerial,
                                                   NULL,
                                                   _IoControlCode,
                                                   HasInputParam ? &MemoryDescriptor : NULL,    // InputBuffer
                                                   NULL,                   // OutputBuffer
                                                   &Options,            // RequestOptions
                                                   &BytesWritten           // BytesReturned
                                                   );
    }
    else
    {
        Status = WdfIoTargetSendWriteSynchronously(_IoTargetSerial,
                                                   NULL,
                                                   HasInputParam ? &MemoryDescriptor : NULL,    // InputBuffer
                                                   NULL,                   // DeviceOffset
                                                   &Options,            // RequestOptions
                                                   &BytesWritten           // BytesReturned
                                                   );
    }

    DoTrace(LEVEL_INFO, TFLAG_IO,("-FdoWriteToDeviceSync: %d BytesWritten %!STATUS!", (ULONG) BytesWritten, Status));

    if (NT_SUCCESS(Status))
    {
        *_BytesWritten = BytesWritten;
    }

Done:
    return Status;
}

VOID
FdoIoQuDeviceControl(
    _In_  WDFQUEUE     _Queue,
    _In_  WDFREQUEST   _Request,
    _In_  size_t       _OutputBufferLength,
    _In_  size_t       _InputBufferLength,
    _In_  ULONG        _IoControlCode
    )
/*++

Routine Description:

    This routine is the dispatch routine for device control requests.

Arguments:

    _Queue - Handle to the framework queue object that is associated
            with the I/O request.
    _Request - Handle to a framework request object.

    _OutputBufferLength - length of the request's output buffer,
                        if an output buffer is available.
    _InputBufferLength - length of the request's input buffer,
                        if an input buffer is available.

    _IoControlCode - the driver-defined or system-defined I/O control code
                    (IOCTL) that is associated with the request.

Return Value:

   VOID

--*/
{
    WDFMEMORY   ReqInMemory = NULL, ReqOutMemory = NULL;
    PVOID   InBuffer = NULL, OutBuffer = NULL;
    size_t  InBufferSize = 0, OutBufferSize = 0;
    PFDO_EXTENSION   FdoExtension;
    NTSTATUS    Status = STATUS_SUCCESS;
    WDFDEVICE   Device;
    BOOLEAN CompleteRequest = FALSE;
    ULONG ControlCode = (_IoControlCode & 0x00003ffc) >> 2;
    size_t HciPayloadAvailable = 0;
    size_t HciContextHeaderSize = FIELD_OFFSET(BTHX_HCI_READ_WRITE_CONTEXT, Data);
    ULONG UartLength = 0;
    BTHX_HCI_PACKET_TYPE PacketType;
    PBTHX_HCI_READ_WRITE_CONTEXT HCIContext;

    DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL fdo entry code=0x%x func=%d inLen=%d outLen=%d",
        _IoControlCode, ControlCode, (ULONG) _InputBufferLength, (ULONG) _OutputBufferLength));

    Device = WdfIoQueueGetDevice(_Queue);

    FdoExtension = FdoGetExtension(Device);

    if (_InputBufferLength)
    {
        Status = WdfRequestRetrieveInputMemory(_Request, &ReqInMemory);
        if (NT_SUCCESS(Status))
        {
            InBuffer = WdfMemoryGetBuffer(ReqInMemory, &InBufferSize);
        }
        else
        {
            DoTrace(LEVEL_ERROR, TFLAG_IOCTL,("BTHX_IOCTL fdo retrieve_input failed code=0x%x status=%!STATUS!", _IoControlCode, Status));
        }
    }

    if (_OutputBufferLength)
    {
        Status = WdfRequestRetrieveOutputMemory(_Request, &ReqOutMemory);
        if (NT_SUCCESS(Status))
        {
            OutBuffer = WdfMemoryGetBuffer(ReqOutMemory, &OutBufferSize);
        }
        else
        {
            DoTrace(LEVEL_ERROR, TFLAG_IOCTL,("BTHX_IOCTL fdo retrieve_output failed code=0x%x status=%!STATUS!", _IoControlCode, Status));
        }
    }

    switch (_IoControlCode)
    {
    case IOCTL_BTHX_WRITE_HCI:
        DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL write_hci code=0x%x inSize=%d outSize=%d", _IoControlCode, (ULONG) InBufferSize, (ULONG) OutBufferSize));
        // Validate input and output parameters
        if (!InBuffer || InBufferSize < HciContextHeaderSize ||
            !OutBuffer || OutBufferSize != sizeof(BTHX_HCI_PACKET_TYPE))
        {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_IOCTL,("BTHX_IOCTL write_hci invalid_buffer status=%!STATUS!", Status));
            break;
        }

        HCIContext = (PBTHX_HCI_READ_WRITE_CONTEXT) InBuffer;

        PacketType = (BTHX_HCI_PACKET_TYPE) HCIContext->Type;
        DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL write_hci type=%d hciLen=%d", PacketType, HCIContext->DataLen));

        if (!BTHX_VALID_WRITE_PACKET_TYPE(PacketType))
        {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_IOCTL,("BTHX_IOCTL write_hci invalid_type type=%d status=%!STATUS!", PacketType, Status));
            break;
        }

        HciPayloadAvailable = InBufferSize - HciContextHeaderSize;
        Status = ValidateHciTxWriteContext(HCIContext, HciPayloadAvailable);
        if (!NT_SUCCESS(Status))
        {
            break;
        }

        UartLength = (ULONG) sizeof(HCIContext->Type) + HCIContext->DataLen;
        TraceHciTxSummary(HCIContext, UartLength, HCIContext->DataLen);

        if (PacketType == HciPacketCommand)
        {
            InterlockedIncrement(&FdoExtension->CntCommandReq);
        }
        else
        {
            InterlockedIncrement(&FdoExtension->CntWriteDataReq);
        }

        Status = FdoWriteDeviceIO(_Request,
                                  Device,
                                  FdoExtension,
                                  HCIContext);
        break;

    case IOCTL_BTHX_READ_HCI:
        DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL read_hci code=0x%x inSize=%d outSize=%d", _IoControlCode, (ULONG) InBufferSize, (ULONG) OutBufferSize));
        // Validate input and output parameters
        if (!InBuffer || InBufferSize != sizeof(BTHX_HCI_PACKET_TYPE) ||
            !OutBuffer || OutBufferSize < sizeof(BTHX_HCI_READ_WRITE_CONTEXT))
        {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_IOCTL,("BTHX_IOCTL read_hci invalid_buffer status=%!STATUS!", Status));
            break;
        }

        PacketType = *((BTHX_HCI_PACKET_TYPE *) InBuffer);
        DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL read_hci type=%d", PacketType));

        if (!BTHX_VALID_READ_PACKET_TYPE(PacketType))
        {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_IOCTL,("BTHX_IOCTL read_hci invalid_type type=%d status=%!STATUS!", PacketType, Status));
            break;
        }

        if (PacketType == HciPacketEvent)
        {
            WdfSpinLockAcquire(FdoExtension->QueueAccessLock);
              // Queue the new request to preserve sequential order
              Status = WdfRequestForwardToIoQueue(_Request, FdoExtension->ReadEventQueue);
              if (NT_SUCCESS(Status))
              {
                  InterlockedIncrement(&FdoExtension->EventQueueCount);
                  InterlockedIncrement(&FdoExtension->CntEventReq);
              }
            WdfSpinLockRelease(FdoExtension->QueueAccessLock);

            if (NT_SUCCESS(Status))
            {
                Status = ReadRequestComplete(FdoExtension,
                                             HciPacketEvent,
                                             0, NULL,
                                             FdoExtension->ReadEventQueue,
                                             &FdoExtension->EventQueueCount,
                                             &FdoExtension->ReadEventList,
                                             &FdoExtension->EventListCount);
            }

        }
        else if (PacketType == HciPacketAclData)
        {
            WdfSpinLockAcquire(FdoExtension->QueueAccessLock);
              // Queue the new request to preserve sequential order
              Status = WdfRequestForwardToIoQueue(_Request, FdoExtension->ReadDataQueue);
              if (NT_SUCCESS(Status))
              {
                  InterlockedIncrement(&FdoExtension->DataQueueCount);
                  InterlockedIncrement(&FdoExtension->CntReadDataReq);
              }
            WdfSpinLockRelease(FdoExtension->QueueAccessLock);

            if (NT_SUCCESS(Status))
            {
                Status = ReadRequestComplete(FdoExtension,
                                             HciPacketAclData,
                                             0, NULL,
                                             FdoExtension->ReadDataQueue,
                                             &FdoExtension->DataQueueCount,
                                             &FdoExtension->ReadDataList,
                                             &FdoExtension->DataListCount);
            }
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
            DoTrace(LEVEL_ERROR, TFLAG_IOCTL,("BTHX_IOCTL read_hci unexpected_type type=%d status=%!STATUS!", PacketType, Status));
            break;
        }
        break;

    case IOCTL_BTHX_GET_VERSION:
        CompleteRequest = TRUE;
        DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL get_version"));

        if (OutBuffer && OutBufferSize >= sizeof(BTHX_VERSION))
        {
            RtlCopyMemory(OutBuffer, &Microsoft_BTHX_DDI_Version, sizeof(BTHX_VERSION));
            WdfRequestCompleteWithInformation(_Request, Status, sizeof(BTHX_VERSION));
            return;
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
        }
        break;

    case IOCTL_BTHX_SET_VERSION:
        CompleteRequest = TRUE;
        DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL set_version"));

        if (InBuffer && InBufferSize >= sizeof(BTHX_VERSION))
        {
            BTHX_VERSION SupportedVersion = *((BTHX_VERSION *)InBuffer);

            DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL set_version version=0x%x", SupportedVersion.Version));

            WdfRequestComplete(_Request, Status);
            return;
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
        }
        break;

    case IOCTL_BTHX_QUERY_CAPABILITIES:
        CompleteRequest = TRUE;
        DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL query_capabilities"));

        if (OutBuffer && OutBufferSize >= sizeof(BTHX_CAPABILITIES))
        {
            BTHX_CAPABILITIES *pCaps = (BTHX_CAPABILITIES *) OutBuffer;

            RtlCopyMemory(pCaps, &FdoExtension->BthXCaps, sizeof(BTHX_CAPABILITIES));
            WdfRequestCompleteWithInformation(_Request, Status, sizeof(BTHX_CAPABILITIES));
            return;
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
        }
        break;

    //
    // This IOCTL is used to support radio on/off feature by doing the following
    //    1. Power up/down the Bluetooth radio function, and
    //    2. Add/remove a PDO for Bluetooth devnode;
    //
    case IOCTL_BUSENUM_SET_RADIO_ONOFF_VENDOR_SPECFIC:
        CompleteRequest = TRUE;
        DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL radio_onoff_vendor_specific"));
        if (InBuffer && InBufferSize >= sizeof(BOOLEAN)) {
            BOOLEAN IsRadioEnabled = *((BOOLEAN *) InBuffer);

            if (IsRadioEnabled) {
                if (!FdoExtension->IsRadioEnabled) {

                    //
                    // 1. Power up the Bluetooth function of this device;
                    //
                    Status = DevicePowerOn(Device);

                    if (NT_SUCCESS(Status)) {

                        //
                        // 2. Create a PDO for the Bluetooth devnode;
                        //
                        if (!IfxBtIsParentTransportReady(FdoExtension)) {
                            Status = FdoExtension->LastFailureStatus;
                            if (Status == STATUS_SUCCESS) {
                                Status = STATUS_DEVICE_NOT_READY;
                            }
                            DoTrace(LEVEL_ERROR, TFLAG_IOCTL,("BTHX_IOCTL radio_onoff create_child blocked parent transport not ready status=%!STATUS!",
                                    Status));
                        }
                        else {
                            Status = FdoCreateOneChildDevice(Device,
                                                             BT_PDO_HARDWARE_IDS,
                                                             BLUETOOTH_FUNC_IDS);
                            if (NT_SUCCESS(Status)) {
                                FdoExtension->IsRadioEnabled = TRUE;
                            }
                        }
                    }
                    DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL radio_onoff enable status=%!STATUS!", Status));
                }
                else {
                    Status = STATUS_SUCCESS;
                    DoTrace(LEVEL_WARNING, TFLAG_IOCTL,("BTHX_IOCTL radio_onoff already_enabled"));
                }
            }
            else {
                if (FdoExtension->IsRadioEnabled) {

                    //
                    // 1. Remove the PDO for the Bluetooth devnode;
                    //
                    Status = FdoRemoveOneChildDevice(Device,
                                                     BLUETOOTH_FUNC_IDS);
                    if (NT_SUCCESS(Status)) {
                        FdoExtension->IsRadioEnabled = FALSE;

                        //
                        // 2. Power down the Bluetooth function (at least the antenna) of this device;
                        //
                        Status = DevicePowerOff(Device);
                    }

                    DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL radio_onoff disable status=%!STATUS!", Status));
                }
                else {
                    Status = STATUS_SUCCESS;
                    DoTrace(LEVEL_WARNING, TFLAG_IOCTL,("BTHX_IOCTL radio_onoff already_disabled"));
                }
            }
        }
        else {
            Status = STATUS_INVALID_PARAMETER;
        }
        break;

    default:
        DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL unsupported code=0x%x func=%d", _IoControlCode, ControlCode));
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    if (!NT_SUCCESS(Status) || CompleteRequest)
    {
        if (!NT_SUCCESS(Status)) {
            DoTrace(LEVEL_ERROR, TFLAG_IOCTL,("BTHX_IOCTL fdo complete code=0x%x status=%!STATUS!", _IoControlCode, Status));
        }
        else {
            DoTrace(LEVEL_INFO, TFLAG_IOCTL,("BTHX_IOCTL fdo complete code=0x%x status=%!STATUS!", _IoControlCode, Status));
        }
        WdfRequestComplete(_Request, Status);
    }

    return;
}
