#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>

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

bool IsGameFocused() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return false;

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProc) {
        wchar_t buf[512];
        DWORD size = sizeof(buf) / sizeof(wchar_t);
        if (QueryFullProcessImageNameW(hProc, 0, buf, &size)) {
            std::wstring path(buf);
            size_t lastSlash = path.find_last_of(L"\\/");
            std::wstring exeName = (lastSlash == std::wstring::npos) ? path : path.substr(lastSlash + 1);

            for (auto& c : exeName) c = towlower(c);

            CloseHandle(hProc);
            return exeName == L"csgo.exe";
        }
        CloseHandle(hProc);
    }
    return false;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "-----------------------------------\n";
    std::cout << "Script by: ITTM20230 (D3ADTE@M)\n";
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
        std::cout << "Error: Cannot open process." << std::endl;
        return 1;
    }

    std::cout << "Bhop is active! Hold SPACE" << std::endl;

    int forceJumpTrue = 5;
    int forceJumpFalse = 4;

    while (true) {
        if ((GetAsyncKeyState(vk_space) & 0x8000) && IsGameFocused()) {

            uintptr_t localPlayer = 0;
            ReadProcessMemory(hProcess, (LPCVOID)(clientModule + dwLocalPlayer), &localPlayer, sizeof(localPlayer), nullptr);

            if (localPlayer != 0) {
                int onGround = 0;
                ReadProcessMemory(hProcess, (LPCVOID)(localPlayer + m_fFlags), &onGround, sizeof(onGround), nullptr);

                if (onGround == 257 || onGround == 263) {
                    WriteProcessMemory(hProcess, (LPVOID)(clientModule + dwForceJump), &forceJumpTrue, sizeof(forceJumpTrue), nullptr);
                }
                else {
                    WriteProcessMemory(hProcess, (LPVOID)(clientModule + dwForceJump), &forceJumpFalse, sizeof(forceJumpFalse), nullptr);
                }
            }
        }

        Sleep(1);
    }

    CloseHandle(hProcess);
    return 0;
}