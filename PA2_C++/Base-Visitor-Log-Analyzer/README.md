Base Visitor Log Analyzer

The class purpose is to analyze logs of visitors that enter a military base.

The base contains multiple zones, some of the zones are connected bidirectionally (undirected graph). The base connects to the outside through gates that can track each visitor's name, time, and date of entering. These are the only logs being analyzed.

Firstly, the program must process a descripion of the base (a graph) in the form of a file containing lines of 3-tuple "ZoneA ZoneB SecurityLevel" (the graph's edges). The higher the SecurityLevel, the more time it takes to pass from ZoneA to ZoneB (+1 min per 1 level).

Secondly, the program then given a log file, containing all the logs of the visits, in one of 3 formats: Text, Big-Endian Binary, Little-Endian Binary.

Thirdly, the program, given the base blueprint and the visitor logs, can now take in queries of form (zoneName, notBefore, notAfter) and find all visitors that visited zoneName notBefore a certain time and notAfter a certain time. The 2 latter parameters can be excluded.

The task tests students on file parsing of various formats (little-endian, big-endian binary, text), graph traversal algorithms (Breadth-First Search), and handling overlapping temporal intervals.