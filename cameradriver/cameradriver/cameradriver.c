#include "cameradriver.h"
#include "public.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, CameraEvtDeviceAdd)
#pragma alloc_text (PAGE, StartCameraStream)
#pragma alloc_text (PAGE, StopCameraStream)
#pragma alloc_text (PAGE, SetCameraFrame)
#pragma alloc_text (PAGE, GetCameraFrame)
#pragma alloc_text (PAGE, CaptureCameraFrame)
#pragma alloc_text (PAGE, CameraEvtIoDeviceControl)
#endif

NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
) {
    NTSTATUS status;
    WDF_DRIVER_CONFIG config;

    KdPrint(("CameraDriver: DriverEntry called\n"));

    WDF_DRIVER_CONFIG_INIT(&config, CameraEvtDeviceAdd);

    status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        KdPrint(("CameraDriver: WdfDriverCreate failed: 0x%X\n", status));
        return status;
    }

    return STATUS_SUCCESS;
}

NTSTATUS CameraEvtDeviceAdd(
    _In_ WDFDRIVER Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
) {
    NTSTATUS status;
    WDFDEVICE device;
    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_IO_QUEUE_CONFIG queueConfig;
    PDEVICE_CONTEXT deviceContext;

    UNREFERENCED_PARAMETER(Driver);
    PAGED_CODE();
    KdPrint(("CameraDriver: CameraEvtDeviceAdd called\n"));

    WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoBuffered);
    WdfDeviceInitSetCharacteristics(DeviceInit, FILE_DEVICE_SECURE_OPEN, FALSE);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DEVICE_CONTEXT);
    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);
    if (!NT_SUCCESS(status)) {
        KdPrint(("CameraDriver: WdfDeviceCreate failed: 0x%X\n", status));
        return status;
    }

    // Create a device interface so that application can find and talk
    // to us.
    //
    status = WdfDeviceCreateDeviceInterface(
        device,
        &GUID_DEVINTERFACE_CAMERA,
        NULL // ReferenceString
    );

    if (!NT_SUCCESS(status)) {
        KdPrint(("CameraDriver: WdfDeviceCreateDeviceInterface: 0x%X\n", status));
        return status;
    }

    deviceContext = GetDeviceContext(device);
    deviceContext->WdfDevice = device;
    deviceContext->IsStreaming = FALSE;
    deviceContext->CurrentFormat = 0;

    InitializeListHead(&deviceContext->FrameList);
    KeInitializeSpinLock(&deviceContext->FrameListLock);

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchSequential);
    queueConfig.EvtIoDeviceControl = CameraEvtIoDeviceControl;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    status = WdfIoQueueCreate(device, &queueConfig, &attributes, &deviceContext->IoQueue);
    if (!NT_SUCCESS(status)) {
        KdPrint(("CameraDriver: WdfIoQueueCreate failed: 0x%X\n", status));
        return status;
    }

    /*
    status = InitializeKsDevice(device);
    if (!NT_SUCCESS(status)) {
        KdPrint(("CameraDriver: InitializeKsDevice failed: 0x%X\n", status));
        return status;
    }
    */

    KdPrint(("CameraDriver: Device added successfully\n"));
    return STATUS_SUCCESS;
}

NTSTATUS StartCameraStream(PDEVICE_CONTEXT deviceContext) {
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(deviceContext);
    PAGED_CODE();
    KdPrint(("CameraDriver: Camera stream started\n"));

    return status;
}

VOID CheckLongAndUlongSize() {
    KdPrint(("Size of BYTE: %zu bytes\n", sizeof(BYTE)));
    KdPrint(("Size of CHAR: %zu bytes\n", sizeof(CHAR)));
    KdPrint(("Size of UCHAR: %zu bytes\n", sizeof(UCHAR)));
    KdPrint(("Size of SHORT: %zu bytes\n", sizeof(SHORT)));
    KdPrint(("Size of USHORT: %zu bytes\n", sizeof(USHORT)));
    //KdPrint(("Size of INT: %zu bytes\n", sizeof(INT)));
    //KdPrint(("Size of UINT: %zu bytes\n", sizeof(UINT)));
    KdPrint(("Size of INT32: %zu bytes\n", sizeof(INT32)));
    KdPrint(("Size of UINT32: %zu bytes\n", sizeof(UINT32)));
    KdPrint(("Size of INT64: %zu bytes\n", sizeof(INT64)));
    KdPrint(("Size of UINT64: %zu bytes\n", sizeof(UINT64)));
    KdPrint(("Size of LONG: %zu bytes\n", sizeof(LONG)));
    KdPrint(("Size of ULONG: %zu bytes\n", sizeof(ULONG)));
    //KdPrint(("Size of WORD: %zu bytes\n", sizeof(WORD)));
    //KdPrint(("Size of DWORD: %zu bytes\n", sizeof(DWORD)));
    KdPrint(("Size of LARGE_INTEGER: %zu bytes\n", sizeof(LARGE_INTEGER)));
    KdPrint(("Size of BOOLEAN: %zu bytes\n", sizeof(BOOLEAN)));
    KdPrint(("Size of WCHAR: %zu bytes\n", sizeof(WCHAR)));
    KdPrint(("Size of UNICODE_STRING: %zu bytes\n", sizeof(UNICODE_STRING)));
}

VOID StopCameraStream(PDEVICE_CONTEXT deviceContext) {
    UNREFERENCED_PARAMETER(deviceContext);
    PAGED_CODE();

    CheckLongAndUlongSize();
    KdPrint(("CameraDriver: Camera stream stopped\n"));
}

NTSTATUS ReadBufferFromFile(LPCWSTR filename, PCHAR pBuffer, size_t *bufSize) {
    HANDLE                      fileHandle = NULL;
    NTSTATUS                    status = STATUS_SUCCESS;
    UNICODE_STRING              absFileName;
    OBJECT_ATTRIBUTES           fileAttributes;
    IO_STATUS_BLOCK             ioStatus;
    FILE_STANDARD_INFORMATION   fileInfo;
    PAGED_CODE ();

    RtlInitUnicodeString(&absFileName, filename);
    InitializeObjectAttributes( &fileAttributes,
                            &absFileName,
                            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                            NULL, // RootDirectory
                            NULL // SecurityDescriptor
                            );
    // 打开文件
    status = ZwOpenFile(&fileHandle,
                        GENERIC_READ,
                        &fileAttributes,
                        &ioStatus,
                        FILE_SHARE_READ,
                        FILE_SYNCHRONOUS_IO_NONALERT);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ReadFileSynchronously: ZwOpenFile  failed with status 0x%X\n", status));
        goto Cleanup;
    }

    // 获取文件信息
    status = ZwQueryInformationFile(fileHandle,
                                &ioStatus,
                                &fileInfo,
                                sizeof(fileInfo),
                                FileStandardInformation);
    if (!NT_SUCCESS(status)) {
        KdPrint(("GetFileSize: ZwQueryInformationFile failed with status 0x%X\n", status));
        goto Cleanup;
    }
    // 获取文件大小
    *bufSize = (size_t)(fileInfo.EndOfFile.QuadPart);

    // 写入文件内容
    status = ZwReadFile(
        fileHandle,
        NULL,
        NULL,
        NULL,
        &ioStatus,
        pBuffer,
        (ULONG)(*bufSize),
        NULL,
        NULL
    );

    if (!NT_SUCCESS(status)) {
        KdPrint(("ReadFileSynchronously: ZwReadFile failed with status 0x%X\n", status));
        goto Cleanup;
    }

Cleanup:
    // 关闭文件句柄
    if (fileHandle != NULL) {
        ZwClose(fileHandle);
    }
    KdPrint(("ReadBufferFromFile status: 0x%X\n", status));
    return status;
}

NTSTATUS WriteBufferToFile(LPCWSTR filename, PCHAR pBuffer, size_t bufSize) {
    HANDLE                      fileHandle = NULL;
    NTSTATUS                    status = STATUS_SUCCESS;
    UNICODE_STRING              absFileName;
    OBJECT_ATTRIBUTES           fileAttributes;
    IO_STATUS_BLOCK             ioStatus;
    PAGED_CODE ();

    KdPrint(("WriteBufferToFile Enter!\n"));
    RtlInitUnicodeString(&absFileName, filename);
    InitializeObjectAttributes( &fileAttributes,
                            &absFileName,
                            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                            NULL, // RootDirectory
                            NULL // SecurityDescriptor
                            );

    status = ZwCreateFile (
                &fileHandle,
                SYNCHRONIZE | GENERIC_WRITE,
                &fileAttributes,
                &ioStatus,
                NULL,// alloc size = none
                FILE_ATTRIBUTE_NORMAL,
                0,
                FILE_OPEN_IF,
                FILE_SYNCHRONOUS_IO_NONALERT |FILE_NON_DIRECTORY_FILE,
                NULL,// eabuffer
                0// ealength
                );
    if (!NT_SUCCESS(status)) {
        KdPrint(("WriteBufferToFile: ZwCreateFile failed with status 0x%X\n", status));
        goto Cleanup;
    }

    // 写入文件内容
    status = ZwWriteFile(
        fileHandle,
        NULL,
        NULL,
        NULL,
        &ioStatus,
        pBuffer,
        (ULONG)bufSize,
        0,
        NULL
    );

    if (!NT_SUCCESS(status)) {
        KdPrint(("WriteBufferToFile: ZwWriteFile failed with status 0x%X\n", status));
        goto Cleanup;
    }

Cleanup:
    // 关闭文件句柄
    if (fileHandle != NULL) {
        ZwClose(fileHandle);
    }
    KdPrint(("WriteBufferToFile status: 0x%X\n", status));
    return status;
}

NTSTATUS SetCameraFrame(PDEVICE_CONTEXT deviceContext, WDFREQUEST request, size_t* information) {
    NTSTATUS status = STATUS_SUCCESS;
    PCHAR  inBuf = NULL;
    size_t bufSize;
    UNREFERENCED_PARAMETER(deviceContext);
    UNREFERENCED_PARAMETER(information);
    PAGED_CODE();
    KdPrint(("CameraDriver: Camera stream started2\n"));

    status = WdfRequestRetrieveInputBuffer(request, 0, &inBuf, &bufSize);
    if (!NT_SUCCESS(status)) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        KdPrint(("CameraDriver: SetCameraFrame failed: 0x%X\n", status));
    }

    //PrintChars(inBuf, bufSize);
    PCWSTR filename = L"\\??\\C:\\test.png";
    status = WriteBufferToFile(filename, inBuf, bufSize);
    if (!NT_SUCCESS(status)) {
        KdPrint(("CameraDriver: failed to save buffer to file in kmd: 0x%X\n", status));
    }

    return status;
}

NTSTATUS CaptureCameraFrame(PDEVICE_CONTEXT deviceContext, PVOID buffer, size_t bufferLength, size_t* information) {
    PAGED_CODE();
    RtlZeroMemory(buffer, bufferLength);
    *information = deviceContext->FrameBufferSize;

    return STATUS_SUCCESS;
}

NTSTATUS GetCameraFrame(PDEVICE_CONTEXT deviceContext, WDFREQUEST request, size_t* information) {
    /*
    NTSTATUS status = STATUS_SUCCESS;
    PVOID outputBuffer = NULL;
    size_t bufferLength = 0;
    PAGED_CODE();
    status = WdfRequestRetrieveOutputBuffer(request, 0, &outputBuffer, &bufferLength);
    if (!NT_SUCCESS(status)) {
        KdPrint(("CameraDriver: WdfRequestRetrieveOutputBuffer failed: 0x%X\n", status));
        return status;
    }

    if (bufferLength < deviceContext->FrameBufferSize) {
        status = STATUS_BUFFER_TOO_SMALL;
        *information = deviceContext->FrameBufferSize;
        return status;
    }

    status = CaptureCameraFrame(deviceContext, outputBuffer, bufferLength, information);

    return status;
    */
    NTSTATUS status = STATUS_SUCCESS;
    //PCHAR  outBuf = NULL;
    size_t bufSize;
    PCHAR  outputBuffer = NULL;
    size_t bufferLength = 0;

    UNREFERENCED_PARAMETER(deviceContext);
    UNREFERENCED_PARAMETER(information);
    PAGED_CODE();
    KdPrint(("CameraDriver: Camera stream started2\n"));

    status = WdfRequestRetrieveOutputBuffer(request, 0, &outputBuffer, &bufferLength);
    if (!NT_SUCCESS(status)) {
        KdPrint(("CameraDriver: fail to retrieve output buffer: 0x%X\n", status));
    }

    PCWSTR filename = L"\\??\\C:\\test.png";
    status = ReadBufferFromFile(filename, outputBuffer, &bufSize);
    if (!NT_SUCCESS(status)) {
        KdPrint(("CameraDriver: failed to save buffer to file in kmd: 0x%X\n", status));
    }

    //RtlCopyMemory(outputBuffer, outBuf, bufSize);
    *information = bufSize;

    return status;
}

VOID CameraEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
) {
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_CONTEXT deviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));
    size_t information = 0;
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);
    PAGED_CODE();
    KdPrint(("CameraDriver: CameraEvtIoDeviceControl called with IOCTL 0x%X\n", IoControlCode));

    switch (IoControlCode) {
    case IOCTL_CAMERA_START_STREAM:
        /*
        if (deviceContext->IsStreaming) {
            status = STATUS_ALREADY_INITIALIZED;
            break;
        }
        */
        status = StartCameraStream(deviceContext);
        if (NT_SUCCESS(status)) {
            deviceContext->IsStreaming = TRUE;
        }
        break;

    case IOCTL_CAMERA_STOP_STREAM:
        /*
        if (!deviceContext->IsStreaming) {
            status = STATUS_NOT_SUPPORTED;
            break;
        }
        */

        StopCameraStream(deviceContext);
        deviceContext->IsStreaming = FALSE;
        break;

    case IOCTL_CAMERA_SET_FRAME:
        /*
        if (!deviceContext->IsStreaming) {
            status = STATUS_NOT_SUPPORTED;
            break;
        }
        */

        status = SetCameraFrame(deviceContext, Request, &information);
        break;

    case IOCTL_CAMERA_GET_FRAME:
        /*
        if (!deviceContext->IsStreaming) {
            status = STATUS_NOT_SUPPORTED;
            break;
        }
        */

        status = GetCameraFrame(deviceContext, Request, &information);
        break;

    case IOCTL_CAMERA_LOAD_FIRMWARE:
        /*load firmware */
        PVOID firmwareBuffer = NULL;
        ULONG firmwareSize = 0;
        UNICODE_STRING firmwareFileName;
        PDEVICE_OBJECT pDeviceObject = NULL;
        PDRIVER_OBJECT pDriverObject = NULL;

        pDeviceObject = WdfDeviceWdmGetDeviceObject(deviceContext->WdfDevice);
         if (pDeviceObject) {
            // 从 DEVICE_OBJECT 获取 DRIVER_OBJECT
            pDriverObject = pDeviceObject->DriverObject;
            KdPrint(("CameraDriver: Got DRIVER_OBJECT from DEVICE_OBJECT\n"));
        }

        RtlInitUnicodeString(&firmwareFileName, L"linlonvd_fw.bin");
        status = LoadFirmware(pDriverObject, &firmwareFileName, &firmwareBuffer, &firmwareSize);
        if (NT_SUCCESS(status)) {
            KdPrint(("Firmware loaded successfully, size: %u bytes\n", firmwareSize));
            // 使用固件数据...
            // PrintHex((PCHAR)firmwareBuffer, firmwareSize);
            ExFreePoolWithTag(firmwareBuffer, 'FrmW');
        } else {
            KdPrint(("Failed to load firmware with status 0x%x\n", status));
        }
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    WdfRequestCompleteWithInformation(Request, status, information);
}

VOID
PrintHex(
    _In_reads_(CountChars) PCHAR BufferAddress,
    _In_ size_t CountChars
    )
{
    LONG loop = 0;
    LONG *p = (LONG*)BufferAddress;

    while (CountChars>4)
    {
        KdPrint(( "%016x ", *p));
        CountChars -= 4;
        p++;

        if ((++loop) % 4 == 0)
        {
            KdPrint (("\n"));
        }
    }
    KdPrint (("\n"));

    return;
}

VOID
PrintChars(
    _In_reads_(CountChars) PCHAR BufferAddress,
    _In_ size_t CountChars
    )
{
    if (CountChars) {

        while (CountChars--) {

            if (*BufferAddress > 31
                 && *BufferAddress != 127) {

                KdPrint (( "%c", *BufferAddress) );

            } else {

                KdPrint(( ".") );

            }
            BufferAddress++;
        }
        KdPrint (("\n"));
    }
    return;
}

// 加载固件函数
NTSTATUS LoadFirmware(PDRIVER_OBJECT DriverObject, PUNICODE_STRING FirmwareName, PVOID* FirmwareBuffer, PULONG FirmwareSize) {
    NTSTATUS status;
    HANDLE driverDirHandle = NULL;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatusBlock;
    PFILE_NAME_INFORMATION fileNameInfo;
    FILE_STANDARD_INFORMATION fileInfo;
    PVOID buffer = NULL;
    ULONG bufferSize = 1024;
#if 0
    // 获取驱动目录
    status = IoGetDriverDirectory(DriverObject, DriverDirectoryImage, 0, &driverDirHandle);
    if (!NT_SUCCESS(status)) {
        KdPrint(("IoGetDriverDirectory failed with status 0x%x\n", status));
        return status;
    }

    // 初始化对象属性
    InitializeObjectAttributes(&objectAttributes, FirmwareName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, driverDirHandle, NULL);

    // 打开文件
    status = ZwCreateFile (
                &fileHandle,
                SYNCHRONIZE | GENERIC_WRITE | GENERIC_READ | FILE_READ_ATTRIBUTES,
                &objectAttributes,
                &ioStatusBlock,
                NULL,// alloc size = none
                FILE_ATTRIBUTE_NORMAL,
                0,
                FILE_OPEN_IF,
                FILE_SYNCHRONOUS_IO_NONALERT |FILE_NON_DIRECTORY_FILE,
                NULL,// eabuffer
                0// ealength
                );
    if (!NT_SUCCESS(status)) {
        KdPrint(("ZwCreateFile failed with status 0x%x\n", status));
        goto cleanup;
    }

    // 获取文件名
    fileNameInfo = (PFILE_NAME_INFORMATION)ExAllocatePool2(POOL_FLAG_NON_PAGED, bufferSize, 'FpNm');
    if (fileNameInfo == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        KdPrint(("ExAllocatePool2 failed with status 0x%x\n", status));
        goto cleanup;
    }

    status = ZwQueryInformationFile(fileHandle, &ioStatusBlock, fileNameInfo, bufferSize, FileNameInformation);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ZwQueryFileNameInformationFile failed with status 0x%x\n", status));
        ExFreePoolWithTag(fileNameInfo, 'FpNm');
        goto cleanup;
    }
    else{
        KdPrint(("absFileName: %.*ws\n", fileNameInfo->FileNameLength / sizeof(WCHAR), fileNameInfo->FileName));
        ExFreePoolWithTag(fileNameInfo, 'FpNm');
    }
#else
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(fileNameInfo);
    UNREFERENCED_PARAMETER(driverDirHandle);
    UNREFERENCED_PARAMETER(bufferSize);
    UNREFERENCED_PARAMETER(FirmwareName);
    // 初始化对象属性
    PCWSTR DirectoryPath = L"\\??\\C:\\Windows\\System32\\drivers\\DriverData\\linlonvd_fw.bin";
    UNICODE_STRING absFileName;
    RtlInitUnicodeString(&absFileName, DirectoryPath);
    //RtlAppendUnicodeStringToString(&absFileName, FirmwareName);
    KdPrint(("absFileName: %.*ws\n", absFileName.Length / sizeof(WCHAR), absFileName.Buffer));

    InitializeObjectAttributes(&objectAttributes, &absFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    status = ZwOpenFile(&fileHandle, GENERIC_READ, &objectAttributes, &ioStatusBlock, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ZwOpenFile failed with status 0x%x\n", status));
        goto cleanup;
    }
#endif

    // 获取文件大小
    status = ZwQueryInformationFile(fileHandle, &ioStatusBlock, &fileInfo, sizeof(fileInfo), FileStandardInformation);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ZwQueryFileStandardInformationFile failed with status 0x%x\n", status));
        goto cleanup;
    }

    // 分配内存
    buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, (SIZE_T)fileInfo.EndOfFile.QuadPart, 'FrmW');
    if (buffer == NULL) {
        KdPrint(("ExAllocatePool2 failed with status 0x%x\n", status));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    // 读取文件内容
    status = ZwReadFile(fileHandle, NULL, NULL, NULL, &ioStatusBlock, buffer, (ULONG)fileInfo.EndOfFile.QuadPart, NULL, NULL);
    if (!NT_SUCCESS(status)) {
        KdPrint(("ZwReadFile failed with status 0x%x\n", status));
        ExFreePoolWithTag(buffer, 'FrmW');
        goto cleanup;
    }

    *FirmwareBuffer = buffer;
    *FirmwareSize = (ULONG)fileInfo.EndOfFile.QuadPart;

cleanup:
    ZwClose(fileHandle);
    return status;
}
