// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_widgets_reader.h"
#include "tb_widgets_common.h"
#include "tb_scroll_container.h"
#include "tb_tab_container.h"
#include "tb_select.h"
#include "tb_editfield.h"
#include "tb_language.h"
#include "parser/TBNodeTree.h"

namespace tinkerbell {

/** Simple widget (that doesn't have any nongeneric data), can use this to create its creation callback. */
#define DECLARE_CREATE_FUNCTION(classname) \
	Widget *Create##classname(CREATE_INFO *info) \
	{ \
		if (classname *new_widget = new classname()) \
			return new_widget; \
		return nullptr; \
	}

const char *GetTranslatableString(TBNode *node, const char *request)
{
	if (const char *string = node->GetValueString(request, nullptr))
	{
		// FIX: If it's a number after @, look it up from the number!
		if (*string == '@')
			string = g_tb_lng->GetString(string + 1);
		return string;
	}
	return nullptr;
}

Widget *CreateTBButton(CREATE_INFO *info)
{
	if (TBButton *button = new TBButton())
	{
		const char *axis = info->node->GetValueString("axis", "x");
		button->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
		return button;
	}
	return nullptr;
}

Widget *CreateTBInlineSelect(CREATE_INFO *info)
{
	if (TBInlineSelect *iselect = new TBInlineSelect())
	{
		const char *axis = info->node->GetValueString("axis", "x");
		iselect->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
		int min = info->node->GetValueInt("min", iselect->GetMinValue());
		int max = info->node->GetValueInt("max", iselect->GetMaxValue());
		iselect->SetLimits(min, max);
		return iselect;
	}
	return nullptr;
}

Widget *CreateTBClickLabel(CREATE_INFO *info)
{
	if (TBClickLabel *label = new TBClickLabel())
	{
		const char *axis = info->node->GetValueString("axis", "x");
		label->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
		return label;
	}
	return nullptr;
}

DECLARE_CREATE_FUNCTION(TBTextField);

Widget *CreateTBEditField(CREATE_INFO *info)
{
	if (TBEditField *editfield = new TBEditField())
	{
		editfield->SetMultiline(info->node->GetValueInt("multiline", 0) ? true : false);
		editfield->SetStyling(info->node->GetValueInt("styling", 0) ? true : false);
		editfield->SetReadOnly(info->node->GetValueInt("readonly", 0) ? true : false);
		editfield->SetWrapping(info->node->GetValueInt("wrap", editfield->GetWrapping()) ? true : false);
		if (const char *text = GetTranslatableString(info->node, "placeholder"))
			editfield->SetPlaceholderText(text);
		if (const char *type = info->node->GetValueString("type", nullptr))
		{
			if (stristr(type, "text"))			editfield->SetEditType(EDIT_TYPE_TEXT);
			else if (stristr(type, "password"))	editfield->SetEditType(EDIT_TYPE_PASSWORD);
			else if (stristr(type, "email"))	editfield->SetEditType(EDIT_TYPE_EMAIL);
			else if (stristr(type, "phone"))	editfield->SetEditType(EDIT_TYPE_PHONE);
			else if (stristr(type, "url"))		editfield->SetEditType(EDIT_TYPE_URL);
			else if (stristr(type, "number"))	editfield->SetEditType(EDIT_TYPE_NUMBER);
		}
		return editfield;
	}
	return nullptr;
}

DECLARE_CREATE_FUNCTION(TBCheckBox);
DECLARE_CREATE_FUNCTION(TBRadioButton);

Widget *CreateTBLayout(CREATE_INFO *info)
{
	if (TBLayout *layout = new TBLayout())
	{
		const char *axis = info->node->GetValueString("axis", "x");
		layout->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
		layout->SetSpacing(info->node->GetValueInt("spacing", SPACING_FROM_SKIN));
		layout->SetGravity(WIDGET_GRAVITY_ALL);
		if (const char *size = info->node->GetValueString("size", nullptr))
		{
			LAYOUT_SIZE ls = LAYOUT_SIZE_PREFERRED;
			if (strstr(size, "available"))
				ls = LAYOUT_SIZE_AVAILABLE;
			else if (strstr(size, "gravity"))
				ls = LAYOUT_SIZE_GRAVITY;
			layout->SetLayoutSize(ls);
		}
		if (const char *pos = info->node->GetValueString("position", nullptr))
		{
			LAYOUT_POSITION lp = LAYOUT_POSITION_CENTER;
			if (strstr(pos, "left") || strstr(pos, "top"))
				lp = LAYOUT_POSITION_LEFT_TOP;
			else if (strstr(pos, "right") || strstr(pos, "bottom"))
				lp = LAYOUT_POSITION_RIGHT_BOTTOM;
			else if (strstr(pos, "gravity"))
				lp = LAYOUT_POSITION_GRAVITY;
			layout->SetLayoutPosition(lp);
		}
		if (const char *pos = info->node->GetValueString("overflow", nullptr))
		{
			LAYOUT_OVERFLOW lo = LAYOUT_OVERFLOW_CLIP;
			if (strstr(pos, "scroll"))
				lo = LAYOUT_OVERFLOW_SCROLL;
			layout->SetLayoutOverflow(lo);
		}
		if (const char *dist = info->node->GetValueString("distribution", nullptr))
		{
			LAYOUT_DISTRIBUTION ld = LAYOUT_DISTRIBUTION_PREFERRED;
			if (strstr(dist, "available"))
				ld = LAYOUT_DISTRIBUTION_AVAILABLE;
			else if (strstr(dist, "gravity"))
				ld = LAYOUT_DISTRIBUTION_GRAVITY;
			layout->SetLayoutDistribution(ld);
		}
		if (const char *dist = info->node->GetValueString("distribution_position", nullptr))
		{
			LAYOUT_DISTRIBUTION_POSITION ld = LAYOUT_DISTRIBUTION_POSITION_CENTER;
			if (strstr(dist, "left") || strstr(dist, "top"))
				ld = LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP;
			else if (strstr(dist, "right") || strstr(dist, "bottom"))
				ld = LAYOUT_DISTRIBUTION_POSITION_RIGHT_BOTTOM;
			layout->SetLayoutDistributionPosition(ld);
		}
		return layout;
	}
	return nullptr;
}

Widget *CreateTBScrollContainer(CREATE_INFO *info)
{
	if (TBScrollContainer *container = new TBScrollContainer())
	{
		container->SetGravity(WIDGET_GRAVITY_ALL);
		return container;
	}
	return nullptr;
}

Widget *CreateTBTabContainer(CREATE_INFO *info)
{
	if (TBTabContainer *container = new TBTabContainer())
	{
		container->SetGravity(WIDGET_GRAVITY_ALL);
		const char *axis = info->node->GetValueString("axis", "y");
		container->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
		if (const char *align = info->node->GetValueString("align", nullptr))
		{
			if (!strcmp(align, "left"))			container->SetAlignment(TB_ALIGN_LEFT);
			else if (!strcmp(align, "top"))		container->SetAlignment(TB_ALIGN_TOP);
			else if (!strcmp(align, "right"))	container->SetAlignment(TB_ALIGN_RIGHT);
			else if (!strcmp(align, "bottom"))	container->SetAlignment(TB_ALIGN_BOTTOM);
		}
		if (TBNode *tabs = info->node->GetNode("tabs"))
			info->reader->LoadNodeTree(container->GetTabLayout(), tabs);
		return container;
	}
	return nullptr;
}

DECLARE_CREATE_FUNCTION(TBSelectDropdown);
DECLARE_CREATE_FUNCTION(TBSelectList);

Widget *CreateTBScrollBar(CREATE_INFO *info)
{
	if (TBScrollBar *scrollbar = new TBScrollBar())
	{
		const char *axis = info->node->GetValueString("axis", "x");
		scrollbar->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
		scrollbar->SetGravity(*axis == 'x' ? WIDGET_GRAVITY_LEFT_RIGHT : WIDGET_GRAVITY_TOP_BOTTOM);
		return scrollbar;
	}
	return nullptr;
}

Widget *CreateTBSlider(CREATE_INFO *info)
{
	if (TBSlider *slider = new TBSlider())
	{
		const char *axis = info->node->GetValueString("axis", "x");
		slider->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
		slider->SetGravity(*axis == 'x' ? WIDGET_GRAVITY_LEFT_RIGHT : WIDGET_GRAVITY_TOP_BOTTOM);
		double min = (double) info->node->GetValueFloat("min", (float) slider->GetMinValue());
		double max = (double) info->node->GetValueFloat("max", (float) slider->GetMaxValue());
		slider->SetLimits(min, max);
		return slider;
	}
	return nullptr;
}

DECLARE_CREATE_FUNCTION(TBSkinImage);
DECLARE_CREATE_FUNCTION(TBSeparator);
DECLARE_CREATE_FUNCTION(TBProgressSpinner);
DECLARE_CREATE_FUNCTION(TBContainer);

// == TBWidgetsReader ===================================

TBWidgetsReader *TBWidgetsReader::Create()
{
	TBWidgetsReader *w_reader = new TBWidgetsReader;
	if (!w_reader || !w_reader->Init())
	{
		delete w_reader;
		return nullptr;
	}
	return w_reader;
}

bool TBWidgetsReader::Init()
{
	bool fail = false;
	fail |= !AddCreator("TBButton", CreateTBButton, TBValue::TYPE_NULL, WIDGET_Z_BOTTOM);
	fail |= !AddCreator("TBInlineSelect", CreateTBInlineSelect, TBValue::TYPE_INT);
	fail |= !AddCreator("TBClickLabel", CreateTBClickLabel, TBValue::TYPE_STRING, WIDGET_Z_BOTTOM);
	fail |= !AddCreator("TBTextField", CreateTBTextField, TBValue::TYPE_STRING);
	fail |= !AddCreator("TBEditField", CreateTBEditField, TBValue::TYPE_STRING);
	fail |= !AddCreator("TBCheckBox", CreateTBCheckBox, TBValue::TYPE_INT);
	fail |= !AddCreator("TBRadioButton", CreateTBRadioButton, TBValue::TYPE_INT);
	fail |= !AddCreator("TBLayout", CreateTBLayout, TBValue::TYPE_NULL);
	fail |= !AddCreator("TBScrollContainer", CreateTBScrollContainer, TBValue::TYPE_NULL);
	fail |= !AddCreator("TBTabContainer", CreateTBTabContainer, TBValue::TYPE_NULL);
	fail |= !AddCreator("TBSelectDropdown", CreateTBSelectDropdown, TBValue::TYPE_INT);
	fail |= !AddCreator("TBSelectList", CreateTBSelectList, TBValue::TYPE_INT);
	fail |= !AddCreator("TBScrollBar", CreateTBScrollBar, TBValue::TYPE_FLOAT);
	fail |= !AddCreator("TBSlider", CreateTBSlider, TBValue::TYPE_FLOAT);
	fail |= !AddCreator("TBSkinImage", CreateTBSkinImage, TBValue::TYPE_NULL);
	fail |= !AddCreator("TBSeparator", CreateTBSeparator, TBValue::TYPE_NULL);
	fail |= !AddCreator("TBProgressSpinner", CreateTBProgressSpinner, TBValue::TYPE_INT);
	fail |= !AddCreator("TBContainer", CreateTBContainer, TBValue::TYPE_NULL);
	return !fail;
}

TBWidgetsReader::~TBWidgetsReader()
{
}

bool TBWidgetsReader::AddCreator(const char *name, WIDGET_CREATE_CB cb, TBValue::TYPE sync_type, WIDGET_Z add_child_z)
{
	WidgetFactory *wc = new WidgetFactory;
	if (!wc)
		return false;
	wc->name = name;
	wc->cb = cb;
	wc->add_child_z = add_child_z;
	wc->sync_type = sync_type;
	callbacks.AddLast(wc);
	return true;
}

bool TBWidgetsReader::LoadFile(Widget *target, const char *filename)
{
	TBNode node;
	if (!node.ReadFile(filename))
		return false;
	LoadNodeTree(target, &node);
	return true;
}

bool TBWidgetsReader::LoadData(Widget *target, const char *data)
{
	TBNode node;
	node.ReadData(data);
	LoadNodeTree(target, &node);
	return true;
}

bool TBWidgetsReader::LoadData(Widget *target, const char *data, int data_len)
{
	TBNode node;
	node.ReadData(data, data_len);
	LoadNodeTree(target, &node);
	return true;
}

void TBWidgetsReader::LoadNodeTree(Widget *target, TBNode *node)
{
	// Iterate through all nodes and create widgets
	for (TBNode *n = node->GetFirstChild(); n; n = n->GetNext())
		CreateWidget(target, n, WIDGET_Z_TOP);
}

void SetIDFromNode(TBID &id, TBNode *node)
{
	if (!node)
		return;
	if (node->GetValue().IsString())
		id.Set(node->GetValue().GetString());
	else
		id.Set(node->GetValue().GetInt());
}

bool TBWidgetsReader::CreateWidget(Widget *target, TBNode *node, WIDGET_Z add_child_z)
{
	CREATE_INFO info = { this, target->GetContentRoot(), node };

	// Find a widget creator from the node name
	WidgetFactory *wc = nullptr;
	for (wc = callbacks.GetFirst(); wc; wc = wc->GetNext())
		if (strcmp(node->GetName(), wc->name) == 0)
			break;

	// Create the widget
	Widget *new_widget = wc ? wc->cb(&info) : nullptr;
	if (!new_widget)
		return false;

	// Read generic properties
	new_widget->SetRect(target->GetContentRoot()->GetPaddingRect());
	SetIDFromNode(new_widget->m_id, node->GetNode("id"));
	SetIDFromNode(new_widget->m_group_id, node->GetNode("group_id"));
	new_widget->SetValue(node->GetValueInt("value", 0));
	new_widget->m_data = node->GetValueInt("data", 0);
	if (const char *text = GetTranslatableString(node, "text"))
		new_widget->SetText(text);
	if (const char *connection = node->GetValueString("connection", nullptr))
	{
		// If we already have a widget value with this name, just connect to it and the widget will
		// adjust its value to it. Otherwise create a new widget value, and give it the value we
		// got from the resource.
		if (TBWidgetValue *value = g_value_group.GetValue(connection))
			new_widget->Connect(value);
		else if (TBWidgetValue *value = g_value_group.CreateValueIfNeeded(connection, wc->sync_type))
		{
			value->SetFromWidget(new_widget);
			new_widget->Connect(value);
		}
	}
	if (const char *bg_skin = node->GetValueString("bg_skin", nullptr))
		new_widget->m_skin_bg.Set(bg_skin);
	if (const char *gravity = node->GetValueString("gravity", nullptr))
	{
		WIDGET_GRAVITY g = 0;
		if (strstr(gravity, "left"))		g |= WIDGET_GRAVITY_LEFT;
		if (strstr(gravity, "top"))			g |= WIDGET_GRAVITY_TOP;
		if (strstr(gravity, "right"))		g |= WIDGET_GRAVITY_RIGHT;
		if (strstr(gravity, "bottom"))		g |= WIDGET_GRAVITY_BOTTOM;
		if (strstr(gravity, "all"))			g |= WIDGET_GRAVITY_ALL;
		if (!(g & WIDGET_GRAVITY_LEFT_RIGHT))
			g |= WIDGET_GRAVITY_LEFT;
		if (!(g & WIDGET_GRAVITY_TOP_BOTTOM))
			g |= WIDGET_GRAVITY_TOP;
		new_widget->SetGravity(g);
	}
	if (const char *state = node->GetValueString("state", nullptr))
	{
		if (strstr(state, "disabled"))
			new_widget->SetState(WIDGET_STATE_DISABLED, true);
	}
	if (const char *skin = node->GetValueString("skin", nullptr))
	{
		new_widget->SetSkinBg(skin);
	}

	target->GetContentRoot()->AddChild(new_widget, add_child_z);

	// Iterate through all nodes and create widgets
	for (TBNode *n = node->GetFirstChild(); n; n = n->GetNext())
		CreateWidget(new_widget, n, wc->add_child_z);

	if (node->GetValueInt("autofocus", 0))
		new_widget->SetFocus(WIDGET_FOCUS_REASON_UNKNOWN);

	return true;
}

}; // namespace tinkerbell
