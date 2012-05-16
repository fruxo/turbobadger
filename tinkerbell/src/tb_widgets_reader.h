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

typedef Widget *(*WIDGET_CREATE_CB)(CREATE_INFO *info);

class WidgetFactory : public TBLinkOf<WidgetFactory>
{
public:
	const char *name;
	WIDGET_CREATE_CB cb;
	WIDGET_Z add_child_z;
	TBValue::TYPE sync_type;
};

class TBWidgetsReader
{
public:
	static TBWidgetsReader *Create();
	~TBWidgetsReader();

	bool AddCreator(const char *name, WIDGET_CREATE_CB cb, TBValue::TYPE sync_type, WIDGET_Z add_child_z = WIDGET_Z_TOP);

	bool LoadFile(Widget *target, const char *filename);
	bool LoadData(Widget *target, const char *data);
	void LoadNodeTree(Widget *target, TBNode *node);
private:
	bool Init();
	bool CreateWidget(Widget *target, TBNode *node, WIDGET_Z add_child_z);
	TBLinkListAutoDeleteOf<WidgetFactory> callbacks;
};

}; // namespace tinkerbell

#endif // TBWIDGETS_READER_H
