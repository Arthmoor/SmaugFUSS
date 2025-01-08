# SmaugFUSS - Fixed Up Smaug Source

SmaugFUSS is an ongoing project to find and eliminate as many bugs in the Smaug MUD codebase as possible, while also modernizing and updating the code where needed.

More information about the project is available at https://smaugmuds.afkmods.com/

## Development Environment

A linux or BSD based system with the GCC compiler installed with G++ support. This should be the default for any standard GCC install these days.

On Windows, CYGWIN can be used if you have no access to a linux or BSD based system.

## Installing and using the code

It should be fairly a straightforward process to install the mud. It is assumed you already know enough about unpacking files and working with linux/CYGWIN.

1. Unpack the archive in whatever folder you will be working in.

2. Compiling

On a Linux based system:

Change to the src directory and type: make

On Cygwin:

Change to the src directory.
Open the Makefile.
Uncomment the Cygwin definition and save the file.
Type make

On FreeBSD:

Change to the src directory and type: gmake
BSD compiling is probably not compatible with pmake.

3. Starting up

After the compile finishes, open the starup file and change the port number to the one you plan to use.
Then type: nohup ./startup &

This will launch the mud in the background, telling it to ignore when you disconnect from your session.

4. First immortal

Log in to the mud, and use the following user/pass:

admin/admin

Once you're in, you can either use the pcrename command to change this character's name to yours ( and then change the password! ) or you can use it to advance another character who logs in afterward. If you choose to advance a different character, delete the admin player for security purposes.

5. Start having some fun! :)
