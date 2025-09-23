/*****************************************************************/ /**
* @file hal_uart.c
* @brief
* @author larson.li@quectel.com
* @date 2024-12-23
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* <tr><td>2024-12-23 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/

#include <windows.h>
#include <stdio.h>
#include "qosa_def.h"
#include "qosa_system.h"
#include "qosa_log.h"
#include "hal_uart.h"


#define COM_PORT    "\\\\.\\"AT_UART""  // Serial port name
#define BAUD_RATE   CBR_115200          // Baud rate (115200)
#define DATABITS    8                   // Data bits
#define STOPBITS    ONESTOPBIT          // Stop bits
#define PARITY      NOPARITY            // Parity bit

static HANDLE           g_hComm;        // Serial port handle
static DWORD            g_dwEvtMask;    // Event mask
static event_callback_t g_callback = NULL;
static osa_task_t       g_uart_task = NULL;

// Wait for serial port events and read data
void wait_for_data(void *argv)
{
    // Set serial port event mask to listen for data reception events
    SetCommMask(g_hComm, EV_RXCHAR);

    while (1)
    {
        // Wait for event to occur
        if (WaitCommEvent(g_hComm, &g_dwEvtMask, NULL))
        {
            if (g_dwEvtMask & EV_RXCHAR)
            {
                g_callback(1);
            }
        }
        //LOG_D("wait serial event");
    }
}

// UART read event callback registration
void qosa_uart_register(event_callback_t event_cb)
{
    if (g_callback == NULL)
    {
        qosa_task_create(&g_uart_task, 8192, QOSA_PRIORITY_NORMAL, "UART_EVENT", wait_for_data, NULL);
    }
    g_callback = event_cb;
}

// Configure serial port parameters function
static int qosa_uart_config(void)
{
    DCB dcbSerialParams = {0};

    if (!GetCommState(g_hComm, &dcbSerialParams))
    {
        LOG_D("uart error: %d\n", GetLastError());
        return -1;
    }

    // Set buffer size
    SetupComm(g_hComm, 10240, 10240);

    // Set serial port parameters
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    dcbSerialParams.BaudRate = BAUD_RATE;  // Baud rate
    dcbSerialParams.ByteSize = DATABITS;   // Data bits
    dcbSerialParams.StopBits = STOPBITS;   // Stop bits
    dcbSerialParams.Parity = PARITY;       // Parity bit

    if (!SetCommState(g_hComm, &dcbSerialParams))
    {
        LOG_D("uart set: %d\n", GetLastError());
        return -1;
    }

    LOG_D("uart set success\n");
    return 0;
}

// Open serial port function
int qosa_uart_open(void)
{
    g_hComm = CreateFile(
        COM_PORT,                      // Serial port name
        GENERIC_READ | GENERIC_WRITE,  // Read/write permissions
        0,                             // No sharing
        NULL,                          // Default security attributes
        OPEN_EXISTING,                 // Open existing serial port
        FILE_FLAG_OVERLAPPED,          // Asynchronous interface
        NULL                           // Default template file
    );

    if (g_hComm == INVALID_HANDLE_VALUE)
    {
        // If opening fails
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            LOG_D("uart not find, %s", COM_PORT);
        }
        else
        {
            LOG_D("uart error: %d, %s", GetLastError(), COM_PORT);
        }
        return -1;
    }

    LOG_D("uart open success");
    qosa_uart_config();
    return 0;
}

int qosa_uart_write(int pos, const char *buffer, size_t size)
{
    DWORD      dwBytesWritten;
    OVERLAPPED osWrite = {0};

    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    LOG_D("size=%d,buffer=%s", size, buffer);
    BOOL result = WriteFile(g_hComm, buffer, size, &dwBytesWritten, &osWrite);
    if (!result)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            // Write operation is in progress, wait for completion
            WaitForSingleObject(osWrite.hEvent, INFINITE);
            GetOverlappedResult(g_hComm, &osWrite, &dwBytesWritten, TRUE);
            LOG_D("write end: %ld\n", dwBytesWritten);
        }
        else
        {
            LOG_E("write error: %ld\n", GetLastError());
        }
    }
    else
    {
        LOG_D("write end: %ld\n", dwBytesWritten);
    }

    return size;
}

int qosa_uart_read(int pos, const char *buffer, size_t size)
{
    DWORD      dwBytesRead;
    OVERLAPPED osReader = {0};

    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    //LOG_D("uart read=%d\n", size);

    BOOL result = ReadFile(g_hComm, buffer, size, &dwBytesRead, &osReader);
    if (!result && GetLastError() == ERROR_IO_PENDING)
    {
        WaitForSingleObject(osReader.hEvent, INFINITE);
        GetOverlappedResult(g_hComm, &osReader, &dwBytesRead, TRUE);
    }
    else if (!result)
    {
        LOG_D("read error: %ld\n", GetLastError());
    }

    LOG_D("read: %d,%ld, %s", size, dwBytesRead, buffer);
    return (int)dwBytesRead;
}
