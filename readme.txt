// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
// ==             See tinkerbell.h for more information and license.             ==
// ================================================================================

Tinkerbell UI Toolkit - ALPHA
-----------------------------

NOTE: This is a alpha version. Some APIs is still experimental and is very
      likely to change.

I created Tinkerbell UI toolkit because i wanted a small UI toolkit for use in
games, that has minimal dependencies and is very easy to work with.

It has no dependency on stl, RTTI or exceptions. Compiling without these features
makes footprint very small. Utility classes (such as string, lists, hash table,
etc.) are quite minimal for the basic needs of tinkerbell itself.

Image loading and font system can be used if wanted, or you can make it use
whatever systems you have for this already.

What features does it have
--------------------------

  Tinkerbell core:

  -Widgets (many common standard widgets)
  -Text editing widget (With clipboard, undo/redo, styling functionality, embedded content...)
  -Extendable skin system with automatic runtime atlas creation.
  -Automatic widget layout (No need to specify widget dimensions by pixels)
  -Text based UI resource format (No need to write C++ to create UI)
  -Keyboard friendly (tab focus etc.)
  -Message handling, with delayed/timed messages.
  -All containers are scrollable/pannable (automatically from mouse/finger
   interaction, and following focus).
  -Very failproof event handling (F.ex deleting a widget that is on the
   stack in a event handler is not dangerous).
  -Widget connections (synchronize multiple widgets with "widget-values")
  -Language string handling
  -No dependency on stl, exceptions, RTTI
  -Very portable & easy to implement new backends (Image loading, fonts, renderer)
  -Support 32/64bit architectures and tested on Windows/Linux/Mac/Android/iOS

  Optional components:

  -TdFont (Optional)
  -TdImage (Optional)
  -TdAnimation (Optional)

What features does it NOT have
-------------------------------

  -BIDI, Unicode support (The latter might be added later)

  -No painting API except the skin & string rendering.
   The idea is that you plug tinkerbell into a OpenGL/Direct2D/3D environment anyway.

  -Not designed to run in multiple threads (running in a dedicated UI thread should
   of course be fine)

Extending tinkerbell - What to think about (if a pull request should be accepted)
---------------------------------------------------------------------------------

  -Do not make it dependant on exceptions, RTTI or stl (or boost or similar)

  -It should not crash or leak if out of memory.

  -Follow the used code style (4 spaces wide tabs, indentation & whitespace style,
   documentation etc.)

Author
------

Emil Segerås (emilsegers@gmail.com)
http://fiffigt.com/
