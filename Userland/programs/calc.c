
#include "usrlib.h"

#define USE "Use: 'calc <num_1> <op> <num_2>' where op in { +, -, *, / }\n"

int calc_main(int argc, char * argv[]) {
    if (argc != 3 || strlen(argv[1]) != 1) {
        print_err("Invalid arguments\n");
        print_err(USE);
        return -1;
    }

    int a = satoi(argv[0]);
    char op = argv[1][0];
    int b = satoi(argv[2]);
    int res;
    
    switch (op) {
        case '+': res = a + b; break;
        case '-': res = a - b; break;
        case '*': res = a * b; break;
        case '/': res = a / b; break;
        default:
            print_err("Invalid operator\n");
            print_err(USE);
            return -1;
    }

    printf("%d %c %d = %d", a, op, b, res);
    return 0;


}