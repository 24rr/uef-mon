#include "../include/recovery.h"
#include <Library/FileHandleLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

// Global variables
static EFI_SYSTEM_TABLE *gST = NULL;
static EFI_RUNTIME_SERVICES *gRT = NULL;
static EFI_BOOT_SERVICES *gBS = NULL;

// Structure for backup file header
typedef struct {
    CHAR16 VariableName[256];
    EFI_GUID VendorGuid;
    UINT32 Attributes;
    UINTN DataSize;
} VARIABLE_BACKUP_HEADER;

// Helper function to open a file
static
EFI_STATUS
OpenFile (
    IN CONST CHAR16 *FilePath,
    IN UINT64 OpenMode,
    OUT EFI_FILE_PROTOCOL **File
    )
{
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_FILE_PROTOCOL *Root;
    
    // Get the file system protocol
    Status = gBS->LocateProtocol(
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        (VOID**)&FileSystem
        );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Open root directory
    Status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Open the file
    Status = Root->Open(
        Root,
        File,
        (CHAR16*)FilePath,
        OpenMode,
        0
        );

    Root->Close(Root);
    return Status;
}

EFI_STATUS
EFIAPI
InitializeRecovery (
    VOID
    )
{
    // Get system table pointers
    gST = gST;
    if (gST == NULL) {
        return EFI_NOT_READY;
    }

    gRT = gST->RuntimeServices;
    gBS = gST->BootServices;

    if (gRT == NULL || gBS == NULL) {
        return EFI_NOT_READY;
    }

    return EFI_SUCCESS;
}

RECOVERY_STATUS
EFIAPI
BackupVariable (
    IN CONST CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN CONST CHAR16 *BackupPath
    )
{
    EFI_STATUS Status;
    UINT32 Attributes = 0;
    UINTN DataSize = 0;
    VOID *Data = NULL;
    EFI_FILE_PROTOCOL *BackupFile = NULL;
    VARIABLE_BACKUP_HEADER Header;
    UINTN WriteSize;

    // Get the size of the variable first
    Status = gRT->GetVariable(
        (CHAR16*)VariableName,
        VendorGuid,
        &Attributes,
        &DataSize,
        NULL
        );

    if (Status != EFI_BUFFER_TOO_SMALL) {
        return RECOVERY_ERROR_VARIABLE_NOT_FOUND;
    }

    // Allocate memory for the variable data
    Data = AllocatePool(DataSize);
    if (Data == NULL) {
        return RECOVERY_ERROR_BACKUP_FAILED;
    }

    // Get the actual variable data
    Status = gRT->GetVariable(
        (CHAR16*)VariableName,
        VendorGuid,
        &Attributes,
        &DataSize,
        Data
        );

    if (EFI_ERROR(Status)) {
        FreePool(Data);
        return RECOVERY_ERROR_BACKUP_FAILED;
    }

    // Prepare backup header
    StrCpyS(Header.VariableName, sizeof(Header.VariableName)/sizeof(CHAR16), VariableName);
    CopyMem(&Header.VendorGuid, VendorGuid, sizeof(EFI_GUID));
    Header.Attributes = Attributes;
    Header.DataSize = DataSize;

    // Create backup file
    Status = OpenFile(
        BackupPath,
        EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ,
        &BackupFile
        );

    if (EFI_ERROR(Status)) {
        FreePool(Data);
        return RECOVERY_ERROR_BACKUP_FAILED;
    }

    // Write header
    WriteSize = sizeof(VARIABLE_BACKUP_HEADER);
    Status = BackupFile->Write(BackupFile, &WriteSize, &Header);
    if (EFI_ERROR(Status)) {
        BackupFile->Close(BackupFile);
        FreePool(Data);
        return RECOVERY_ERROR_BACKUP_FAILED;
    }

    // Write data
    WriteSize = DataSize;
    Status = BackupFile->Write(BackupFile, &WriteSize, Data);
    if (EFI_ERROR(Status)) {
        BackupFile->Close(BackupFile);
        FreePool(Data);
        return RECOVERY_ERROR_BACKUP_FAILED;
    }

    BackupFile->Close(BackupFile);
    FreePool(Data);
    return RECOVERY_SUCCESS;
}

RECOVERY_STATUS
EFIAPI
RestoreVariable (
    IN CONST CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN CONST CHAR16 *BackupPath
    )
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL *BackupFile = NULL;
    VARIABLE_BACKUP_HEADER Header;
    VOID *Data = NULL;
    UINTN ReadSize;

    // Open backup file
    Status = OpenFile(BackupPath, EFI_FILE_MODE_READ, &BackupFile);
    if (EFI_ERROR(Status)) {
        return RECOVERY_ERROR_RESTORE_FAILED;
    }

    // Read header
    ReadSize = sizeof(VARIABLE_BACKUP_HEADER);
    Status = BackupFile->Read(BackupFile, &ReadSize, &Header);
    if (EFI_ERROR(Status) || ReadSize != sizeof(VARIABLE_BACKUP_HEADER)) {
        BackupFile->Close(BackupFile);
        return RECOVERY_ERROR_RESTORE_FAILED;
    }

    // Verify variable name and GUID match
    if (StrCmp(Header.VariableName, VariableName) != 0 ||
        CompareMem(&Header.VendorGuid, VendorGuid, sizeof(EFI_GUID)) != 0) {
        BackupFile->Close(BackupFile);
        return RECOVERY_ERROR_RESTORE_FAILED;
    }

    // Allocate memory for data
    Data = AllocatePool(Header.DataSize);
    if (Data == NULL) {
        BackupFile->Close(BackupFile);
        return RECOVERY_ERROR_RESTORE_FAILED;
    }

    // Read data
    ReadSize = Header.DataSize;
    Status = BackupFile->Read(BackupFile, &ReadSize, Data);
    if (EFI_ERROR(Status) || ReadSize != Header.DataSize) {
        BackupFile->Close(BackupFile);
        FreePool(Data);
        return RECOVERY_ERROR_RESTORE_FAILED;
    }

    // Restore variable
    Status = gRT->SetVariable(
        Header.VariableName,
        &Header.VendorGuid,
        Header.Attributes,
        Header.DataSize,
        Data
        );

    BackupFile->Close(BackupFile);
    FreePool(Data);

    return EFI_ERROR(Status) ? RECOVERY_ERROR_RESTORE_FAILED : RECOVERY_SUCCESS;
}

// Structure to hold monitoring context
typedef struct {
    CHAR16 *VariableName;
    EFI_GUID VendorGuid;
    VOID (*CallbackFn)(UEFI_VARIABLE_INFO *OldValue, UEFI_VARIABLE_INFO *NewValue);
    UEFI_VARIABLE_INFO LastValue;
    EFI_EVENT Timer;
} MONITOR_CONTEXT;

// Timer callback for monitoring
static
VOID
EFIAPI
MonitorTimerCallback (
    IN EFI_EVENT Event,
    IN VOID *Context
    )
{
    MONITOR_CONTEXT *MonitorCtx = (MONITOR_CONTEXT*)Context;
    EFI_STATUS Status;
    UINT32 Attributes;
    UINTN DataSize = 0;
    VOID *Data = NULL;
    UEFI_VARIABLE_INFO NewValue;

    // Get current variable size
    Status = gRT->GetVariable(
        MonitorCtx->VariableName,
        &MonitorCtx->VendorGuid,
        &Attributes,
        &DataSize,
        NULL
        );

    if (Status != EFI_BUFFER_TOO_SMALL) {
        return;
    }

    // Allocate memory for data
    Data = AllocatePool(DataSize);
    if (Data == NULL) {
        return;
    }

    // Get variable data
    Status = gRT->GetVariable(
        MonitorCtx->VariableName,
        &MonitorCtx->VendorGuid,
        &Attributes,
        &DataSize,
        Data
        );

    if (EFI_ERROR(Status)) {
        FreePool(Data);
        return;
    }

    // Check if value changed
    if (DataSize != MonitorCtx->LastValue.DataSize ||
        CompareMem(Data, MonitorCtx->LastValue.Data, DataSize) != 0) {
        // Prepare new value info
        NewValue.Name = MonitorCtx->VariableName;
        NewValue.Guid = MonitorCtx->VendorGuid;
        NewValue.Attributes = Attributes;
        NewValue.DataSize = DataSize;
        NewValue.Data = Data;

        // Call callback
        MonitorCtx->CallbackFn(&MonitorCtx->LastValue, &NewValue);

        // Update last value
        FreePool(MonitorCtx->LastValue.Data);
        MonitorCtx->LastValue.Data = AllocatePool(DataSize);
        if (MonitorCtx->LastValue.Data != NULL) {
            MonitorCtx->LastValue.DataSize = DataSize;
            MonitorCtx->LastValue.Attributes = Attributes;
            CopyMem(MonitorCtx->LastValue.Data, Data, DataSize);
        }
    }

    FreePool(Data);
}

RECOVERY_STATUS
EFIAPI
MonitorVariable (
    IN CONST CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN VOID (*CallbackFn)(UEFI_VARIABLE_INFO *OldValue, UEFI_VARIABLE_INFO *NewValue)
    )
{
    EFI_STATUS Status;
    MONITOR_CONTEXT *MonitorCtx;
    UINT32 Attributes;
    UINTN DataSize = 0;

    // Allocate and initialize monitor context
    MonitorCtx = AllocatePool(sizeof(MONITOR_CONTEXT));
    if (MonitorCtx == NULL) {
        return RECOVERY_ERROR_STORAGE_INIT_FAILED;
    }

    // Initialize context
    MonitorCtx->VariableName = AllocatePool(StrSize(VariableName));
    if (MonitorCtx->VariableName == NULL) {
        FreePool(MonitorCtx);
        return RECOVERY_ERROR_STORAGE_INIT_FAILED;
    }
    StrCpyS(MonitorCtx->VariableName, StrSize(VariableName)/sizeof(CHAR16), VariableName);
    CopyMem(&MonitorCtx->VendorGuid, VendorGuid, sizeof(EFI_GUID));
    MonitorCtx->CallbackFn = CallbackFn;

    // Get initial variable value
    Status = gRT->GetVariable(VariableName, VendorGuid, &Attributes, &DataSize, NULL);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        FreePool(MonitorCtx->VariableName);
        FreePool(MonitorCtx);
        return RECOVERY_ERROR_VARIABLE_NOT_FOUND;
    }

    MonitorCtx->LastValue.Data = AllocatePool(DataSize);
    if (MonitorCtx->LastValue.Data == NULL) {
        FreePool(MonitorCtx->VariableName);
        FreePool(MonitorCtx);
        return RECOVERY_ERROR_STORAGE_INIT_FAILED;
    }

    Status = gRT->GetVariable(
        VariableName,
        VendorGuid,
        &Attributes,
        &DataSize,
        MonitorCtx->LastValue.Data
        );

    if (EFI_ERROR(Status)) {
        FreePool(MonitorCtx->LastValue.Data);
        FreePool(MonitorCtx->VariableName);
        FreePool(MonitorCtx);
        return RECOVERY_ERROR_VARIABLE_NOT_FOUND;
    }

    MonitorCtx->LastValue.Name = MonitorCtx->VariableName;
    MonitorCtx->LastValue.Guid = *VendorGuid;
    MonitorCtx->LastValue.Attributes = Attributes;
    MonitorCtx->LastValue.DataSize = DataSize;

    // Create periodic timer for monitoring
    Status = gBS->CreateEvent(
        EVT_TIMER | EVT_NOTIFY_SIGNAL,
        TPL_CALLBACK,
        MonitorTimerCallback,
        MonitorCtx,
        &MonitorCtx->Timer
        );

    if (EFI_ERROR(Status)) {
        FreePool(MonitorCtx->LastValue.Data);
        FreePool(MonitorCtx->VariableName);
        FreePool(MonitorCtx);
        return RECOVERY_ERROR_STORAGE_INIT_FAILED;
    }

    // Start timer (check every second)
    Status = gBS->SetTimer(
        MonitorCtx->Timer,
        TimerPeriodic,
        10000000  // 1 second in 100ns units
        );

    if (EFI_ERROR(Status)) {
        gBS->CloseEvent(MonitorCtx->Timer);
        FreePool(MonitorCtx->LastValue.Data);
        FreePool(MonitorCtx->VariableName);
        FreePool(MonitorCtx);
        return RECOVERY_ERROR_STORAGE_INIT_FAILED;
    }

    return RECOVERY_SUCCESS;
} 