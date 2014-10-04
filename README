You should use the 'make' command to build your filesystem executable
and the formatting program.  We have provided skeleton code in
3600fs.c and 3600mkfs.c to get you started.  You will need to fill
in the stub code as specified by the assignment.

In brief, you should first compile everything by running

make

You should then create your disk by running

./3600mkfs <disksize>

(of course, this doesn't do anything right now except create a zero-ed out
disk, but you'll add code to 3600mkfs.c to make it write out the data structures
that your filesystem will use).  Finally, you'll attach your disk to the
real filesystem by running

./3600fs -d fuse-dir

This will make your disk appear mounted at the local fuse-dir directory (the
actual directory and its contents will be hidden as long as your process
is running).  Any file operations (create/delete files, read/write data will
be redirected to the various vfs_ functions in your 3600fs.c file).

Note that you should pass in the '-d' flag to your FUSE executable to
produce debugging output that will help you understand what's going on.
If you do so, however, your program will NOT go into daemon mode, so you
will need to use another terminal to exercise the directory.  Your
executable will exit when you unmount the file system.

We are using FUSE version 2.6. This is important to know because
different FUSE versions have slightly different interfaces.

You will find a great deal of resources about FUSE on the Web.  Please
use Piazza to ask questions if you're stuck.
