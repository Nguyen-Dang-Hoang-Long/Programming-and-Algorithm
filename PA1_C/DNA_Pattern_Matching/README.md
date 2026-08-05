DNA_Pattern_Matching

Input:
    The program maintains a list of DNA samples (a string of characters A,T,C,G and of length of multiple of 3).

Output:
    Then, given a target string to search for (containing A,T,C,G), output a list of DNA samples that contain the searched string (only if the searched string is aligned on a triplet boundary).
    
E.g.
    DNA database:
    80.5:ATAAGCCTA
    60.2:GTACCTATAACA
    40.8:CCCTATCCTTAG
    1000:GGAGGAGGA
    5.02:CCTCCCCCC

    Searches:
    CCT
    Found: 3
    > GTACCTATAACA
    > CCCTATCCTTAG
    > CCTCCCCCC
    ATA
    Found: 2
    > ATAAGCCTA
    > GTACCTATAACA
    ATAACA
    Found: 1
    > GTACCTATAACA
    AAA
    Found: 0
    GAG
    Found: 0
    AAB
    Invalid input.