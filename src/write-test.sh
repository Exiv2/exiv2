#! /bin/sh
# Test driver for the write unit tests

# Case 1
echo "------------------------------------------------------------"
testFile="../test/img_1771.jpg"
rm -f test1.jpg thumb1.jpg
rm -f iii ttt; 
./exifprint $testFile > iii;
cp $testFile ./test1.jpg; 
./write-test $testFile 1 > ttt; 
diff iii ttt

# Case 2
echo "------------------------------------------------------------"
testFile="../test/img_1771.jpg"
rm -f test2.jpg thumb2.jpg
rm -f iii ttt; 
./exifprint $testFile > iii;
cp $testFile ./test2.jpg; 
./write-test $testFile 2 > ttt; 
diff iii ttt

# Case 3
echo "------------------------------------------------------------"
testFile="../test/kodak-dc210.jpg"
rm -f test3.jpg thumb3.jpg
rm -f iii ttt; 
./exifprint $testFile > iii;
cp $testFile ./test3.jpg; 
./write-test $testFile 3 > ttt; 
diff iii ttt

# Case 4
echo "------------------------------------------------------------"
testFile="../test/img_1771.jpg"
rm -f test4.jpg thumb4.jpg
rm -f iii ttt; 
./exifprint $testFile > iii;
cp $testFile ./test4.jpg; 
./write-test $testFile 4 > ttt; 
diff iii ttt

# Case 5
echo "------------------------------------------------------------"
testFile="../test/img_1771.jpg"
rm -f test5.jpg thumb5.jpg
rm -f iii ttt; 
./exifprint $testFile > iii;
cp $testFile ./test5.jpg; 
./write-test $testFile 5 > ttt; 
diff iii ttt

# Case 6
echo "------------------------------------------------------------"
testFile="../test/kodak-dc210.jpg"
rm -f test6.jpg thumb6.jpg
rm -f iii ttt; 
./exifprint $testFile > iii;
cp $testFile ./test6.jpg; 
./write-test $testFile 6 > ttt; 
diff iii ttt
