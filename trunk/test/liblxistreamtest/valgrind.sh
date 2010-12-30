#!/bin/sh

valgrind --leak-check=full --num-callers=32 --suppressions=./valgrind.supp --gen-suppressions=yes ../../bin/lxistreamtest -silent
