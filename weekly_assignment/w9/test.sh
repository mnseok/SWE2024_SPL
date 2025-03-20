#!/bin/bash
# 컴파일
test_file="./w9.c"
gcc -o w9 $test_file
# 테스트 결과를 저장할 파일
result_file="./test_results.txt"
> $result_file

# result 폴더 초기화
rm -f result/*

echo "" >> $result_file
# 테스트 함수
run_test1() {
    test_number=$1
    test_name=$2
    input=$3

    echo "Test Case $test_number: $test_name" >> $result_file
    output=$(echo -e "$input" | ./w9)

    if diff --ignore-all-space "./result/result$test_number.txt" "./testcase/answer$test_number.txt" >/dev/null; then
        echo "PASS" >> $result_file
    else
        echo "FAIL" >> $result_file
    fi
    echo "" >> $result_file
}

run_test2() {
    test_number=$1
    test_name=$2
    input=$3

    echo "Test Case $test_number: $test_name" >> $result_file
    output=$(echo -e "$input" | ./w9)

    echo "$output" > "result/result$test_number.txt"

    if diff --ignore-all-space "result/result$test_number.txt" "testcase/answer$test_number.txt" >/dev/null; then
        echo "PASS" >> $result_file
    else
        echo "FAIL" >> $result_file
    fi
    echo "" >> $result_file
}



# 테스트 케이스
run_test1 1 "> redirection" "ls ./testcase > ./result/result1.txt\nquit"
run_test2 2 "< redirection" "cat < ./testcase/test.c\nquit"
run_test2 3 "| pipe test" "head -n 5 ./testcase/test.c | tail -n 1\nquit"


# 결과 출력
cat $result_file
# 점수 계산
total_tests=$(grep -c "Test Case" $result_file)
passed_tests=$(grep -c "PASS" $result_file)
score=$((passed_tests * 100 / (total_tests)))  # +1 for string.h check
echo "Total tests: $((total_tests))"  # +1 for string.h check
echo "Passed tests: $passed_tests"
echo "Score: $score%"
rm $result_file
