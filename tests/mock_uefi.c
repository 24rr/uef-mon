#include "mock_uefi.h"

// Global mock variable storage
MOCK_VARIABLE MockVariables[MAX_MOCK_VARIABLES];
int MockVariableCount = 0;

// Initialize mock environment with some test variables
void InitializeMockEnvironment(void) {
    MockVariableCount = 0;

    // Add a test boot variable
    MOCK_VARIABLE *bootVar = &MockVariables[MockVariableCount++];
    wcscpy(bootVar->Name, L"BootOrder");
    memset(&bootVar->Guid, 1, sizeof(EFI_GUID)); // Simple test GUID
    bootVar->Attributes = 1;
    bootVar->DataSize = 4;
    bootVar->Data[0] = 0;
    bootVar->Data[1] = 1;
    bootVar->Data[2] = 2;
    bootVar->Data[3] = 3;
    bootVar->IsValid = 1;

    // Add a test secure boot variable
    MOCK_VARIABLE *secureVar = &MockVariables[MockVariableCount++];
    wcscpy(secureVar->Name, L"SecureBoot");
    memset(&secureVar->Guid, 2, sizeof(EFI_GUID));
    secureVar->Attributes = 1;
    secureVar->DataSize = 1;
    secureVar->Data[0] = 1; // Secure boot enabled
    secureVar->IsValid = 1;
}

// Clean up mock environment
void CleanupMockEnvironment(void) {
    MockVariableCount = 0;
}

// Find a variable in mock storage
static MOCK_VARIABLE* FindVariable(CHAR16 *VariableName, EFI_GUID *VendorGuid) {
    for (int i = 0; i < MockVariableCount; i++) {
        if (MockVariables[i].IsValid &&
            wcscmp(MockVariables[i].Name, VariableName) == 0 &&
            memcmp(&MockVariables[i].Guid, VendorGuid, sizeof(EFI_GUID)) == 0) {
            return &MockVariables[i];
        }
    }
    return NULL;
}

// Mock GetVariable implementation
EFI_STATUS
MockGetVariable(
    CHAR16 *VariableName,
    EFI_GUID *VendorGuid,
    UINT32 *Attributes,
    UINTN *DataSize,
    VOID *Data
    )
{
    MOCK_VARIABLE *var = FindVariable(VariableName, VendorGuid);
    if (var == NULL) {
        return EFI_NOT_FOUND;
    }

    if (*DataSize < var->DataSize) {
        *DataSize = var->DataSize;
        return EFI_BUFFER_TOO_SMALL;
    }

    if (Data != NULL) {
        *Attributes = var->Attributes;
        *DataSize = var->DataSize;
        memcpy(Data, var->Data, var->DataSize);
    }

    return EFI_SUCCESS;
}

// Mock SetVariable implementation
EFI_STATUS
MockSetVariable(
    CHAR16 *VariableName,
    EFI_GUID *VendorGuid,
    UINT32 Attributes,
    UINTN DataSize,
    VOID *Data
    )
{
    if (DataSize > sizeof(MOCK_VARIABLE)) {
        return EFI_BUFFER_TOO_SMALL;
    }

    MOCK_VARIABLE *var = FindVariable(VariableName, VendorGuid);
    if (var == NULL) {
        if (MockVariableCount >= MAX_MOCK_VARIABLES) {
            return EFI_BUFFER_TOO_SMALL;
        }
        var = &MockVariables[MockVariableCount++];
        wcscpy(var->Name, VariableName);
        memcpy(&var->Guid, VendorGuid, sizeof(EFI_GUID));
    }

    var->Attributes = Attributes;
    var->DataSize = DataSize;
    if (DataSize > 0) {
        memcpy(var->Data, Data, DataSize);
    }
    var->IsValid = 1;

    return EFI_SUCCESS;
} 