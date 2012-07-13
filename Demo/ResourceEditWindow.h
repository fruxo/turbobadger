#ifndef ResourceEditWindow_H
#define ResourceEditWindow_H

#include "tb_widgets.h"
#include "tb_select.h"
#include "tb_widgets_common.h"
#include "tb_widgets_listener.h"
#include "tb_msg.h"

using namespace tinkerbell;

class ResourceItem : public TBGenericStringItem
{
public:
	ResourceItem(Widget *widget, const char *str);
	Widget *GetWidget() { return m_widget; }
private:
	Widget *m_widget;
};

class ResourceEditWindow : public TBWindow, public TBMessageHandler, public TBGlobalWidgetListener
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("ResourceEditWindow", TBWindow);

	ResourceEditWindow();
	~ResourceEditWindow();

	void UpdateWidgetList(bool immediately);

	struct ITEM_INFO {
		ResourceItem *item;
		int index;
	};
	ITEM_INFO GetItemFromWidget(Widget *widget);
	Widget *GetSelectedWidget();

	void Load(const char *resource_file);
	void RefreshFromSource();

	// == TBWindow ======================================================================
	virtual bool OnEvent(const TBWidgetEvent &ev);
	virtual void OnPaintChildren(const PaintProps &paint_props);

	// == TBMessageHandler ==============================================================
	virtual void OnMessageReceived(TBMessage *msg);

	// == TBGlobalWidgetListener ========================================================
	virtual bool OnWidgetInvokeEvent(const TBWidgetEvent &ev);
	virtual void OnWidgetAdded(Widget *widget);
	virtual void OnWidgetRemove(Widget *widget);
private:
	TBSelectList *m_widget_list;
	TBSelectItemSourceList<ResourceItem> m_widget_list_source;
	Widget *m_build_container;
	TBEditField *m_source_edit;
	TBStr m_resource_filename;
	void AddWidgetListItemsRecursive(Widget *widget, int depth);
};

#endif // ResourceEditWindow_H
