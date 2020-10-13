/* This is a C89 memory library for Windows.

Features:
 - No libc
 - Syscall support for x86-32(wow64) and x86-64.

Setup:
Define the 'NMD_MEMORY_IMPLEMENTATION' macro in one source file before the include statement to instantiate the implementation.
#define NMD_MEMORY_IMPLEMENTATION
#include "nmd_memory.h"

Using syscalls:
There're two ways to use syscalls. The first is to call a helper function for the syscall(only for popular syscalls) like nmd_open_process().
The second is to use the generic variadic syscall function nmd_syscall() which takes the syscall id as the first parameter and the arguments
used by the syscall for the remaining parameters.

nmd_get_module_handle(): Similar to GetModuleHandleW()
nmd_get_proc_addr(): Similar to GetProcAddress()

*/

#ifndef NMD_MEMORY_H
#define NMD_MEMORY_H

#ifdef NMD_MEMORY_NO_INCLUDES

#ifndef __cplusplus

#define bool  _Bool
#define false 0
#define true  1

#endif /* __cplusplus */

typedef signed char        int8_t;
typedef unsigned char      uint8_t;

typedef signed short       int16_t;
typedef unsigned short     uint16_t;

typedef signed int         int32_t;
typedef unsigned int       uint32_t;

typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

#if defined(_WIN64) && defined(_MSC_VER)
	typedef unsigned __int64 size_t;
	typedef __int64          ptrdiff_t;
#elif (defined(_WIN32) || defined(WIN32)) && defined(_MSC_VER)
	typedef unsigned __int32 size_t
	typedef __int32          ptrdiff_t;
#elif defined(__GNUC__) || defined(__clang__)
	#if defined(__x86_64__) || defined(__ppc64__)
		typedef unsigned long size_t
		typedef long          ptrdiff_t
	#else
		typedef unsigned int size_t
		typedef int          ptrdiff_t
	#endif
#else
	typedef unsigned long size_t
	typedef long          ptrdiff_t
#endif

#else

/* Dependencies when 'NMD_MEMORY_NO_INCLUDES' is not defined. */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#endif /* NMD_MEMORY_NO_INCLUDES */

#ifdef __clang__
    #define _NMD_NAKED __attribute__((naked))
#else
    #define _NMD_NAKED __declspec(naked)
#endif

#include <Windows.h>

typedef struct nmd_mex
{
	HANDLE h_process;
} nmd_mex;

enum NMD_INJECTION_METHOD
{
	NMD_INJECTION_METHOD_LOAD_LIBRARY, /* LoadLibraryW at kernel32.dll */
	NMD_INJECTION_METHOD_LDR_LOAD_DLL, /* LdrLoadDll at ntdll.dll */
	NMD_INJECTION_METHOD_MANUAL_MAPPING,
};

/* Taken from x64dbg */

typedef struct _NMD_PEB_LDR_DATA
{
    ULONG Length;
    BOOLEAN Initialized;
    HANDLE SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
    BOOLEAN ShutdownInProgress;
    HANDLE ShutdownThreadId;
} NMD_PEB_LDR_DATA, *NMD_PPEB_LDR_DATA;

#ifndef _WIN64
#define GDI_HANDLE_BUFFER_SIZE 34
#else
#define GDI_HANDLE_BUFFER_SIZE 60
#endif
typedef ULONG GDI_HANDLE_BUFFER[GDI_HANDLE_BUFFER_SIZE];

typedef struct _NMD_UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} NMD_UNICODE_STRING, *NMD_PUNICODE_STRING;

typedef struct _NMD_LDR_MODULE
{
    LIST_ENTRY              InLoadOrderModuleList;
    LIST_ENTRY              InMemoryOrderModuleList;
    LIST_ENTRY              InInitializationOrderModuleList;
    PVOID                   BaseAddress;
    PVOID                   EntryPoint;
    ULONG                   SizeOfImage;
    NMD_UNICODE_STRING      FullDllName;
    NMD_UNICODE_STRING      BaseDllName;
    ULONG                   Flags;
    SHORT                   LoadCount;
    SHORT                   TlsIndex;
    LIST_ENTRY              HashTableEntry;
    ULONG                   TimeDateStamp;
} NMD_LDR_MODULE, * NMD_PLDR_MODULE;

typedef struct _NMD_PEB
{
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union
    {
        BOOLEAN BitField;
        struct
        {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN IsProtectedProcess : 1;
            BOOLEAN IsImageDynamicallyRelocated : 1;
            BOOLEAN SkipPatchingUser32Forwarders : 1;
            BOOLEAN IsPackagedProcess : 1;
            BOOLEAN IsAppContainer : 1;
            BOOLEAN IsProtectedProcessLight : 1;
            BOOLEAN IsLongPathAwareProcess : 1;
        } s1;
    } u1;

    HANDLE Mutant;

    PVOID ImageBaseAddress;
    NMD_PPEB_LDR_DATA Ldr;
    PVOID ProcessParameters; // PRTL_USER_PROCESS_PARAMETERS
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PRTL_CRITICAL_SECTION FastPebLock;
    PVOID AtlThunkSListPtr;
    PVOID IFEOKey;
    union
    {
        ULONG CrossProcessFlags;
        struct
        {
            ULONG ProcessInJob : 1;
            ULONG ProcessInitializing : 1;
            ULONG ProcessUsingVEH : 1;
            ULONG ProcessUsingVCH : 1;
            ULONG ProcessUsingFTH : 1;
            ULONG ProcessPreviouslyThrottled : 1;
            ULONG ProcessCurrentlyThrottled : 1;
            ULONG ReservedBits0 : 25;
        } s2;
    } u2;
    union
    {
        PVOID KernelCallbackTable;
        PVOID UserSharedInfoPtr;
    } u3;
    ULONG SystemReserved[1];
    ULONG AtlThunkSListPtr32;
    PVOID ApiSetMap;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];

    PVOID ReadOnlySharedMemoryBase;
    PVOID SharedData; // HotpatchInformation
    PVOID* ReadOnlyStaticServerData;

    PVOID AnsiCodePageData; // PCPTABLEINFO
    PVOID OemCodePageData; // PCPTABLEINFO
    PVOID UnicodeCaseTableData; // PNLSTABLEINFO

    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

    LARGE_INTEGER CriticalSectionTimeout;
    SIZE_T HeapSegmentReserve;
    SIZE_T HeapSegmentCommit;
    SIZE_T HeapDeCommitTotalFreeThreshold;
    SIZE_T HeapDeCommitFreeBlockThreshold;

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID* ProcessHeaps; // PHEAP

    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    ULONG GdiDCAttributeList;

    PRTL_CRITICAL_SECTION LoaderLock;

    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    ULONG_PTR ActiveProcessAffinityMask;
    GDI_HANDLE_BUFFER GdiHandleBuffer;
    PVOID PostProcessInitRoutine;

    PVOID TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];

    ULONG SessionId;

    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    PVOID pShimData;
    PVOID AppCompatInfo; // APPCOMPAT_EXE_DATA

    NMD_UNICODE_STRING CSDVersion;

    PVOID ActivationContextData; // ACTIVATION_CONTEXT_DATA
    PVOID ProcessAssemblyStorageMap; // ASSEMBLY_STORAGE_MAP
    PVOID SystemDefaultActivationContextData; // ACTIVATION_CONTEXT_DATA
    PVOID SystemAssemblyStorageMap; // ASSEMBLY_STORAGE_MAP

    SIZE_T MinimumStackCommit;

    PVOID* FlsCallback;
    LIST_ENTRY FlsListHead;
    PVOID FlsBitmap;
    ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
    ULONG FlsHighIndex;

    PVOID WerRegistrationData;
    PVOID WerShipAssertPtr;
    PVOID pUnused; // pContextData
    PVOID pImageHeaderHash;
    union
    {
        ULONG TracingFlags;
        struct
        {
            ULONG HeapTracingEnabled : 1;
            ULONG CritSecTracingEnabled : 1;
            ULONG LibLoaderTracingEnabled : 1;
            ULONG SpareTracingBits : 29;
        } s3;
    } u4;
    ULONGLONG CsrServerReadOnlySharedMemoryBase;
    PVOID TppWorkerpListLock;
    LIST_ENTRY TppWorkerpList;
    PVOID WaitOnAddressHashTable[128];
    PVOID TelemetryCoverageHeader; // REDSTONE3
    ULONG CloudFileFlags;
} NMD_PEB, *NMD_PPEB;

/* Returns the last error code set. */
uint32_t nmd_get_error_code();

/* Sets the last error code.
Parameters:
 - error_code [in] The error code.
 */
void nmd_set_error_code(uint32_t error_code);

/* Performs a system call using the specified id.
Be aware: On WoW64 the syscall may expect structures with 8-byte sizes(such as pointers and SIZE_T).
Also, on WoW64 every parameter after the fourth must be 8-byte long.
Example: nmd_syscall(0x1234, arg1, arg2, arg3, arg4, (uint64_t)arg5, (uint64_t)arg6)
Parameters:
 - id  [in] The syscall id.
 - ... [in] The parameters used by the syscall.
*/
NTSTATUS nmd_syscall(size_t id, ...);

/* Open a handle to the process specified by the id. Returns the handle if successful, zero otherwise.
Parameters:
 - pid         [in] The process identifier(PID).
 - access_mask [in] The desired access.
*/
HANDLE nmd_open_process(uint32_t pid, uint32_t access_mask);

/* Returns a handle to a module(module base) given its name.
Parameters:
 - module_name [in] The name of the module. If null, the image base address is returned.
*/
HMODULE nmd_get_module_handle(const wchar_t* module_name);

/* Returns the procedure address being exported by the specified module, or zero if an error occurred.
Parameters:
 - h_module  [in] The module base.
 - proc_name [in] The name of the procedure exported by 'h_module'.
*/
void* nmd_get_proc_addr(HMODULE h_module, const char* proc_name);

/* Retrieves the specified system information. Reference MSDN's documentation fo more information.
Parameters:
 - info        [in]      A member of 'SYSTEM_INFORMATION_CLASS'. Specifies the kind of information to be retrieved.
 - buffer      [out]     A pointer to a buffer that receives the information.
 - size        [in]      The size of the buffer(in bytes) specified by 'system_information'.
 - return_size [out/opt] An optional pointer to a variable that receives the number of bytes written to the buffer.
*/
//NTSTATUS nmd_query_system_information(SYSTEM_INFORMATION_CLASS info, PVOID buffer, ULONG size, PULONG return_size);

// NTSTATUS nmd_allocate_virtual_memory

/* Returns a pointer to the PEB of the current process. */
NMD_PPEB nmd_get_peb();

/* Scans the specified memory range for a pattern.
Parameters:
 - pattern    [in] The pattern. e.g. "\x10\x20\x30\x40\x50".
 - mask       [in] The mask. e.g. "xxxxx", "x????xxxxx?xx".
 - start      [in] The range's start address.
 - end        [in] The range's end address.
 - protection [in] The memory protection the page must match. Specify '-1' for any protection.
*/
void* nmd_pattern_scan_range(const char* pattern, const char* mask, uint8_t* start, uint8_t* end, uint32_t protection);

/* Iterates through all processes to find the process with a matching name. Returns the PID of the specified process, or zero if the operation failed.
Parameters:
 - process_name [in] The wide-char process name.
*/
uint32_t nmd_mex_get_pid_by_wname(const wchar_t* process_name);

/* "Opens" a process by verifying if the given handle is a valid process handle. Returns true if the operation is successful, false otherwise.
Parameters:
 - mex    [out] A pointer to a variablev of type 'nmd_mex'.
 - handle [in]  The process handle.
*/
bool nmd_mex_open_by_handle(nmd_mex* mex, HANDLE handle);

/* Injects a dll on the specified process. Returns the base address of the injected module or zero if he operation failed. 
Parameters:
 - mex  [in] A pointer to a variable of type 'nmd_mex'.
 - path [in] The path to the dll.

 - */
uintptr_t nmd_mex_inject(nmd_mex* m, const wchar_t* path);

#ifdef NMD_MEMORY_IMPLEMENTATION

uint32_t _nmd_error_code;

/* Returns the last error code set. */
uint32_t nmd_get_error_code()
{
    return _nmd_error_code;
}

/* Sets the last error code.
Parameters:
 - error_code [in] The error code.
*/
void nmd_set_error_code(uint32_t error_code)
{
    _nmd_error_code = error_code;
}

/* Returns a pointer to the PEB of the current process. */
NMD_PPEB nmd_get_peb()
{
#ifdef _WIN64
    return (NMD_PPEB)__readgsqword(0x60);
#else
    return (NMD_PPEB)__readfsdword(0x30); 
#endif
}

size_t _nmd_strlen(const char* str)
{
    size_t len = 0;
    while (*str++)
        len++;

    return len;
}

size_t _nmd_strlenw(const wchar_t* str)
{
    size_t len = 0;
    while (*str++)
        len++;

    return len;
}

/* Returns true if s1 matches s2 exactly. */
bool _nmd_strcmp(const char* s1, const char* s2)
{
    for (; *s1 && *s2; s1++, s2++)
    {
        if (*s1 != *s2)
            return false;
    }

    return !*s1 && !*s2;
}

wchar_t _nmd_tolowerw(const wchar_t c)
{
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

/* Returns true if s1 matches s2 exactly. */
bool _nmd_strcmpiw(const wchar_t* s1, const wchar_t* s2)
{
    for (; *s1 && *s2; s1++, s2++)
    {
        if (_nmd_tolowerw(*s1) != _nmd_tolowerw(*s2))
            return false;
    }

    return !*s1 && !*s2;
}

/* Returns a handle to a module(module base) given its name.
Parameters:
 - module_name [in] The name of the module. If null, the image base address is returned.
*/
HMODULE nmd_get_module_handle(const wchar_t* module_name)
{
    if (!module_name)
        return (HMODULE)nmd_get_peb()->ImageBaseAddress;

    NMD_PLDR_MODULE final_mod = nmd_get_peb()->Ldr->InLoadOrderModuleList.Flink;
    NMD_PLDR_MODULE mod = nmd_get_peb()->Ldr->InLoadOrderModuleList.Blink;
    while (mod != final_mod)
    {
        if(_nmd_strcmpiw(mod->BaseDllName.Buffer, module_name))
            return (HMODULE)mod->BaseAddress;
        mod = mod->InLoadOrderModuleList.Blink;
    }

    return 0;
}

/* Returns the procedure address being exported by the specified module, or zero if an error occurred.
Parameters:
 - h_module  [in] The module base.
 - proc_name [in] The name of the procedure exported by 'h_module'.
*/
void* nmd_get_proc_addr(HMODULE h_module, const char* proc_name)
{
    const uint8_t* base = h_module;
    const PIMAGE_OPTIONAL_HEADER optional_header = base + *(uint32_t*)(base + 0x3c) + 4 + sizeof(IMAGE_FILE_HEADER);
    const uint32_t export_directory_rva = optional_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!export_directory_rva)
        return 0;
    const PIMAGE_EXPORT_DIRECTORY export_directory = base + export_directory_rva;
    const uint32_t* names = base + export_directory->AddressOfNames;

    size_t i = 0;
    for (; i < export_directory->NumberOfNames; i++)
    {
        const char* name = base + names[i];
        if (_nmd_strcmp(name, proc_name))
        {
            const uint16_t ordinal = ((uint16_t*)(base + export_directory->AddressOfNameOrdinals))[i];
            const uint32_t address = ((uint32_t*)(base + export_directory->AddressOfFunctions))[ordinal];
            return base + address;
        }
    }

    return 0;
}

/* Retrieves the specified system information. Reference MSDN's documentation for more information.
Parameters:
 - info        [in]      A member of 'SYSTEM_INFORMATION_CLASS'. Specifies the kind of information to be retrieved.
 - buffer      [out]     A pointer to a buffer that receives the information.
 - size        [in]      The size of the buffer(in bytes) specified by 'system_information'.
 - return_size [out/opt] An optional pointer to a variable that receives the number of bytes written to the buffer.

_NMD_NAKED NTSTATUS nmd_query_system_information(SYSTEM_INFORMATION_CLASS info, PVOID buffer, ULONG size, PULONG return_size)
{
    _asm
    {
#ifdef _WIN64
        mov r10, rcx

        mov rax, 0x36; NtQuerySystemInformation syscall id
        syscall

        ret
#else
        
#endif
    }
}

/* Injects a dll on the specified process. Returns the base address of the injected module or zero if he operation failed.
Parameters:
 - mex              [in] A pointer to a variable of type 'nmd_mex'.
 - dll_path         [in] The path to the dll.
 - injection_method [in] A member of the 
 - */
uintptr_t nmd_mex_inject(nmd_mex* m, const wchar_t* dll_path)
{
#ifdef _WIN64
    uint8_t shellcode_x64_load_library[] = {
        0x68, 0x00, 0x00, 0x00, 0x00,                               // mov r8 lpLibFileName: 'path_buffer'
        0x6a, 0x00,                                                 // mov rdx hFile: NULL
        0x6a, 0x00,                                                 // mov rcx dwFlags: 0
        0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, LoadLibraryExW
        0xff, 0xd0                                                  // call rax
    };

    uint8_t shellcode_x64_ldr_load_dll[] = {
        0x68, 0x00, 0x00, 0x00, 0x00,                               // mov r9 DllHandle: 'path_buffer'
        0x6a, 0x00,                                                 // mov r8 PUNICODE_STRING : NULL
        0x6a, 0x00,                                                 // mov rdx DllCharacteristics: 0
        0x6a, 0x00,                                                 // mov rcx DllPath: 0
        0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, LdrLoadDll
        0xff, 0xd0                                                  // call rax
    };
#else
    uint8_t shellcode_x86_load_library[] = {
        0x68, 0x00, 0x00, 0x00, 0x00, // push lpLibFileName: 'path_buffer'
        0x6a, 0x00,                   // push hFile: NULL
        0x6a, 0x00,                   // push dwFlags: 0
        0xe8, 0x00, 0x00, 0x00, 0x00, // call LoadLibraryExW
    };

    uint8_t shellcode_x86_ldr_load_dll[] = {
        0x68, 0x00, 0x00, 0x00, 0x00, // push DllHandle: 'path_buffer'
        0x6a, 0x00,                   // push PUNICODE_STRING : NULL
        0x6a, 0x00,                   // push DllCharacteristics: 0
        0x6a, 0x00,                   // push DllPath: 0
        0xe8, 0x00, 0x00, 0x00, 0x00, // call LdrLoadDll
    };
#endif

    /* Allocate a buffer on the target process that will be used to store the dll path, shellcode and return value. */
    uint8_t* buffer = (uint8_t*)VirtualAllocEx(m->h_process, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!buffer)
        return 0;

    /* Copy the dll path to the buffer */
    const size_t dll_path_size = (_nmd_strlenw(dll_path) + 1) * sizeof(wchar_t);
    if (!WriteProcessMemory(m->h_process, buffer + 8, dll_path, dll_path_size, NULL))
        return 0;

    /* Create a thread on the target process with the entry point as LoadLibraryA(), passing the address of the buffer containing the path as the parameter */
    HANDLE h_thread = CreateRemoteThread(m->h_process, NULL, 0, buffer + 8 + dll_path_size, buffer, NULL, NULL);
    if (!h_thread)
        return 0;

    /* Wait for the thread to terminate */
    if (WaitForSingleObject(h_thread, INFINITE) == WAIT_FAILED)
        return 0;

    uintptr_t module_base;
    if (!GetExitCodeThread(h_thread, &module_base))
        return 0;

    /* Free resources */
    CloseHandle(h_thread);
    VirtualFreeEx(m->h_process, buffer, 0, MEM_RELEASE);

    return module_base;

    ///* Open file */
    //HANDLE h_file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    //if (h_file == INVALID_HANDLE_VALUE)
    //    return 0;
    //
    ///* Get file size */
    //DWORD file_size = GetFileSize(h_file, NULL);
    //
    ///* Allocate buffer for the file */
    //LPVOID file_buffer = VirtualAlloc(NULL, file_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    //if (!file_buffer)
    //    return 0;
    //
    ///* Read file */ 
    //if (!ReadFile(h_file, file_buffer, file_size, NULL, NULL))
    //    return 0;
    //
    ///* Call 'nmd_mex_inject_from_memory()' */
}

/* Scans the specified memory range for a pattern.
Parameters:
 - pattern    [in] The pattern. e.g. "\x10\x20\x30\x40\x50".
 - mask       [in] The mask. e.g. "xxxxx", "x????xxxxx?xx".
 - start      [in] The range's start address.
 - end        [in] The range's end address.
 - protection [in] The memory protection the page must match. Specify '-1' for any protection.
*/
void* nmd_pattern_scan_range(const char* pattern, const char* mask, uint8_t* start, uint8_t* end, uint32_t protection)
{
    MEMORY_BASIC_INFORMATION mbi;
    const size_t mask_length = _nmd_strlen(mask);
    size_t num_matches = 0;
    while (start < end && VirtualQuery(start, &mbi, sizeof(mbi)))
    {
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD) || !(protection & mbi.Protect))
        {
            start = (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
            num_matches = 0;
        }
        else
        {
            const uint8_t* region_end = (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
            while (start < region_end)
            {
                for (; num_matches < mask_length; num_matches++)
                {
                    if (mask[num_matches] != '?' && pattern[num_matches] != start[num_matches])
                        goto not_match;
                }

                if (start != pattern)
                    return start;
            not_match:
                num_matches = 0;
                start++;
            }
        }
    }

    return 0;
}

#ifdef _WIN32
#define _NMD_DB(x) _asm _emit x
_NMD_NAKED void _nmd_wow64_syscall()
{
    _asm
    {
        ; Transition to x86-64
        push 0x33
        _NMD_DB(0xe8) _NMD_DB(0x00) _NMD_DB(0x00) _NMD_DB(0x00) _NMD_DB(0x00) ; call $+0
        _NMD_DB(0x83) _NMD_DB(0x04) _NMD_DB(0x24) _NMD_DB(0x05)               ; add dword ptr[esp], 5
        retf

        ; Adjust parameters and execute syscall
        _NMD_DB(0x4C) _NMD_DB(0x63) _NMD_DB(0x54) _NMD_DB(0x24) _NMD_DB(0x04) ; movsxd r10, dword ptr[rsp + 0x4]
        _NMD_DB(0x48) _NMD_DB(0x63) _NMD_DB(0x54) _NMD_DB(0x24) _NMD_DB(0x08) ; movsxd rdx, dword ptr[rsp + 0x8]
        _NMD_DB(0x4C) _NMD_DB(0x63) _NMD_DB(0x44) _NMD_DB(0x24) _NMD_DB(0x0c) ; movsxd r8, dword ptr[rsp + 0xc]
        _NMD_DB(0x4C) _NMD_DB(0x63) _NMD_DB(0x4C) _NMD_DB(0x24) _NMD_DB(0x10) ; movsxd r9, dword ptr[rsp + 0x10]
        _NMD_DB(0x48) _NMD_DB(0x83) _NMD_DB(0xEC) _NMD_DB(0x14)               ; Allocate space for the shadow space
        _NMD_DB(0x0f) _NMD_DB(0x05)                                           ; syscall
        _NMD_DB(0x48) _NMD_DB(0x83) _NMD_DB(0xC4) _NMD_DB(0x14)               ; Deallocate space for the shadow space

        ; Transition back to x86-32
        _NMD_DB(0xe8) _NMD_DB(0x00) _NMD_DB(0x00) _NMD_DB(0x00) _NMD_DB(0x00) ; call $+0
        _NMD_DB(0xC7) _NMD_DB(0x44) _NMD_DB(0x24) _NMD_DB(0x04) _NMD_DB(0x23) _NMD_DB(0x00) _NMD_DB(0x20) _NMD_DB(0x00) ; mov dword ptr[rsp + 0x4], 0x23
        _NMD_DB(0x83) _NMD_DB(0x04) _NMD_DB(0x24) _NMD_DB(0x0d)               ; add dword ptr[rsp], 13
        retf

        ret
    }
}
#endif

/* Attemps to open a handle to the process specified by the id. Returns the handle if successful, zero otherwise.
Parameters:
 - pid         [in] The process identifier(PID).
 - access_mask [in] The desired access.
*/
_NMD_NAKED HANDLE nmd_open_process(uint32_t pid, uint32_t access_mask)
{
    _asm
    {
#ifdef _WIN64
        sub rsp, 64              ; Allocate space for two 8-byte variables and an OBJECT_ATTRIBUTES variable

        mov [rsp + 8], rcx       ; Store pid
        and qword ptr [rsp], 0   ; Clear handle space

        mov al, 0                ; value
        mov rcx, 48              ; size
        lea rdi, [rsp + 0x10]    ; dst
        rep stosb

        lea r10, [rsp + 0x00]    ; &handle
                                 ; rdx = access_mask          
        lea r8, [rsp + 0x10]     ; &object_attributes
        lea r9, [rsp + 8]        ; &pid
                                 
        mov rax, 0x26            ; NtOpenProcess syscall id
        syscall

        mov _nmd_error_code, rax ; Set error code
        mov rax, [rsp]           ; Set return value
        add rsp, 64              ; Restore stack
        ret
#else
        push ebp                      ; Prologue
        mov ebp, esp                  ;
        sub esp, 64                   ;
         
        mov eax, [ebp + 8]            ; ClientId
        and dword ptr [ebp - 0x0c], 0 ;
        mov [ebp - 0x10], eax         ;
        lea eax, [ebp - 0x10]         ;
        push eax

        mov al, 0                     ; ObjectAttributes / al=value
        mov ecx, 48                   ; size
        lea edi, [ebp - 64]           ; dst
        push edi                      ;
        rep stosb                     ;
        
        push access_mask              ; DesiredAccess

        mov dword ptr[ebp - 8], 0     ; ProcessHandle
        mov dword ptr[ebp - 4], 0     ;
        lea eax, [ebp - 8]            ;
        push eax                      ;

        mov eax, 0x26                 ; NtOpenProcess syscall id
        call _nmd_wow64_syscall       ;

        mov _nmd_error_code, eax      ; Set error code
        mov eax, [ebp - 0x8]          ; Set return value
        
        mov esp, ebp                  ; Epilogue
        pop ebp                       ;
        ret                           ;
#endif
    }
}

/* Performs a system call using the specified id.
Be aware: On WoW64 the syscall may expect structures with 8-byte sizes(such as pointers and SIZE_T).
Also, on WoW64 every parameter after the fourth must be 8-byte long.
Example: nmd_syscall(0x1234, arg1, arg2, arg3, arg4, (uint64_t)arg5, (uint64_t)arg6)
Parameters:
 - id  [in] The syscall id.
 - ... [in] The parameters used by the syscall.
*/
_NMD_NAKED NTSTATUS nmd_syscall(size_t id, ...)
{
    _asm
    {
#ifdef _WIN64
        mov rax, [rsp]        ; Save return value
        mov [rsp - 0x20], rax ;
        mov rax, rcx          ; Set syscall index
                              
        mov r10, rdx;         ; Set register parameters
        mov rdx, r8;          ;
        mov r8, r9            ;
        mov r9, [rsp + 0x28]  ; offset(0x28) = ret address(0x8) + shadow space(0x20)
                              
        add rsp, 0x08         ; Adjust stack parameters
        syscall               ; Execute syscall
        sub rsp, 0x08         ; Restore stack
        mov rcx, [rsp - 0x20] ; Adjust return address
        mov [rsp], rcx        ;
        ret                   ; Return
#else
        mov eax, [esp]          ; Save return value
        mov [esp - 0x10], eax   ;
        mov eax, [esp + 4]      ; Set syscall index
        add esp, 8              ; Adjust parameters
        call _nmd_wow64_syscall ; Execute syscall
        sub esp, 8              ; Restore stack
        mov ecx, [esp - 0x10]   ; Save return value
        mov [esp], ecx          ;
        ret                     ; Return
#endif
    }
}

#endif /* NMD_MEMORY_IMPLEMENTATION */

#endif /* NMD_MEMORY_H */