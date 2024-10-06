#include <base64sve.h>

void printArray(uint8_t *input, int length)
{

    printf("hex: ");
    for (int i = 0; i < length; i++)
    {
        printf("0x%02X ", input[i]);
    }
    printf("\n");

    printf("dec: ");
    for (int i = 0; i < length; i++)
    {
        printf("%d ", input[i]);
    }
    printf("\n\n");
}

#define N 6

int main(void)
{
    uint8_t a[N] = {25, 53, 62, 57, 63, 23};
    uint8_t c_sve_intr[N] = {0, 0, 0, 0, 0, 0};

    printArray(a,N);
    testSaturatingSubstraction(a,c_sve_intr,N);
    printArray(c_sve_intr,N);

    return 0;
}