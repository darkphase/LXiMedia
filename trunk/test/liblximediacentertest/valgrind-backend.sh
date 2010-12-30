#!/bin/sh

valgrind --leak-check=full --suppressions=../liblxistreamtest/valgrind.supp --suppressions=./valgrind.supp --gen-suppressions=yes ../../bin/lximcbackend --run
