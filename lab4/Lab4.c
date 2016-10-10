#include <stdio.h>
#include <stdbool.h>


void print_answer(char *question, bool answer)
{
    printf("(%s) %s\n", question, answer ? "true" : "false");
}

int main()
{
    {
        int b[4] = {2, 4, 2, 8};
        int *p = &b[0];
        int *q = &b[1];
        int *r = &b[2];

        bool x;
        char *question;

        question = "2.a";
        x = p < q < r;
        print_answer(question, x);

        question = "2.b";
        x = (p != r && *p == *r);
        print_answer(question, x);

        question = "2.c";
        x = (q - b == &b[3] - &p[1]);
        print_answer(question, x);

        question = "2.d";
        x = (p[1] == r[-1]);
        print_answer(question, x);

        question = "2.e";
        x = (&r[-2] == &b[0]);
        print_answer(question, x);

        question = "2.f";
//    x = (q-p+q-p == q+q-p-p);
//    printf("(%c) %s\n", question, x ? "true" : "false");
        printf("(%s) %s\n", question, "*compiler error when uncommented*");
    }

    {
        char *question;

#define n 4
        int b[n] = {12, 13, 14, 15};
        int u = 20, v = 30, *x = &u, *y, *z;
        y = &u;
        z = &b[2];

        // <----- Position 1
        question = "3.a";

        printf("(%s) b[4] = { ", question);
        for (int i = 0; i < 4; i++) {
            printf("%d", b[i]);
            if (i != n - 1) {
                printf(", ");
            }
        }
        printf(" }\n");

        printf("      u = %d; v = %d; *x = %d, *y = %d, *z = %d;\n",
            u, v, *x, *y, *z);

        ++*x; // (i.e., *x = *x + 1)
        y = &v;
        --z;
        z[1] = 20;

        // <----- Position 2
        question = "3.b";

        printf("(%s) b[4] = { ", question);
        for (int i = 0; i < 4; i++) {
            printf("%d", b[i]);
            if (i != n - 1) {
                printf(", ");
            }
        }
        printf(" }\n");

        printf("      u = %d; v = %d; *x = %d, *y = %d, *z = %d;\n",
               u, v, *x, *y, *z);
    }

    return 0;
}