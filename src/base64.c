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

static int Base64encode_len(int len)
{
    return ((len + 2) / 3 * 4) + 1;
}

int main(void)
{

    // uint8_t data[N] = {0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0XFA};
    char data[N] = "HalloDasIsteinTestwiegehtesdirAVCB";

    const size_t encoded_length = Base64encode_len(N);
    char output[encoded_length] = {};

    // printArray(data, N);
    base64_encode(data, output, N);
    // printArray(output, N);

    // Erwartetes Ergebnis
    printf("SGFsbG9EYXNJc3RlaW5UZXN0d2llZ2VodGVzZGlyQVZDQg==\n");

    for (int i = 0; i < encoded_length; i++)
    {
        printf("%c", output[i]);
    }
    printf("\n\n");

    char base64_data[48] = "SGFsbG9EYXNJc3RlaW5UZXN0d2llZ2VodGVzZGlyQVZDQg==";
    char result[48];
    size_t decoded_length;

    base64_decode(base64_data, result, 48, &decoded_length);

    for (int i = 0; i < 48; i++)
    {
        printf("0x%02X ", base64_data[i]);
    }
    printf("\n");

    for (int i = 0; i < 48; i++)
    {
        printf("%c", result[i]);
    }
    printf("\n");

    return 0;
}