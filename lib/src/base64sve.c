#include <base64sve.h>

void test(void){
    printf("Hello\n");
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