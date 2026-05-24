SmaugFUSS - Fixed Up Smaug Source
=================================

SmaugFUSS is an ongoing project to find and eliminate as many bugs in the Smaug MUD codebase as possible, while also modernizing and updating the code where needed.

More information about the project is available at https://smaugmuds.afkmods.com/

Licensing
=========

See the file 'license.txt' in your doc directory. Read, obey. Downloading and installing this code implies your agreement to the terms.

Installation Notes
==================

There are some things that you need to be aware of prior to installing the game. Please read this stuff BEFORE you try and run the game. You'll avoid a lot of hassle, and we'll be asking if you checked this stuff anyway.

Release Frequency
=================

Codebase releases will not be following any kind of set schedule. This is usually done on the fly based on the number of bug reports received between releases. If there are a lot of them, then it could be a relatively short period. If there are none, then it will depend on how confident we feel about new features which might be added. If you absolutely cannot wait, there's always the GitHub repository.

Development Environment
=======================

A Linux or BSD based system with the GCC compiler installed with G++ support. This should be the default for any standard GCC install these days.

On Windows, CYGWIN can be used if you have no access to a Linux or BSD based system.

CPU, Memory, and Storage Requirements
=====================================

SmaugFUSS should take up about 35 MB in a stock configuration, with the executable file already compiled. It would be prudent to allow another 10 MB at the very least for world expansion and player data storage.

RAM usage should expect to hover around 30 MB with a stock world.

CPU usage should hover at or near 0.0% during idle, and around 0.5% during moderate load. We don't have any high stress usage figures to offer, but it shouldn't cause you any grief.

[Note: This was a concern back in 2002, but in 2026, any halfway decent VPS or other server won't even notice these levels of resource use. MUDs just aren't that demanding.]

Adding Commands, Skills, and spec_funs
======================================

When you find yourself wanting to add a new command to the code, either from a snippet you've downloaded and decided to use, or from something you've written yourself, if you've been working with Smaug for long you know all about the tables.c file and the places you need to insert things to make it work.

With SmaugFUSS, you no longer need to worry about that. The code used the dlsym functionality to handle the required lookups. It isn't even necessary to have DECLARE_DO_FUN statements anywhere. Just insert, compile, and create the command online. It's that easy now.

In the unlikely event your system does not have the right library support, you'll need to have that fixed by your sysadmin. Most Linux installs have this natively.

The Makefile
============

Configuring the Makefile should no longer be a necessity for SmaugFUSS as all of the options that could be have been rewritten for automated detection of code features.

With one exception - if you want to use a compiler other than g++ you will need to set that at the top of the Makefile.

Installing and Using the Code
=============================

It should be fairly a straightforward process to install the MUD. It is assumed you already know enough about unpacking files and working with Linux/CYGWIN.

Unpack the archive in whatever folder you will be working in. It will default to creating a "smaugfuss" directory. Change the name of this as you see fit.

On a Linux or CYGWIN based system:

Change to the smaugfuss/src directory and type: make

On FreeBSD or OpenBSD:

Change to the smaugfuss/src directory and type: gmake
BSD compiling is probably not compatible with pmake.

Starting the MUD After Compiling
================================

Once you have everything compiled, you're likely anxious to start things up.

In your terminal, from the src directory, type in:

./startup.sh &

The MUD should be booted up within a few seconds. To connect and verify that it's up and running, type:

telnet localhost 4020

You should be presented with the MUDs login screen.

Please note that the older "startup" script is no longer being distributed with the package as it required an extra shell scripting package to be installed on the server. Pretty much any Linux or Linux-like system should have /bin/bash available by default.

You can change the default port the MUD operates on by editing the startup.sh script and changing "4020" to the port number you want.

First Immortal
==============

A pfile named "Admin" is included, with password "admin". Use this account to log in for the first time. You should then rename this player to something else and CHANGE THE PASSWORD.

Alternatively, connect a new player and use the Admin player to advance the new one. When this is done, be sure to DELETE the Admin player.
