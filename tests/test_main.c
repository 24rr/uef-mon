#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UnitTestLib.h>
#include "test_recovery.h"

/**
  The application entry point for UefMon test application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS      The test completed successfully.
  @retval other           Error occurred during testing.
**/
EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
    )
{
    EFI_STATUS Status;
    UNIT_TEST_FRAMEWORK_HANDLE Framework;
    UNIT_TEST_SUITE_HANDLE RecoverySuite;

    Print(L"UefMon Test Application Starting...\n");

    // Create Unit Test Framework
    Status = InitUnitTestFramework(
        &Framework,
        L"UefMon Test Suite",
        gEfiCallerBaseName,
        1  // Major version
        );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to create test framework: %r\n", Status);
        return Status;
    }

    // Create and add the recovery test suite
    Status = CreateRecoveryTestSuite(Framework);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to create recovery test suite: %r\n", Status);
        return Status;
    }

    // Execute the tests
    Status = RunAllTestSuites(Framework);
    if (EFI_ERROR(Status)) {
        Print(L"One or more tests failed: %r\n", Status);
    } else {
        Print(L"All tests passed!\n");
    }

    // Clean up
    FreeUnitTestFramework(Framework);

    return Status;
} 