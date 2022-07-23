#include <bits/stdc++.h>
#include <windows.h>
using namespace std;

bool IsRunAsAdministrator()
{
    BOOL fIsRunAsAdmin = 0;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&NtAuthority, 2,
                                  SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0, &pAdministratorsGroup))
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
}

void ElevateNow()
{
    char szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
    SHELLEXECUTEINFO sei = {sizeof(sei)};
    sei.lpVerb = "runas";
    sei.lpFile = szPath;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;
    ShellExecuteEx(&sei);
}

string GetOutput(string Process)
{
    Process = "cmd.exe /c " + Process;
    char process[4096];
    strcpy(process, Process.c_str());
    HANDLE hpiperead = NULL, hpipewrite = NULL;
    SECURITY_ATTRIBUTES ai;
    ai.nLength = sizeof(SECURITY_ATTRIBUTES);
    ai.bInheritHandle = true;
    ai.lpSecurityDescriptor = NULL;
    CreatePipe(&hpiperead, &hpipewrite, &ai, 0);
    STARTUPINFO si;
    GetStartupInfo(&si);
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = hpipewrite;
    si.hStdOutput = hpipewrite;
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    PROCESS_INFORMATION pi;
    CreateProcess(NULL, process, NULL, NULL, true, NULL, NULL, NULL, &si, &pi);
    CloseHandle(hpipewrite);
    char outbuff[4096] = {0};
    DWORD byteread;
    ReadFile(hpiperead, outbuff, 4095, &byteread, NULL);
    CloseHandle(hpiperead);
    return outbuff;
}

int main()
{
    if (IsRunAsAdministrator())
    {
        string path;
        getline(istringstream(GetOutput("ECHO %APPDATA%\\TyporaTimeMachine\\path.txt")), path, '\r');
        string typora;
        if (ifstream(path).good())
            getline(ifstream(path), typora, '\r');
        else
            typora = "\"%ProgramFiles%\\Typora\\Typora.exe\"";
        string now;
        istringstream(GetOutput("DATE /T")) >> now;
        bool change = 0;
        system("DATE 2021/11/22");
        while (system(("START \"\" " + typora).c_str()))
        {
            system(("DATE " + now).c_str());
            cout << "Please input a correct path: ";
            getline(cin, typora);
            change = 1;
            system("DATE 2021/11/22");
        }
        Sleep(1000);
        system(("DATE " + now).c_str());
        if (change)
        {
            system("MD %APPDATA%\\TyporaTimeMachine");
            ofstream(path) << typora;
        }
    }
    else
        ElevateNow();
    return 0;
}