#include <base64sve.h>
#include <limits.h>

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

void base64_decode(char *base64_data, uint8_t *output, size_t encoded_length)
{
}
