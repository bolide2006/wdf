/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    registers.h

Abstract:

Environment:

    Kernel mode

--*/
#pragma once

#include <pshpack4.h>
#pragma warning(push)
//
// #### TODO: Add controller data structure definitions here ####
//
#pragma warning(pop)
#include <poppack.h>

typedef struct _REGISTERS_CONTEXT {

    PUCHAR RegisterBase;

    SIZE_T RegistersLength;

} REGISTERS_CONTEXT, *PREGISTERS_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REGISTERS_CONTEXT, DeviceGetRegistersContext)

NTSTATUS
RegistersCreate(
    _In_ WDFDEVICE Device,
    _In_ PCM_PARTIAL_RESOURCE_DESCRIPTOR  RegistersResource
    );