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

    // printArray(data, N);
    base64sve_encode_x2(data, output, N);
    // printArray(output, N);

    // Erwartetes Ergebnis
    // printf("SGFsbG9EYXNJc3RlaW5UZXN0d2llZ2VodGVzZGlyQVZDQg==\n");

    for (int i = 0; i < encoded_length; i++)
    {
        printf("%c", output[i]);
    }
    printf("\n\n");

    char base64_data[48] = "SGFsbG9EYXNJc3RlaW5UZXN0d2llZ2VodGVzZGlyQVZDQg==";
    char result[48];
    size_t decoded_length = 0;

    base64sve_decode(base64_data, result, 48, &decoded_length);

    for (int i = 0; i < 48; i++)
    {
        printf("0x%02X ", base64_data[i]);
    }
    printf("\n");

    for (int i = 0; i < decoded_length; i++)
    {
        printf("%c", result[i]);
    }
    printf("\n");

    // void testTuple(uint64_t * data, uint64_t * output, size_t length);

    uint64_t data2[] = {1, 2, 3, 4};
    uint64_t out[4];

    testTuple(data2, out, 4);

    for (int i = 0; i < 4; i++)
    {
        printf("%" PRId64" ", data2[i]);
    }
    printf("\n");

    for (int i = 0; i < 4; i++)
    {
        printf("%" PRId64" ", out[i]);
    }
    printf("\n");

    return 0;
}