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

#ifdef NMD_MEMORY_DEFINE_INT_TYPES

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

/* Dependencies when 'NMD_MEMORY_DEFINE_INT_TYPES' is not defined. */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#endif /* NMD_MEMORY_DEFINE_INT_TYPES */


#ifdef __clang__
    #define _NMD_NAKED __attribute__((naked))
#else
    #if defined(_WIN64) and defined(_MSC_VER)
        #define _NMD_NAKED
    #else
        #define _NMD_NAKED __declspec(naked)
    #endif
#endif /* __clang__ */

#include <Windows.h>

typedef struct nmd_proc
{
	HANDLE h_process;
} nmd_proc;

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
    PVOID ProcessParameters; /* PRTL_USER_PROCESS_PARAMETERS */
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
    PVOID SharedData; /* HotpatchInformation */
    PVOID* ReadOnlyStaticServerData;

    PVOID AnsiCodePageData; /* PCPTABLEINFO */
    PVOID OemCodePageData; /* PCPTABLEINFO */
    PVOID UnicodeCaseTableData; /* PNLSTABLEINFO */

    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

    LARGE_INTEGER CriticalSectionTimeout;
    SIZE_T HeapSegmentReserve;
    SIZE_T HeapSegmentCommit;
    SIZE_T HeapDeCommitTotalFreeThreshold;
    SIZE_T HeapDeCommitFreeBlockThreshold;

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID* ProcessHeaps; /* PHEAP */

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
    PVOID AppCompatInfo; /* APPCOMPAT_EXE_DATA */

    NMD_UNICODE_STRING CSDVersion;

    PVOID ActivationContextData; /* ACTIVATION_CONTEXT_DATA */
    PVOID ProcessAssemblyStorageMap; /* ASSEMBLY_STORAGE_MAP */
    PVOID SystemDefaultActivationContextData; /* ACTIVATION_CONTEXT_DATA */
    PVOID SystemAssemblyStorageMap; /* ASSEMBLY_STORAGE_MAP */

    SIZE_T MinimumStackCommit;

    PVOID* FlsCallback;
    LIST_ENTRY FlsListHead;
    PVOID FlsBitmap;
    ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
    ULONG FlsHighIndex;

    PVOID WerRegistrationData;
    PVOID WerShipAssertPtr;
    PVOID pUnused; /* pContextData */
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
    PVOID TelemetryCoverageHeader; /* REDSTONE3 */
    ULONG CloudFileFlags;
} NMD_PEB, *NMD_PPEB;

enum NMD_INJECTION_FLAGS
{
    /* Common flags */
    NMD_INJECTION_FLAGS_NONE          = 0,
    NMD_INJECTION_FLAGS_ERASE_HEADER  = (1 << 0),
    NMD_INJECTION_FLAGS_FAKE_HEADER   = (1 << 1),

    /* Load Library flags */
    NMD_INJECTION_FLAGS_UNLINK_MODULE = (1 << 2),

    /* Manual mapping flags */
    NMD_INJECTION_FLAGS_LINK_MODULE   = (1 << 3),
    NMD_INJECTION_FLAGS_TLS_CALLBACKS = (1 << 4), /* Execute TLS callbacks */
    NMD_INJECTION_FLAGS_EXCEPTIONS    = (1 << 5), /* Add support for exceptions */
};

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
 - ... [in] The syscall's parameters.
*/
NTSTATUS nmd_syscall(size_t id, ...);

/* Calls a function written for x86-64
Be aware: On WoW64 the function may expect structures with 8-byte sizes(such as pointers and size_t).
Also, on WoW64 every parameter must be 8-byte long.
Example: nmd_syscall(some_x64_function, (uint64_t)arg1, (uint64_t)arg2, (uint64_t)arg3, (uint64_t)arg4, (uint64_t)arg5, (uint64_t)arg6)
Parameters:
 - func [in] The function to be called.
 - ...  [in] The function's parameters.
*/
int64_t nmd_call_x64(size_t id, ...);

/* Performs a system call using the specified id.
Parameters:
 - func [in] The function to be called.
 - ...  [in] The function's parameters.
*/
int64_t nmd_call_x86(size_t id, ...);

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

/* Returns the image size of the specified module. 
Parameters:
 - h_module The module base.
*/
DWORD nmd_get_module_size(HMODULE h_module);

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
/* NTSTATUS nmd_query_system_information(SYSTEM_INFORMATION_CLASS info, PVOID buffer, ULONG size, PULONG return_size); */

/* NTSTATUS nmd_allocate_virtual_memory */

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
uint8_t* nmd_pattern_scan_range(const char* pattern, const char* mask, uint8_t* start, uint8_t* end, uint32_t protection);

/*
Hooks a function. Returns true if successful, false otherwise.
Parameters:
 - target   [in]      The function to be hooked.
 - detour   [in]      The function to override the 'target'.
 - original [out/opt] An optional pointer to a variable that recieves the address of the original function.
 */
bool nmd_hook(void* target, void* detour, void* original);

/* Iterates through all processes to find the process with a matching name. Returns the PID of the specified process, or zero if the operation failed.
Parameters:
 - process_name [in] The wide-char process name.
*/
uint32_t nmd_mex_get_pid_by_wname(const wchar_t* process_name);

/* "Opens" a process by verifying if the given handle is a valid process handle. Returns true if the operation is successful, false otherwise.
Parameters:
 - mex    [out] A pointer to a variablev of type 'nmd_proc'.
 - handle [in]  The process handle.
*/
bool nmd_mex_open_by_handle(nmd_proc* mex, HANDLE handle);

/* Injects a dll on the specified process. Returns the base address of the injected module or zero if he operation failed. 
Parameters:
 - h_process [in] A handle to the target process.
 - dll_path  [in] The path to the dll.
 - flags     [in] A mask of flags(members of the NMD_INJECTION_FLAGS enum) which change the behaviour of the function.
*/
uintptr_t nmd_inject_load_library(HANDLE h_process, const wchar_t* dll_path, uint32_t flags);

/* Injects a dll on the specified process. Returns the base address of the injected module or zero if he operation failed.
Parameters:
 - h_process [in] A handle to the target process.
 - dll       [in] A pointer to the dll in memory.
 - flags     [in] A mask of flags(members of the NMD_INJECTION_FLAGS enum) which change the behaviour of the function.
*/
uintptr_t nmd_inject_manual(HANDLE h_process, const void* dll, uint32_t flags);

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

    NMD_PLDR_MODULE final_mod = (NMD_PLDR_MODULE)nmd_get_peb()->Ldr->InLoadOrderModuleList.Flink;
    NMD_PLDR_MODULE mod = (NMD_PLDR_MODULE)nmd_get_peb()->Ldr->InLoadOrderModuleList.Blink;
    while (mod != final_mod)
    {
        if(_nmd_strcmpiw(mod->BaseDllName.Buffer, module_name))
            return (HMODULE)mod->BaseAddress;
        mod = (NMD_PLDR_MODULE)mod->InLoadOrderModuleList.Blink;
    }

    return 0;
}

/* Returns the image size of the specified module.
Parameters:
 - h_module The module base.
*/
DWORD nmd_get_module_size(HMODULE h_module)
{
    return ((PIMAGE_OPTIONAL_HEADER)((uint8_t*)h_module + ((PIMAGE_DOS_HEADER)h_module)->e_lfanew + 4 + sizeof(IMAGE_FILE_HEADER)))->SizeOfImage;
}

/* Returns the procedure address being exported by the specified module, or zero if an error occurred.
Parameters:
 - h_module  [in] The module base.
 - proc_name [in] The name of the procedure exported by 'h_module'.
*/
void* nmd_get_proc_addr(HMODULE h_module, const char* proc_name)
{
    const uint8_t* base = (uint8_t*)h_module;
    const PIMAGE_OPTIONAL_HEADER optional_header = (PIMAGE_OPTIONAL_HEADER)(base + *(uint32_t*)(base + 0x3c) + 4 + sizeof(IMAGE_FILE_HEADER));
    const uint32_t export_directory_rva = optional_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!export_directory_rva)
        return 0;
    const PIMAGE_EXPORT_DIRECTORY export_directory = (PIMAGE_EXPORT_DIRECTORY)(base + export_directory_rva);
    const uint32_t* names = (uint32_t*)(base + export_directory->AddressOfNames);

    size_t i = 0;
    for (; i < export_directory->NumberOfNames; i++)
    {
        const char* name = (const char*)(base + names[i]);
        if (_nmd_strcmp(name, proc_name))
        {
            const uint16_t ordinal = ((uint16_t*)(base + export_directory->AddressOfNameOrdinals))[i];
            const uint32_t address = ((uint32_t*)(base + export_directory->AddressOfFunctions))[ordinal];
            return (void*)(base + address);
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
}*/

extern "C" uint32_t __stdcall NtAllocateVirtualMemory(HANDLE ProcessHandle, void** BaseAddress, uint32_t ZeroBits, size_t* RegionSize, uint32_t AllocationType, uint32_t Protect);
extern "C" uint32_t __stdcall NtFreeVirtualMemory(HANDLE ProcessHandle, void** BaseAddress, PULONG RegionSize, ULONG FreeType);
extern "C" uint32_t __stdcall NtProtectVirtualMemory(HANDLE ProcessHandle, void** BaseAddress, uint32_t* NumberOfBytesToProtect, uint32_t NewAccessProtection, uint32_t* OldAccessProtection);
extern "C" uint32_t __stdcall NtQueryVirtualMemory(HANDLE ProcessHandle, void* BaseAddress, uint32_t MemoryInformationClass, void* Buffer, size_t Length, size_t* ResultLength);
extern "C" uint32_t __stdcall NtReadVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToRead, PULONG NumberOfBytesReaded);
extern "C" uint32_t __stdcall NtWriteVirtualMemory(HANDLE ProcessHandle, void* BaseAddress, void* Buffer, uint32_t NumberOfBytesToWrite, uint32_t* NumberOfBytesWritten);
extern "C" uint32_t __stdcall NtCreateThreadEx(HANDLE * pHandle, ACCESS_MASK DesiredAccess, void* pAttr, HANDLE hProc, void* pFunc, void* pArg, ULONG Flags, SIZE_T ZeroBits, SIZE_T StackSize, SIZE_T MaxStackSize, void* pAttrListOut);
extern "C" uint32_t __stdcall NtWaitForSingleObject(HANDLE ObjectHandle, BOOLEAN Alertable, PLARGE_INTEGER TimeOut);
extern "C" uint32_t __stdcall NtClose(HANDLE ObjectHandle);
extern "C" uint32_t __stdcall NtQueryInformationThread(HANDLE ThreadHandle, uint32_t ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength);

#define nmd_memset(ptr, value, num) { size_t _nmd_index = num-1; while(_nmd_index){((uint8_t*)ptr)[_nmd_index--] = value;}}

typedef struct NMD_CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} NMD_CLIENT_ID;

typedef struct NMD_THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    NMD_CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    uint32_t Priority;
    uint32_t BasePriority;
} NMD_THREAD_BASIC_INFORMATION, * NMD_PTHREAD_BASIC_INFORMATION;

/* Injects a dll on the specified process. Returns the base address of the injected module or zero if he operation failed.
Parameters:
 - h_process [in] A handle to the target process.
 - dll       [in] A pointer to the dll in memory.
 - flags     [in] A mask of flags(members of the NMD_INJECTION_FLAGS enum) which change the behaviour of the function.
*/
uintptr_t nmd_inject_load_library(HANDLE h_process, const wchar_t* dll_path, uint32_t flags)
{
    nmd_set_error_code(0);

    uintptr_t module_base = 0;
    HMODULE ntdll = nmd_get_module_handle(L"ntdll.dll");
    uintptr_t ldrloaddll = (uintptr_t)nmd_get_proc_addr(ntdll, "LdrLoadDll");

    /* Shellcode */
#ifdef _WIN64
    uint8_t shellcode_ldr_load_dll[] = {
        0x49, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* mov r9, ModuleHandle */
        0x4d, 0x89, 0xcc,                                           /* mov r12, r9 ; save ModuleHandle */
        0x4c, 0x8b, 0xc1,                                           /* mov r8, rcx(PUNICODE_STRING) */
        0x33, 0xd2,                                                 /* mov rdx, DllCharacteristics: 0 */
        0x33, 0xc9,                                                 /* mov rcx, DllPath: 0 */
        0x48, 0x83, 0xec, 0x20,                                     /* sub rsp, 20h ; Allocate shadow space */
        0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* mov rax, LdrLoadDll */
        0xff, 0xd0,                                                 /* call rax */
        0x48, 0x83, 0xc4, 0x20,                                     /* add rsp, 20h ; Deallocate shadow space */

        /* Return if the function failed */
        0x85, 0xc0,                                                 /* test eax, eax */
        0x75, 0x02,                                                 /* jnz ret */

        /* unlink from PEB */
        0xeb, 0x00,                                                 /* jmp $+1 if b_unlink_from_peb else $+0 */
        0xc3,                                                       /* ret */
        0xb1, 0x03,                                                 /* mov cl, 3 ; set # of lists */
        0x4d, 0x8b, 0x1c, 0x24,                                     /* mov r11, [r12] ; save module base */
        0x65, 0x48, 0x8b, 0x04, 0x25, 0x60, 0x00, 0x00, 0x00,       /* mov rax, gs:[60h] ; ppeb */
        0x48, 0x8b, 0x40, 0x18,                                     /* mov rax, [rax+18] ; Ldr */
        0x4c, 0x8d, 0x50, 0x10,                                     /* lea r10, [rax+10h] ; first list */
        /* parse_list_entry: */
        0x49, 0x8b, 0x02,                                           /* mov rax, [r10]  ; dereference first entry */
        0x4d, 0x8d, 0x42, 0x08,                                     /* lea r8, [r10 + 8] ; last entry */
        /* check_entry: */
        0x4c, 0x39, 0x58, 0x30,                                     /* cmp [rax+30h], r11 ; NMD_LDR_MODULE::BaseAddress == module_base(r11) */
        0x75, 0x08,                                                 /* jne next_entry */
        0x48, 0x8b, 0x00,                                           /* mov rax, [rax] ; get next entry */
        0x49, 0x89, 0x01,                                           /* mov [r9], rax ; unlink entry */
        0xeb, 0x0b,                                                 /* jmp next_list */

        /* next_entry: */
        0x49, 0x89, 0xc1,                                           /* mov r9, rax ; save entry */
        0x48, 0x8b, 0x00,                                           /* mov rax, [rax] */
        0x4c, 0x39, 0xc0,                                           /* cmp rax, r8 ; this_entry == last_entry */
        0x75, 0xe7,                                                 /* jne check_entry */
        /* next_list*/
        0xfe, 0xc9,                                                 /* dec cl */
        0x49, 0x83, 0xc2, 0x10,                                     /* add r10, 10h */
        0x84, 0xc9,                                                 /* test cl, cl */
        0x75, 0xd6,                                                 /* jnz parse_list_entry */
        0xc3                                                        /* ret */
    };
    /* Copy LdrLoadDll address to shellcode */
    *(uint64_t*)(shellcode_ldr_load_dll + 26) = (uint64_t)ldrloaddll;

    /* Determine if the shellcode unlinks the module */
    if (flags & NMD_INJECTION_FLAGS_UNLINK_MODULE)
        shellcode_ldr_load_dll[45] = 1;
#else
    uint8_t shellcode_ldr_load_dll[] = {
        0x68, 0x00, 0x00, 0x00, 0x00, /* push ModuleHandle */
        0x8b, 0x45, 0x04              /* mov eax, [ebp+4] */
        0x50,                         /* push eax(PUNICODE_STRING) */
        0x6a, 0x00,                   /* push DllCharacteristics: 0 */
        0x6a, 0x00,                   /* push DllPath: 0 */
        0xb8, 0x00, 0x00, 0x00, 0x00, /* mov eax, LdrLoadDll */
        0xff, 0xd0,                   /* call eax */
        0xc3,                         /* ret */
    };
    /* Copy LdrLoadDll address to shellcode */
    *(uint32_t*)(shellcode_ldr_load_dll + 14) = (uint32_t)ldrloaddll;
#endif

    /* Allocate a buffer on the target process that will be used to store the dll path, shellcode.
    The buffer will have the following format:
    uintptr_t dll_base [OUT] The handle of the loaded dll
    wchar_t[] dll_path [IN]  A wide string containing the dll path
    UNICODE_STRING     [IN]  A unicode string describing the dll path
    shellcode          [IN]  The shellcode that executes ntdll!LdrLoadDll
    */
    void* buffer = 0;
    size_t size = 0x1000;
    if (!NtAllocateVirtualMemory(h_process, &buffer, 0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
    {
        /* Copy ModuleHandle */
#ifdef _WIN64
        *(uint64_t*)(shellcode_ldr_load_dll + 2) = (uint64_t)buffer;
#else
        *(uint32_t*)(shellcode_ldr_load_dll + 1) = (uint32_t)buffer;
#endif

        /* Copy the dll path */
        const size_t dll_path_size = _nmd_strlenw(dll_path) * sizeof(wchar_t) + 2;
        if (!NtWriteVirtualMemory(h_process, (uint8_t*)buffer + sizeof(uintptr_t), (void*)dll_path, dll_path_size, NULL))
        {
            /* Copy the unicode string that describes the dll path */
            NMD_UNICODE_STRING us;
            nmd_memset(&us, 0, sizeof(us));
            us.Length = dll_path_size - 2;
            us.MaximumLength = dll_path_size;
            us.Buffer = (PWSTR)((uint8_t*)buffer + sizeof(uintptr_t));
            if (!NtWriteVirtualMemory(h_process, (uint8_t*)buffer + sizeof(uintptr_t) + dll_path_size, &us, sizeof(us), NULL))
            {
                /* Copy the shellcode */
                if (!NtWriteVirtualMemory(h_process, (uint8_t*)buffer + sizeof(uintptr_t) + dll_path_size + sizeof(us), shellcode_ldr_load_dll, sizeof(shellcode_ldr_load_dll), NULL))
                {
                    /* Create a thread on the target process with the entry point as 'buffer', passing the address of the unicode string that describes the dll path as a parameter */
                    HANDLE h_thread = 0;
                    if (!NtCreateThreadEx(&h_thread, THREAD_ALL_ACCESS, 0, h_process, (LPTHREAD_START_ROUTINE)((uint8_t*)buffer + sizeof(uintptr_t) + dll_path_size + sizeof(us)), (uint8_t*)buffer + 8 + dll_path_size, FALSE, 0, 0, 0, 0))
                    {
                        /* Wait for the thread to terminate */
                        if (!NtWaitForSingleObject(h_thread, FALSE, 0))
                        {
                            /* Get thread's exit code */
                            NMD_THREAD_BASIC_INFORMATION tbi;
                            if (!NtQueryInformationThread(h_thread, 0/*ThreadBasicInformation*/, &tbi, sizeof(tbi), 0))
                            {
                                /* Read module base */
                                if (!NtReadVirtualMemory(h_process, buffer, &module_base, sizeof(uintptr_t), 0))
                                {
                                    /* Set error code */
                                    nmd_set_error_code(module_base ? 0 : tbi.ExitStatus);

                                    if (tbi.ExitStatus == 0/*NTSUCCESS*/)
                                    {
                                        if (flags & NMD_INJECTION_FLAGS_ERASE_HEADER)
                                        {
                                            uint8_t null_buffer[0x1000];
                                            nmd_memset(null_buffer, 0, 0x1000);
                                            NtWriteVirtualMemory(h_process, buffer, null_buffer, 0x1000, 0);
                                        }
                                        else if (flags & NMD_INJECTION_FLAGS_FAKE_HEADER)
                                        {
                                            NtWriteVirtualMemory(h_process, buffer, ntdll, 0x1000, 0);
                                        }
                                    }
                                }
                            }
                        }

                        /* Close handle */
                        NtClose(h_thread);
                    }
                }
            }
        }

        /* Free memory */
        NtFreeVirtualMemory(h_process, &buffer, (PULONG)&size, MEM_RELEASE);
    }
    
    return module_base;
}

/* Injects a dll on the specified process. Returns the base address of the injected module or zero if he operation failed.
Parameters:
 - h_process [in] A handle to the target process.
 - dll       [in] A pointer to the dll in memory.
 - flags     [in] A mask of flags(members of the NMD_INJECTION_FLAGS enum) which change the behaviour of the function.
*/
uintptr_t nmd_inject_manual(HANDLE h_process, const void* dll, uint32_t flags)
{
    /* Allocate memory to map dll */
    void* address = 0;
    const PIMAGE_OPTIONAL_HEADER optional_header = (const PIMAGE_OPTIONAL_HEADER)(((uint8_t*)dll + *(uint32_t*)((uint8_t*)dll + 0x3c)) + 4 + sizeof(IMAGE_FILE_HEADER));
    size_t size = optional_header->SizeOfImage;
    if (NtAllocateVirtualMemory(h_process, &address, 0, &size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE))
        return 0;

    /* Copy image header */
    uint32_t num_bytes_written;
    if (NtWriteVirtualMemory(h_process, address, (void*)dll, 0x1000, &num_bytes_written))
        return 0;

    /* Copy image sections */

    /* Copy shellcode(resolver imports, tls,exceptions,adjust page protection, */
}

void* nmd_read_multi_level_pointer(void* base, int32_t* offsets, size_t num_offsets)
{    
    MEMORY_BASIC_INFORMATION mbi;
    size_t i = 0;
    for (; i < num_offsets; i++)
    {   
        if (NtQueryVirtualMemory((void*)(-1), base, 0/*MemoryBasicInformation*/, &mbi, sizeof(mbi), 0) || mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return 0;

        /* Dereference and apply offset */
        base = (void*)(*(uintptr_t*)base + offsets[i]);
    }

    return base;
}

/* Scans the specified memory range for a pattern.
Parameters:
 - pattern    [in] The pattern. e.g. "\x10\x20\x30\x40\x50".
 - mask       [in] The mask. e.g. "xxxxx", "x????xxxxx?xx".
 - start      [in] The range's start address.
 - end        [in] The range's end address.
 - protection [in] The memory protection the page must match. Specify '-1' for any protection.
*/
uint8_t* nmd_pattern_scan_range(const char* pattern, const char* mask, uint8_t* start, uint8_t* end, uint32_t protection)
{
    MEMORY_BASIC_INFORMATION mbi;
    const size_t mask_length = _nmd_strlen(mask);
    size_t num_matches = 0;
    
    while (start < end && !NtQueryVirtualMemory((void*)(-1), start, 0/*MemoryBasicInformation*/, &mbi, sizeof(mbi), 0))
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
                    if (mask[num_matches] != '?' && ((uint8_t*)pattern)[num_matches] != start[num_matches])
                        goto not_match;
                }

                if (start != (uint8_t*)pattern)
                    return start;
            not_match:
                num_matches = 0;
                start++;
            }
        }
    }

    return 0;
}

#define _NMD_MEM_R (*b >> 4)
#define _NMD_MEM_C (*b & 0xF)
bool _nmd_mem_find_byte(const uint8_t* arr, const size_t N, const uint8_t x) { for (size_t i = 0; i < N; i++) { if (arr[i] == x) { return true; } }; return false; }

void _nmd_mem_parse_modrm(uint8_t** b, const bool addressPrefix)
{
    uint8_t modrm = *++ * b;

    if (!addressPrefix || (addressPrefix && **b >= 0x40))
    {
        bool hasSIB = false; /* Check for SIB byte */
        if (**b < 0xC0 && (**b & 0b111) == 0b100 && !addressPrefix)
            hasSIB = true, (*b)++;

        if (modrm >= 0x40 && modrm <= 0x7F) /* disp8 (ModR/M) */
            (*b)++;
        else if ((modrm <= 0x3F && (modrm & 0b111) == 0b101) || (modrm >= 0x80 && modrm <= 0xBF)) /* disp16,32 (ModR/M) */
            *b += (addressPrefix) ? 2 : 4;
        else if (hasSIB && (**b & 0b111) == 0b101) /* disp8,32 (SIB) */
            *b += (modrm & 0b01000000) ? 1 : 4;
    }
    else if (addressPrefix && modrm == 0x26)
        *b += 2;
};

size_t _nmd_mem_ldisasm(const void* const address, const bool x86_64_mode)
{
    const uint8_t prefixes[] = { 0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65, 0x66, 0x67 };
    const uint8_t op1modrm[] = { 0x62, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC4, 0xC5, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE, 0xFF };
    const uint8_t op1imm8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
    const uint8_t op1imm32[] = { 0x68, 0x69, 0x81, 0xA9, 0xC7, 0xE8, 0xE9 };
    const uint8_t op2modrm[] = { 0x0D, 0xA3, 0xA4, 0xA5, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF };

    size_t offset = 0;
    bool operandPrefix = false, addressPrefix = false, rexW = false;
    uint8_t* b = (uint8_t*)(address);

    /* Parse legacy prefixes & REX prefixes */
    for (int i = 0; i < 14 && _nmd_mem_find_byte(prefixes, sizeof(prefixes), *b) || ((x86_64_mode) ? (_NMD_MEM_R == 4) : false); i++, b++)
    {
        if (*b == 0x66)
            operandPrefix = true;
        else if (*b == 0x67)
            addressPrefix = true;
        else if (_NMD_MEM_R == 4 && _NMD_MEM_C >= 8)
            rexW = true;
    }

    /* Parse opcode(s) */
    if (*b == 0x0F) /* 2 bytes */
    {
        if (_NMD_MEM_R == 8) /* disp32 */
            offset += 4;
        else if ((_NMD_MEM_R == 7 && _NMD_MEM_C < 4) || *b == 0xA4 || *b == 0xC2 || (*b > 0xC3 && *b <= 0xC6) || *b == 0xBA || *b == 0xAC) /* imm8 */
            offset++;

        /* Check for ModR/M, SIB and displacement */
        if (_nmd_mem_find_byte(op2modrm, sizeof(op2modrm), *b) || (_NMD_MEM_R != 3 && _NMD_MEM_R > 0 && _NMD_MEM_R < 7) || *b >= 0xD0 || (_NMD_MEM_R == 7 && _NMD_MEM_C != 7) || _NMD_MEM_R == 9 || _NMD_MEM_R == 0xB || (_NMD_MEM_R == 0xC && _NMD_MEM_C < 8) || (_NMD_MEM_R == 0 && _NMD_MEM_C < 4))
            _nmd_mem_parse_modrm(&b, addressPrefix);
    }
    else /* 1 byte */
    {
        /* Check for immediate field */
        if ((_NMD_MEM_R == 0xE && _NMD_MEM_C < 8) || (_NMD_MEM_R == 0xB && _NMD_MEM_C < 8) || _NMD_MEM_R == 7 || (_NMD_MEM_R < 4 && (_NMD_MEM_C == 4 || _NMD_MEM_C == 0xC)) || (*b == 0xF6 && !(*(b + 1) & 48)) || _nmd_mem_find_byte(op1imm8, sizeof(op1imm8), *b)) /* imm8 */
            offset++;
        else if (*b == 0xC2 || *b == 0xCA) /* imm16 */
            offset += 2;
        else if (*b == 0xC8) /* imm16 + imm8 */
            offset += 3;
        else if ((_NMD_MEM_R < 4 && (_NMD_MEM_C == 5 || _NMD_MEM_C == 0xD)) || (_NMD_MEM_R == 0xB && _NMD_MEM_C >= 8) || (*b == 0xF7 && !(*(b + 1) & 48)) || _nmd_mem_find_byte(op1imm32, sizeof(op1imm32), *b)) /* imm32,16 */
            offset += (rexW) ? 8 : (operandPrefix ? 2 : 4);
        else if (_NMD_MEM_R == 0xA && _NMD_MEM_C < 4)
            offset += (rexW) ? 8 : (addressPrefix ? 2 : 4);
        else if (*b == 0xEA || *b == 0x9A) /* imm32,48 */
            offset += operandPrefix ? 4 : 6;

        /* Check for ModR/M, SIB and displacement */
        if (_nmd_mem_find_byte(op1modrm, sizeof(op1modrm), *b) || (_NMD_MEM_R < 4 && (_NMD_MEM_C < 4 || (_NMD_MEM_C >= 8 && _NMD_MEM_C < 0xC))) || _NMD_MEM_R == 8 || (_NMD_MEM_R == 0xD && _NMD_MEM_C >= 8))
            _nmd_mem_parse_modrm(&b, addressPrefix);
    }

    return (size_t)((ptrdiff_t)(++b + offset) - (ptrdiff_t)(address));
}

typedef struct _nmd_hook_page
{
    struct _nmd_hook_page* next;
} _nmd_hook_page;

enum _NMD_HOOK_DATA_TYPE
{
    _NMD_HOOK_DATA_TYPE_NONE          = 0,
    _NMD_HOOK_DATA_TYPE_TRAMPOLINE    = 1,
    _NMD_HOOK_DATA_TYPE_ABSOLUTE_JUMP = 2,
};

typedef struct _nmd_hook_data
{
    uint16_t type; /* A member of the '_NMD_HOOK_DATA_TYPE' enum */
    uint16_t size;
}_nmd_hook_data;

_nmd_hook_page* _nmd_first_hook_page = 0;

void* _nmd_alloc_page_near(void* addr)
{
    size_t size = 0x1000;
#ifdef _WIN64
    /* Start (1<<31) bytes before 'addr' */
    void* target_addr = (void*)((uintptr_t)addr + 0x1000);
    while (NtAllocateVirtualMemory((void*)(-1), (void**)&target_addr, NULL, &size, 0x00002000 | 0x00001000/*MEM_RESERVE | MEM_COMMIT*/, 0x40/*PAGE_EXECUTE_READWRITE*/))
    {
        /* Return zero if we're in an invalid range */
        if ((uintptr_t)target_addr >= ((uintptr_t)1<<(sizeof(uintptr_t)*8-1)) || (uintptr_t)target_addr >= ((uintptr_t)addr + ((uintptr_t)1 << 31) - 0x1000))
            return 0;

        target_addr = (uint8_t*)target_addr + 0x1000;
    }

    return target_addr;
#else
    void* target_addr = 0;
    const bool success = !NtAllocateVirtualMemory((void*)(-1), (void**)&target_addr, NULL, &size, 0x00002000 | 0x00001000/*MEM_RESERVE | MEM_COMMIT*/, 0x40/*PAGE_EXECUTE_READWRITE*/);
    return success ? target_addr : 0;
#endif
}

_nmd_hook_data* _nmd_find_free_hook_data(_nmd_hook_page* hook_page, size_t size)
{
    _nmd_hook_data* hook_data = (_nmd_hook_data*)((uint8_t*)hook_page + sizeof(_nmd_hook_page));
    while (hook_data->type != _NMD_HOOK_DATA_TYPE_NONE || size > hook_data->size)
    {
        /* Check if we're parsed the whole page */
        if (!((uintptr_t)hook_data & 0xfff))
            return 0;

        hook_data = (_nmd_hook_data*)((uint8_t*)hook_data + sizeof(_nmd_hook_data) + hook_data->size);
    }

    return hook_data;
}

uint8_t* _nmd_alloc_hook_data_near(void* target, uint8_t type, size_t size)
{
    /* Allocate the first page if it does not exist */
    if (!_nmd_first_hook_page)
    {
        if (!(_nmd_first_hook_page = (_nmd_hook_page*)_nmd_alloc_page_near(target)))
            return 0;

        _nmd_hook_data* hook_data = (_nmd_hook_data*)((uint8_t*)_nmd_first_hook_page + sizeof(_nmd_hook_page));
        hook_data->size = size;
        hook_data->type = type;

        _nmd_hook_data* next_hook_data = (_nmd_hook_data*)((uint8_t*)hook_data + sizeof(_nmd_hook_data) + size);
        next_hook_data->size = 0x1000 - (sizeof(_nmd_hook_page*) + sizeof(_nmd_hook_data) + size);

        return (uint8_t*)hook_data + sizeof(_nmd_hook_data);
    }

    /* Parse existing hook pages */
    _nmd_hook_page* hook_page = _nmd_first_hook_page;
    while (true)
    {
        /* Check if the hook page is within range */
        if ((uintptr_t)hook_page < (uintptr_t)target + ((uintptr_t)1 << 31) && (uintptr_t)hook_page > (((uint8_t*)target <= (uint8_t*)(((uintptr_t)1 << 31) - 0x1000)) ? (uintptr_t)0x1000 : ((uintptr_t)target - ((uintptr_t)1 << 31) + 0x1000)))
        {
            _nmd_hook_data* hook_data = _nmd_find_free_hook_data(hook_page, size);
            if (hook_data)
            {
                uint32_t remaining_bytes = ((0x1000 - (((uintptr_t)hook_data + sizeof(_nmd_hook_data) + size) & 0xfff)) % 0x1000);

                if (remaining_bytes < 10 + sizeof(_nmd_hook_data))
                    size += remaining_bytes;
                else
                {
                    _nmd_hook_data* next_hook_data = (_nmd_hook_data*)((uint8_t*)hook_data + sizeof(_nmd_hook_data) + size);
                    next_hook_data->size = remaining_bytes - sizeof(_nmd_hook_data);
                }

                hook_data->size = size;

                hook_data->type = type;
                return (uint8_t*)hook_data + sizeof(_nmd_hook_data);
            }
        }

        /* Allocate a new hook page if this was the last one */
        if (!hook_page->next)
        {
            if ((hook_page->next = (_nmd_hook_page*)_nmd_alloc_page_near(target)))
                return 0;
            _nmd_hook_data* hook_data = (_nmd_hook_data*)((uint8_t*)hook_page->next + sizeof(_nmd_hook_page));
            hook_data->size = size;
            hook_data->type = type;

            _nmd_hook_data* next_hook_data = (_nmd_hook_data*)((uint8_t*)hook_data + sizeof(_nmd_hook_data) + size);
            next_hook_data->size = 0x1000 - (sizeof(_nmd_hook_page*) + sizeof(_nmd_hook_data) + size);
            
            return (uint8_t*)hook_data + sizeof(_nmd_hook_data);
        }

        /* Go to the next hook page */
        hook_page = hook_page->next;
    }
}

uint8_t* _nmd_alloc_trampoline(void* target, size_t num_original_bytes)
{
    return _nmd_alloc_hook_data_near(target, _NMD_HOOK_DATA_TYPE_TRAMPOLINE, num_original_bytes + 5);
}

uint8_t* _nmd_alloc_absolute_jump(void* target)
{
    return _nmd_alloc_hook_data_near(target, _NMD_HOOK_DATA_TYPE_ABSOLUTE_JUMP, 12);
}

/*
Hooks a function. Returns true if successful, false otherwise.
Parameters:
 - target   [in]      The function to be hooked.
 - detour   [in]      The function to override the 'target'.
 - original [out/opt] An optional pointer to a variable that recieves the address of the original function.
 */
bool nmd_hook(void* target, void* detour, void* original)
{
    const ptrdiff_t delta = (uintptr_t)detour - ((uintptr_t)target + 5);

    /* Calculate the number of bytes to be copied to the trampoline */
    size_t num_copy_bytes = 0;
#ifdef _WIN64
    while(num_copy_bytes < 5)
        num_copy_bytes += _nmd_mem_ldisasm((uint8_t*)target + num_copy_bytes, true);
#else
    while (num_copy_bytes < 5)
        num_copy_bytes += _nmd_mem_ldisasm((uint8_t*)target + num_copy_bytes, false);
#endif

    uint8_t* trampoline = _nmd_alloc_trampoline(target, num_copy_bytes);
    if (!trampoline)
        return false;
    
    if (*(uint8_t*)target == 0xe9)
    {
        trampoline[0] = 0xe9;
        *(int32_t*)(trampoline + 1) = (int32_t)((*(int32_t*)((uint8_t*)target + 1) + ((uintptr_t)target + 5)) - (uintptr_t)(trampoline + 5));
    }
    else
    {
        /* Copy original instructions */
        size_t i = 0;
        for (; i < num_copy_bytes; i++)
            trampoline[i] = ((uint8_t*)target)[i];
    }
    
    /* Place near jump back to the original function */
    trampoline[num_copy_bytes] = 0xE9;
    *((int32_t*)(trampoline + num_copy_bytes + 1)) = (int32_t)(((uintptr_t)target + num_copy_bytes) - (uintptr_t)(trampoline + num_copy_bytes + 5));

    /* Set 'original' */
    if (original)
        *(uint8_t**)original = trampoline;

    uint32_t old_protection;
    void* base_addr = target;
    if (NtProtectVirtualMemory((void*)(-1), &base_addr, (uint32_t*)&num_copy_bytes, PAGE_EXECUTE_READWRITE, &old_protection))
        return false;

#ifdef _WIN64
    /* Check if 'target' and 'detour' are too far from each other */
    if (delta < -(ptrdiff_t)0x80000000 || delta > (ptrdiff_t)0x7fffffff)
    {
        uint8_t* absolute_jump = _nmd_alloc_absolute_jump(target);
        if (!absolute_jump)
            return false;

        /* Absolute jump to 'detour' */
        *(uint16_t*)(absolute_jump + 0) = 0xB848;
        *(uint64_t*)(absolute_jump + 2) = (uint64_t)detour;
        *(uint16_t*)(absolute_jump + 10) = 0xE0FF;

        /* Near jump to the absolute jump */
        *(uint8_t*)target = 0xE9;
        *((int32_t*)((uint8_t*)target + 1)) = (int32_t)((uintptr_t)absolute_jump - ((uintptr_t)target + 5));
    }
    else
#endif
    {
        /* Near jump to 'detour' */
        *(uint8_t*)target = 0xE9;
        *((int32_t*)((uint8_t*)target + 1)) = (int32_t)delta;
    }

    /* Restore protection */
    base_addr = target;
    NtProtectVirtualMemory((void*)(-1), &base_addr, (uint32_t*)&num_copy_bytes, old_protection, &old_protection);
    FlushInstructionCache((HANDLE)(-1), target, 5);

    return true;
}

bool _nmd_unhook_page(void* target, _nmd_hook_page* hook_page)
{
    _nmd_hook_data* hook_data = (_nmd_hook_data*)((uint8_t*)hook_page + sizeof(_nmd_hook_page));
    do
    {
        if (hook_data->type == _NMD_HOOK_DATA_TYPE_TRAMPOLINE)
        {
            uint8_t* trampoline = (uint8_t*)hook_data + sizeof(_nmd_hook_data) + hook_data->size - 4;
            const size_t num_copy_bytes = hook_data->size - 5;
            if ((uintptr_t)(*(int32_t*)trampoline + (trampoline + 4)) == ((uintptr_t)target + num_copy_bytes))
            {
                /* TODO delete absolute jump if there's any */

                /* Change protection */
                uint32_t old_protection;
                void* base_addr = target;
                size_t num_copy_bytes_tmp = num_copy_bytes;
                if (NtProtectVirtualMemory((void*)(-1), &base_addr, (uint32_t*)&num_copy_bytes_tmp, PAGE_EXECUTE_READWRITE, &old_protection))
                    return false;

                /* Restore bytes */
                uint8_t* original_bytes = (uint8_t*)hook_data + sizeof(_nmd_hook_data);
                size_t i = 0;
                for (; i < num_copy_bytes; i++)
                    ((uint8_t*)target)[i] = original_bytes[i];

                /* Restore protection */
                base_addr = target;
                num_copy_bytes_tmp = num_copy_bytes;
                NtProtectVirtualMemory((void*)(-1), &base_addr, (uint32_t*)&num_copy_bytes_tmp, old_protection, &old_protection);

                hook_data->type = _NMD_HOOK_DATA_TYPE_NONE;

                /* Try to merge */
                _nmd_hook_data* next_hook_data = (_nmd_hook_data*)((uint8_t*)hook_data + sizeof(_nmd_hook_data) + hook_data->size);
                if ((uintptr_t)next_hook_data & 0xfff && next_hook_data->type == _NMD_HOOK_DATA_TYPE_NONE)
                    hook_data->size += sizeof(_nmd_hook_data) + next_hook_data->size;

                return true;
            }
        }

        hook_data = (_nmd_hook_data*)((uint8_t*)hook_data + sizeof(_nmd_hook_data) + hook_data->size);            
    } while ((uintptr_t)hook_data & 0xfff);

    return false;
}

/* Unhooks a function. Returns true if successful, false otherwise.
Parameters:
 - target [in] The function to be unhooked.
 */
bool nmd_unhook(void* target)
{
    _nmd_hook_page* hook_page = _nmd_first_hook_page;
    if (!hook_page)
        return false;

    while (!_nmd_unhook_page(target, hook_page))
    {
        if (!hook_page->next)
            return false;

        hook_page = hook_page->next;
    }

    return true;
}

#ifndef _WIN64
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
#ifdef _WIN64
#ifndef _MSC_VER
    _asm
    {
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
    }
#else
    return 0; /* hack for visual studio */
#endif /* _MSC_VER */
#else
    _asm
    {
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
    }
#endif
}

/* Performs a system call using the specified id.
Be aware: On WoW64 the syscall may expect structures with 8-byte sizes(such as pointers and SIZE_T).
Also, on WoW64 every parameter after the fourth must be 8-byte long.
Example: nmd_syscall(0x1234, arg1, arg2, arg3, arg4, (uint64_t)arg5, (uint64_t)arg6)
Parameters:
 - id  [in] The syscall id.
 - ... [in] The syscall's parameters.
*/
_NMD_NAKED NTSTATUS nmd_syscall(size_t id, ...)
{
#ifdef _WIN64
#ifndef _MSC_VER
    _asm
    {
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
    }
#else
    return 0; /* hack for visual studio */
#endif /* _MSC_VER */
#else
    _asm
    {
        mov eax, [esp]          ; Save return value
        mov [esp - 0x10], eax   ;
        mov eax, [esp + 4]      ; Set syscall index
        add esp, 8              ; Adjust parameters
        call _nmd_wow64_syscall ; Execute syscall
        sub esp, 8              ; Restore stack
        mov ecx, [esp - 0x10]   ; Restore return value
        mov [esp], ecx          ;
        ret                     ; Return
    }
#endif
}

/* Calls a function written for x86-64
Be aware: On WoW64 the function may expect structures with 8-byte sizes(such as pointers and size_t).
Also, on WoW64 every parameter must be 8-byte long.
Example: nmd_syscall(some_x64_function, (uint64_t)arg1, (uint64_t)arg2, (uint64_t)arg3, (uint64_t)arg4, (uint64_t)arg5, (uint64_t)arg6)
Parameters:
 - func [in] The function to be called.
 - ...  [in] The function's parameters.
*/
_NMD_NAKED int64_t nmd_call_x64(void* func, ...)
{
#ifdef _WIN64
    return 0;
#else
    _asm
    {
        ; Transition to x86-64
        push 0x33
        _NMD_DB(0xe8) _NMD_DB(0x00) _NMD_DB(0x00) _NMD_DB(0x00) _NMD_DB(0x00) ; call $+0
        _NMD_DB(0x83) _NMD_DB(0x04) _NMD_DB(0x24) _NMD_DB(0x05)               ; add dword ptr[esp], 5
        retf

        mov rax, func
        call rax

        ; Transition back to x86-32
        _NMD_DB(0xe8) _NMD_DB(0x00) _NMD_DB(0x00) _NMD_DB(0x00) _NMD_DB(0x00) ; call $+0
        _NMD_DB(0xC7) _NMD_DB(0x44) _NMD_DB(0x24) _NMD_DB(0x04) _NMD_DB(0x23) _NMD_DB(0x00) _NMD_DB(0x20) _NMD_DB(0x00) ; mov dword ptr[rsp + 0x4], 0x23
        _NMD_DB(0x83) _NMD_DB(0x04) _NMD_DB(0x24) _NMD_DB(0x0d)               ; add dword ptr[rsp], 13
        retf

        ret                     ; Return
    }
#endif
}

/* Calls a function written for x86-32
Parameters:
 - func [in] The function to be called.
 - ...  [in] The function's parameters.
*/
_NMD_NAKED int64_t nmd_call_x86(size_t id, ...)
{

}

#endif /* NMD_MEMORY_IMPLEMENTATION */

#endif /* NMD_MEMORY_H */