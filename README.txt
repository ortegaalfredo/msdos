Readme for MSDOS Version 1.0


What is MSDOS?
--------------
It's a distribution of 50+ Denial of Service attacks that i found in
the net, I arranged them and put a easy configurable graphical interface.
Think like a Linux distribution, but with DOS attacks...
The victim of the attacks are usually win95, win98,win98sr2 and winNT4
boxes, but may work with other systems too.
The main objetive is to remotely hang the box.

Usage
-------
./MSDOS [-c]
  -c:  Force MSDOS to run the Console interface

Requerimients
-------------
MSDOS requieres the following packages:

 cdialog-0.9a          console interface
 gnome-utils-1.4.0     gnome   interface
 
May work with older versiones, but i only test with this ones.

MSDOS work out-of-the-box with Mandrake 8.0 (gnome workstation), 
and maybe with redhat 7.1 too.

Compilation
-----------
The distribution comes with pre-compiled binaries for linux-glibc 2.1
if they don't work or you have a different system, go to the ./src dir
and ejecute ./compile.sh
the generated binaries must then be moved to the ./bin directory.

