/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Setup Library
 * FILE:            base/setup/lib/bldrsup.h
 * PURPOSE:         NT 5.x family (MS Windows <= 2003, and ReactOS)
 *                  boot loaders management.
 * PROGRAMMER:      Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

// TODO: Add support for NT 6.x family! (detection + BCD manipulation).

#pragma once

typedef enum _NTOS_BOOT_LOADER_TYPE     // _BOOT_STORE_TYPE
{
    FreeLdr,    // ReactOS' FreeLoader
    NtLdr,      // Windows <= 2k3 NT "FlexBoot" OS Loader NTLDR
//  BootMgr,    // Vista+ BCD-oriented BOOTMGR
    BldrTypeMax
} NTOS_BOOT_LOADER_TYPE;

/*
 * Some references about EFI boot entries:
 * https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/overview-of-boot-options-in-efi
 * https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/identifying-backup-files-for-existing-boot-entries
 */

/*
 * This structure is inspired from the EFI boot entry structure
 * BOOT_OPTIONS that is defined in ndk/iotypes.h .
 */
typedef struct _NTOS_BOOT_OPTIONS       // _BOOT_STORE_OPTIONS
{
    // ULONG Version;
    // ULONG Length;
    ULONG Timeout;
    ULONG_PTR CurrentBootEntryKey;
    // ULONG_PTR NextBootEntryKey;
    // WCHAR HeadlessRedirection[1];
} NTOS_BOOT_OPTIONS, *PNTOS_BOOT_OPTIONS;

/*
 * These macros are used to set a value for the BootEntryKey member of a
 * NTOS_BOOT_ENTRY structure, much in the same idea as MAKEINTRESOURCE and
 * IS_INTRESOURCE macros for Win32 resources.
 *
 * A key consists of either a boot ID number,
 * comprised between 0 and MAX_USHORT == 0xFFFF == 65535, or can be a pointer
 * to a human-readable string (section name), as in the case of FreeLDR, or
 * to a GUID, as in the case of BOOTMGR.
 *
 * If IS_INTKEY(BootEntryKey) == TRUE, i.e. the key is <= 65535, this means
 * the key is a boot ID number, otherwise it is typically a pointer to a string.
 */
#define MAKESTRKEY(i)   ((ULONG_PTR)(i))
#define MAKEINTKEY(i)   ((ULONG_PTR)((USHORT)(i)))
#define IS_INTKEY(i)    (((ULONG_PTR)(i) >> 16) == 0)

/*
 * This structure is inspired from the EFI boot entry structures
 * BOOT_ENTRY and FILE_PATH that are defined in ndk/iotypes.h .
 */
typedef struct _NTOS_BOOT_ENTRY         // _BOOT_STORE_ENTRY
{
    // ULONG Version; // Equivalent of the "BootType" in FreeLdr
    PWCHAR Version; // HACK!!!
    // ULONG Length;
    ULONG_PTR BootEntryKey; // Boot entry "key"
    PCWSTR FriendlyName;    // Human-readable boot entry description        // LoadIdentifier
    PCWSTR BootFilePath;    // Path to e.g. osloader.exe, or winload.efi    // EfiOsLoaderFilePath
    ULONG OsOptionsLength;  // Loader-specific options blob (can be a string, or a binary structure...)
    UCHAR OsOptions[ANYSIZE_ARRAY];
/*
 * In packed form, this structure would contain offsets to 'FriendlyName'
 * and 'BootFilePath' strings and, after the OsOptions blob, there would
 * be the following data:
 *
 *  WCHAR FriendlyName[ANYSIZE_ARRAY];
 *  FILE_PATH BootFilePath;
 */
} NTOS_BOOT_ENTRY, *PNTOS_BOOT_ENTRY;

/* "NTOS" (aka. ReactOS or MS Windows NT) <= 5.x options */
typedef struct _NTOS_OPTIONS
{
    UCHAR Signature[8];     // "NTOS_5\0\0"
    // ULONG Version;
    // ULONG Length;
    PCWSTR OsLoadPath;      // The OS SystemRoot path                       // OsLoaderFilePath // OsFilePath
    PCWSTR OsLoadOptions;                                                   // OsLoadOptions
/*
 * In packed form, this structure would contain an offset to the 'OsLoadPath'
 * string, and the 'OsLoadOptions' member would be:
 *  WCHAR OsLoadOptions[ANYSIZE_ARRAY];
 * followed by:
 *  FILE_PATH OsLoadPath;
 */
} NTOS_OPTIONS, *PNTOS_OPTIONS;

#define NTOS_OPTIONS_SIGNATURE "NTOS_5\0\0"

/* Options for boot-sector boot entries */
typedef struct _BOOT_SECTOR_OPTIONS
{
    UCHAR Signature[8];     // "BootSect"
    // ULONG Version;
    // ULONG Length;
    PCWSTR Drive;
    PCWSTR Partition;
    PCWSTR BootSectorFileName;
} BOOT_SECTOR_OPTIONS, *PBOOT_SECTOR_OPTIONS;

#define BOOT_SECTOR_OPTIONS_SIGNATURE "BootSect"


typedef NTSTATUS
(NTAPI *PENUM_BOOT_ENTRIES_ROUTINE)(
    IN NTOS_BOOT_LOADER_TYPE Type,
    IN PNTOS_BOOT_ENTRY BootEntry,
    IN PVOID Parameter OPTIONAL);


NTSTATUS
FindNTOSBootLoader( // By handle
    IN HANDLE PartitionDirectoryHandle, // OPTIONAL
    IN NTOS_BOOT_LOADER_TYPE Type,
    OUT PULONG Version);


NTSTATUS
OpenNTOSBootLoaderStoreByHandle(
    OUT PVOID* Handle,
    IN HANDLE PartitionDirectoryHandle, // OPTIONAL
    IN NTOS_BOOT_LOADER_TYPE Type,
    IN BOOLEAN CreateNew);

NTSTATUS
OpenNTOSBootLoaderStore_UStr(
    OUT PVOID* Handle,
    IN PUNICODE_STRING SystemPartitionPath,
    IN NTOS_BOOT_LOADER_TYPE Type,
    IN BOOLEAN CreateNew);

NTSTATUS
OpenNTOSBootLoaderStore(
    OUT PVOID* Handle,
    IN PCWSTR SystemPartition,
    IN NTOS_BOOT_LOADER_TYPE Type,
    IN BOOLEAN CreateNew);

NTSTATUS
CloseNTOSBootLoaderStore(
    IN PVOID Handle);

NTSTATUS
AddNTOSBootEntry(
    IN PVOID Handle,
    IN PNTOS_BOOT_ENTRY BootEntry,
    IN ULONG_PTR BootEntryKey);

NTSTATUS
DeleteNTOSBootEntry(
    IN PVOID Handle,
    IN ULONG_PTR BootEntryKey);

NTSTATUS
ModifyNTOSBootEntry(
    IN PVOID Handle,
    IN PNTOS_BOOT_ENTRY BootEntry);

NTSTATUS
QueryNTOSBootEntry(
    IN PVOID Handle,
    IN ULONG_PTR BootEntryKey,
    OUT PNTOS_BOOT_ENTRY BootEntry); // Technically this should be PNTOS_BOOT_ENTRY*

NTSTATUS
QueryNTOSBootOptions(
    IN PVOID Handle,
    IN OUT PNTOS_BOOT_OPTIONS BootOptions
/* , IN PULONG BootOptionsLength */ );

NTSTATUS
SetNTOSBootOptions(
    IN PVOID Handle,
    IN PNTOS_BOOT_OPTIONS BootOptions,
    IN ULONG FieldsToChange);

NTSTATUS
EnumerateNTOSBootEntries(
    IN PVOID Handle,
//  IN ULONG Flags, // Determine which data to retrieve
    IN PENUM_BOOT_ENTRIES_ROUTINE EnumBootEntriesRoutine,
    IN PVOID Parameter OPTIONAL);

/* EOF */
