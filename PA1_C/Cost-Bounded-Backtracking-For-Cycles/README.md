Cost-bounded Backtracking for Cycles

In this program, I implement the findTrips(const char [], const char [], int) function, with following functionalities:

Input:
    Given a list of all edges (directed edges, each with a cost), the destination vertice, and a limit on total cost

Output:
    Find all possible paths such that:
    - Ends in the vertice it starts with 
    - Contains at least 2 vertices, none of which visited twice or more
    - Total cost below a the limit


E.g.
    findTrips(data1, "London", 400 )

    Output:
    81: London -> Paris -> London
    171: London -> Wien -> London
    198: London -> Paris -> Wien -> London
    204: London -> Wien -> Paris -> London
    207: London -> Prague -> London
    221: London -> Paris -> Prague -> London
    225: London -> Prague -> Paris -> London
    252: London -> Wien -> Prague -> London
    258: London -> Prague -> Wien -> London
    270: London -> Wien -> Prague -> Paris -> London
    272: London -> Paris -> Prague -> Wien -> London
    279: London -> Paris -> Wien -> Prague -> London
    291: London -> Prague -> Wien -> Paris -> London
    342: London -> Prague -> Paris -> Wien -> London
    344: London -> Wien -> Paris -> Prague -> London
