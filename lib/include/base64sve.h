#ifndef LIBB64SVE_H
#define LIBB64SVE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif /* __ARM_FEATURE_SVE */

#include <stdint.h>

    void base64sve_encode(void *input, char *output, size_t length);
    void base64sve_decode(char *base64_data, void *output, size_t encoded_length, size_t *decoded_length);

#ifdef __cplusplus
}
#endif
#endif