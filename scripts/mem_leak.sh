valgrind --leak-check=full --show-reachable=yes -v ./run.sh > mem_leak.out 2>&1
less mem_leak.out
