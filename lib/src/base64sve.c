#include <base64sve.h>

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
    static const int8_t offsets[68] = {71, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -19, -16, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    svbool_t predicate8 = svwhilelt_b8(0, (int)bytes_per_round);
    svbool_t predicateMax = svwhilelt_b8(0, (int)bytes_per_vec);
    svbool_t predicate32Max = svwhilelt_b32(0, (int)bytes_per_round);
    svbool_t predicate16Max = svwhilelt_b16(0, (int)bytes_per_round);

    svuint8_t vec = svld1(predicate8, (uint8_t *)input);

    svuint8_t vec_lookup_table = svld1(predicateMax, encode_lookup_table);
    svint8_t offset_vec = svld1(predicateMax, offsets);

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

    svuint8_t saturated_vec = svqsub_m(predicateMax, svreinterpret_u8(vec_index), 52);

    svbool_t mask_lower_26 = svcmplt_n_u8(predicateMax, svreinterpret_u8(vec_index), 26);

    const svuint8_t vec_lookup = svadd_m(mask_lower_26, saturated_vec, 13);

    offset_vec = svtbl(offset_vec, vec_lookup);

    const svint8_t ascii_vec = svadd_m(predicateMax, svreinterpret_s8(vec_index), offset_vec);

    svst1(predicate8, (int8_t *)output, ascii_vec);

    // for (int i = 0; i < length - bytes_per_vec; i += bytes_per_vec, output += bytes_per_vec, input += bytes_per_vec)
    // {
    // svbool_t predicate8 = svwhilelt_b8(0, (int)length);
    // svuint8_t vec = svld1(predicate8, (uint8_t *)input);
    // svst1(predicate8, output, vec);
    // }
}
