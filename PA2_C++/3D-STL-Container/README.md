3D STL Container

I implemented 2 classes. One is a general multidimensional array (1, 2, 3 dimensions supported) with templates that can hold any data types (CTensor) and the other is a lightweight accessing wrapper (CTensorView) that may read the data of but not manage the memory nor modify the content of CTensor.