#include <base64sve.h>

void printArray(uint32_t *input, int length)
{

    for (int i = 0; i < length; i++)
    {
        printf("%d ", input[i]);
    }
    printf("\n");
}

#define N 6

int main(void)
{
    uint32_t a[N] = {1, 2, 3, 4, 5, 6};
    uint32_t c_sve_intr[N] = {0, 0, 0, 0, 0, 0};

    testShuffle(a,c_sve_intr,N);
    printArray(c_sve_intr,N);

    return 0;
}