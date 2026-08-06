Copy-on-Write Binary File

The program implements a class that simulate a binary file and practice shallow/deep object-copying.

Supported methods:
- constructor, destructor, copy constructor, assignment operator=
- write (data, len): writes a block of bytes (data) of a certain length (len) into the file. The writing starts at the actual file position, the position is advanced by the number of bytes written. The method overwrites/adds the bytes to the file as needed. 
- read (data, len): reads bytes from the actual file position. The bytes are read into buffer data; the method reads up to len bytes. The method advances the file position by the number of bytes it actually read.
- seek ( pos ): changes the file position to pos.
- truncate(): truncates the file to the actual position in the file.
- fileSize(): returns the actual size of the file.
- addVersion(): adds the current version of the instance (a snapshot) into the list of file versions. Both file contents and file position are archived in the file versions.
- undoVersion(): reverts the contents (and file position) of the file to the previous version. Any previously saved versions may be recovered.

Modification methods requires fine-grained copy-on-write deep-copying while other operators use shallow copy (share pointer and increase reference count).