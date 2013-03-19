// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TBWIDGETS_READER_H
#define TBWIDGETS_READER_H

#include "tb_linklist.h"
#include "tb_widgets.h"

namespace tinkerbell {

class TBWidgetsReader;
class TBWidget;
class TBNode;

struct CREATE_INFO {
	TBWidgetsReader *reader;
	TBWidget *target;
	TBNode *node;
};

/** TBWidgetFactory creates a widget from a TBNode. */
class TBWidgetFactory : public TBLinkOf<TBWidgetFactory>
{
public:
	TBWidgetFactory(const char *name, TBValue::TYPE sync_type, WIDGET_Z add_child_z);

	/** Create and return the new widget or nullptr on out of memory. */
	virtual TBWidget *Create(CREATE_INFO *info) = 0;

	void Register();
public:
	const char *name;
	TBValue::TYPE sync_type;
	WIDGET_Z add_child_z;
	TBWidgetFactory *next_registered_wf;
};

/** This macro creates a new TBWidgetFactory for the given class name (that should
	inherit TBWidget). It register it so it can be used to create widgets from
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
		virtual TBWidget *Create(CREATE_INFO *info) \
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

	Resource name:		TBWidget property:		Values:

	id					TBWidget::m_id			TBID (string or int)
	group-id			TBWidget::m_group_id	TBID (string or int)
	value				TBWidget::SetValue		integer
	data				TBWidget::m_data		integer
	text				TBWidget::SetText		string (translatable using @string)
	connection			TBWidget::Connect		string
	gravity				TBWidget::SetGravity	string (combination of left, top, right, bottom, or all)
	state				TBWidget::SetState		string (disabled)
	skin				TBWidget::SetSkinBg		TBID (string or int)
	autofocus			The TBWidget will be focused automatically the first time its TBWindow is activated.
	font>name			Font name
	font>size			Font size
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
		is preceded with a @, it will be looked up from the global TBLanguage.
		If the value is not specified, it returns the default value (def). */
	const char *GetTranslatableString(TBNode *node, const char *request, const char *def);

	/** Set the id from the given node. */
	void SetIDFromNode(TBID &id, TBNode *node);

	bool LoadFile(TBWidget *target, const char *filename);
	bool LoadData(TBWidget *target, const char *data);
	bool LoadData(TBWidget *target, const char *data, int data_len);
	void LoadNodeTree(TBWidget *target, TBNode *node);
private:
	bool Init();
	bool CreateWidget(TBWidget *target, TBNode *node, WIDGET_Z add_child_z);
	TBLinkListOf<TBWidgetFactory> factories;
};

}; // namespace tinkerbell

#endif // TBWIDGETS_READER_H
