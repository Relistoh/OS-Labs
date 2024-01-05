#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Windows.h>

std::vector<int> arr;
std::vector<HANDLE> marker_threads;
CRITICAL_SECTION cs;
std::vector<HANDLE> start_events;
HANDLE* terminate_events;
std::vector<bool> terminated_threads;

DWORD WINAPI marker_thread(LPVOID arg) {
    WaitForSingleObject(start_events[(int)arg - 1], INFINITE);
    int marker_id = (int)arg;
    srand(time(NULL) + marker_id);
    int marked_elements = 0;

    while (true) {

        EnterCriticalSection(&cs);

        int index = rand() % arr.size();

        if (arr[index] == 0) {
            marked_elements++;
            Sleep(5);
            arr[index] = marker_id;
            Sleep(5);
            LeaveCriticalSection(&cs);
        }
        else {
            std::cout << "Marker " << marker_id - 1 << ":" << '\n' << "Amount of marked elements: " << marked_elements << '\n' << "Failed to mark index: " << index << '\n' << '\n';
            marked_elements = 0;
            ResetEvent(start_events[marker_id - 1]);
            SetEvent(terminate_events[marker_id - 1]);
            LeaveCriticalSection(&cs);
            WaitForSingleObject(start_events[marker_id - 1], INFINITE);
            EnterCriticalSection(&cs);
            if (!terminated_threads[marker_id - 1])
            {
                for (int i = 0; i < arr.size(); i++)
                    if (arr[i] == marker_id)
                        arr[i] = 0;
                SetEvent(terminate_events[marker_id - 1]);
                LeaveCriticalSection(&cs);
                break;
            }
            else
            {
                LeaveCriticalSection(&cs);
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
    arr.resize(n, 0);

    int num_threads;
    std::cout << "Enter number of markers: ";
    std::cin >> num_threads;
    std::cout << '\n';

    int num_stopped = 0;

    DWORD* dwThreads = new DWORD[num_threads];

    terminate_events = new HANDLE[num_threads];

    terminated_threads.resize(num_threads, true);

    InitializeCriticalSection(&cs);

    for (int i = 0; i < num_threads; i++) {
        HANDLE event = CreateEvent(NULL, TRUE, FALSE, NULL);
        start_events.push_back(event);
        terminate_events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        HANDLE marker = CreateThread(NULL, 0, marker_thread, (LPVOID)(i + 1), NULL, &dwThreads[i - 1]);
        marker_threads.push_back(marker);
    }

    for (int i = 0; i < num_threads; i++)
        SetEvent(start_events[i]);

    while (true) {
        WaitForMultipleObjects(num_threads, terminate_events, TRUE, INFINITE);

        EnterCriticalSection(&cs);

        for (int i = 0; i < num_threads; i++)
            if (terminated_threads[i])
                ResetEvent(terminate_events[i]);

        std::cout << "Array:" << '\n';
        for (int i = 0; i < n; i++) {
            std::cout << arr[i] << " ";
        }
        std::cout << '\n' << '\n';

        int marker_to_terminate;
        std::cout << "Enter thread index to terminate: ";
        std::cin >> marker_to_terminate;
        std::cout << '\n';

        if (terminated_threads[marker_to_terminate]) {
            num_stopped++;
            terminated_threads[marker_to_terminate] = false;
            LeaveCriticalSection(&cs);
            SetEvent(start_events[marker_to_terminate]);
            WaitForSingleObject(marker_threads[marker_to_terminate], INFINITE);
            EnterCriticalSection(&cs);
        }

        std::cout << "Array after termination:" << '\n';
        for (int i = 0; i < n; i++) {
            std::cout << arr[i] << " ";
        }
        std::cout << '\n' << '\n';

        LeaveCriticalSection(&cs);

        for (int i = 0; i < num_threads; i++)
            SetEvent(start_events[i]);

        if (num_stopped == num_threads)
            break;
    }

    DeleteCriticalSection(&cs);

    for (int i = 0; i < num_threads; i++)
    {
        CloseHandle(marker_threads[i]);
        CloseHandle(start_events[i]);
        CloseHandle(terminate_events[i]);
    }
}
