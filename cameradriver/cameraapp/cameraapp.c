// cameraapp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <DriverSpecs.h>
_Analysis_mode_(_Analysis_code_type_user_code_)
#define INITGUID

#include <windows.h>

#include <strsafe.h>
#include <cfgmgr32.h>
#include <stdio.h>
#include <stdlib.h>

#include "public.h"

#define MAX_DEVPATH_LENGTH                       256
WCHAR G_DevicePath[MAX_DEVPATH_LENGTH];

#define MAX_BUFFER_LENGTH (20*1024*1024)
static char* g_InputBuffer = NULL;
static char* g_OutputBuffer = NULL;

BOOL
GetDevicePath(
    IN  LPGUID InterfaceGuid,
    _Out_writes_(BufLen) PWCHAR DevicePath,
    _In_ size_t BufLen
);

int MyReadFile(LPCWSTR lpFileName, PCHAR InputBuffer, size_t *pfileSize)

{
    DWORD bytesRead;
    HANDLE hFile = CreateFile(
        lpFileName,                 // �ļ���
        GENERIC_READ,                // ��ȡȨ��
        FILE_SHARE_READ,             // �����ȡ
        NULL,                        // Ĭ�ϰ�ȫ����
        OPEN_EXISTING,               // ���Ѵ��ڵ��ļ�
        FILE_ATTRIBUTE_NORMAL,       // ��ͨ�ļ�
        NULL                         // ��ģ���ļ�
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("failed to open file!\n");
        return -1;
    }

    // ��ȡ�ļ���С
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        printf("failed to get file size!\n");
        CloseHandle(hFile);
        return -1;
    }

    // ��ȡ�ļ�����
    BOOL result = ReadFile(
        hFile,                       // �ļ����
        InputBuffer,                 // ������
        fileSize,                    // ��ȡ��С
        &bytesRead,                  // ʵ�ʶ�ȡ��С
        NULL                         // ���첽����
    );

    if (!result) {
        printf("failed to read file!\n");
        CloseHandle(hFile);
        return -1;
    }

    // ����ַ���������
    InputBuffer[bytesRead] = '\0';

    *pfileSize = (size_t)bytesRead;

    CloseHandle(hFile);
    return 0;
}

int MyWriteFile(LPCWSTR lpFileName, PCHAR OutputBuffer, size_t bufSize)

{
    DWORD bytesWritten;
    HANDLE hFile = CreateFile(
        lpFileName,                 // �ļ���
        GENERIC_WRITE,               // д��Ȩ��
        0,                          // No sharing
        NULL,                        // Ĭ�ϰ�ȫ����
        CREATE_ALWAYS,               // ���Ѵ��ڵ��ļ�
        FILE_ATTRIBUTE_NORMAL,       // ��ͨ�ļ�
        NULL                         // ��ģ���ļ�
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("failed to open file!\n");
        return -1;
    }

    // ��ȡ�ļ�����
    BOOL result = WriteFile(
        hFile,                       // �ļ����
        OutputBuffer,                 // ������
        (DWORD)bufSize,                    // ��ȡ��С
        &bytesWritten,                  // ʵ�ʶ�ȡ��С
        NULL                         // ���첽����
    );

    if (!result) {
        printf("Failed to write file! Error code: %d\n", GetLastError());
    }

    CloseHandle(hFile);
    return 0;
}

VOID
DoIoctls(
    HANDLE hDevice
)
{
    BOOL bRc;
    ULONG bytesReturned;

    // Printing Input & Output buffer pointers and size
    char InputBuffer[1024];
    char OutputBuffer[1024];

    printf("InputBuffer Pointer = %p, BufLength = %Id\n", InputBuffer,
        sizeof(InputBuffer));
    printf("OutputBuffer Pointer = %p BufLength = %Id\n", OutputBuffer,
        sizeof(OutputBuffer));
    //
    // Performing METHOD_BUFFERED
    //

    if (FAILED(StringCchCopy(InputBuffer, sizeof(InputBuffer),
        "this String is from User Application; start stream.")))
    {
        return;
    }

    printf("\nCalling DeviceIoControl IOCTL_CAMERA_START_STREAM:\n");

    memset(OutputBuffer, 0, sizeof(OutputBuffer));

    bRc = DeviceIoControl(hDevice,
        (DWORD)IOCTL_CAMERA_START_STREAM,
        InputBuffer,
        (DWORD)strlen(InputBuffer) + 1,
        OutputBuffer,
        sizeof(OutputBuffer),
        &bytesReturned,
        NULL
    );

    if (!bRc)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return;
    }

    if (FAILED(StringCchCopy(InputBuffer, sizeof(InputBuffer),
        "this String is from User Application; stop stream.")))
    {
        return;
    }

    printf("\nCalling DeviceIoControl IOCTL_CAMERA_STOP_STREAM:\n");

    bRc = DeviceIoControl(hDevice,
        (DWORD)IOCTL_CAMERA_STOP_STREAM,
        InputBuffer,
        (DWORD)strlen(InputBuffer) + 1,
        OutputBuffer,
        sizeof(OutputBuffer),
        &bytesReturned,
        NULL
    );
    if (!bRc)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return;
    }

    return;
}


VOID
DoIoctls2(
    HANDLE hDevice
)
{
    BOOL bRc;
    ULONG bytesReturned;
    size_t fileSize = 0;

    g_InputBuffer = (char*) malloc(MAX_BUFFER_LENGTH *sizeof(char));
    g_OutputBuffer = (char*) malloc(MAX_BUFFER_LENGTH * sizeof(char));

    if (g_InputBuffer == NULL)
    {
        printf("failed to malloc memory for g_InputBuffer.\n");
        return;
    }

    if (g_OutputBuffer == NULL)
    {
        printf("failed to malloc memory for g_OutputBuffer.\n");
        return;
    }
    memset(g_InputBuffer, 0, MAX_BUFFER_LENGTH * sizeof(char));
    memset(g_OutputBuffer, 0, MAX_BUFFER_LENGTH * sizeof(char));

    // IOCTL_CAMERA_START_STREAM
    printf("\nCalling DeviceIoControl IOCTL_CAMERA_START_STREAM:\n");
    bRc = DeviceIoControl(hDevice,
        (DWORD)IOCTL_CAMERA_START_STREAM,
        g_InputBuffer,
        (DWORD)MAX_BUFFER_LENGTH,
        g_OutputBuffer,
        MAX_BUFFER_LENGTH,
        &bytesReturned,
        NULL
    );
    if (!bRc)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return;
    }


    // IOCTL_CAMERA_SET_FRAME
    printf("\nCalling DeviceIoControl IOCTL_CAMERA_SET_FRAME:\n");
    MyReadFile(L"test.png", g_InputBuffer, &fileSize);
    bRc = DeviceIoControl(hDevice,
        (DWORD)IOCTL_CAMERA_SET_FRAME,
        g_InputBuffer,
        (DWORD)fileSize,
        NULL,
        (DWORD)0,
        &bytesReturned,
        NULL
    );
    if (!bRc)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return;
    }

    // IOCTL_CAMERA_GET_FRAME
    printf("\nCalling DeviceIoControl IOCTL_CAMERA_GET_FRAME:\n");
    bRc = DeviceIoControl(hDevice,
        (DWORD)IOCTL_CAMERA_GET_FRAME,
        NULL,
        (DWORD)0,
        g_OutputBuffer,
        (DWORD)MAX_BUFFER_LENGTH,
        &bytesReturned,
        NULL
    );
    if (!bRc)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return;
    }
    else
    {
        // write the bytesReturned to test2.png
        MyWriteFile(L"test_pullout.png", g_OutputBuffer, bytesReturned);
    }
    // IOCTL_CAMERA_LOAD_FIRMWARE
    printf("\nCalling DeviceIoControl IOCTL_CAMERA_LOAD_FIRMWARE:\n");
    bRc = DeviceIoControl(hDevice,
        (DWORD)IOCTL_CAMERA_LOAD_FIRMWARE,
        NULL,
        (DWORD)0,
        NULL,
        0,
        NULL,
        NULL
    );
    if (!bRc)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return;
    }
    // IOCTL_CAMERA_STOP_STREAM
    printf("\nCalling DeviceIoControl IOCTL_CAMERA_STOP_STREAM:\n");
    bRc = DeviceIoControl(hDevice,
        (DWORD)IOCTL_CAMERA_STOP_STREAM,
        g_InputBuffer,
        (DWORD)MAX_BUFFER_LENGTH,
        g_OutputBuffer,
        MAX_BUFFER_LENGTH,
        &bytesReturned,
        NULL
    );
    if (!bRc)
    {
        printf("Error in DeviceIoControl : %d", GetLastError());
        return;
    }

    if (g_InputBuffer)
        free(g_InputBuffer);

    if (g_OutputBuffer)
        free(g_OutputBuffer);

    return;
}

int __cdecl
main(void)
{
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    BOOLEAN result = TRUE;

    if (!GetDevicePath(
        (LPGUID)&GUID_DEVINTERFACE_CAMERA,
        G_DevicePath,
        sizeof(G_DevicePath) / sizeof(G_DevicePath[0])))
    {
        result = FALSE;
        goto exit;
    }
    printf("DevicePath: %ws\n", G_DevicePath);

    hDevice = CreateFile(G_DevicePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open device. Error %d\n", GetLastError());
        result = FALSE;
        goto exit;
    }

    printf("Opened camera device successfully\n");

    //DoIoctls(hDevice);
    DoIoctls2(hDevice);

exit:
    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
    }
    printf("Close camera device successfully\n");

    return ((result == TRUE) ? 0 : 1);
}


// ��ȡָ���ӿ� GUID ��Ӧ���豸·����������·���洢�ڴ���� DevicePath �������С�
// ����ɹ���ȡ·�������� TRUE�����򷵻� FALSE��
// ������
//   InterfaceGuid: ָ���豸�ӿڵ� GUID ��ָ�롣
//   DevicePath: ���ڴ洢�豸·���Ļ�������
//   BufLen: �������Ĵ�С�����ַ�Ϊ��λ����
BOOL
GetDevicePath(
    _In_ LPGUID InterfaceGuid,
    _Out_writes_(BufLen) PWCHAR DevicePath,
    _In_ size_t BufLen
)
{
    CONFIGRET cr = CR_SUCCESS;
    PWSTR deviceInterfaceList = NULL;
    ULONG deviceInterfaceListLength = 0;
    PWSTR nextInterface;
    HRESULT hr = E_FAIL;
    BOOL bRet = TRUE;

    // ��ȡ�豸�ӿ��б�Ĵ�С
    cr = CM_Get_Device_Interface_List_Size(
        &deviceInterfaceListLength,
        InterfaceGuid,
        NULL,
        CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
    if (cr != CR_SUCCESS) {
        printf("Error 0x%x retrieving device interface list size.\n", cr);
        goto Cleanup;
    }

    if (deviceInterfaceListLength <= 1) {
        bRet = FALSE;
        printf("Error: No active device interfaces found.\n"
            " Is the sample driver loaded?");
        goto Cleanup;
    }

    deviceInterfaceList = (PWSTR)malloc(deviceInterfaceListLength * sizeof(WCHAR));
    if (deviceInterfaceList == NULL) {
        printf("Error allocating memory for device interface list.\n");
        goto Cleanup;
    }
    ZeroMemory(deviceInterfaceList, deviceInterfaceListLength * sizeof(WCHAR));

    cr = CM_Get_Device_Interface_List(
        InterfaceGuid,
        NULL,
        deviceInterfaceList,
        deviceInterfaceListLength,
        CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
    if (cr != CR_SUCCESS) {
        printf("Error 0x%x retrieving device interface list.\n", cr);
        goto Cleanup;
    }

    nextInterface = deviceInterfaceList + wcslen(deviceInterfaceList) + 1;
    if (*nextInterface != UNICODE_NULL) {
        printf("Warning: More than one device interface instance found. \n"
            "Selecting first matching device.\n\n");
    }

    hr = StringCchCopy(DevicePath, BufLen, deviceInterfaceList);
    if (FAILED(hr)) {
        bRet = FALSE;
        printf("Error: StringCchCopy failed with HRESULT 0x%x", hr);
        goto Cleanup;
    }

Cleanup:
    if (deviceInterfaceList != NULL) {
        free(deviceInterfaceList);
    }
    if (CR_SUCCESS != cr) {
        bRet = FALSE;
    }

    return bRet;
}


