#include <bits/stdc++.h>
#include <windows.h>
using namespace std;

BOOL IsRunAsAdministrator()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
            &NtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pAdministratorsGroup))
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
    {
        throw dwError;
    }

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

int main()
{
    if (IsRunAsAdministrator())
    {
        system("DATE /T > date.txt");
        ifstream get_path("path.txt"), get_date("date.txt");
        string typora, now;
        getline(get_path, typora);
        get_date >> now;
        get_path.close();
        get_date.close();
        system("DEL date.txt");
        system("DATE 2021/11/11");
        system(("START \"\" " + typora).c_str());
        Sleep(1000);
        system(("DATE " + now).c_str());
    }
    else
        ElevateNow();
    return 0;
}