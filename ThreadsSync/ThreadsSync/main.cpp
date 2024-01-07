#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Windows.h>
#include "MainProcess.h"

MainProcess* mainProcess;

static DWORD WINAPI marker_thread(LPVOID arg) {
    WaitForSingleObject(mainProcess->start_events[(int)arg - 1], INFINITE);
    int marker_id = (int)arg;
    srand(time(NULL) + marker_id);
    int marked_elements = 0;

    while (true) {

        EnterCriticalSection(&mainProcess->cs);

        int index = rand() % mainProcess->arr.size();

        if (mainProcess->arr[index] == 0) {
            marked_elements++;
            Sleep(5);
            mainProcess->arr[index] = marker_id;
            Sleep(5);
            LeaveCriticalSection(&mainProcess->cs);
        }
        else {
            std::cout << "Marker " << marker_id - 1 << ":" << '\n' << "Amount of marked elements: " << marked_elements << '\n' << "Failed to mark index: " << index << '\n' << '\n';
            marked_elements = 0;
            ResetEvent(mainProcess->start_events[marker_id - 1]);
            SetEvent(mainProcess->terminate_events[marker_id - 1]);
            LeaveCriticalSection(&mainProcess->cs);
            WaitForSingleObject(mainProcess->start_events[marker_id - 1], INFINITE);
            EnterCriticalSection(&mainProcess->cs);
            if (!mainProcess->terminated_threads[marker_id - 1])
            {
                for (int i = 0; i < mainProcess->arr.size(); i++)
                    if (mainProcess->arr[i] == marker_id)
                        mainProcess->arr[i] = 0;
                SetEvent(mainProcess->terminate_events[marker_id - 1]);
                LeaveCriticalSection(&mainProcess->cs);
                break;
            }
            else
            {
                LeaveCriticalSection(&mainProcess->cs);
                continue;
            }
        }
    }
    return 0;
}

int main() {
    int n;
    std::cout << "Enter size of array: ";
    std::cin >> n;

    int num_threads;
    std::cout << "Enter number of markers: ";
    std::cin >> num_threads;

    mainProcess = new MainProcess(n, num_threads);

    mainProcess->createEvents();
    mainProcess->createMarkers(marker_thread);
    mainProcess->startEventsSet();
    mainProcess->mainCycle();
}
