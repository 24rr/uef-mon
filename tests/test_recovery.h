#ifndef UEF_MON_TEST_RECOVERY_H
#define UEF_MON_TEST_RECOVERY_H

#include <Uefi.h>
#include <Library/UnitTestLib.h>
#include "../include/recovery.h"

// Test case entry points
EFI_STATUS
EFIAPI
TestBackupVariableBasic (
    IN UNIT_TEST_CONTEXT           Context
    );

EFI_STATUS
EFIAPI
TestRestoreVariableBasic (
    IN UNIT_TEST_CONTEXT           Context
    );

EFI_STATUS
EFIAPI
TestMonitorVariableBasic (
    IN UNIT_TEST_CONTEXT           Context
    );

// Test suite entry point
EFI_STATUS
EFIAPI
CreateRecoveryTestSuite (
    IN UNIT_TEST_FRAMEWORK_HANDLE  Framework
    );

#endif // UEF_MON_TEST_RECOVERY_H 