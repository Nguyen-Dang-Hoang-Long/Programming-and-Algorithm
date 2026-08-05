Circular_Array_Minimum_Difference_Partition

Input:
    Given a circular bypass with n sections (and their costs)
    Each section can also be individually updated to change their cost (in different periods of time as well, but only increasing over time)
    Then, given a period of time (from date and to date)

Output:
    Partition the sections into 2 supersections such that the difference in total construction cost of both supersections is minimized (a supersection = a continuous series of sections)
    Output the minimum difference, number of partitions, and the partitions themselves.

E.g.
Daily cost:
{ 10, 20,
 10 , 20,


              10       ,	20, 10
,
20
}
? 2000-01-01 2000-01-01
Difference: 0, options: 4
* 0 - 3, 4 - 7
* 1 - 4, 5 - 0
* 2 - 5, 6 - 1
* 3 - 6, 7 - 2
? 2000-01-01 2001-01-01
Difference: 0, options: 4
* 0 - 3, 4 - 7
* 1 - 4, 5 - 0
* 2 - 5, 6 - 1
* 3 - 6, 7 - 2
= 2000-02-03 1: 40
? 2000-01-01 2001-01-01
Difference: 660, options: 2
* 1 - 3, 4 - 0
* 2 - 6, 7 - 1
= 2000-04-12 1: 50
= 2000-04-16 2: 15
? 2000-01-01 2001-01-01
Difference: 685, options: 1
* 2 - 6, 7 - 1