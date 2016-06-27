/** @mainpage Introduction

Turbo Badger is a small GUI library that provides a large number of skinnable widgets,
a flexible event system, and well-abstracted rendering, all in a very nicely designed and 
packaged programming interface.

<a href="https://github.com/fruxo/turbobadger">The main GitHub repository is here.</a>

The default skin looks like this:
\image html Demo/screenshot/screenshot_01.png

*/

/** @page page_gettingstarted Getting Started

This covers the basic steps you need to take to incorporate Turbo Badger
into your own project.  This mainly involves interfacing with TB's event system, 
providing resources to TB, and finally drawing the GUI.

\section sec_confopt Configuration Options

TB's configuration is taken from the file src/tb/tb_config.h, there is a template config file
with a full list of available configuration options in tb_config.h.in .  CMake can be
used to configure the template tb_config.h.in into a correct src/tb/tb_config.h for your
setup.

\section sec_events Events

\subsection subsec_input Input

You need to create at least one "root widget" for your UI widgets. A TBWidget that
act as receiver for input, painting etc, and is parent to your other widgets.
Turbo Badger will handle input capture, focus, etc. automatically.

List of input triggers (root is your root TBWidget):

	root->InvokePointer[Down/Up/Move] - Mouse or touch input.
	root->InvokeWheel                 - Mouse wheel input
	root->InvokeKey                   - Keyboard input

\subsection subsec_anim Animations, Layout and Painting

Before painting, it is a good time to run animations, update widgets input states
and layout, since all that affect the rendering. This is done by calling:

	TBAnimationManager::Update()
	root->InvokeProcessStates()
	root->InvokeProcess()

Before painting the root widget (and all its children), you need to prepare
the renderer to set up the correct matrix etc.

	g_renderer->BeginPaint(window_width, window_height);
	root->InvokePaint(TBWidget::PaintProps())
	g_renderer->EndPaint();

\subsection subsec_msghandle Message handling

TBMessageHandler::ProcessMessages() needs to be called frequently to process
the message queue.

If you have a game running its loop continuously, you can just call ProcessMessages()
each time.

If you have a event driven application that should only consume CPU when needed
you can call TBMessageHandler::GetNextMessageFireTime() for scheduleing when to run
ProcessMessage next. Also, TBSystem::RescheduleTimer will be called back if the time
it needs to run next is changed.

\section sec_rendering Rendering

There are four example render targets provided with TurboBadger: OpenGL 1.1, OpenGL 3.2, 
OpenGL ES1, and OpenGL ES2.  You may need to modify them to fit into your own project, or they 
may just work.  Furthermore, TB makes it easy to write and incorporate your own renderer
with a clean separation of the rendering interface from the rest of the UI.

TBRenderer & TBBitmap are the interfaces that implements everything that Turbo Badger
needs for rendering. There is already a OpenGL renderer implementation that can be
used, or you can implement your own (See tb_config.h).

Some apps need to unload all graphics and reload it again, f.ex when the app
is put to sleep in the background and the graphics context is lost.
Turbo Badger implements a renderer listener to handle this in all places that needs
it, so the only thing you need to do is to call:

	g_renderer->InvokeContextLost(); // Forget all bitmaps
	...
	g_renderer->InvokeContextRestored(); // Reload all bitmaps

\subsection subsec_fonts Font system

There is font implementations for freetype and stb_truetype.h, and a bitmap font
system (tbbf - "turbo badger bitmap font") that is very simple to use and supports
any range or characters, glow & shadow. You can choose any of these or add your own.
It possible to use multiple backends, so you can f.ex use tbbf for some bitmap fonts,
and a freetype backend for other fonts.

The implementation API render glyph by glyph, but if it's requested to externalize
the entire string measuring & drawing, support for that shouldn't be hard to add.

If no font implementation is choosen (or if using font ID 0), a test dummy font
is used which renders squares.

\subsection subsec_system System integration

Here are some additional API's that needs implementations:

TBImageLoader
API for loading images. There's a implementation available using STBI. Note: STBI
should only be used to load your well known resources and nothing from the net,
since the library doesn't handle corrupt files safely which might be a security
issue.

TBSystem
Needs to be implemented for timers & some settings.

TBClipboard
Needs to be implemented for clipboard support.

TBFile
Needs to be implemented for file loading.

\section sec_layout Layout
\section sec_widgets Widgets
\section sec_serialization Serialization

*/

/** @page page_widgets Widgets

\section sec_commonwidg List of common widgets

Defined in tb_widgets_common.h, TB provides a number of pre-defined widgets out of the box:


\section sec_customwidg Creating custom widgets

Widgets are cheap to create and layout and using many widgets to get what you want
is encouraged. A custom widget should typically not implement any layouting and
drawing of sub-elements if it can be solved by wrapping a TBLayout and other
existing widgets.

Any widget may contain other widgets and may control into which sub-widget other
widgets are inflated from resources (using `TBWidget::GetContentRoot`).

Custom widgets can be inflated from UI resources too. See the use of the
`TB_WIDGET_FACTORY` macro in `tb_widgets_reader.cpp` for examples on how to do
this.

*/

/** @page page_layout Layout

 */

/** @page page_resources Serialization (the .tb.txt format)


 */
