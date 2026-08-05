// Libraries
#include <stdio.h>
#include <stdlib.h>

int main()
{
    // Get track length input
    // Invalidate input if non-numeric, negative, zero, or equal lengths
    printf("Track length:\n");
    long long length1 = 0, length2 = 0;
    if (scanf ("%lld %lld", &length1, &length2) != 2 
    || length1 <= 0 || length2 <= 0 || length1 == length2)
    {
        printf("Invalid input.\n");
        return EXIT_FAILURE;
    }

    // Get distance input
    // Invalidate input if negative, non-numeric distance, no sign, wrong sign
    printf("Distance:\n");
    long long distance = 0;
    char sign = '\0';
    if (scanf(" %c %lld", &sign, &distance) != 2
    || distance < 0 || (sign != '+' && sign != '-'))
    {
        printf("Invalid input.\n");
        return EXIT_FAILURE;
    }

    // Buffer variables for variant count, quotient of length 1 and 2, tmp buffer for distance
    long long quo1 = 0, quo2 = 0;
    int variant = 0;
    long long tmp = 0;

    // Go through each combination
    for (long long i = 0; i <= distance / length1; i++)
    {
        tmp = distance;
        quo2 = 0;
        quo1 = i;
        tmp = tmp - length1*quo1;
        quo2 = tmp / length2;
        tmp = tmp - length2*quo2;

        // If a variant is found, variant++ and print out (if '+')
        if (tmp == 0)
        {
            variant++;
            if (sign == '+')
                printf("= %lld * %lld + %lld * %lld\n", length1, quo1, length2, quo2);
        }
    }
    // No solution if 0 variant
    if (variant == 0)
    {
        printf("No solution.\n");
        return EXIT_SUCCESS;
    }

    // Otherwise, print variant count
    printf("Total variants: %d\n", variant);
    return EXIT_SUCCESS;
}
