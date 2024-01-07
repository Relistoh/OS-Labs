#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Windows.h>

class MainProcess {
public:
    std::vector<int> arr;
	std::vector<HANDLE> marker_threads;
	CRITICAL_SECTION cs;
	std::vector<HANDLE> start_events;
	HANDLE* terminate_events;
	std::vector<bool> terminated_threads;
	int n;
	int num_threads;
	int num_stopped = 0;

	MainProcess(int n, int num_threads)
	{
		this->n = n;
		this->num_threads = num_threads;

		arr.resize(n, 0);

		terminate_events = new HANDLE[num_threads];

		terminated_threads.resize(num_threads, true);

		InitializeCriticalSection(&cs);
	}

	~MainProcess()
	{
		DeleteCriticalSection(&cs);

		for (int i = 0; i < num_threads; i++)
		{
			if (i < marker_threads.size()) CloseHandle(marker_threads[i]);
			if (i < start_events.size()) CloseHandle(start_events[i]);

			if (terminate_events && i < num_threads && terminate_events[i] != nullptr) {
				CloseHandle(terminate_events[i]);
			}
		}

		delete[] terminate_events;
	}

	void createEvents()
	{
		for (int i = 0; i < num_threads; i++) {
			HANDLE event = CreateEvent(NULL, TRUE, FALSE, NULL);
			start_events.push_back(event);
			terminate_events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		}
	}

	void createMarkers(LPTHREAD_START_ROUTINE marker_thread)
	{
		const SIZE_T STACK_SIZE = 1 * 1024 * 1024;

		for (int i = 0; i < num_threads; i++) {
			HANDLE marker = CreateThread(NULL, STACK_SIZE, marker_thread, (LPVOID)(i + 1), CREATE_SUSPENDED, NULL);

			if (marker != NULL) {
				ResumeThread(marker);
				marker_threads.push_back(marker);
			}
			else {
				std::cerr << "Failed to create thread " << i + 1 << std::endl;
			}
		}
	}

	void startEventsSet()
	{
		for (int i = 0; i < num_threads; i++)
			SetEvent(start_events[i]);
	}

	void arrCout()
	{
		for (int i = 0; i < n; i++) {
			std::cout << arr[i] << " ";
		}
		std::cout << '\n' << '\n';
	} 

	void mainCycle()
	{
		while (true) {
			WaitForMultipleObjects(num_threads, terminate_events, TRUE, INFINITE);

			EnterCriticalSection(&cs);

			for (int i = 0; i < num_threads; i++)
				if (terminated_threads[i])
					ResetEvent(terminate_events[i]);

			std::cout << "Array:" << '\n';
			arrCout();

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
			arrCout();

			LeaveCriticalSection(&cs);

			startEventsSet();

			if (num_stopped == num_threads)
				break;
		}
	}

	void mainCycle_test()
	{
		for (int i = 0; i < num_threads; i++) {
			WaitForMultipleObjects(num_threads, terminate_events, TRUE, INFINITE);

			EnterCriticalSection(&cs);

			for (int i = 0; i < num_threads; i++)
				if (terminated_threads[i])
					ResetEvent(terminate_events[i]);

			int marker_to_terminate = i;

			if (terminated_threads[marker_to_terminate]) {
				num_stopped++;
				terminated_threads[marker_to_terminate] = false;
				LeaveCriticalSection(&cs);
				SetEvent(start_events[marker_to_terminate]);
				WaitForSingleObject(marker_threads[marker_to_terminate], INFINITE);
				EnterCriticalSection(&cs);
			}

			LeaveCriticalSection(&cs);

			startEventsSet();

			if (num_stopped == num_threads)
				break;
		}
	}
};