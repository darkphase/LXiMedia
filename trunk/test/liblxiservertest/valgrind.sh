#!/bin/sh

valgrind --leak-check=full --suppressions=./valgrind.supp --gen-suppressions=yes ../../bin/lxiservertest -silent
