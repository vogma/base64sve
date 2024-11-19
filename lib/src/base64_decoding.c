#include <base64sve.h>
#include <limits.h>

#define BASE64_PAD '='
#define BASE64DE_FIRST '+'
#define BASE64DE_LAST 'z'

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

/* ASCII order for BASE 64 decode, 255 in unused character */
static const unsigned char base64de[] = {
    /* nul, soh, stx, etx, eot, enq, ack, bel, */
    255, 255, 255, 255, 255, 255, 255, 255,

    /*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
    255, 255, 255, 255, 255, 255, 255, 255,

    /* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
    255, 255, 255, 255, 255, 255, 255, 255,

    /* can,  em, sub, esc,  fs,  gs,  rs,  us, */
    255, 255, 255, 255, 255, 255, 255, 255,

    /*  sp, '!', '"', '#', '$', '%', '&', ''', */
    255, 255, 255, 255, 255, 255, 255, 255,

    /* '(', ')', '*', '+', ',', '-', '.', '/', */
    255, 255, 255, 62, 255, 255, 255, 63,

    /* '0', '1', '2', '3', '4', '5', '6', '7', */
    52, 53, 54, 55, 56, 57, 58, 59,

    /* '8', '9', ':', ';', '<', '=', '>', '?', */
    60, 61, 255, 255, 255, 255, 255, 255,

    /* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
    255, 0, 1, 2, 3, 4, 5, 6,

    /* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
    7, 8, 9, 10, 11, 12, 13, 14,

    /* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
    15, 16, 17, 18, 19, 20, 21, 22,

    /* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
    23, 24, 25, 255, 255, 255, 255, 255,

    /* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
    255, 26, 27, 28, 29, 30, 31, 32,

    /* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
    33, 34, 35, 36, 37, 38, 39, 40,

    /* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
    41, 42, 43, 44, 45, 46, 47, 48,

    /* 'x', 'y', 'z', '{', '|', '}', '~', del, */
    49, 50, 51, 255, 255, 255, 255, 255};

static const uint8_t index_decode[] = {2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, 18, 17, 16, 22, 21, 20, 26, 25, 24, 30, 29, 28, 34, 33, 32, 38, 37, 36, 42, 41, 40, 46, 45, 44, 50, 49, 48, 54, 53, 52, 58, 57, 56, 62, 61, 60, 66, 65, 64, 70, 69, 68, 74, 73, 72, 78, 77, 76, 82, 81, 80, 86, 85, 84, 90, 89, 88, 94, 93, 92, 98, 97, 96, 102, 101, 100, 106, 105, 104, 110, 109, 108, 114, 113, 112, 118, 117, 116, 122, 121, 120, 126, 125, 124};

unsigned int base64_decode_tail(const char *in, unsigned int inlen, uint8_t *out)
{
    unsigned int i;
    unsigned int j;
    unsigned char c;

    for (i = j = 0; i < inlen; i++)
    {
        if (in[i] == BASE64_PAD)
        {
            break;
        }
        if (in[i] < BASE64DE_FIRST || in[i] > BASE64DE_LAST)
        {
            return 0;
        }

        c = base64de[(unsigned char)in[i]];
        if (c == 255)
        {
            return 0;
        }

        switch (i & 0x3)
        {
        case 0:
            out[j] = (c << 2) & 0xFF;
            break;
        case 1:
            out[j++] |= (c >> 4) & 0x3;
            out[j] = (c & 0xF) << 4;
            break;
        case 2:
            out[j++] |= (c >> 2) & 0xF;
            out[j] = (c & 0x3) << 6;
            break;
        case 3:
            out[j++] |= c;
            break;
        }
    }

    return j;
}

void base64sve_decode(char *base64_data, void *output, size_t encoded_length, size_t *decoded_length)
{
    size_t bytes_per_register = svcntb();
    size_t output_bytes_per_round = (bytes_per_register / 4) * 3;

    // set needed predicates
    svbool_t predicateSave = svwhilelt_b8(0, (int)output_bytes_per_round);
    svbool_t predicate8Max = svptrue_b8();
    svbool_t predicate32Max = svptrue_b32();

    const svint8_t shift_lut_vec = svld1(predicate8Max, shift_lut);
    const svuint8_t index_decode_lut_vec = svld1(predicate8Max, index_decode);

    size_t output_length = 0;

    for (; encoded_length - 1 >= bytes_per_register; encoded_length -= bytes_per_register, base64_data += bytes_per_register, output += output_bytes_per_round, output_length += output_bytes_per_round)
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
    if (encoded_length != 0)
    {
        output_length += base64_decode_tail(base64_data, encoded_length, (uint8_t *)output);
    }

    *decoded_length = output_length;
}

void base64sve_decode_x2(char *base64_data, void *output, size_t encoded_length, size_t *decoded_length)
{
    size_t bytes_per_register = svcntb() * 2;
    size_t output_bytes_per_round = (bytes_per_register / 4) * 3;

    // set needed predicates
    svbool_t predicateSave = svwhilelt_b8(0, (int)output_bytes_per_round);
    svbool_t predicate8Max = svptrue_b8();
    svbool_t predicate32Max = svptrue_b32();

    const svint8_t shift_lut_vec = svld1(predicate8Max, shift_lut);
    const svuint8_t index_decode_lut_vec = svld1(predicate8Max, index_decode);

    size_t output_length = 0;

    for (; encoded_length - 1 >= bytes_per_register; encoded_length -= bytes_per_register, base64_data += bytes_per_register, output += output_bytes_per_round, output_length += output_bytes_per_round)
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
    if (encoded_length != 0)
    {
        output_length += base64_decode_tail(base64_data, encoded_length, (uint8_t *)output);
    }

    *decoded_length = output_length;
}

void testTuple(uint64_t *data, uint64_t *output, size_t length)
{
    size_t doubles_per_register = svcntd();
    svbool_t predicate8Max = svptrue_b64();

    svuint64x2_t data_vec = svld2(predicate8Max, data);

    svst2(predicate8Max, output, data_vec);
}
