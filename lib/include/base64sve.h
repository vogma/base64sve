#include <stdio.h>

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif /* __ARM_FEATURE_SVE */

#include <stdint.h>

void base64_encode(void *input, char *output, size_t length);
void base64_decode(char *base64_data, void *output, size_t encoded_length, size_t *decoded_length);