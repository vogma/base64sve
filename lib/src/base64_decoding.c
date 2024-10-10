#include <base64sve.h>
#include <limits.h>

const int8_t LOWER_INVALID = 1;
const int8_t UPPER_INVALID = 1;

const int8_t lower_bound_lut[16] =
    {LOWER_INVALID, LOWER_INVALID, 0x2B, 0x30,
     0x41, 0x50, 0x61, 0x70,
     LOWER_INVALID, LOWER_INVALID, LOWER_INVALID, LOWER_INVALID,
     LOWER_INVALID, LOWER_INVALID, LOWER_INVALID, LOWER_INVALID};

const int8_t upper_bound_lut[16] =
    {
        UPPER_INVALID, UPPER_INVALID, 0x2b, 0x39,
        0x4f, 0x5a, 0x6f, 0x7a,
        UPPER_INVALID, UPPER_INVALID, UPPER_INVALID, UPPER_INVALID,
        UPPER_INVALID, UPPER_INVALID, UPPER_INVALID, UPPER_INVALID};

static const int8_t shift_lut[16] = {
    /* 0 */ 0x00, /* 1 */ 0x00, /* 2 */ 0x3e - 0x2b, /* 3 */ 0x34 - 0x30,
    /* 4 */ 0x00 - 0x41, /* 5 */ 0x0f - 0x50, /* 6 */ 0x1a - 0x61, /* 7 */ 0x29 - 0x70,
    /* 8 */ 0x00, /* 9 */ 0x00, /* a */ 0x00, /* b */ 0x00,
    /* c */ 0x00, /* d */ 0x00, /* e */ 0x00, /* f */ 0x00};

static const uint8_t index_decode[] = {2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, 18, 17, 16, 22, 21, 20, 26, 25, 24, 30, 29, 28, 34, 33, 32, 38, 37, 36, 42, 41, 40, 46, 45, 44, 50, 49, 48, 54, 53, 52, 58, 57, 56, 62, 61, 60, 66, 65, 64, 70, 69, 68, 74, 73, 72, 78, 77, 76, 82, 81, 80, 86, 85, 84, 90, 89, 88, 94, 93, 92, 98, 97, 96, 102, 101, 100, 106, 105, 104, 110, 109, 108, 114, 113, 112, 118, 117, 116, 122, 121, 120, 126, 125, 124};

void printRegister(svuint8_t vec)
{
    size_t bytes_per_vec = svcntb();
    uint8_t memory[bytes_per_vec];

    svbool_t predicate = svwhilelt_b8(0, (int)bytes_per_vec);
    svst1(predicate, memory, vec);

    printf("register contents: ");
    for (int i = 0; i < bytes_per_vec; i++)
    {
        printf("%c ", memory[i]);
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

void base64_decode(char *base64_data, void *output, size_t encoded_length, size_t *decoded_length)
{
    size_t bytes_per_register = svcntb();
    size_t output_bytes_per_round = (bytes_per_register / 3) * 4;

    // set needed predicates
    svbool_t predicateSave = svwhilelt_b8(0, (int)output_bytes_per_round);
    svbool_t predicate8Max = svptrue_b8();
    svbool_t predicate32Max = svptrue_b32();

    const svint8_t shift_lut_vec = svld1(predicate8Max, shift_lut);
    const svuint8_t index_decode_lut_vec = svld1(predicate8Max, index_decode);

    for (; encoded_length >= bytes_per_register; encoded_length -= bytes_per_register, base64_data += bytes_per_register, output += output_bytes_per_round)
    {
        svint8_t data_vec = svld1(predicate8Max, (int8_t *)base64_data);

        svint8_t higher_nibble = svasr_m(predicate8Max, data_vec, 4);

        svbool_t eq = svcmpeq(predicate8Max, data_vec, 0x2f);

        svint8_t shift_vec = svtbl(shift_lut_vec, svreinterpret_u8(higher_nibble));

        data_vec = svadd_m(predicate8Max, data_vec, shift_vec);

        data_vec = svadd_m(eq, data_vec, -3);

        svuint32_t data_vec32 = svreinterpret_u32(data_vec);

        svuint32_t ac_vec = svand_m(predicate32Max, data_vec32, 0x003f003f);
        ac_vec = svlsl_m(predicate32Max, ac_vec, 6);

        svuint32_t bd_vec = svand_m(predicate32Max, data_vec32, 0x3f003f00);
        bd_vec = svlsr_m(predicate32Max, bd_vec, 8);

        svuint32_t t0 = svorr_m(predicate32Max, ac_vec, bd_vec);
        svuint32_t t1 = svlsl_m(predicate32Max, t0, 12);
        svuint32_t t2 = svlsr_m(predicate32Max, t0, 16);

        svuint32_t packed_data = svorr_m(predicate32Max, t1, t2);

        svuint8_t result = svtbl(svreinterpret_u8(packed_data), index_decode_lut_vec);

        svst1(predicateSave, (uint8_t *)output, result);
    }
}
