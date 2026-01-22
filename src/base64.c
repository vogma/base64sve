#include <base64sve.h>
#include <inttypes.h>

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

#define N 313

static int Base64encode_len(int len)
{
    return ((len + 2) / 3 * 4) + 1;
}

int main(void)
{

    // uint8_t data[N] = {0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0XFA};
    char data[N] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam ex nunc, rhoncus in blandit at, rutrum sed turpis. Aenean in bibendum dolor, vitae facilisis dolor. Quisque imperdiet et nulla non feugiat. Fusce elementum est eu nibh efficitur aliquet. Quisque elementum diam libero, eget auctor nunc condimentum in.";

    const size_t encoded_length = Base64encode_len(N);
    char output[encoded_length];
    char output2[encoded_length];

    // base64sve_encode_shift(data, output, N);
    base64sve_encode_bdepx4(data, output, N);
    base64sve_encode_shift(data, output2, N);


    for (int i = 0; i < encoded_length; i++)
    {
        printf("%c", output[i]);
    }
    printf("\n\n");
    for (int i = 0; i < encoded_length; i++)
    {
        printf("%c", output[i]);
    }
    printf("\n\n");
    return 0;
}