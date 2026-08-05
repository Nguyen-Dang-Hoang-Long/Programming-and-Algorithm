Camera_Report_Query

Input:
    Given a list of camera reports on cars passing by at specific time

Output:
    Then, given a query (include the car plate and a time), find the exact report that spot that car. Else, find the closest reports (before and after that time) of that car being spotted.
    
E.g.
    Camera reports:
    {10: ABC-12-34 Oct 1 7:30,
    289: XYZ-98-76 Oct 10 15:40,
    25: ABC-12-34 Oct 1 8:50,
    42: Slartibartfast Dec 21 6:00,
    11: ABC-12-34 Oct 1 10:50,
    10: ABC-12-34 Oct 1 7:30,
    17: ABC-12-34 Oct 1 7:30}
    Search:
    ABC-12-34 Oct 1 9:30
    > Previous: Oct 1 08:50, 1x [25]
    > Next: Oct 1 10:50, 1x [11]
    ABC-12-34 Oct 1 8:30
    > Previous: Oct 1 07:30, 3x [10, 10, 17]
    > Next: Oct 1 08:50, 1x [25]
    ABC-12-34 Oct 1 7:30
    > Exact: Oct 1 07:30, 3x [10, 10, 17]
    XYZ-98-76 Nov 30 0:00
    > Previous: Oct 10 15:40, 1x [289]
    > Next: N/A
    abc-12-34 Oct 12 1:23
    > Car not found.
    Slartibartfast Jan 24 10:42
    > Previous: N/A
    > Next: Dec 21 06:00, 1x [42]