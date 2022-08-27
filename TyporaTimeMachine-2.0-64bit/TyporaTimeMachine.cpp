#include <bits/stdc++.h>
#include <windows.h>
using namespace std;
int main(int argc, char **argv)
{
    auto self = GetForegroundWindow();
    ShowWindow(self, SW_HIDE);
    auto is_run_as_administrator = []() -> bool
    {
        BOOL fIsRunAsAdmin = FALSE;
        DWORD dwError = ERROR_SUCCESS;
        PSID pAdministratorsGroup = NULL;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup))
        {
            dwError = GetLastError();
            goto Cleanup;
        }
        if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
        {
            dwError = GetLastError();
            goto Cleanup;
        }
    Cleanup:
        if (pAdministratorsGroup)
        {
            FreeSid(pAdministratorsGroup);
            pAdministratorsGroup = NULL;
        }
        if (ERROR_SUCCESS != dwError)
            throw dwError;
        return fIsRunAsAdmin;
    };
    if (is_run_as_administrator())
    {
        auto get_output = [](string process) -> string
        {
            HANDLE hpiperead = NULL, hpipewrite = NULL;
            SECURITY_ATTRIBUTES ai;
            ai.nLength = sizeof(SECURITY_ATTRIBUTES);
            ai.lpSecurityDescriptor = NULL;
            ai.bInheritHandle = true;
            CreatePipe(&hpiperead, &hpipewrite, &ai, 0);
            STARTUPINFO si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(STARTUPINFO);
            si.hStdError = hpipewrite;
            si.hStdOutput = hpipewrite;
            si.dwFlags = STARTF_USESTDHANDLES;
            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(pi));
            char tmp[4096];
            strcpy(tmp, ("cmd.exe /c " + process).c_str());
            CreateProcess(NULL, tmp, NULL, NULL, true, 0, NULL, NULL, &si, &pi);
            CloseHandle(hpipewrite);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            char outbuff[4096] = {0};
            DWORD byteread;
            ReadFile(hpiperead, outbuff, 4095, &byteread, NULL);
            CloseHandle(hpiperead);
            return outbuff;
        };
        string path, typora;
        getline(istringstream(get_output("ECHO %APPDATA%\\TyporaTimeMachine\\path.txt")), path, '\r');
        if (ifstream(path).good())
            getline(ifstream(path), typora);
        else
            getline(istringstream(get_output("ECHO %ProgramFiles%\\Typora\\Typora.exe")), typora, '\r');
        SYSTEMTIME fake, begin;
        fake.wYear = 2021;
        fake.wMonth = 11;
        fake.wDayOfWeek = 2;
        fake.wDay = 23;
        fake.wHour = 0;
        fake.wMinute = 0;
        fake.wSecond = 0;
        fake.wMilliseconds = 0;
        auto zero = clock();
        GetSystemTime(&begin);
        auto set_time = [&]()
        {
            auto now = begin;
            now.wMilliseconds += clock() - zero;
            now.wSecond += now.wMilliseconds / 1000;
            now.wMilliseconds %= 1000;
            now.wMinute += now.wSecond / 60;
            now.wSecond %= 60;
            now.wHour += now.wMinute / 60;
            now.wMinute %= 60;
            now.wDayOfWeek += now.wHour / 24;
            now.wDay += now.wHour / 24;
            now.wHour %= 24;
            now.wDayOfWeek %= 7;
            SetSystemTime(&now);
        };
        auto show_error = [](intptr_t error) -> int
        {
            auto message = [](string s) -> int
            {
                return MessageBox(NULL, (s + "\nDo you want to reset path?").c_str(), NULL, MB_YESNOCANCEL);
            };
            switch (error)
            {
            case 0:
                return message("The operating system is out of memory or resources.");
            case ERROR_FILE_NOT_FOUND:
                return message("The specified file was not found.");
            case ERROR_PATH_NOT_FOUND:
                return message("The specified path was not found.");
            case ERROR_BAD_FORMAT:
                return message("The .exe file is invalid (non-Win32 .exe or error in .exe image).");
            case SE_ERR_ACCESSDENIED:
                return message("The operating system denied access to the specified file.");
            case SE_ERR_ASSOCINCOMPLETE:
                return message("The file name association is incomplete or invalid.");
            case SE_ERR_DDEBUSY:
                return message("The DDE transaction could not be completed because other DDE transactions were being processed.");
            case SE_ERR_DDEFAIL:
                return message("The DDE transaction failed.");
            case SE_ERR_DDETIMEOUT:
                return message("The DDE transaction could not be completed because the request timed out.");
            case SE_ERR_DLLNOTFOUND:
                return message("The specified DLL was not found.");
            case SE_ERR_NOASSOC:
                return message("There is no application associated with the given file name extension. This error will also be returned if you attempt to print a file that is not printable.");
            case SE_ERR_OOM:
                return message("There was not enough memory to complete the operation.");
            case SE_ERR_SHARE:
                return message("A sharing violation occurred.");
            }
        };
        intptr_t error;
        SetSystemTime(&fake);
        while ((error = (intptr_t)ShellExecute(NULL, NULL, typora.c_str(), argc > 1 ? ('\"' + (string)argv[1] + '\"').c_str() : NULL, NULL, SW_NORMAL)) <= 32)
        {
            set_time();
            switch (show_error(error))
            {
            case IDYES:
                ShowWindow(self, SW_SHOW);
                cout << "Old Path: " << typora << "\nNew Path: ";
                getline(cin, typora);
                ShowWindow(self, SW_HIDE);
                system("MD %APPDATA%\\TyporaTimeMachine > nul");
                system("CLS");
                ofstream(path) << typora;
                break;
            case IDCANCEL:
                return 0;
            }
            SetSystemTime(&fake);
        }
        Sleep(1000);
        set_time();
    }
    else
        ShellExecute(NULL, "runas", argv[0], argc > 1 ? ('\"' + (string)argv[1] + '\"').c_str() : NULL, NULL, SW_NORMAL);
    return 0;
}