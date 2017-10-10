#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char message[32];

void setup()
{
    memset(message, 0, 32);
    sprintf(message, "Hello world!");
}

void loop()
{
    printf("[ %s ]\n", message);
    sleep(1);
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
