#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *message[2];

void setup()
{
    message[0] = (char*) malloc(sizeof(char) * 32);
    message[1] = (char*) malloc(sizeof(char) * 32);

    memset(message[0], 0, 32);
    memset(message[1], 0, 32);

    sprintf(message[0], "Hello world!");
    sprintf(message[1], "How are you?");
}

void loop()
{
    unsigned long int t = (unsigned long int) rand() / 10000;
    printf("[ %s ]\n", message[t % 2 ? 1 : 0]);
    usleep(t);
}

/**
 * In case that i want to run my module standalone
 */
__attribute__((weak)) int main()
{
    setup();
    for (;;) {
        loop();
    }

    return 0;
}
