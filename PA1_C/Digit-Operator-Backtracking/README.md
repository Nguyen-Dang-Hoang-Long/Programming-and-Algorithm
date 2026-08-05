Digit Operator Backtracking

Input:
    Given an input sequence e.g. 12345

Output:
    Then, given a target number e.g. 20
    Compute all posible ways to reach that number with the given sequence by slotting operators +, -, * in between the sequence e.g. 1+2*3*4-5 = 20

E.g.
    Digits:
    234567
    Problems:
    ? 10
    = 2+3*4-5-6+7
    = 2*3-4-5+6+7
    Total: 2
    ? 2
    = 2+3-45+6*7
    = 2-3+45-6*7
    = 23-4*5+6-7
    = 2*3+4+5-6-7
    Total: 4
    ? 17
    = 2-34+56-7
    = 2+3+4-5+6+7
    Total: 2
    ? 234567
    = 234567
    Total: 1
    # 5
    Total: 5
    # 1000
    Total: 0