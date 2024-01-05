#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Message.h"

int main() {
    std::string fileName;
    std::cout << "Enter name of shared file:" << '\n' << '\n';
    std::cin >> fileName;

    int messageCount;
    std::cout << '\n' << "Enter max number of messages:" << '\n' << '\n';
    std::cin >> messageCount;

    int senderCount;
    std::cout << '\n' << "Enter number of sender processes:" << '\n' << '\n';
    std::cin >> senderCount;

    std::wstring wstrFileName = std::wstring(fileName.begin(), fileName.end());
    LPWSTR lpwstrFileName = &wstrFileName[0];

    HANDLE messageFile = CreateFile(
        lpwstrFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        0,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (messageFile == INVALID_HANDLE_VALUE)
    {
        std::cerr << "CreateFile failed with error " << GetLastError() << '\n';
        return 1;
    }

    CloseHandle(messageFile);

    HANDLE mtx = CreateMutex(NULL, FALSE, L"SharedMutex");

    if (mtx == INVALID_HANDLE_VALUE)
    {
        std::cerr << "CreateMutex failed with error " << GetLastError() << '\n';
        return 1;
    }

    HANDLE receiverEvent = CreateEvent(NULL, TRUE, FALSE, _T("SharedEvent"));

    if (receiverEvent == INVALID_HANDLE_VALUE)
    {
        std::cerr << "CreateEvent failed with error " << GetLastError() << '\n';
        return 1;
    }

    HANDLE* senderEvent = new HANDLE[senderCount];

    for (int i = 0; i < senderCount; i++)
    {
        std::wstring wstrEventName = std::to_wstring(i + 1);
        LPWSTR lpwstrEventName = &wstrEventName[0];
        senderEvent[i] = CreateEvent(NULL, TRUE, FALSE, lpwstrEventName);
        if (senderEvent[i] == INVALID_HANDLE_VALUE)
        {
            std::cerr << "CreateEvent failed with error " << GetLastError() << '\n';
            return 1;
        }
    }

    HANDLE fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int), L"SharedMemoryName");

    if (fileMapping == NULL)
    {
        std::cerr << "CreateFileMapping failed with error " << GetLastError() << std::endl;
        return 1;
    }

    int* pMessageCount = (int*)MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));

    if (pMessageCount == NULL)
    {
        std::cerr << "MapViewOfFile failed with error " << GetLastError() << std::endl;
        CloseHandle(fileMapping);
        return 1;
    }

    *pMessageCount = 0;

    for (int i = 0; i < senderCount; i++)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);

        std::string strProcessName = "SenderProcess.exe " + fileName + " " + std::to_string(i + 1) + " " + std::to_string(messageCount);
        std::wstring wstrProcessName = std::wstring(strProcessName.begin(), strProcessName.end());
        LPWSTR lpwstrProcessName = &wstrProcessName[0];

        if (!CreateProcess(
            NULL,
            lpwstrProcessName,
            NULL,
            NULL,
            TRUE,
            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &si,
            &pi
        )) {
            std::cout << "Failed to create sender process" << '\n';
            return 1;
        }

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    WaitForMultipleObjects(senderCount, senderEvent, TRUE, INFINITE);

    std::cout << "Sender processes are ready" << '\n' << '\n';

    SetEvent(receiverEvent);
    Sleep(10);
    ResetEvent(receiverEvent);

    int messageCounter = 0;

    while (true)
    {
        std::string receiverCommand;
        std::cout << "Choose opion for receiver (r - for read/e - for exit)" << '\n';
        std::cin >> receiverCommand;

        if (receiverCommand == "e")
            break;

        if (receiverCommand == "r")
        {
            if (WaitForSingleObject(mtx, INFINITE) == WAIT_OBJECT_0) {

                int currentMessageCount = *pMessageCount;

                if (currentMessageCount == 0)
                {
                    std::cout << "Waiting for message. Stand by" << '\n';
                    ResetEvent(receiverEvent);
                    ReleaseMutex(mtx);
                    WaitForSingleObject(receiverEvent, INFINITE);
                    WaitForSingleObject(mtx, INFINITE);
                }

                HANDLE messageFile = CreateFile(
                    lpwstrFileName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                    0,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );

                if (messageFile == INVALID_HANDLE_VALUE)
                {
                    std::cerr << "CreateFile failed with error " << GetLastError() << std::endl;
                    return 1;
                }
                
                LARGE_INTEGER li;
                li.QuadPart = messageCounter * sizeof(Message);
                SetFilePointerEx(messageFile, li, NULL, FILE_BEGIN);

                Message message;
                DWORD bytesRead;

                ReadFile(messageFile, &message, sizeof(Message), &bytesRead, NULL);

                (*pMessageCount)--;
                messageCounter++;

                std::cout << '\n' << "Received message from Sender " << message.senderName << ": " << '\n' << '\n' << message.content << '\n';

                CloseHandle(messageFile);

                for (int i = 0; i < senderCount; i++)
                    SetEvent(senderEvent[i]);

                ReleaseMutex(mtx);
            }
        }
    }

    CloseHandle(mtx);

    UnmapViewOfFile(pMessageCount);
    CloseHandle(fileMapping);

    for (int i = 0; i < senderCount; i++)
    {
        CloseHandle(senderEvent);
    }

    return 0;
}
