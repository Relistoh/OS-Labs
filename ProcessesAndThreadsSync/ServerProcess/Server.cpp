#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <algorithm>
#include "Employee.h"
#include "string"

HANDLE* hStarted;

HANDLE* hPipe;
HANDLE* hThreads;
HANDLE* hSemaphore;
std::vector<Employee> emps;
int number_of_employees;
int number_of_clients;
std::string file_name;

DWORD WINAPI operations(LPVOID pipe) {
	HANDLE hPipe = (HANDLE)pipe;
	DWORD dwBytesRead;
	DWORD dwBytesWrite;

	int message;

	while (true) {
		if (!ReadFile(
			hPipe,
			&message,
			sizeof(message),
			&dwBytesRead,
			NULL))
		{
			break;
		}
		else {
			int ID = message / 10;
			int chosen_option = message % 10;
			int n;

			for (int i = 0; i < number_of_employees; i++)
				if (emps[i].num == ID)
				{
					n = i;
					break;
				}
			if (chosen_option == 1) {
				for (int i = 0; i < number_of_clients; i++)
				{
					WaitForSingleObject(hSemaphore[n], INFINITE);
				}
				Employee* emp_to_push = new Employee();
				emp_to_push->num = emps[n].num;
				emp_to_push->hours = emps[n].hours;
				strcpy_s(emp_to_push->name, emps[n].name);
				bool checker = WriteFile(hPipe, emp_to_push, sizeof(Employee), &dwBytesWrite, NULL);
				if (checker) {
					std::cout << "Client info to edit was sent.\n";
				}
				else {
					std::cout << "Client info to edit wasn't sent\n";
				}
				ReadFile(hPipe, emp_to_push, sizeof(Employee), &dwBytesWrite, NULL);
				emps[n].hours = emp_to_push->hours;
				strcpy_s(emps[n].name, emp_to_push->name);
				std::ofstream file;
				file.open(file_name, std::ios::binary);
				for (int i = 0; i < number_of_employees; i++)
					file.write((char*)&emps[i], sizeof(Employee));
				file.close();
				int msg;
				ReadFile(hPipe, &msg, sizeof(msg), &dwBytesWrite, NULL);
				if (msg == 1) {
					for (int i = 0; i < number_of_clients; i++)
					{
						ReleaseSemaphore(hSemaphore[n], 1, NULL);
					}
				}
			}
			if (chosen_option == 2) {

				WaitForSingleObject(hSemaphore[n], INFINITE);
				Employee* emp_to_push = new Employee();
				emp_to_push->num = emps[n].num;
				emp_to_push->hours = emps[n].hours;
				strcpy_s(emp_to_push->name, emps[n].name);
				bool checker = WriteFile(hPipe, emp_to_push, sizeof(Employee), &dwBytesWrite, NULL);
				if (checker) {
					std::cout << "Data to read was sent.\n";
				}
				else {
					std::cout << "Data to read wasn't sent.\n";
				}
				int msg;
				ReadFile(hPipe, &msg, sizeof(msg), &dwBytesWrite, NULL);
				if (msg == 1)
					ReleaseSemaphore(hSemaphore[n], 1, NULL);
			}
		}
	}
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	return 0;
}

int main() {
	std::cout << "Input name of file: ";
	std::cin >> file_name;
	std::cout << '\n';

	std::cout << "Input number of employees: ";
	std::cin >> number_of_employees;
	std::cout << '\n';

	std::ofstream file(file_name, std::ios::binary);

	for (int i = 0; i < number_of_employees; i++)
	{
		Employee tmp;

		std::cout << "Input " << i + 1 << " employee (ID Name Hours): ";
		std::cin >> tmp.num;

		std::string buf;
		std::cin >> buf;
		
		for (int i = 0; i < 20; i++)
		{
			if (i > buf.length())
				tmp.name[i] = ' ';
			else
				tmp.name[i] = buf[i];
		}

		std::cin >> tmp.hours;

		emps.push_back(tmp);

		file.write((char*)&tmp, sizeof(Employee));
	}
	
	file.close();

	std::ifstream in;
	in.open(file_name, std::ios::binary);

	for (int i = 0; i < number_of_employees; i++)
	{
		Employee tmp;
		in.read((char*)&tmp, sizeof(Employee));
		std::cout << "\nID of employee: " << tmp.num << ".\nName of employee: " << tmp.name << ".\nHours of employee: " << tmp.hours << ".\n";
	}
	
	in.close();

	std::cout << "Input number of clients:\n";
	std::cin >> number_of_clients;

	hStarted = new HANDLE[number_of_clients];
	hSemaphore = new HANDLE[number_of_employees];
	hPipe = new HANDLE[number_of_clients];

	for (int i = 0; i < number_of_employees; i++)
	{
		hSemaphore[i] = CreateSemaphore(NULL, number_of_clients, number_of_clients, NULL);
	}

	for (int i = 0; i < number_of_clients; i++)
	{
		std::string pipe_name = "\\\\.\\pipe\\" + std::to_string(i);
		std::wstring wstr_pipe_name = std::wstring(pipe_name.begin(), pipe_name.end());
		LPWSTR lpwstr_pipe_name = &wstr_pipe_name[0];
		hPipe[i] = CreateNamedPipe(lpwstr_pipe_name, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0, 0, INFINITE, NULL);

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);

		std::string strProcessName = "ClientProcess.exe " + std::to_string(i);
		std::wstring wstrProcessName = std::wstring(strProcessName.begin(), strProcessName.end());
		LPWSTR lpwstrProcessName = &wstrProcessName[0];

		CreateProcess(
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
		);

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	hThreads = new HANDLE[number_of_clients];

	for (int i = 0; i < number_of_clients; i++)
	{
		SetEvent(hStarted[i]);
		hThreads[i] = CreateThread(NULL, 0, operations, static_cast<LPVOID>(hPipe[i]), 0, NULL);
	}

	WaitForMultipleObjects(number_of_clients, hThreads, TRUE, INFINITE);

	std::cout << "All clients has ended their work.";
	
	in.open(file_name, std::ios::binary);

	for (int i = 0; i < number_of_employees; i++)
	{
		Employee tmp;
		in.read((char*)&tmp, sizeof(Employee));
		std::cout << "\nID of employee: " << tmp.num << ".\nName of employee: " << tmp.name << ".\nHours of employee: " << tmp.hours << ".\n";
	}

	in.close();

	std::cout << "Press any key to exit...\n";
	_getch();
	return 0;
}