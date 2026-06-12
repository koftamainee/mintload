#include "test_mmesh.h"
#include "test_mmat.h"
#include <stdio.h>

int main() {
    run_mmesh_tests();
    run_mmat_tests();
    printf("All tests passed.\n");
    return 0;
}
