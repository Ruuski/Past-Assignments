# Tests for comp10002 assignment 2 2016 sem 2 Indexed Document Retrevial
# Written by Liam Aharon

#!/bin/bash

echo "Running all test cases..."
./ass2_sol test0-ind.txt < test0.txt > test0-myout.txt
./ass2_sol test1-ind.txt < test1.txt > test1-myout.txt
./ass2_sol pg11-ind.txt < test2.txt > test2-myout.txt
echo "Completed - checking"

echo "Checking test 0"
diff test0-myout.txt test0-out.txt
echo "Checking test 1"
diff test1-myout.txt test1-out.txt
echo "Checking test 2"
diff test2-myout.txt test2-out.txt
