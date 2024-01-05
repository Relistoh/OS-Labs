#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <vector>
#include "Employee.h"
#include <conio.h>

using namespace std;

int main()
{
	string binary_name;
	string txt_name;
	string amount_of_notes;
	ifstream input_name;
	ifstream output_name;
	string pay_for_hour;
	vector<Employee> employees;
	Employee emp;

	cout << "Name of binary file: ";
	cin >> binary_name;
	cout << "Amount of notes: ";
	cin >> amount_of_notes;

	string creator_exe = "Creator.exe " + binary_name + " " + amount_of_notes;
	wstring creator_to_lpwstr = wstring(creator_exe.begin(), creator_exe.end());
	LPWSTR lpsz_creator_process = &creator_to_lpwstr[0];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if (!CreateProcess(NULL, lpsz_creator_process, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		std::cout << "The Creator process is not running.\n";
		return GetLastError();
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	input_name.open(binary_name, ios::binary);
	if (input_name.is_open()) {
		while (!input_name.eof()) {
			input_name.read((char*)&emp, sizeof(Employee));
			employees.push_back(emp);
		}
		employees.pop_back();
	}

	std::cout << "INFO:\nNUMBER\tNAME\tHOURS\n";
	for (int i = 0; i < employees.size(); i++)
	{
		cout << employees[i].num << "\t" << employees[i].name << "\t" << employees[i].hours << "\n";
	}

	cout << "Name of the report file ";
	cin >> txt_name;
	cout << "Pay per our ";
	cin >> pay_for_hour;

	string reporter_exe = "Reporter.exe " + binary_name + " " + txt_name + " " + pay_for_hour;
	wstring reporter_to_lpwstr = std::wstring(reporter_exe.begin(), reporter_exe.end());
	LPWSTR lpsz_reporter_process = &reporter_to_lpwstr[0];

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if (!CreateProcess(NULL, lpsz_reporter_process, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		cout << "The Reporter process is not running.\n";
		return GetLastError();
	}
	WaitForSingleObject(pi.hProcess, INFINITE);


	output_name.open(txt_name);
	if (output_name.is_open()) {
		while (!output_name.eof()) {
			string temporary;
			getline(output_name, temporary);
			cout << temporary << "\n";
		}
	}
	output_name.close();

	_getch();
	TerminateProcess(pi.hProcess, 1);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	cout << "Main Process has been terminated.\n";
	_getch();
	return 0;
}