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

NTSTATUS
IfxBtFirmwareValidatePlaceholderState(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig
    );

VOID
IfxBtFirmwareLogPlaceholderState(
    _In_ const IFXBT_PLATFORM_CONFIG* PlatformConfig
    );

#endif
