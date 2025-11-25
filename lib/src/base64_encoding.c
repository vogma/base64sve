#include <base64sve.h>
#include <limits.h>

static const uint8_t encode_lookup_table[] = {1, 0, 2, 1, 4, 3, 5, 4, 7, 6, 8, 7, 10, 9, 11, 10, 13, 12, 14, 13, 16, 15, 17, 16, 19, 18, 20, 19, 22, 21, 23, 22, 25, 24, 26, 25, 28, 27, 29, 28, 31, 30, 32, 31, 34, 33, 35, 34, 37, 36, 38, 37, 40, 39, 41, 40, 43, 42, 44, 43, 46, 45, 47, 46, 49, 48, 50, 49, 52, 51, 53, 52, 55, 54, 56, 55, 58, 57, 59, 58, 61, 60, 62, 61};
static const int8_t offsets[68] = {71, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -19, -16, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static const uint8_t shuffle_table[64] = { 2, 1, 0, 1, 5, 4, 3, 4, 8, 7, 6, 7, 11, 10, 9, 10, 14, 13, 12, 13, 17, 16, 15, 16, 20, 19, 18, 19, 23, 22, 21, 22, 26, 25, 24, 25, 29, 28, 27, 28, 32, 31, 30, 31, 35, 34, 33, 34, 38, 37, 36, 37, 41, 40, 39, 40, 44, 43, 42, 43, 47, 46, 45, 46 };
static const unsigned char b64chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * scalar tail encoding routine
 */
void base64_encode_tail(char *encoded, const void *data, size_t len)
{
    size_t i;
    char *p;

    uint8_t *input = (uint8_t *)data;

    p = encoded;
    for (i = 0; i < len - 2; i += 3)
    {
        *p++ = b64chars[(input[i] >> 2) & 0x3F];
        *p++ = b64chars[((input[i] & 0x3) << 4) |
                        ((int)(input[i + 1] & 0xF0) >> 4)];
        *p++ = b64chars[((input[i + 1] & 0xF) << 2) |
                        ((int)(input[i + 2] & 0xC0) >> 6)];
        *p++ = b64chars[input[i + 2] & 0x3F];
    }
    if (i < len)
    {
        *p++ = b64chars[(input[i] >> 2) & 0x3F];
        if (i == (len - 1))
        {
            *p++ = b64chars[((input[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else
        {
            *p++ = b64chars[((input[i] & 0x3) << 4) |
                            ((int)(input[i + 1] & 0xF0) >> 4)];
            *p++ = b64chars[((input[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }
    *p++ = '\0';
}

void printRegister(svuint8_t vec)
{
    size_t bytes_per_vec = svcntb();
    uint8_t memory[bytes_per_vec];

    svbool_t predicate = svwhilelt_b8(0, (int)bytes_per_vec);
    svst1(predicate, memory, vec);

    // printf("register contents: ");
    for (int i = 0; i < bytes_per_vec; i++)
    {
        printf("0x%02X ", memory[i]);
    }
    printf("\n");
}

void printRegister32(svuint32_t vec)
{
    size_t words_per_vec = svcntw();
    uint32_t memory[words_per_vec];

    svbool_t predicate = svwhilelt_b32(0, (int)words_per_vec);
    svst1(predicate, memory, vec);

    // printf("register contents: ");
    for (int i = 0; i < words_per_vec; i++)
    {
        printf("0x%08X ", memory[i]);
    }
    printf("\n");
}

svuint8_t createLookupIndices(svbool_t predicate32, svbool_t predicate8)
{
    svuint32_t index_reg = svindex_u32(0, 1);
    svuint32_t shift8 = svlsl_m(predicate32, index_reg, 8);
    svuint32_t shift16 = svlsl_m(predicate32, index_reg, 16);
    svuint32_t shift24 = svlsl_m(predicate32, index_reg, 24);

    shift8 = svorr_m(predicate32, shift8, shift16);
    shift8 = svorr_m(predicate32, shift8, shift24);
    index_reg = svorr_m(predicate32, shift8, index_reg);

    const svuint8_t const_vec_3 = svdup_n_u8(3);
    const svuint8_t const_index_vec = svreinterpret_u8(svdup_n_u32(0x01020001));

    svuint8_t index_vec = svmul_m(predicate8, svreinterpret_u8(index_reg), const_vec_3);
    return svadd_m(predicate8, index_vec, const_index_vec);
}

void base64sve_encode_shift(void *input, char *output, size_t length)
{
    // store number of bytes each vector register can hold
    size_t bytes_per_vec = svcntb();

    // calculate how many bytes will be processed per loop iteration
    size_t bytes_per_round = (bytes_per_vec / 4) * 3;

    // set needed predicates
    svbool_t predicate8 = svwhilelt_b8(0, (int)bytes_per_round);
    svbool_t predicateMax = svptrue_b8();
    svbool_t predicate32Max = svptrue_b32();
    svbool_t predicate16Max = svptrue_b16();

    svuint8_t vec_lookup_table = svld1(predicateMax, encode_lookup_table);
    svint8_t offset_vec = svld1(predicateMax, offsets);

    for (; length - 2 >= bytes_per_round; length -= bytes_per_round, output += bytes_per_vec, input += bytes_per_round)
    {

        // load input data into vector register
        svuint8_t vec = svld1(predicate8, (uint8_t *)input);

        vec = svtbl(vec, vec_lookup_table);

        // populate vector registers with shift values
        svuint32_t vec_shift_ac = svdup_u32(0x04000040);
        svuint32_t vec_shift_bd = svdup_u32(0x01000010);

        svuint32_t data = svreinterpret_u32(vec);

        // mask out indices ac and bd
        svuint32_t vec_ac = svand_m(predicate32Max, data, 0x0FC0FC00);
        svuint32_t vec_bd = svand_m(predicate32Max, data, 0x003F03F0);

        svuint16_t vec_shifted_ac = svmulh_m(predicate16Max, svreinterpret_u16(vec_ac), svreinterpret_u16(vec_shift_ac));
        svuint16_t vec_shifted_bd = svmul_m(predicate16Max, svreinterpret_u16(vec_bd), svreinterpret_u16(vec_shift_bd));

        // first step finished
        svuint32_t vec_index = svorr_m(predicate32Max, svreinterpret_u32(vec_shifted_ac), svreinterpret_u32(vec_shifted_bd));

        // saturated substraction
        svuint8_t saturated_vec = svqsub(svreinterpret_u8(vec_index), 51);

        // extract mask of values lower than 26
        svbool_t mask_lower_26 = svcmplt_n_u8(predicateMax, svreinterpret_u8(vec_index), 26);

        // all values, which were lower than 26 will be set to 13
        const svuint8_t vec_lookup = svadd_m(mask_lower_26, saturated_vec, 13);

        // register shuffle with offset vector
        svint8_t shuffled_offset_vec = svtbl(offset_vec, vec_lookup);

        // add offset values to the indices to compute base64 characters without memory lookup
        const svint8_t ascii_vec = svadd_m(predicateMax, svreinterpret_s8(vec_index), shuffled_offset_vec);

        // store whole vector register to memory
        svst1(predicateMax, (int8_t *)output, ascii_vec);
    }
    // because we process 'bytes_per_round' bytes in each loop iteration,
    // there may be more data to be encoded if length % bytes_per_round != 0, this data will be encoded with a scalar encoding algorithm
    if (length != 0)
    {
        base64_encode_tail(output, input, length);
    }
}

void base64sve_encode_bdep(void *input, char *output, size_t length)
{
    // store number of bytes each vector register can hold
    size_t bytes_per_vec = svcntb();

    // calculate how many bytes will be processed per loop iteration
    size_t bytes_per_round = (bytes_per_vec / 4) * 3;

    // set needed predicates
    svbool_t predicate8 = svwhilelt_b8(0, (int)bytes_per_round);
    svbool_t predicateMax = svptrue_b8();
    svbool_t predicate32Max = svptrue_b32();

    svuint8_t vec_lookup_table = svld1(predicateMax, encode_lookup_table);
    svuint8_t shuffle_vec = svld1(predicateMax, shuffle_table);
    svint8_t offset_vec = svld1(predicateMax, offsets);

    svuint32_t pdep_mask_vec = svdup_n_u32(0x3f3f3f3f);

    for (; length - 2 >= bytes_per_round; length -= bytes_per_round, output += bytes_per_vec, input += bytes_per_round)
    {

        // load input data into vector register
        svuint8_t vec = svld1(predicate8, (uint8_t *)input);
        vec = svtbl(vec, shuffle_vec);
        vec = svreinterpret_u8(svbdep(svreinterpret_u32(vec), pdep_mask_vec));
        svuint32_t vec_index = svrevb_m(svdup_n_u32(0x00000000), predicate32Max, svreinterpret_u32(vec));

        // saturated substraction
        svuint8_t saturated_vec = svqsub(svreinterpret_u8(vec_index), 51);

        // extract mask of values lower than 26
        svbool_t mask_lower_26 = svcmplt_n_u8(predicateMax, svreinterpret_u8(vec_index), 26);

        // all values, which were lower than 26 will be set to 13
        const svuint8_t vec_lookup = svadd_m(mask_lower_26, saturated_vec, 13);

        // register shuffle with offset vector
        svint8_t shuffled_offset_vec = svtbl(offset_vec, vec_lookup);

        // add offset values to the indices to compute base64 characters without memory lookup
        const svint8_t ascii_vec = svadd_m(predicateMax, svreinterpret_s8(vec_index), shuffled_offset_vec);

        // store whole vector register to memory
        svst1(predicateMax, (int8_t *)output, ascii_vec);
    }
    // because we process 'bytes_per_round' bytes in each loop iteration,
    // there may be more data to be encoded if length % bytes_per_round != 0, this data will be encoded with a scalar encoding algorithm
    if (length != 0)
    {
        base64_encode_tail(output, input, length);
    }
}

void base64sve_encode_x2(void *input, char *output, size_t length)
{
    // store number of bytes each vector register can hold
    size_t bytes_per_vec = svcntb();
    size_t input_slice = (bytes_per_vec / 4) * 3;

    // calculate how many bytes will be processed per loop iteration
    size_t bytes_per_round = ((bytes_per_vec * 2) / 4) * 3;

    // set needed predicates
    svbool_t predicate8 = svwhilelt_b8(0, (int)bytes_per_round);
    svbool_t predicateMax = svptrue_b8();
    svbool_t predicate32Max = svptrue_b32();
    svbool_t predicate16Max = svptrue_b16();

    svuint8_t vec_lookup_table = svld1(predicateMax, encode_lookup_table);
    svint8_t offset_vec = svld1(predicateMax, offsets);

    for (; length - 2 >= bytes_per_round; length -= bytes_per_round)
    {

        // load input data into vector register
        svuint8_t vec_1 = svld1(predicate8, (uint8_t *)input);
        input += input_slice;
        svuint8_t vec_2 = svld1(predicate8, (uint8_t *)input);
        input += input_slice;

        vec_1 = svtbl(vec_1, vec_lookup_table);
        vec_2 = svtbl(vec_2, vec_lookup_table);

        // populate vector registers with shift values
        svuint32_t vec_shift_ac = svdup_u32(0x04000040);
        svuint32_t vec_shift_bd = svdup_u32(0x01000010);

        svuint32_t data_1 = svreinterpret_u32(vec_1);
        svuint32_t data_2 = svreinterpret_u32(vec_2);

        // mask out indices ac and bd
        svuint32_t vec_ac_1 = svand_m(predicate32Max, data_1, 0x0FC0FC00);
        svuint32_t vec_ac_2 = svand_m(predicate32Max, data_2, 0x0FC0FC00);
        svuint32_t vec_bd_1 = svand_m(predicate32Max, data_1, 0x003F03F0);
        svuint32_t vec_bd_2 = svand_m(predicate32Max, data_2, 0x003F03F0);

        svuint16_t vec_shifted_ac_1 = svmulh_m(predicate16Max, svreinterpret_u16(vec_ac_1), svreinterpret_u16(vec_shift_ac));
        svuint16_t vec_shifted_ac_2 = svmulh_m(predicate16Max, svreinterpret_u16(vec_ac_2), svreinterpret_u16(vec_shift_ac));
        svuint16_t vec_shifted_bd_1 = svmul_m(predicate16Max, svreinterpret_u16(vec_bd_1), svreinterpret_u16(vec_shift_bd));
        svuint16_t vec_shifted_bd_2 = svmul_m(predicate16Max, svreinterpret_u16(vec_bd_2), svreinterpret_u16(vec_shift_bd));

        // first step finished
        svuint32_t vec_index_1 = svorr_m(predicate32Max, svreinterpret_u32(vec_shifted_ac_1), svreinterpret_u32(vec_shifted_bd_1));
        svuint32_t vec_index_2 = svorr_m(predicate32Max, svreinterpret_u32(vec_shifted_ac_2), svreinterpret_u32(vec_shifted_bd_2));

        // saturated substraction
        svuint8_t saturated_vec_1 = svqsub(svreinterpret_u8(vec_index_1), 51);
        svuint8_t saturated_vec_2 = svqsub(svreinterpret_u8(vec_index_2), 51);

        // extract mask of values lower than 26
        svbool_t mask_lower_26_1 = svcmplt_n_u8(predicateMax, svreinterpret_u8(vec_index_1), 26);
        svbool_t mask_lower_26_2 = svcmplt_n_u8(predicateMax, svreinterpret_u8(vec_index_2), 26);

        // all values, which were lower than 26 will be set to 13
        const svuint8_t vec_lookup_1 = svadd_m(mask_lower_26_1, saturated_vec_1, 13);
        const svuint8_t vec_lookup_2 = svadd_m(mask_lower_26_2, saturated_vec_2, 13);

        // register shuffle with offset vector
        svint8_t shuffled_offset_vec_1 = svtbl(offset_vec, vec_lookup_1);
        svint8_t shuffled_offset_vec_2 = svtbl(offset_vec, vec_lookup_2);

        // add offset values to the indices to compute base64 characters without memory lookup
        const svint8_t ascii_vec_1 = svadd_m(predicateMax, svreinterpret_s8(vec_index_1), shuffled_offset_vec_1);
        const svint8_t ascii_vec_2 = svadd_m(predicateMax, svreinterpret_s8(vec_index_2), shuffled_offset_vec_2);

        // store whole vector register to memory
        svst1(predicateMax, (int8_t *)output, ascii_vec_1);
        output += bytes_per_vec;
        svst1(predicateMax, (int8_t *)output, ascii_vec_2);
        output += bytes_per_vec;
    }
    // because we process 'bytes_per_round' bytes in each loop iteration,
    // there may be more data to be encoded if length % bytes_per_round != 0, this data will be encoded with a scalar encoding algorithm
    if (length != 0)
    {
        base64_encode_tail(output, input, length);
    }
}
