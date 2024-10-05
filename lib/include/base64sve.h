#include <stdio.h>
#include <arm_sve.h>

void test(void);

void vec_add_sve_intrinsics(int *restrict out, int *restrict a, int *restrict b, int length);
