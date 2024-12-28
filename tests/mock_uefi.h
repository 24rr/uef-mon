#ifndef MOCK_UEFI_H
#define MOCK_UEFI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock UEFI types
typedef unsigned long long UINTN;
typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef wchar_t CHAR16;
typedef void VOID;
typedef int EFI_STATUS;
typedef struct { UINT8 Data[16]; } EFI_GUID;

#define EFI_SUCCESS 0
#define EFI_NOT_FOUND 1
#define EFI_BUFFER_TOO_SMALL 2

// Mock variable storage
typedef struct {
    CHAR16 Name[256];
    EFI_GUID Guid;
    UINT32 Attributes;
    UINTN DataSize;
    UINT8 Data[1024];
    int IsValid;
} MOCK_VARIABLE;

#define MAX_MOCK_VARIABLES 100
extern MOCK_VARIABLE MockVariables[MAX_MOCK_VARIABLES];
extern int MockVariableCount;

// Mock UEFI functions
EFI_STATUS
MockGetVariable(
    CHAR16 *VariableName,
    EFI_GUID *VendorGuid,
    UINT32 *Attributes,
    UINTN *DataSize,
    VOID *Data
    );

EFI_STATUS
MockSetVariable(
    CHAR16 *VariableName,
    EFI_GUID *VendorGuid,
    UINT32 Attributes,
    UINTN DataSize,
    VOID *Data
    );

// Initialize and cleanup mock environment
void InitializeMockEnvironment(void);
void CleanupMockEnvironment(void);

#endif // MOCK_UEFI_H 