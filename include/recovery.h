#ifndef UEF_MON_RECOVERY_H
#define UEF_MON_RECOVERY_H

#include <Uefi.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Protocol/SimpleFileSystem.h>

// Status codes for recovery operations
typedef enum {
    RECOVERY_SUCCESS = 0,
    RECOVERY_ERROR_VARIABLE_NOT_FOUND,
    RECOVERY_ERROR_BACKUP_FAILED,
    RECOVERY_ERROR_RESTORE_FAILED,
    RECOVERY_ERROR_STORAGE_INIT_FAILED
} RECOVERY_STATUS;

// Structure to hold variable information
typedef struct {
    CHAR16 *Name;
    EFI_GUID Guid;
    UINT32 Attributes;
    UINTN DataSize;
    VOID *Data;
} UEFI_VARIABLE_INFO;

// Function prototypes
EFI_STATUS
EFIAPI
InitializeRecovery (
    VOID
    );

RECOVERY_STATUS
EFIAPI
BackupVariable (
    IN CONST CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN CONST CHAR16 *BackupPath
    );

RECOVERY_STATUS
EFIAPI
RestoreVariable (
    IN CONST CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN CONST CHAR16 *BackupPath
    );

RECOVERY_STATUS
EFIAPI
MonitorVariable (
    IN CONST CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN VOID (*CallbackFn)(UEFI_VARIABLE_INFO *OldValue, UEFI_VARIABLE_INFO *NewValue)
    );

#endif // UEF_MON_RECOVERY_H 