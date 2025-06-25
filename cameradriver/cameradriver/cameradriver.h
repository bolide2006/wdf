#pragma once

#define INITGUID

#include <ntddk.h>
#include <wdf.h>

// 设备上下文结构
typedef struct _DEVICE_CONTEXT {
    WDFDEVICE WdfDevice;
    WDFQUEUE IoQueue;
    WDFIOTARGET IoTarget;

    // 相机相关
    //PKSDEVICE KsDevice;
    BOOLEAN IsStreaming;
    ULONG CurrentFormat;
    LIST_ENTRY FrameList;
    KSPIN_LOCK FrameListLock;

    // 资源
    WDFMEMORY FrameBufferMemory;
    size_t FrameBufferSize;
} DEVICE_CONTEXT, * PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)

// 函数声明
DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD CameraEvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL CameraEvtIoDeviceControl;
NTSTATUS StartCameraStream(PDEVICE_CONTEXT deviceContext);
VOID StopCameraStream(PDEVICE_CONTEXT deviceContext);
NTSTATUS CaptureCameraFrame(PDEVICE_CONTEXT deviceContext, PVOID buffer, size_t bufferLength, size_t* information);
NTSTATUS SetCameraFrame(PDEVICE_CONTEXT deviceContext, WDFREQUEST request, size_t* information);
NTSTATUS GetCameraFrame(PDEVICE_CONTEXT deviceContext, WDFREQUEST request, size_t* information);

VOID
PrintHex(
    _In_reads_(CountChars) PCHAR BufferAddress,
    _In_ size_t CountChars
    );

VOID
PrintChars(
    _In_reads_(CountChars) PCHAR BufferAddress,
    _In_ size_t CountChars
    );

NTSTATUS
LoadFirmware(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING FirmwareFileName,
    PVOID* FirmwareBuffer,
    PULONG FirmwareSize);