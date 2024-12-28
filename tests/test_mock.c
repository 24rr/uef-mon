#include <stdio.h>
#include "mock_uefi.h"

// Test helper function to print a variable's contents
void PrintVariable(CHAR16 *Name, EFI_GUID *Guid) {
    UINT32 Attributes;
    UINTN DataSize = 1024;
    UINT8 Data[1024];
    
    EFI_STATUS Status = MockGetVariable(Name, Guid, &Attributes, &DataSize, Data);
    if (Status == EFI_SUCCESS) {
        printf("Variable: %ls\n", Name);
        printf("  Size: %llu\n", DataSize);
        printf("  Data: ");
        for (UINTN i = 0; i < DataSize; i++) {
            printf("%02x ", Data[i]);
        }
        printf("\n");
    } else {
        printf("Failed to get variable %ls (Status: %d)\n", Name, Status);
    }
}

int main() {
    printf("Starting UEFI Variable Recovery Tool Tests\n");
    printf("=========================================\n\n");

    // Initialize mock environment
    InitializeMockEnvironment();

    // Test 1: Read existing variables
    printf("Test 1: Reading existing variables\n");
    EFI_GUID TestGuid1 = {0};
    memset(&TestGuid1, 1, sizeof(EFI_GUID));
    PrintVariable(L"BootOrder", &TestGuid1);

    EFI_GUID TestGuid2 = {0};
    memset(&TestGuid2, 2, sizeof(EFI_GUID));
    PrintVariable(L"SecureBoot", &TestGuid2);

    // Test 2: Modify a variable
    printf("\nTest 2: Modifying a variable\n");
    UINT8 NewData[] = {5, 4, 3, 2};
    MockSetVariable(L"BootOrder", &TestGuid1, 1, sizeof(NewData), NewData);
    PrintVariable(L"BootOrder", &TestGuid1);

    // Test 3: Create a new variable
    printf("\nTest 3: Creating a new variable\n");
    UINT8 TestData[] = {0xAA, 0xBB, 0xCC};
    EFI_GUID TestGuid3 = {0};
    memset(&TestGuid3, 3, sizeof(EFI_GUID));
    MockSetVariable(L"TestVar", &TestGuid3, 1, sizeof(TestData), TestData);
    PrintVariable(L"TestVar", &TestGuid3);

    // Test 4: Try to read non-existent variable
    printf("\nTest 4: Reading non-existent variable\n");
    PrintVariable(L"NonExistentVar", &TestGuid1);

    // Cleanup
    CleanupMockEnvironment();

    printf("\nAll tests completed!\n");
    return 0;
} 