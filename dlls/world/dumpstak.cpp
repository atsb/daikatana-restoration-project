// Win32 stack-dump helper modernized for current dbghelp.h.
// The original source used obsolete IMAGEHLP_SYMBOL/SymGetSymFromAddr APIs
// and defined its own SYMBOL_INFO, which now conflicts with dbghelp.h.

#include <windows.h>
#include <crtdbg.h>
#include <dbghelp.h>
#include <string.h>
#include "dumpstak.h"

#pragma comment(lib, "dbghelp.lib")

#define ERROR0(msg) _RPT0(_CRT_ERROR, msg)
#define ERROR1(msg, arg1) _RPT1(_CRT_ERROR, msg, arg1)

static void SendStackError(class CTraceClipboardData &output, const char *text);

class CTraceClipboardData
{
    HGLOBAL m_hMemory;
    DWORD   m_dwSize;
    DWORD   m_dwUsed;
    DWORD   m_dwTarget;

public:
    void SendOut(LPCSTR pszData);
    CTraceClipboardData(DWORD dwTarget);
    ~CTraceClipboardData();
};

CTraceClipboardData::CTraceClipboardData(DWORD dwTarget)
    : m_hMemory(NULL), m_dwSize(0), m_dwUsed(0), m_dwTarget(dwTarget)
{
}

CTraceClipboardData::~CTraceClipboardData()
{
    if (m_hMemory == NULL)
        return;

    if (!OpenClipboard(NULL))
    {
        GlobalFree(m_hMemory);
        return;
    }

    if (!EmptyClipboard() || SetClipboardData(CF_TEXT, m_hMemory) == NULL)
    {
        GlobalFree(m_hMemory);
    }
    else
    {
        // Ownership transfers to the clipboard on success.
    }

    CloseClipboard();
}

void CTraceClipboardData::SendOut(LPCSTR pszData)
{
    DWORD nLength;

    if (pszData == NULL || pszData[0] == '\0')
        return;

    nLength = (DWORD)lstrlenA(pszData);

    if (m_dwTarget & AFX_STACK_DUMP_TARGET_ODS)
        OutputDebugStringA(pszData);

    if (!(m_dwTarget & AFX_STACK_DUMP_TARGET_CLIPBOARD))
        return;

    if (m_hMemory == NULL)
    {
        m_dwSize = 1024;
        while (m_dwSize < nLength + 1)
            m_dwSize *= 2;

        m_hMemory = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, m_dwSize);
        if (m_hMemory == NULL)
        {
            ERROR0("DumpStack Error: No memory available for clipboard.\n");
            m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
            return;
        }
        m_dwUsed = 0;
    }
    else if (m_dwUsed + nLength + 1 >= m_dwSize)
    {
        DWORD newSize = m_dwSize;
        HGLOBAL hMemory;

        while (m_dwUsed + nLength + 1 >= newSize)
            newSize *= 2;

        if (newSize > (1024UL * 1024UL))
        {
            ERROR0("DumpStack Error: more than one megabyte on clipboard.\n");
            m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
            return;
        }

        hMemory = GlobalReAlloc(m_hMemory, newSize, GMEM_MOVEABLE);
        if (hMemory == NULL)
        {
            ERROR1("DumpStack Error: Couldn't get %d bytes!\n", newSize);
            m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
            return;
        }

        m_hMemory = hMemory;
        m_dwSize = newSize;
    }

    {
        LPSTR pstr = (LPSTR)GlobalLock(m_hMemory);
        if (pstr == NULL)
        {
            ERROR0("DumpStack Error: Couldn't lock memory!\n");
            m_dwTarget &= ~AFX_STACK_DUMP_TARGET_CLIPBOARD;
            return;
        }

        CopyMemory(pstr + m_dwUsed, pszData, nLength);
        m_dwUsed += nLength;
        pstr[m_dwUsed] = '\0';
        GlobalUnlock(m_hMemory);
    }
}

static const char *ModuleNameFromPath(const char *path)
{
    const char *slash;

    if (path == NULL)
        return "<no module>";

    slash = strrchr(path, '\\');
    if (slash != NULL)
        return slash + 1;

    slash = strrchr(path, '/');
    if (slash != NULL)
        return slash + 1;

    return path;
}

static BOOL ResolveSymbol(HANDLE hProcess, DWORD64 address, char *moduleName,
                          DWORD moduleNameSize, char *symbolName,
                          DWORD symbolNameSize)
{
    SYMBOL_INFO *symbol;
    IMAGEHLP_MODULE64 moduleInfo;
    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
    DWORD64 displacement = 0;
    BOOL found = FALSE;

    moduleName[0] = '\0';
    symbolName[0] = '\0';

    memset(&moduleInfo, 0, sizeof(moduleInfo));
    moduleInfo.SizeOfStruct = sizeof(moduleInfo);

    if (SymGetModuleInfo64(hProcess, address, &moduleInfo))
    {
        const char *name = ModuleNameFromPath(moduleInfo.ImageName);
        lstrcpynA(moduleName, name, (int)moduleNameSize);
    }
    else
    {
        lstrcpynA(moduleName, "<no module>", (int)moduleNameSize);
    }

    memset(buffer, 0, sizeof(buffer));
    symbol = (SYMBOL_INFO *)buffer;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;

    if (SymFromAddr(hProcess, address, &displacement, symbol))
    {
        char undecorated[MAX_SYM_NAME];
        const char *name = symbol->Name;

        if (UnDecorateSymbolName(symbol->Name, undecorated,
                                 sizeof(undecorated),
                                 UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS))
        {
            name = undecorated;
        }

        if (displacement != 0)
        {
            wsprintfA(symbolName, "%s + %lu bytes", name,
                      (unsigned long)displacement);
        }
        else
        {
            lstrcpynA(symbolName, name, (int)symbolNameSize);
        }

        found = TRUE;
    }
    else
    {
        lstrcpynA(symbolName, "<no symbol>", (int)symbolNameSize);
    }

    return found;
}

void DumpStack(DWORD dwTarget /* = AFX_STACK_DUMP_TARGET_DEFAULT */)
{
    CTraceClipboardData output(dwTarget);
    HANDLE hProcess = GetCurrentProcess();
    PVOID frames[256];
    USHORT frameCount;
    USHORT i;

    output.SendOut("=== begin DumpStack output ===\r\n");

    SymSetOptions(SymGetOptions() | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);

    if (!SymInitialize(hProcess, NULL, TRUE))
    {
        char text[128];
        wsprintfA(text, "DumpStack Error: SymInitialize failed (0x%8.8X)\r\n",
                  GetLastError());
        output.SendOut(text);
        output.SendOut("=== end DumpStack() output ===\r\n");
        return;
    }

    frameCount = CaptureStackBackTrace(0, 256, frames, NULL);

    for (i = 0; i < frameCount; ++i)
    {
        DWORD64 address = (DWORD64)(ULONG_PTR)frames[i];
        char line[64];
        char moduleName[MAX_PATH];
        char symbolName[MAX_SYM_NAME];

        wsprintfA(line, "%016I64X: ", (unsigned __int64)address);
        output.SendOut(line);

        if (ResolveSymbol(hProcess, address, moduleName, sizeof(moduleName),
                          symbolName, sizeof(symbolName)))
        {
            char qualified[MAX_PATH + MAX_SYM_NAME + 8];
            wsprintfA(qualified, "%s! %s", moduleName, symbolName);
            output.SendOut(qualified);
        }
        else
        {
            output.SendOut("symbol not found");
        }

        output.SendOut("\r\n");
    }

    SymCleanup(hProcess);
    output.SendOut("=== end DumpStack() output ===\r\n");
}
