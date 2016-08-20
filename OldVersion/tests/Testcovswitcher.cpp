#include "covstdlib.h"
#include <iostream>
#include <string>
int main()
{
    std::string in;
    getline(std::cin, in);

    SWITCH(in) {
        CASE("clear") {
            printf("Test case clear\n");
        }
        BREAK;

        CASE("stop") {
            printf("Test case stop\n");
        }
        BREAK;

        CASE("next") {
            printf("Test case next\n");
            int ret;
            std::cin >> ret;
            SWITCH(ret) {
                CASE(0) printf("Test case zero\n");
                BREAK;
                CASE(1) printf("Test case one\n");
                BREAK;
                DEFAULT printf("Test case default-number\n");
                BREAK;
            }
            ENDSWITCH;
        }
        BREAK;

        DEFAULT {
            printf("Test case default-string\n");
        }
        BREAK;

    }
    ENDSWITCH;
    return 0;
}