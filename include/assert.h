#ifndef ASSERT_H_
#define ASSERT_H_

#include "stdio.h"
#include "stdlib.h"

#define assert(_expr_)                                        \
        do {                                                  \
                if ((_expr_) == false) {                      \
                        printf("Assert fail! " #_expr_ "\n"); \
                        abort();                              \
                }                                             \
        } while (0)


#endif
