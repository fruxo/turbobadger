// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_widgets_reader.h"
#include "tb_widgets_common.h"
#include "tb_scroll_container.h"
#include "tb_tab_container.h"
#include "tb_select.h"
#include "tb_inline_select.h"
#include "tb_editfield.h"
#include "tb_node_tree.h"
#include "tb_font_renderer.h"

namespace tb {

TB_WIDGET_FACTORY(TBWidget, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}

TB_WIDGET_FACTORY(TBButton, TBValue::TYPE_NULL, WIDGET_Z_BOTTOM)
{
	const char *axis = info->node->GetValueString("axis", "x");
	widget->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
}

TB_WIDGET_FACTORY(TBInlineSelect, TBValue::TYPE_INT, WIDGET_Z_TOP)
{
	const char *axis = info->node->GetValueString("axis", "x");
	widget->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
	int min = info->node->GetValueInt("min", widget->GetMinValue());
	int max = info->node->GetValueInt("max", widget->GetMaxValue());
	widget->SetLimits(min, max);
}

TB_WIDGET_FACTORY(TBClickLabel, TBValue::TYPE_STRING, WIDGET_Z_BOTTOM)
{
	const char *axis = info->node->GetValueString("axis", "x");
	widget->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
}

TB_WIDGET_FACTORY(TBEditField, TBValue::TYPE_STRING, WIDGET_Z_TOP)
{
	widget->SetMultiline(info->node->GetValueInt("multiline", 0) ? true : false);
	widget->SetStyling(info->node->GetValueInt("styling", 0) ? true : false);
	widget->SetReadOnly(info->node->GetValueInt("readonly", 0) ? true : false);
	widget->SetWrapping(info->node->GetValueInt("wrap", widget->GetWrapping()) ? true : false);
	widget->SetAdaptToContentSize(info->node->GetValueInt("adapt-to-content", widget->GetAdaptToContentSize()) ? true : false);
	if (const char *virtual_width = info->node->GetValueString("virtual-width", nullptr))
		widget->SetVirtualWidth(g_tb_skin->GetDimensionConverter()->GetPxFromString(virtual_width, widget->GetVirtualWidth()));
	if (const char *text = info->node->GetValueString("placeholder", nullptr))
		widget->SetPlaceholderText(text);
	if (const char *type = info->node->GetValueString("type", nullptr))
	{
		if (stristr(type, "text"))			widget->SetEditType(EDIT_TYPE_TEXT);
		else if (stristr(type, "search"))	widget->SetEditType(EDIT_TYPE_SEARCH);
		else if (stristr(type, "password"))	widget->SetEditType(EDIT_TYPE_PASSWORD);
		else if (stristr(type, "email"))	widget->SetEditType(EDIT_TYPE_EMAIL);
		else if (stristr(type, "phone"))	widget->SetEditType(EDIT_TYPE_PHONE);
		else if (stristr(type, "url"))		widget->SetEditType(EDIT_TYPE_URL);
		else if (stristr(type, "number"))	widget->SetEditType(EDIT_TYPE_NUMBER);
	}
}

TB_WIDGET_FACTORY(TBLayout, TBValue::TYPE_NULL, WIDGET_Z_TOP)
{
	const char *axis = info->node->GetValueString("axis", "x");
	widget->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
	if (const char *spacing = info->node->GetValueString("spacing", nullptr))
		widget->SetSpacing(g_tb_skin->GetDimensionConverter()->GetPxFromString(spacing, SPACING_FROM_SKIN));
	widget->SetGravity(WIDGET_GRAVITY_ALL);
	if (const char *size = info->node->GetValueString("size", nullptr))
	{
		LAYOUT_SIZE ls = LAYOUT_SIZE_PREFERRED;
		if (strstr(size, "available"))
			ls = LAYOUT_SIZE_AVAILABLE;
		else if (strstr(size, "gravity"))
			ls = LAYOUT_SIZE_GRAVITY;
		widget->SetLayoutSize(ls);
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
		widget->SetLayoutPosition(lp);
	}
	if (const char *pos = info->node->GetValueString("overflow", nullptr))
	{
		LAYOUT_OVERFLOW lo = LAYOUT_OVERFLOW_CLIP;
		if (strstr(pos, "scroll"))
			lo = LAYOUT_OVERFLOW_SCROLL;
		widget->SetLayoutOverflow(lo);
	}
	if (const char *dist = info->node->GetValueString("distribution", nullptr))
	{
		LAYOUT_DISTRIBUTION ld = LAYOUT_DISTRIBUTION_PREFERRED;
		if (strstr(dist, "available"))
			ld = LAYOUT_DISTRIBUTION_AVAILABLE;
		else if (strstr(dist, "gravity"))
			ld = LAYOUT_DISTRIBUTION_GRAVITY;
		widget->SetLayoutDistribution(ld);
	}
	if (const char *dist = info->node->GetValueString("distribution-position", nullptr))
	{
		LAYOUT_DISTRIBUTION_POSITION ld = LAYOUT_DISTRIBUTION_POSITION_CENTER;
		if (strstr(dist, "left") || strstr(dist, "top"))
			ld = LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP;
		else if (strstr(dist, "right") || strstr(dist, "bottom"))
			ld = LAYOUT_DISTRIBUTION_POSITION_RIGHT_BOTTOM;
		widget->SetLayoutDistributionPosition(ld);
	}
}

TB_WIDGET_FACTORY(TBScrollContainer, TBValue::TYPE_NULL, WIDGET_Z_TOP)
{
	widget->SetGravity(WIDGET_GRAVITY_ALL);
	widget->SetAdaptContentSize(info->node->GetValueInt("adapt-content", widget->GetAdaptContentSize()) ? true : false);
	widget->SetAdaptToContentSize(info->node->GetValueInt("adapt-to-content", widget->GetAdaptToContentSize()) ? true : false);
	if (const char *mode = info->node->GetValueString("scroll-mode", nullptr))
	{
		if (!strcmp(mode, "xy"))				widget->SetScrollMode(SCROLL_MODE_X_Y);
		if (!strcmp(mode, "y"))					widget->SetScrollMode(SCROLL_MODE_Y);
		if (!strcmp(mode, "y-auto"))			widget->SetScrollMode(SCROLL_MODE_Y_AUTO);
		if (!strcmp(mode, "auto"))				widget->SetScrollMode(SCROLL_MODE_X_AUTO_Y_AUTO);
		if (!strcmp(mode, "off"))				widget->SetScrollMode(SCROLL_MODE_OFF);
	}
}

TB_WIDGET_FACTORY(TBTabContainer, TBValue::TYPE_NULL, WIDGET_Z_TOP)
{
	const char *axis = info->node->GetValueString("axis", "y");
	widget->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
	if (const char *align = info->node->GetValueString("align", nullptr))
	{
		if (!strcmp(align, "left"))			widget->SetAlignment(TB_ALIGN_LEFT);
		else if (!strcmp(align, "top"))		widget->SetAlignment(TB_ALIGN_TOP);
		else if (!strcmp(align, "right"))	widget->SetAlignment(TB_ALIGN_RIGHT);
		else if (!strcmp(align, "bottom"))	widget->SetAlignment(TB_ALIGN_BOTTOM);
	}
	if (TBNode *tabs = info->node->GetNode("tabs"))
		info->reader->LoadNodeTree(widget->GetTabLayout(), tabs);
}

TB_WIDGET_FACTORY(TBScrollBar, TBValue::TYPE_FLOAT, WIDGET_Z_TOP)
{
	const char *axis = info->node->GetValueString("axis", "x");
	widget->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
	widget->SetGravity(*axis == 'x' ? WIDGET_GRAVITY_LEFT_RIGHT : WIDGET_GRAVITY_TOP_BOTTOM);
}

TB_WIDGET_FACTORY(TBSlider, TBValue::TYPE_FLOAT, WIDGET_Z_TOP)
{
	const char *axis = info->node->GetValueString("axis", "x");
	widget->SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
	widget->SetGravity(*axis == 'x' ? WIDGET_GRAVITY_LEFT_RIGHT : WIDGET_GRAVITY_TOP_BOTTOM);
	double min = (double) info->node->GetValueFloat("min", (float) widget->GetMinValue());
	double max = (double) info->node->GetValueFloat("max", (float) widget->GetMaxValue());
	widget->SetLimits(min, max);
}

void ReadItems(TBWidgetsReader *reader, TBNode *node, TBGenericStringItemSource *target_source)
{
	// If there is a items node, loop through all its children and add
	// items to the target item source.
	if (TBNode *items = node->GetNode("items"))
	{
		for (TBNode *n = items->GetFirstChild(); n; n = n->GetNext())
		{
			if (strcmp(n->GetName(), "item") != 0)
				continue;
			const char *item_str = n->GetValueString("text", "");
			TBID item_id;
			if (TBNode *n_id = n->GetNode("id"))
				reader->SetIDFromNode(item_id, n_id);

			TBGenericStringItem *item = new TBGenericStringItem(item_str, item_id);
			if (!item || !target_source->AddItem(item))
			{
				// Out of memory
				delete item;
				break;
			}
		}
	}
}

TB_WIDGET_FACTORY(TBSelectList, TBValue::TYPE_INT, WIDGET_Z_TOP)
{
	// Read items (if there is any) into the default source
	ReadItems(info->reader, info->node, widget->GetDefaultSource());
}

TB_WIDGET_FACTORY(TBSelectDropdown, TBValue::TYPE_INT, WIDGET_Z_TOP)
{
	// Read items (if there is any) into the default source
	ReadItems(info->reader, info->node, widget->GetDefaultSource());
}

TB_WIDGET_FACTORY(TBCheckBox, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBRadioButton, TBValue::TYPE_INT, WIDGET_Z_TOP) {}

TB_WIDGET_FACTORY(TBTextField, TBValue::TYPE_STRING, WIDGET_Z_TOP)
{
	if (const char *text_align = info->node->GetValueString("text-align", nullptr))
	{
		if (!strcmp(text_align, "left"))		widget->SetTextAlign(TB_TEXT_ALIGN_LEFT);
		else if (!strcmp(text_align, "center"))	widget->SetTextAlign(TB_TEXT_ALIGN_CENTER);
		else if (!strcmp(text_align, "right"))	widget->SetTextAlign(TB_TEXT_ALIGN_RIGHT);
	}
}

TB_WIDGET_FACTORY(TBSkinImage, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBSeparator, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBProgressSpinner, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBContainer, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}

// == TBWidgetFactory ===================================

// We can't use a linked list object since we don't know if its constructor
// would run before of after any widget factory constructor that add itself
// to it. Using a manual one way link list is very simple.
TBWidgetFactory *g_registered_factories = nullptr;

TBWidgetFactory::TBWidgetFactory(const char *name, TBValue::TYPE sync_type, WIDGET_Z add_child_z)
	: name(name)
	, sync_type(sync_type)
	, add_child_z(add_child_z)
	, next_registered_wf(nullptr)
{
}

void TBWidgetFactory::Register()
{
	next_registered_wf = g_registered_factories;
	g_registered_factories = this;
}

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
	for (TBWidgetFactory *wf = g_registered_factories; wf; wf = wf->next_registered_wf)
		if (!AddFactory(wf))
			return false;
	return true;
}

TBWidgetsReader::~TBWidgetsReader()
{
}

bool TBWidgetsReader::LoadFile(TBWidget *target, const char *filename)
{
	TBNode node;
	if (!node.ReadFile(filename))
		return false;
	LoadNodeTree(target, &node);
	return true;
}

bool TBWidgetsReader::LoadData(TBWidget *target, const char *data)
{
	TBNode node;
	node.ReadData(data);
	LoadNodeTree(target, &node);
	return true;
}

bool TBWidgetsReader::LoadData(TBWidget *target, const char *data, int data_len)
{
	TBNode node;
	node.ReadData(data, data_len);
	LoadNodeTree(target, &node);
	return true;
}

void TBWidgetsReader::LoadNodeTree(TBWidget *target, TBNode *node)
{
	// Iterate through all nodes and create widgets
	for (TBNode *child = node->GetFirstChild(); child; child = child->GetNext())
		CreateWidget(target, child, WIDGET_Z_TOP);
}

void TBWidgetsReader::SetIDFromNode(TBID &id, TBNode *node)
{
	if (!node)
		return;
	if (node->GetValue().IsString())
		id.Set(node->GetValue().GetString());
	else
		id.Set(node->GetValue().GetInt());
}

bool TBWidgetsReader::CreateWidget(TBWidget *target, TBNode *node, WIDGET_Z add_child_z)
{
	CREATE_INFO info = { this, target->GetContentRoot(), node };

	// Find a widget creator from the node name
	TBWidgetFactory *wc = nullptr;
	for (wc = factories.GetFirst(); wc; wc = wc->GetNext())
		if (strcmp(node->GetName(), wc->name) == 0)
			break;

	// Create the widget
	TBWidget *new_widget = wc ? wc->Create(&info) : nullptr;
	if (!new_widget)
		return false;

	// Read generic properties

	SetIDFromNode(new_widget->GetID(), node->GetNode("id"));

	SetIDFromNode(new_widget->GetGroupID(), node->GetNode("group-id"));

	if (wc->sync_type == TBValue::TYPE_FLOAT)
		new_widget->SetValueDouble(node->GetValueFloat("value", 0));
	else
		new_widget->SetValue(node->GetValueInt("value", 0));

	if (TBNode *data_node = node->GetNode("data"))
		new_widget->data.Copy(data_node->GetValue());

	new_widget->SetIgnoreInput(node->GetValueInt("ignore-input", new_widget->GetIgnoreInput()) ? true : false);

	if (const char *text = node->GetValueString("text", nullptr))
		new_widget->SetText(text);

	if (const char *connection = node->GetValueStringRaw("connection", nullptr))
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
	if (const char *gravity = node->GetValueString("gravity", nullptr))
	{
		WIDGET_GRAVITY g = WIDGET_GRAVITY_NONE;
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

	if (TBNode *lp = node->GetNode("lp"))
	{
		LayoutParams layout_params;
		if (new_widget->GetLayoutParams())
			layout_params = *new_widget->GetLayoutParams();
		const TBDimensionConverter *dc = g_tb_skin->GetDimensionConverter();
		if (const char *str = lp->GetValueString("width", nullptr))
			layout_params.SetWidth(dc->GetPxFromString(str, LayoutParams::UNSPECIFIED));
		if (const char *str = lp->GetValueString("height", nullptr))
			layout_params.SetHeight(dc->GetPxFromString(str, LayoutParams::UNSPECIFIED));
		if (const char *str = lp->GetValueString("min-width", nullptr))
			layout_params.min_w = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (const char *str = lp->GetValueString("max-width", nullptr))
			layout_params.max_w = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (const char *str = lp->GetValueString("pref-width", nullptr))
			layout_params.pref_w = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (const char *str = lp->GetValueString("min-height", nullptr))
			layout_params.min_h = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (const char *str = lp->GetValueString("max-height", nullptr))
			layout_params.max_h = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (const char *str = lp->GetValueString("pref-height", nullptr))
			layout_params.pref_h = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		new_widget->SetLayoutParams(layout_params);
	}

	target->GetContentRoot()->OnInflateChild(new_widget);

	// Add the new widget to the hiearchy
	target->GetContentRoot()->AddChild(new_widget, add_child_z);

	// Read the font now when the widget is in the hiearchy so inheritance works.
	if (TBNode *font = node->GetNode("font"))
	{
		TBFontDescription fd = new_widget->GetCalculatedFontDescription();
		if (const char *size = font->GetValueString("size", nullptr))
		{
			int new_size = g_tb_skin->GetDimensionConverter()->GetPxFromString(size, fd.GetSize());
			fd.SetSize(new_size);
		}
		if (const char *name = font->GetValueString("name", nullptr))
			fd.SetID(name);
		new_widget->SetFontDescription(fd);
	}

	// Iterate through all nodes and create widgets
	for (TBNode *n = node->GetFirstChild(); n; n = n->GetNext())
		CreateWidget(new_widget, n, wc->add_child_z);

	if (node->GetValueInt("autofocus", 0))
		new_widget->SetFocus(WIDGET_FOCUS_REASON_UNKNOWN);

	return true;
}

}; // namespace tb
