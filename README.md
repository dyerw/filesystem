README

To Do List
=================

* vfs read
* vfs write
* vfs truncate
* Deal with FAT blocks in delete, unmount, and potentially others
* Fill in the actual README
* Optimize to deal with smaller Dirents

Our Approach
=============
* Our file system is a single-level directory implementation of a FAT File System

Challenges Faced
==================



Features/Properties
===================
* Support for up to 150 files // Depending on how the implementation goes, need to change this
* Ability to mount a file system and then create, read from, write to, and delete files. 
* Blah blah blah 


Testing
=========
We tested our code using fprintf statements in each function so we could so when each function was called. We also used gdb while running specific commands so as to run through how our functions were actually working. Lastly, we used the supplied test script to check whether we were passing the required tests or not.
