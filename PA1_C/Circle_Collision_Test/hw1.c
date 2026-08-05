// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

// Constants
#define MAX_CIRCLE 2
#define ALL_SUCCESS_CASE 6
#define PI 3.14159265358979

int main()
{   
    // Circle 1: x1, y1, r1 ; Circle 2: x2, y2, r2
    double x1 = 0, y1 = 0, r1 = 0, x2 = 0, y2 = 0, r2 = 0;

    // Get input, reject invalid input
    for (int i=1; i<=MAX_CIRCLE; i++)
    {
        printf("Enter circle #%d parameters:\n", i);
        double a = 0, b = 0, r = 0;
        if (scanf("%lf%lf%lf", &a, &b, &r) != 3 || r <= 0) 
        {
            printf("Invalid input.\n");
            return EXIT_FAILURE;
        }
        
        if (i==1)
        {
            x1 = a;
            y1 = b;
            r1 = r;        
        }
        else 
        {
            x2 = a;
            y2 = b;
            r2 = r;
        }
    }

    // Distance
    double d = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));

    // Epsilon
    const double ESL = DBL_EPSILON * d;

    // Min and Max
    int small = 1, big = 2;
    double min = fmin(r1, r2), max = fmax(r1, r2);
    if (r1 - r2 >= ESL)
    {
        small = 2;
        big = 1;
    }

    // Normal Overlap
    double overlap = PI*min*min;

    // Intersect Overlap
    double r_1 = r1*r1*acos((r1*r1 + d*d - r2*r2) / (2.0*d*r1));
    double r_2 = r2*r2*acos((r2*r2 + d*d - r1*r1) / (2.0*d*r2));
    double r_0 = (0.5)*sqrt((4.0*d*d*r1*r1) - pow((r1*r1 + d*d - r2*r2), 2));
    double intersect_overlap = r_1 + r_2 - r_0;
    
    // Other
    double r_diff = fabs(max - min);
    double r_sum = r1 + r2;

    // Cases
    const char* cases[ALL_SUCCESS_CASE] = {
        "The circles are identical, overlap: %lf",
        "Circle #%d lies inside circle #%d, overlap: %lf",
        "Internal touch, circle #%d lies inside circle #%d, overlap: %lf",
        "The circles intersect, overlap: %lf",
        "External touch, no overlap.",
        "The circles lie outside each other, no overlap.",
    };

    // Case #1: Identical, overlap
    if ((d < ESL  || d == 0.0) && (r_diff < ESL || r_diff == 0.0))
        printf(cases[0], overlap);

    // Case #2: Circle in circle, overlap
    else if (r_diff - d >= ESL)
        printf(cases[1], small, big, overlap);

    // Case #3: Internal touch, overlap
    else if (fabs(d - r_diff) < ESL || fabs(d-r_diff) == 0.0)
        printf(cases[2], small, big, overlap);

    // Case #4: Intersect, overlap
    else if (d - r_diff >= ESL && r_sum - d >= ESL)
        printf(cases[3], intersect_overlap);

    //Case #5: External touch, no overlap
    else if (fabs(d - r_sum) < ESL || fabs(d - r_sum) == 0.0)
        printf("%s", cases[4]);    

    // Case #6: Lie outside, no overlap
    else
        printf("%s", cases[5]);
    printf("\n");

    // End program
    return EXIT_SUCCESS;
}