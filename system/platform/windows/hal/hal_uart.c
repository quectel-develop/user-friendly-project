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


#define COM_PORT    "\\\\.\\"AT_UART""  // 串口名称
#define BAUD_RATE   CBR_115200          // 波特率（115200）
#define DATABITS    8                   // 数据位
#define STOPBITS    ONESTOPBIT          // 停止位
#define PARITY      NOPARITY            // 校验位

static HANDLE           g_hComm;        // 串口句柄
static DWORD            g_dwEvtMask;    // 事件掩码
static event_callback_t g_callback = NULL;
static osa_task_t       g_uart_task = NULL;

// 等待串口事件并读取数据
void wait_for_data(void *argv)
{
    // 设置串口事件掩码，监听接收数据事件
    SetCommMask(g_hComm, EV_RXCHAR);

    while (1)
    {
        // 等待事件发生
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

// uart 读事件回调注册
void qosa_uart_register(event_callback_t event_cb)
{
    if (g_callback == NULL)
    {
        qosa_task_create(&g_uart_task, 8192, QOSA_PRIORITY_NORMAL, "UART_EVENT", wait_for_data, NULL);
    }
    g_callback = event_cb;
}

// 配置串口参数函数
static int qosa_uart_config(void)
{
    DCB dcbSerialParams = {0};

    if (!GetCommState(g_hComm, &dcbSerialParams))
    {
        LOG_D("uart error: %d\n", GetLastError());
        return -1;
    }

    // 设置缓冲区大小
    SetupComm(g_hComm, 10240, 10240);

    // 设置串口参数
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    dcbSerialParams.BaudRate = BAUD_RATE;  // 波特率
    dcbSerialParams.ByteSize = DATABITS;   // 数据位
    dcbSerialParams.StopBits = STOPBITS;   // 停止位
    dcbSerialParams.Parity = PARITY;       // 校验位

    if (!SetCommState(g_hComm, &dcbSerialParams))
    {
        LOG_D("uart set: %d\n", GetLastError());
        return -1;
    }

    LOG_D("uart set success\n");
    return 0;
}

// 打开串口函数
int qosa_uart_open(void)
{
    g_hComm = CreateFile(
        COM_PORT,                      // 串口名称
        GENERIC_READ | GENERIC_WRITE,  // 读写权限
        0,                             // 不共享
        NULL,                          // 默认安全属性
        OPEN_EXISTING,                 // 打开已存在的串口
        FILE_FLAG_OVERLAPPED,          // 异步接口
        NULL                           // 默认模板文件
    );

    if (g_hComm == INVALID_HANDLE_VALUE)
    {
        // 如果打开失败
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
            // 写入操作正在进行，等待完成
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
