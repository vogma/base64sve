#include <base64sve.h>

void printArray(uint8_t *input, int length)
{

    // printf("hex: ");
    for (int i = 0; i < length; i++)
    {
        printf("0x%02X ", input[i]);
    }
    printf("\n");

    // printf("dec: ");
    // for (int i = 0; i < length; i++)
    // {
    //     printf("%d ", input[i]);
    // }
    // printf("\n");
}

#define N 34

int main(void)
{

    // uint8_t data[N] = {0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0XFA};
    char data[N] = "HalloDasIsteinTestwiegehtesdirAVCB";

    char output[N] = {0};

    // printArray(data, N);
    base64_encode(data, output, N);
    // printArray(output, N);

    printf("SGFsbG9EYXNJc3RlaW5UZXN0d2llZ2VodGVzZGlyQVZDQg==\n");

    for (int i = 0; i < N; i++)
    {
        printf("%c", output[i]);
    }
    printf("\n");

    return 0;
}