#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/BaseMemoryLib.h>
#include "../include/recovery.h"

// Global GUID for system configuration variables
static EFI_GUID gEfiGlobalVariableGuid = EFI_GLOBAL_VARIABLE;

// Default backup directory
static CONST CHAR16 *DEFAULT_BACKUP_DIR = L"\\EFI\\UefMon\\backup\\";

// Callback function for variable monitoring
static
VOID
EFIAPI
VariableChangeCallback (
    UEFI_VARIABLE_INFO *OldValue,
    UEFI_VARIABLE_INFO *NewValue
    )
{
    Print(L"Variable '%s' changed:\n", NewValue->Name);
    Print(L"  Old size: %d, New size: %d\n", OldValue->DataSize, NewValue->DataSize);
    
    // Automatically backup the changed variable
    CHAR16 BackupPath[256];
    UnicodeSPrint(BackupPath, sizeof(BackupPath), L"%s%s.bin", DEFAULT_BACKUP_DIR, NewValue->Name);
    
    RECOVERY_STATUS Status = BackupVariable(NewValue->Name, &NewValue->Guid, BackupPath);
    if (Status == RECOVERY_SUCCESS) {
        Print(L"  Automatic backup created at %s\n", BackupPath);
    } else {
        Print(L"  Failed to create automatic backup\n");
    }
}

// Function to backup all UEFI variables
static
VOID
BackupAllVariables (
    VOID
    )
{
    EFI_STATUS Status;
    UINTN VariableNameSize;
    CHAR16 VariableName[256];
    EFI_GUID VendorGuid;
    CHAR16 BackupPath[256];
    BOOLEAN KeepGoing = TRUE;

    Print(L"Starting backup of all UEFI variables...\n");

    // Start with empty string to get the first variable
    VariableName[0] = L'\0';
    
    while (KeepGoing) {
        VariableNameSize = sizeof(VariableName);
        Status = gRT->GetNextVariableName(
            &VariableNameSize,
            VariableName,
            &VendorGuid
            );

        if (Status == EFI_NOT_FOUND) {
            // No more variables
            break;
        } else if (EFI_ERROR(Status)) {
            Print(L"Error getting next variable: %r\n", Status);
            break;
        }

        // Create backup path for this variable
        UnicodeSPrint(BackupPath, sizeof(BackupPath), L"%s%s.bin", DEFAULT_BACKUP_DIR, VariableName);

        // Backup the variable
        RECOVERY_STATUS RecoveryStatus = BackupVariable(VariableName, &VendorGuid, BackupPath);
        if (RecoveryStatus == RECOVERY_SUCCESS) {
            Print(L"Backed up: %s\n", VariableName);
        } else {
            Print(L"Failed to backup: %s\n", VariableName);
        }
    }

    Print(L"Backup complete!\n");
}

// Function to restore a variable from backup
static
VOID
RestoreVariableFromBackup (
    VOID
    )
{
    CHAR16 VariableName[256];
    CHAR16 BackupPath[256];
    UINTN MaxLen = sizeof(VariableName) / sizeof(CHAR16);

    Print(L"Enter variable name to restore: ");
    gST->ConIn->Reset(gST->ConIn, FALSE);
    ShellPromptForResponse(ShellPromptResponseTypeBuffer, L"", VariableName, MaxLen);

    // Create backup path
    UnicodeSPrint(BackupPath, sizeof(BackupPath), L"%s%s.bin", DEFAULT_BACKUP_DIR, VariableName);

    // Restore the variable
    RECOVERY_STATUS Status = RestoreVariable(VariableName, &gEfiGlobalVariableGuid, BackupPath);
    if (Status == RECOVERY_SUCCESS) {
        Print(L"Successfully restored %s\n", VariableName);
    } else {
        Print(L"Failed to restore %s\n", VariableName);
    }
}

// Function to start variable monitoring
static
VOID
StartVariableMonitoring (
    VOID
    )
{
    CHAR16 VariableName[256];
    UINTN MaxLen = sizeof(VariableName) / sizeof(CHAR16);

    Print(L"Enter variable name to monitor: ");
    gST->ConIn->Reset(gST->ConIn, FALSE);
    ShellPromptForResponse(ShellPromptResponseTypeBuffer, L"", VariableName, MaxLen);

    // Start monitoring
    RECOVERY_STATUS Status = MonitorVariable(VariableName, &gEfiGlobalVariableGuid, VariableChangeCallback);
    if (Status == RECOVERY_SUCCESS) {
        Print(L"Now monitoring %s for changes\n", VariableName);
        Print(L"(Changes will be automatically backed up)\n");
    } else {
        Print(L"Failed to start monitoring %s\n", VariableName);
    }
}

/**
  The application entry point.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point executed successfully.
  @retval other            Some error occurred.
**/
EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
    )
{
    EFI_STATUS Status;
    
    // Initialize the recovery subsystem
    Status = InitializeRecovery();
    if (EFI_ERROR(Status)) {
        Print(L"Failed to initialize recovery system: %r\n", Status);
        return Status;
    }

    // Simple menu loop
    BOOLEAN Exit = FALSE;
    UINTN Index;
    
    while (!Exit) {
        Print(L"\n=== UEF-Mon: UEFI Variable Recovery Tool ===\n");
        Print(L"1. Backup UEFI Variables\n");
        Print(L"2. Restore UEFI Variables\n");
        Print(L"3. Monitor Variables\n");
        Print(L"4. Exit\n");
        Print(L"\nSelect an option (1-4): ");

        // Get user input
        Status = SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
        if (EFI_ERROR(Status)) {
            continue;
        }

        EFI_INPUT_KEY Key;
        while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY);

        // Convert key to number and process
        if (Key.UnicodeChar >= '1' && Key.UnicodeChar <= '4') {
            Print(L"%c\n", Key.UnicodeChar);
            switch (Key.UnicodeChar) {
                case '1':
                    BackupAllVariables();
                    break;
                case '2':
                    RestoreVariableFromBackup();
                    break;
                case '3':
                    StartVariableMonitoring();
                    break;
                case '4':
                    Exit = TRUE;
                    break;
            }
        }
    }

    return EFI_SUCCESS;
} 