#include <windows.h>
#include <conio.h>
#include <iostream>
#include <string>
#include "C:\Users\anton\source\repos\ProcessesAndThreadsSync\ServerProcess\Employee.h"

int main(int argc, char* argv[]) {
    std::string client_number = argv[1];

    std::string pipe_name = "\\\\.\\pipe\\" + client_number;
    std::wstring wstr_pipe_name = std::wstring(pipe_name.begin(), pipe_name.end());
    LPWSTR lpwstr_pipe_name = &wstr_pipe_name[0];

    HANDLE hPipe = CreateFile(lpwstr_pipe_name, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        std::cout << "Creation of the named pipe failed.\n The last error code: " << GetLastError() << "\n";
        std::cout << "Press any char to finish server: ";
        _getch();
        return (0);
    }

    while (true) {
        std::string client_command;
        std::cout << "Choose option:\n 1. Modify client info;\n 2. Read client info;\n 3. Exit.\n";
        std::cin >> client_command;

        if (client_command == "3") {
            break;
        }

        else if (client_command == "1") {
            DWORD dwBytesWritten;
            DWORD dwBytesReaden;
            int ID;

            std::cout << "Input an ID of employee:\n";
            std::cin >> ID;
            int message_to_send = ID * 10 + 1;
            bool checker = WriteFile(
                hPipe,
                &message_to_send,
                sizeof(message_to_send),
                &dwBytesWritten,
                NULL);
            if (checker) {
                std::cout << "Message was sent.\n";
            }
            else {
                std::cout << "Message wasn't sent.\n";
            }

            Employee* tmp = new Employee();

            ReadFile(
                hPipe,
                tmp,
                sizeof(Employee),
                &dwBytesReaden,
                NULL);

            std::cout << "ID of employee: " << tmp->num << ".\nName of employee: " << tmp->name << ".\nHours of employee: " << tmp->hours << ".\n";
            std::cout << "Input new Name:\n";
            std::cin >> tmp->name;
            std::cout << "Input new Hours:\n";
            std::cin >> tmp->hours;

            checker = WriteFile(
                hPipe,
                tmp,
                sizeof(Employee),
                &dwBytesWritten,
                NULL);

            if (checker) {
                std::cout << "Message was sent.\n";
            }

            else {
                std::cout << "Message wasn't sent.\n";
            }

            std::cout << "Press any key to confirm the option...\n";
            _getch();
            message_to_send = 1;
            WriteFile(hPipe, &message_to_send, sizeof(message_to_send), &dwBytesWritten, NULL);
        }

        else if (client_command == "2") {
            DWORD dwBytesWritten;
            DWORD dwBytesReaden;
            int ID;

            std::cout << "Input an ID of employee:\n";
            std::cin >> ID;

            int message_to_send = ID * 10 + 2;

            bool checker = WriteFile(
                hPipe,
                &message_to_send,
                sizeof(message_to_send),
                &dwBytesWritten,
                NULL);

            if (checker) {
                std::cout << "Message was sent.\n" << "Stand by...\n";
            }
            else {
                std::cout << "Message wasn't sent.\n";
            }
            
            Employee* tmp = new Employee();
            ReadFile(
                hPipe,
                tmp,
                sizeof(Employee),
                &dwBytesReaden,
                NULL);

            std::cout << "ID of employee: " << tmp->num << ".\nName of employee: " << tmp->name << ".\nHours of employee: " << tmp->hours << ".\n";
            std::cout << "Press any key to continue...\n";
            _getch();
            message_to_send = 1;
            WriteFile(hPipe, &message_to_send, sizeof(message_to_send), &dwBytesWritten, NULL);
        }
    }

    return 0;
}