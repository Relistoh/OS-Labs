#include <iostream>
#include <fstream>
#include <windows.h>
#include <tchar.h>
#include <string>
#include "C:\Users\anton\source\repos\ProcessSync\ReceiverProcess\Message.h"

int main(int argc, char* argv[]) {
    std::string fileName = argv[1];
    int senderNumber = std::stoi(argv[2]);
    int maxMessageCount = std::stoi(argv[3]);
    std::cout << "Max messages count:" << maxMessageCount << '\n' << '\n';

    HANDLE hMutex = OpenMutex(SYNCHRONIZE, FALSE, L"SharedMutex");

    std::wstring wstrFileName = std::wstring(fileName.begin(), fileName.end());
    LPWSTR lpwstrFileName = &wstrFileName[0];

    std::wstring wstrEventName = std::to_wstring(senderNumber);
    LPWSTR lpwstrEventName = &wstrEventName[0];
    HANDLE senderEvent = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, lpwstrEventName);

    if (senderEvent == NULL)
    {
        std::cerr << "OpenEvent failed with error " << GetLastError() << '\n';
        return 1;
    }

    HANDLE receiverEvent = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, _T("SharedEvent"));

    if (receiverEvent == NULL)
    {
        std::cerr << "OpenEvent failed with error " << GetLastError() << '\n';
        return 1;
    }

    HANDLE fileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"SharedMemoryName");

    if (fileMapping == NULL)
    {
        std::cerr << "OpenFileMapping failed with error " << GetLastError() << '\n';
        return 1;
    }

    int* pMessageCount = (int*)MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));

    if (pMessageCount == NULL)
    {
        std::cerr << "MapViewOfFile failed with error " << GetLastError() << '\n';
        CloseHandle(fileMapping);
        return 1;
    }

    SetEvent(senderEvent);

    WaitForSingleObject(receiverEvent, INFINITE);

    while (true) {
        std::cout << "Choose option for sender (w - for write/e - for exit)" << '\n' << '\n';
        std::string senderCommand;
        std::cin >> senderCommand;

        if (senderCommand == "e") {
            break;
        }

        if (senderCommand == "w") {
            if (WaitForSingleObject(hMutex, INFINITE) == WAIT_OBJECT_0)
            {
                int currentMessageCount = *pMessageCount;

                if (currentMessageCount == maxMessageCount)
                {
                    std::cout << "File is full. Stand by" << '\n' << '\n';
                    ResetEvent(senderEvent);
                    ReleaseMutex(hMutex);
                    WaitForSingleObject(senderEvent, INFINITE);
                    WaitForSingleObject(hMutex, INFINITE);
                }
                ReleaseMutex(hMutex);

                Message message;
                std::cout << '\n' << "Enter message for receiver (max length = 20): " << '\n' << '\n';

                std::string buf;
                std::cin.ignore();
                std::getline(std::cin, buf);

                currentMessageCount = *pMessageCount;

                while (currentMessageCount == maxMessageCount)
                    currentMessageCount = *pMessageCount;

                WaitForSingleObject(hMutex, INFINITE);

                std::cout << '\n';

                for (int i = 0; i < 20; i++)
                {
                    if (i > buf.length())
                        message.content[i] = ' ';
                    else
                        message.content[i] = buf[i];
                }
                message.senderName = senderNumber;

                HANDLE messageFile = CreateFile(
                    lpwstrFileName,
                    GENERIC_WRITE,
                    FILE_SHARE_WRITE | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );

                if (messageFile == INVALID_HANDLE_VALUE)
                {
                    std::cerr << "CreateFile failed with error " << GetLastError() << '\n';
                    return 1;
                }

                LARGE_INTEGER li;
                li.QuadPart = 0;
                SetFilePointerEx(messageFile, li, NULL, FILE_END);

                DWORD bytesWritten;
                if (!WriteFile(messageFile, &message, sizeof(Message), &bytesWritten, NULL)) {
                    std::cerr << "Error writing to file." << '\n';
                }

                (*pMessageCount)++;

                CloseHandle(messageFile);

                SetEvent(receiverEvent);

                ReleaseMutex(hMutex);
            }
        }
    }

    UnmapViewOfFile(pMessageCount);
    CloseHandle(fileMapping);

    return 0;
}