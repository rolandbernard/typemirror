Typemirror
==========

This is a simple application for mirroing key strikes between multiple computer on the same network
broadcast domain.
The program can be started as a server in which case it will listen for keypresses and then
broadcast them over the network using UPD packets.
If the program is started as a client using the `-c` option, it will listen on the network for
broadcast messages and create fake key presses corresponding to the ones sent by the server.

