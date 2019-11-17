visor text editor & text editor framework
=========================================

About
-----
*visor* is a lightweight, system-independent, device-independent, maximally
portable, easily embeddable text editor framework. *visor* is also a concrete
implementation of a text editor for a number of systems, based on the visor
framework.

To avoid confusion, from this point forward we'll refer to the framework as
`libvisor`, and the editor as `visor`, but keep in mind that the main focus of
this project is actually the editor framework, and not the editor program.

Libvisor implements all the functionality needed by a complete text editor.
From low level text manipulation operations, to (optional) high level user
interaction layered onto those low level operations, designed to operate similar
to the `vi` text editor. All the while maintaining complete system independence,
able to operate (with help from the client application) under any operating
system, or completely freestanding for embedded devices, boot loaders,
exokernels, retro hacking, or bare metal programs.

For details about each component, see the `README.md` files in their
subdirectories.

License
-------
Copyright (C) 2019 John Tsiombikas <nuclear@member.fsf.org>
All parts of the visor project are free software. Feel free to use, modify,
and/or redistribute under the terms of their respective licenses.

 - *libvisor* (the framework) is released under the terms of the GNU Lesser
   General Public License (LGPL) version 3 or later. See `libvisor/COPYING` and
   `libvisor/COPYING.LESSER` for details.

 - *visor* (the editor program) is released under the terms of the GNU General
   Public License (GPL) version 3 or later. See `visor/COPYING` for details.

Download
--------
The source code for both parts of the visor project are included in the same git
repository, and released as one.

No releases are available yet, but you can grab the current source code from
github: https://github.com/jtsiomb/visor
