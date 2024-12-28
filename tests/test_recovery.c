#include "test_recovery.h"
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UnitTestLib.h>

// Test data
static CHAR16 *TEST_VARIABLE_NAME = L"TestVar";
static EFI_GUID TEST_VENDOR_GUID = {
    0x12345678, 0x1234, 0x1234, {0x12, 0x34, 0x12, 0x34, 0x12, 0x34, 0x12, 0x34}
};
static CHAR16 *TEST_BACKUP_PATH = L"\\EFI\\UefMon\\backup\\test.bin";
static UINT8 TEST_DATA[] = {0x01, 0x02, 0x03, 0x04};
static UINTN TEST_DATA_SIZE = sizeof(TEST_DATA);

// Helper function to set up test variable
static
EFI_STATUS
SetupTestVariable (
    VOID
    )
{
    return gRT->SetVariable(
        TEST_VARIABLE_NAME,
        &TEST_VENDOR_GUID,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        TEST_DATA_SIZE,
        TEST_DATA
        );
}

// Helper function to clean up test variable
static
EFI_STATUS
CleanupTestVariable (
    VOID
    )
{
    return gRT->SetVariable(
        TEST_VARIABLE_NAME,
        &TEST_VENDOR_GUID,
        0,
        0,
        NULL
        );
}

// Test case for basic backup functionality
EFI_STATUS
EFIAPI
TestBackupVariableBasic (
    IN UNIT_TEST_CONTEXT           Context
    )
{
    EFI_STATUS Status;
    RECOVERY_STATUS RecoveryStatus;

    // Set up test variable
    Status = SetupTestVariable();
    UT_ASSERT_NOT_EFI_ERROR(Status);

    // Test backup functionality
    RecoveryStatus = BackupVariable(
        TEST_VARIABLE_NAME,
        &TEST_VENDOR_GUID,
        TEST_BACKUP_PATH
        );
    UT_ASSERT_EQUAL(RecoveryStatus, RECOVERY_SUCCESS);

    // Clean up
    Status = CleanupTestVariable();
    UT_ASSERT_NOT_EFI_ERROR(Status);

    return EFI_SUCCESS;
}

// Test case for basic restore functionality
EFI_STATUS
EFIAPI
TestRestoreVariableBasic (
    IN UNIT_TEST_CONTEXT           Context
    )
{
    EFI_STATUS Status;
    RECOVERY_STATUS RecoveryStatus;
    UINT8 ReadData[sizeof(TEST_DATA)];
    UINTN DataSize = sizeof(ReadData);
    UINT32 Attributes;

    // Set up test variable and back it up
    Status = SetupTestVariable();
    UT_ASSERT_NOT_EFI_ERROR(Status);

    RecoveryStatus = BackupVariable(TEST_VARIABLE_NAME, &TEST_VENDOR_GUID, TEST_BACKUP_PATH);
    UT_ASSERT_EQUAL(RecoveryStatus, RECOVERY_SUCCESS);

    // Delete the variable
    Status = CleanupTestVariable();
    UT_ASSERT_NOT_EFI_ERROR(Status);

    // Test restore functionality
    RecoveryStatus = RestoreVariable(TEST_VARIABLE_NAME, &TEST_VENDOR_GUID, TEST_BACKUP_PATH);
    UT_ASSERT_EQUAL(RecoveryStatus, RECOVERY_SUCCESS);

    // Verify restored data
    Status = gRT->GetVariable(
        TEST_VARIABLE_NAME,
        &TEST_VENDOR_GUID,
        &Attributes,
        &DataSize,
        ReadData
        );
    UT_ASSERT_NOT_EFI_ERROR(Status);
    UT_ASSERT_EQUAL(DataSize, TEST_DATA_SIZE);
    UT_ASSERT_MEM_EQUAL(ReadData, TEST_DATA, TEST_DATA_SIZE);

    // Clean up
    Status = CleanupTestVariable();
    UT_ASSERT_NOT_EFI_ERROR(Status);

    return EFI_SUCCESS;
}

// Callback function for monitor test
static
VOID
EFIAPI
TestMonitorCallback (
    UEFI_VARIABLE_INFO *OldValue,
    UEFI_VARIABLE_INFO *NewValue
    )
{
    // Just verify the values changed
    UT_ASSERT_NOT_EQUAL(OldValue->DataSize, NewValue->DataSize);
}

// Test case for basic monitoring functionality
EFI_STATUS
EFIAPI
TestMonitorVariableBasic (
    IN UNIT_TEST_CONTEXT           Context
    )
{
    EFI_STATUS Status;
    RECOVERY_STATUS RecoveryStatus;
    UINT8 NewData[] = {0x05, 0x06, 0x07, 0x08};

    // Set up initial test variable
    Status = SetupTestVariable();
    UT_ASSERT_NOT_EFI_ERROR(Status);

    // Start monitoring
    RecoveryStatus = MonitorVariable(
        TEST_VARIABLE_NAME,
        &TEST_VENDOR_GUID,
        TestMonitorCallback
        );
    UT_ASSERT_EQUAL(RecoveryStatus, RECOVERY_SUCCESS);

    // Change the variable to trigger monitoring
    Status = gRT->SetVariable(
        TEST_VARIABLE_NAME,
        &TEST_VENDOR_GUID,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        sizeof(NewData),
        NewData
        );
    UT_ASSERT_NOT_EFI_ERROR(Status);

    // Clean up
    Status = CleanupTestVariable();
    UT_ASSERT_NOT_EFI_ERROR(Status);

    return EFI_SUCCESS;
}

// Create and initialize the test suite
EFI_STATUS
EFIAPI
CreateRecoveryTestSuite (
    IN UNIT_TEST_FRAMEWORK_HANDLE  Framework
    )
{
    EFI_STATUS Status;
    UNIT_TEST_SUITE_HANDLE RecoverySuite;

    Status = CreateUnitTestSuite(
        &RecoverySuite,
        Framework,
        L"UEFI Variable Recovery Tests",
        L"UefMon.Recovery",
        NULL,
        NULL
        );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Add test cases to the suite
    AddTestCase(
        RecoverySuite,
        L"Test Basic Variable Backup",
        L"Recovery.Backup.Basic",
        TestBackupVariableBasic,
        NULL,
        NULL,
        NULL
        );

    AddTestCase(
        RecoverySuite,
        L"Test Basic Variable Restore",
        L"Recovery.Restore.Basic",
        TestRestoreVariableBasic,
        NULL,
        NULL,
        NULL
        );

    AddTestCase(
        RecoverySuite,
        L"Test Basic Variable Monitoring",
        L"Recovery.Monitor.Basic",
        TestMonitorVariableBasic,
        NULL,
        NULL,
        NULL
        );

    return EFI_SUCCESS;
} 