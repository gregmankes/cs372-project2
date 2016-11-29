Greg Mankes
Project 2
CS372


Compiling and running ftserver.c

To Compile:
gcc ftserver.c

To run:
./a.out <server_port>

--------------------------------------------------------------------------------

Running ftclient.py

Give it executable permissions:
chmod +x ftclient.py

Run the executable
./ftclient.py <flip1-3> <server_port> -l <data_port>

OR

./ftclient.py <flip1-3> <server_port> -g <file_to_get> <data_port>

If it complains about the interpretter, then this is because this program is
meant for flip.

You can then run it with:

python ftclient.py <flip1-3> <server_port> -l <data_port>

OR

python ftclient.py <flip1-3> <server_port> -g <file_to_get> <data_port>

--------------------------------------------------------------------------------

Other:
In order for this program to work, you must have the compile executable and the
python client on two separate flip servers (flip1, flip2, or flip3). They MUST also
be in two different directories on the server. This is because the file that will be
transferred will be opened for writing and then there wont be anything to send.

**** MAKE SURE THAT THE EXECUTABLES ARE IN DIFFERENT DIRECTORIES ****

This program was tested on flip2.engr.oregonstate.edu and flip3.engr.oregonstate.edu
