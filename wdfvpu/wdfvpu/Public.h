/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//
#pragma once

DEFINE_GUID (GUID_DEVINTERFACE_WDFVPU,
    0x3b150a0f,0xae2c,0x4df8,0xbb,0x24,0xa5,0x94,0xe1,0x20,0xc8,0x58);

// {3b150a0f-ae2c-4df8-bb24-a594e120c858}
