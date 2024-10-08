#include <base64sve.h>

void test(void)
{
    printf("Hello\n");
}

void printRegister(svuint8_t vec)
{
    size_t bytes_per_vec = svcntb();
    uint8_t memory[bytes_per_vec];

    svbool_t predicate = svwhilelt_b8(0, (int)bytes_per_vec);
    svst1(predicate, memory, vec);

    printf("register contents: ");
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

    printf("register contents: ");
    for (int i = 0; i < words_per_vec; i++)
    {
        printf("0x%08X ", memory[i]);
    }
    printf("\n");
}

void base64_encode(void *input, char *output, size_t length)
{
    size_t bytes_per_vec = svcntb();
    size_t bytes_per_round = (bytes_per_vec / 4) * 3;

    const uint8_t encode_lookup_table[] = {1, 0, 2, 1, 4, 3, 5, 4, 7, 6, 8, 7, 10, 9, 11, 10, 13, 12, 14, 13, 16, 15, 17, 16, 19, 18, 20, 19, 22, 21, 23, 22, 25, 24, 26, 25, 28, 27, 29, 28, 31, 30, 32, 31, 34, 33, 35, 34, 37, 36, 38, 37, 40, 39, 41, 40, 43, 42, 44, 43, 46, 45, 47, 46, 49, 48, 50, 49, 52, 51, 53, 52, 55, 54, 56, 55, 58, 57, 59, 58, 61, 60, 62, 61};

    svbool_t predicate8 = svwhilelt_b8(0, (int)bytes_per_round);
    svbool_t predicateMax = svwhilelt_b8(0, (int)bytes_per_vec);
    svbool_t predicate32Max = svwhilelt_b32(0, (int)bytes_per_round);
    svbool_t predicate16Max = svwhilelt_b16(0, (int)bytes_per_round);

    svuint8_t vec = svld1(predicate8, (uint8_t *)input);
    svuint8_t vec_lookup_table = svld1(predicateMax, encode_lookup_table);

    printRegister(vec);

    vec = svtbl(vec, vec_lookup_table);

    printRegister(vec);

    // populate vector registers with shift values
    svuint32_t vec_shift_ac = svdup_u32(0x04000040);
    svuint32_t vec_shift_bd = svdup_u32(0x01000010);

    svuint32_t data = svreinterpret_u32(vec);

    printRegister32(data);


    // mask out indices ac and bd
    svuint32_t vec_ac = svand_m(predicate32Max, data, 0x0FC0FC00);
    svuint32_t vec_bd = svand_m(predicate32Max, data, 0x0FC0FC00);

    svuint16_t vec_shifted_ac = svmulh_m(predicate16Max, svreinterpret_u16(vec_ac), svreinterpret_u16(vec_shift_ac));
    svuint16_t vec_shifted_bd = svmul_m(predicate16Max, svreinterpret_u16(vec_bd), svreinterpret_u16(vec_shift_bd));

    svuint32_t vec_index = svorr_m(predicate32Max, svreinterpret_u32(vec_shifted_ac), svreinterpret_u32(vec_shifted_bd));

    printRegister32(vec_index);


    svst1(predicate8, (int8_t *)output, svreinterpret_s8(vec));

    // for (int i = 0; i < length - bytes_per_vec; i += bytes_per_vec, output += bytes_per_vec, input += bytes_per_vec)
    // {
    // svbool_t predicate8 = svwhilelt_b8(0, (int)length);
    // svuint8_t vec = svld1(predicate8, (uint8_t *)input);
    // svst1(predicate8, output, vec);
    // }
}

void testShiftingByMultiplication(uint32_t *input, uint32_t *output, size_t length)
{
    svbool_t predicate32 = svwhilelt_b32(0, (int)length);

    svuint32_t vec = svld1(predicate32, input);

    svbool_t predicate16 = svwhilelt_b16(0, (int)length);

    svuint32_t vec_constants = svdup_u32(0x00080020);

    // svuint32_t vec_constants = svdupq_n_u32(0x00800004,0x00800004,0x00800004,0x00800004);
    // svuint16_t vec_constants = svdup_u16(0x0080);
    svuint16_t result = svmulh_m(predicate16, svreinterpret_u16(vec), svreinterpret_u16(vec_constants));
    // svuint32_t result = svmul_m(predicate, vec, vec_constants);

    // svst1_u16(predicate, output, svreinterpret_u16(vec_constants));
    svst1(predicate32, output, svreinterpret_u32(result));
}

void testSaturatingSubstraction(uint8_t *input, uint8_t *output, size_t length)
{
    svbool_t predicate = svwhilelt_b8(0, (int)length);

    svuint8_t vec = svld1(predicate, input);

    vec = svqsub_m(predicate, vec, 52);

    svst1(predicate, output, vec);
}

void testShuffle(uint32_t *input, uint32_t *ouput, size_t length)
{

    uint32_t indices[6] = {1, 0, 2, 3, 4, 5};

    svbool_t predicate = svwhilelt_b32(0, (int)length);
    svuint32_t vec = svld1(predicate, input);

    svuint32_t vec_indices = svld1(predicate, indices);

    vec = svtbl(vec, vec_indices);

    svst1(predicate, ouput, vec);
}

void vec_add_sve_intrinsics(int *restrict out, int *restrict a, int *restrict b, int length)
{
    svbool_t predicate;
    svint32_t a_vec;
    svint32_t b_vec;

    for (int i = 0; i < length; i += svcntw())
    {
        predicate = svwhilelt_b32(i, length);
        a_vec = svld1(predicate, &a[i]);
        b_vec = svld1(predicate, &b[i]);
        a_vec = svadd_z(predicate, a_vec, b_vec);
        svst1(predicate, &out[i], a_vec);
    }
}