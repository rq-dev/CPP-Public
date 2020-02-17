Task:

1) Download a book from https://www.gutenberg.org/

2) The program should read the file and print the count of occurrences for every word.

3) Use WinAPI:

CreateFile - to open the file
GetFileSizeEx - to get size of the file -> Size
Text = new char[Size+1] - allocate the buffer
ReadFile - to read the file
CloseHandle - to close the file
delete[] - to delete the buffer

You should display the working time of the program.

You should implement the custom allocator.

Requirements:

Do not use "string" objects.
Use char* for strings.
Use only Text buffer, do not allocate another strings.

The custom allocator should allocate a large buffer (e.g. 1MB).
Then, this large buffer should be used for subsequent allocations.
If the space in the large buffer will be all used, then another buffer should be allocated.
All buffers should be freed at the end of the program.





