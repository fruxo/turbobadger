// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TBWIDGETS_READER_H
#define TBWIDGETS_READER_H

#include "tb_linklist.h"
#include "tb_widgets.h"

namespace tinkerbell {

class TBWidgetsReader;
class Widget;
class TBNode;

struct CREATE_INFO {
	TBWidgetsReader *reader;
	Widget *target;
	TBNode *node;
};

/** TBWidgetFactory creates a widget from a TBNode. */
class TBWidgetFactory : public TBLinkOf<TBWidgetFactory>
{
public:
	TBWidgetFactory(const char *name, TBValue::TYPE sync_type, WIDGET_Z add_child_z);

	/** Create and return the new widget or nullptr on out of memory. */
	virtual Widget *Create(CREATE_INFO *info) = 0;

	void Register();
public:
	const char *name;
	TBValue::TYPE sync_type;
	WIDGET_Z add_child_z;
	TBWidgetFactory *next_registered_wf;
};

/** This macro creates a new TBWidgetFactory for the given class name (that should
	inherit Widget). It register it so it can be used to create widgets from
	TBWidgetsReader.

	classname - The name of the class.
	sync_type - The data type that should be synchronized through TBWidgetValue.
	add_child_z - The order in which children should be added to it by default.

	It should be followed by a section for reading custom data.

	Example:

		TB_WIDGET_FACTORY(MyWidget, TBValue::TYPE_INT, WIDGET_Z_TOP) {}

	Example:

		TB_WIDGET_FACTORY(MyWidget, TBValue::TYPE_INT, WIDGET_Z_TOP)
		{
			// If "customstring" is specified in the resource, set it on the widget.
			if (const char *string = info->node->GetValueString("customstring", nullptr))
				widget->SetCustomString(string);
		}

	*/
#define TB_WIDGET_FACTORY(classname, sync_type, add_child_z) \
	class classname##WidgetFactory : public TBWidgetFactory \
	{ \
	public: \
		classname##WidgetFactory() \
			: TBWidgetFactory(#classname, sync_type, add_child_z) { Register(); } \
		virtual Widget *Create(CREATE_INFO *info) \
		{ \
			classname *widget = new classname(); \
			if (widget) \
				ReadCustomProps(widget, info); \
			return widget; \
		} \
		void ReadCustomProps(classname *widget, CREATE_INFO *info); \
	}; \
	static classname##WidgetFactory classname##_wf; \
	void classname##WidgetFactory::ReadCustomProps(classname *widget, CREATE_INFO *info)

/**
	TBWidgetsReader parse a resource file (or buffer) into a TBNode tree,
	and turn it into a hierarchy of widgets. It can create all types of widgets
	that have a registered factory (TBWidgetFactory). All default tinkerbell
	widgets have a factory by default, and you can also add your own.

	Each factory may have its own set of properties, but a set of generic
	properties is always supported on all widgets. Those are:

	Resource name:		Widget property:		Values:

	id					Widget::m_id			TBID (string or int)
	group_id			Widget::m_group_id		TBID (string or int)
	value				Widget::SetValue		integer
	data				Widget::m_data			integer
	text				Widget::SetText			string (translatable using @string)
	connection			Widget::Connect			string
	gravity				Widget::SetGravity		string (combination of left, top, right, bottom, or all)
	state				Widget::SetState		string (disabled)
	skin				Widget::SetSkinBg		TBID (string or int)
	autofocus			The Widget will be focused automatically the first time its TBWindow is activated.
*/
class TBWidgetsReader
{
public:
	static TBWidgetsReader *Create();
	~TBWidgetsReader();

	/** Add a widget factory. Does not take ownership of the factory.
		The easiest way to add factories for custom widget types, is using the
		TB_WIDGET_FACTORY macro that automatically register it during startup. */
	bool AddFactory(TBWidgetFactory *wf) { factories.AddLast(wf); return true; }
	void RemoveFactory(TBWidgetFactory *wf) { factories.Remove(wf); }

	/** Return a string with the given request from node. If the string
		is preceded with a @, it will be looked up from the global TBLanguage. */
	const char *GetTranslatableString(TBNode *node, const char *request);

	bool LoadFile(Widget *target, const char *filename);
	bool LoadData(Widget *target, const char *data);
	bool LoadData(Widget *target, const char *data, int data_len);
	void LoadNodeTree(Widget *target, TBNode *node);
private:
	bool Init();
	bool CreateWidget(Widget *target, TBNode *node, WIDGET_Z add_child_z);
	TBLinkListOf<TBWidgetFactory> factories;
};

}; // namespace tinkerbell

#endif // TBWIDGETS_READER_H
