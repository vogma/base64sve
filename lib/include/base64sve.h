#include <stdio.h>
#include <arm_sve.h>
#include <stdint.h>

void test(void);

void testShuffle(uint32_t *input, uint32_t *ouput, size_t length);
void vec_add_sve_intrinsics(int *restrict out, int *restrict a, int *restrict b, int length);
