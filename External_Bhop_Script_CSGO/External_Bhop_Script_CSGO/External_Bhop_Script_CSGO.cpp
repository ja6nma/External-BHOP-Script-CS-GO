#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>

#pragma comment(lib, "winmm.lib")

const DWORD dwForceJump = 0x52C0F50;
const DWORD dwLocalPlayer = 0xDEF97C;
const DWORD m_fFlags = 0x104;
const int vk_space = 0x20;

DWORD GetProcessId(const std::wstring& processName) {
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(processEntry);
        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (processName == processEntry.szExeFile) {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
    return processId;
}

uintptr_t GetModuleBaseAddress(DWORD processId, const std::wstring& moduleName) {
    uintptr_t moduleBaseAddress = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W moduleEntry;
        moduleEntry.dwSize = sizeof(moduleEntry);
        if (Module32FirstW(snapshot, &moduleEntry)) {
            do {
                if (moduleName == moduleEntry.szModule) {
                    moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
                    break;
                }
            } while (Module32NextW(snapshot, &moduleEntry));
        }
        CloseHandle(snapshot);
    }
    return moduleBaseAddress;
}

int main() {
    timeBeginPeriod(1);

    std::cout << "-----------------------------------\n";
    std::cout << "External Bhop Script by: ITTM20230 (D3ADTE@M)\n";
    std::cout << "-----------------------------------\n";

    DWORD processId = 0;
    while (processId == 0) {
        processId = GetProcessId(L"csgo.exe");
        Sleep(100);
    }

    uintptr_t clientModule = 0;
    while (clientModule == 0) {
        clientModule = GetModuleBaseAddress(processId, L"client.dll");
        Sleep(100);
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        timeEndPeriod(1);
        return 1;
    }

    std::cout << "Bhop is active! Hold SPACE" << std::endl;

    const int jumpPlus = 5;
    const int jumpMinus = 4;

    while (true) {
        if (GetAsyncKeyState(vk_space) & 0x8000) {
            uintptr_t localPlayer = 0;
            ReadProcessMemory(hProcess, (LPCVOID)(clientModule + dwLocalPlayer), &localPlayer, sizeof(localPlayer), nullptr);

            if (localPlayer != 0) {
                int flags = 0;
                ReadProcessMemory(hProcess, (LPCVOID)(localPlayer + m_fFlags), &flags, sizeof(flags), nullptr);

                if (flags & 1) {
                    WriteProcessMemory(hProcess, (LPVOID)(clientModule + dwForceJump), &jumpPlus, sizeof(jumpPlus), nullptr);
                }
                else {
                    WriteProcessMemory(hProcess, (LPVOID)(clientModule + dwForceJump), &jumpMinus, sizeof(jumpMinus), nullptr);
                }
            }
        }
        Sleep(1);
    }

    CloseHandle(hProcess);
    timeEndPeriod(1);
    return 0;
}