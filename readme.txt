

TinkerBell UI Toolkit - BETA*
-----------------------------------------------------------------------------------

I created TinkerBell UI toolkit because i wanted a small UI toolkit for use in
games and hardware accelerated applications.

It has no dependency on stl, RTTI or exceptions. Compiling without these features
makes footprint very small. Utility classes (such as string, lists, hash table,
etc.) are quite minimal for the basic needs of TinkerBell itself.

I named it TinkerBell UI (TBUI for short) because it's lightweight (small
footprint) compared to many other UI toolkits.

See integration.txt for details about integrating image loading, renderer, font
system etc.


BETA notice
-----------------------------------------------------------------------------------

I call this an beta since some of the APIs are still a bit experimental and may
change. That does not mean that the code is very unstable and can't be used for
anything yet.

If you use this right now, don't be surprised if you need to adjust your code to
API changes. Since there is no release changelogs during beta, you should keep an
eye on the commit changelog to see what's going on.

I'll try to respond to questions and feature requests but can't make any promises
about frequent updates. There's lots of things on the backlog already and this is
a non funded hobby project so time is precious!

I'm working on this because i need it and because it's fun, but tomorrow i might
focus on some other project instead :)


License
-----------------------------------------------------------------------------------

This software is provided 'as-is', without any express or implied warranty. In no
event will the authors be held liable for any damages arising from the use of this
software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to the
following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim
  that you wrote the original software. If you use this software in a product,
  an acknowledgment in the product documentation would be appreciated but is not
  required.

  2. Altered source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.


What features does it have
-----------------------------------------------------------------------------------

  TinkerBell core:

	• Widgets (many common standard widgets)
	• Text editing widget (With clipboard, undo/redo, styling functionality,
	  embedded content (for read only text flow)...)
	• Extendable skin system with automatic runtime atlas creation,
	  conditions (simple selectors), overrides, overlays, children, multiple
	  pixel density support etc.
	• Automatic widget layout (No need to specify widget dimensions by pixels)
	• Text based UI resource format
	• Keyboard friendly (tab focus etc.)
	• Message handling, with delayed/timed messages.
	• All containers are scrollable/pannable (automatically from mouse/finger
	  interaction, and following focus).
	• Very failproof event handling (F.ex deleting a widget that is on the
	  stack in a event handler is not dangerous).
	• Widget connections (synchronize multiple widgets with "widget-values")
	• Language string handling
	• No dependency on stl, exceptions, RTTI
	• Very portable & easy to implement new backends (Image loading, fonts,
	  renderer)
	• Support 32/64bit architectures and tested on Windows/Linux/Mac/iOS
	• Uses constexpr by default for all ID comparisions on compilers that support
	  it.
	• Unicode support (UTF8)
	• Multiple font systems can be used (Implementations of TBBF, freetype and stbf
	  provided)
	• Animation system.
	• Unit tests.

  TinkerBell addons:

	• Image manager & Image widget (load graphics into widgets on demand)


The code design & developing TinkerBell - What to think about
-----------------------------------------------------------------------------------

If you try to familiarize yourself with the code, you might notice the heavily used
object TBID that may seem like a string. It's actually an uint32 that can be set
from a string (using the strings hash value). It's used for hash keys, as
substitute for enums and all kind of things needing custom IDs.

The thing about it deserving mention is that it's using constexpr, a C++ 11
feature that (if supported by the compiler) converts constant strings into the
uint32 at compile time.

If you want to add new stuff to TinkerBell, here are some hints that will make it
easier to get a pull requests accepted:

	• Do not make it dependant on exceptions, RTTI or stl (or boost or similar)

	• It should not leak or crash if running out of memory.

	• Follow the used code style (4 spaces wide tabs, indentation & whitespace style,
	  documentation etc.)


Building the demo
-----------------------------------------------------------------------------------

The demo use GLFW as a submodule for some platform window & input handling.
After the first git clone, you need to run:

	git submodule init
	git submodule update

This will clone GLFW into the right directory.

On Windows, use the project files for Visual Studio 2010 in Demo/VisualStudio.
On Linux & Mac, simply run make. It will create a executable called RunDemo.
On Linux & Mac, you can also use the Sublime Text 2 project in the root folder
that is set up to work with debugging using SublimeGDB.


Author
-----------------------------------------------------------------------------------

Emil Segerås (emilsegers@gmail.com)
Web: http://fiffigt.com/
