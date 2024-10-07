#include <base64sve.h>

void printArray(uint32_t *input, int length)
{

    printf("\n\nhex: ");
    for (int i = 0; i < length; i++)
    {
        printf("0x%08X ", input[i]);
    }
    printf("\n");

    printf("dec: ");
    for (int i = 0; i < length; i++)
    {
        printf("%d ", input[i]);
    }
    printf("\n");
}

void printArray16(uint16_t *input, int length)
{

    printf("\n\nhex: ");
    for (int i = 0; i < length; i++)
    {
        printf("0x%04X ", input[i]);
    }
    printf("\n");

    printf("dec: ");
    for (int i = 0; i < length; i++)
    {
        printf("%d ", input[i]);
    }
    printf("\n");
}

#define N 6

int main(void)
{
    uint32_t a[N] = {0xA0000000, 0x00010001, 0x2, 0x4, 0xC, 0x2};
    uint32_t c_sve_intr[N * 2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    printArray(a, N);
    testShiftingByMultiplication(a, c_sve_intr, N);
    // testSaturatingSubstraction(a,c_sve_intr,N);
    printArray(c_sve_intr, N );

    return 0;
}