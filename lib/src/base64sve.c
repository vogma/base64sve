#include <base64sve.h>

void test(void)
{
    printf("Hello\n");
}

void testShiftingByMultiplication(uint32_t *input, uint32_t *output, size_t length)
{
    svbool_t predicate32 = svwhilelt_b32(0, (int)length);

    svuint32_t vec = svld1(predicate32, input);

    svbool_t predicate16 = svwhilelt_b16(0, (int)length);

    svuint32_t vec_constants = svdup_u32(0x00020004);

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