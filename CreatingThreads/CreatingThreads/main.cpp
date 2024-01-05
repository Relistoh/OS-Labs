#include <windows.h>
#include <iostream>

using namespace std;

int* arr = nullptr;
int size_of_array = 0;
int max_value = 0;
int min_value = 0;
int sum_of_elements = 0;
int arifm = 0;

CRITICAL_SECTION cs;

DWORD WINAPI min_max(LPVOID)
{
	max_value = arr[0];
	min_value = arr[0];

	for (int i = 0; i < size_of_array; i++)
	{
		if (arr[i] <= min_value)
		{
			min_value = arr[i];
		}
		Sleep(7);
		if (arr[i] >= max_value)
		{
			max_value = arr[i];
		}
		Sleep(7);
	}

	EnterCriticalSection(&cs);

	cout << "Minimal element of array: " << min_value << "\n";
	cout << "Maximal element of array: " << max_value << "\n";

	LeaveCriticalSection(&cs);

	return 0;
}


DWORD WINAPI average(LPVOID)
{
	for (int i = 0; i < size_of_array; i++)
	{
		sum_of_elements += arr[i];
		Sleep(12);
	}

	arifm = sum_of_elements / size_of_array;

	EnterCriticalSection(&cs);

	cout << "Arithmetical mean: " << arifm << "\n";

	LeaveCriticalSection(&cs);

	return 0;
}


int main()
{
	HANDLE min_max_hThread;
	DWORD  min_max_IDThread;

	HANDLE average_hThread;
	DWORD average_IDThread;

	InitializeCriticalSection(&cs);

	cout << "Eneter size of array ";
	cin >> size_of_array;

	arr = new int[size_of_array];

	cout << "Enter " << size_of_array << " elements of array \n";
	for (int i = 0; i < size_of_array; i++)
	{
		cin >> arr[i];
	}

	min_max_hThread = CreateThread(NULL, 0, min_max, NULL, 0, &min_max_IDThread);
	average_hThread = CreateThread(NULL, 0, average, NULL, 0, &average_IDThread);

	if (min_max_hThread == NULL || average_hThread == NULL)
	{
		return GetLastError();
	}

	WaitForSingleObject(average_hThread, INFINITE);
	WaitForSingleObject(min_max_hThread, INFINITE);


	for (int i = 0; i < size_of_array; i++)
	{
		if (arr[i] == max_value || arr[i] == min_value)
		{
			arr[i] = arifm;
		}
	}

	for (int i = 0; i < size_of_array; i++)
	{
		cout << arr[i] << " ";
	}

	DeleteCriticalSection(&cs);

	CloseHandle(min_max_hThread);
	CloseHandle(average_hThread);

	delete[] arr;

	return 0;
}