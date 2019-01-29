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
#include "tb_toggle_container.h"
#include "image/tb_image_widget.h"

namespace tb {

static auto strstr(const TBStr & s1, const char *s2) { return ::strstr(s1.CStr(), s2); }
static auto stristr(const TBStr & s1, const char *s2) { return stristr(s1.CStr(), s2); }
static auto strcmp(const TBStr & s1, const char *s2) { return ::strcmp(s1.CStr(), s2); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

TB_WIDGET_FACTORY(TBWidget, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
void TBWidget::OnInflate(const INFLATE_INFO &info)
{
	TBWidgetsReader::SetIDFromNode(GetID(), info.node->GetNode("id"));

	TBWidgetsReader::SetIDFromNode(GetGroupID(), info.node->GetNode("group-id"));

	if (m_sync_type == TBValue::TYPE_FLOAT)
		SetValueDouble(info.node->GetValueFloat("value", 0));
	else if (m_sync_type == TBValue::TYPE_STRING)
		SetText(info.node->GetValueString("value", nullptr));
	else // if (info.sync_type == TBValue::TYPE_INT)
		SetValue(info.node->GetValueInt("value", 0));

	if (TBNode *data_node = info.node->GetNode("data"))
		data.Copy(data_node->GetValue());

	SetIsGroupRoot(info.node->GetValueInt("is-group-root", GetIsGroupRoot()) ? true : false);

	SetIsFocusable(info.node->GetValueInt("is-focusable", GetIsFocusable()) ? true : false);

	SetWantLongClick(info.node->GetValueInt("want-long-click", GetWantLongClick()) ? true : false);

	SetIgnoreInput(info.node->GetValueInt("ignore-input", GetIgnoreInput()) ? true : false);

	SetOpacity(info.node->GetValueFloat("opacity", GetOpacity()));

	if (TBStr text = info.node->GetValueString("text", nullptr))
		SetText(text);

	if (TBStr connection = info.node->GetValueStringRaw("connection", nullptr))
	{
		// If we already have a widget value with this name, just connect to it and the widget will
		// adjust its value to it. Otherwise create a new widget value, and give it the value we
		// got from the resource.
		if (TBWidgetValue *value = g_value_group.GetValue(connection))
			Connect(value);
		else if (TBWidgetValue *value = g_value_group.CreateValueIfNeeded(connection, m_sync_type))
		{
			value->SetFromWidget(this);
			Connect(value);
		}
	}
	if (TBStr axis = info.node->GetValueString("axis", nullptr))
		SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
	if (TBStr gravity = info.node->GetValueString("gravity", nullptr))
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
		SetGravity(g);
	}
	if (TBStr visibility = info.node->GetValueString("visibility", nullptr))
	{
		if (!strcmp(visibility, "visible"))			SetVisibility(WIDGET_VISIBILITY_VISIBLE);
		else if (!strcmp(visibility, "invisible"))	SetVisibility(WIDGET_VISIBILITY_INVISIBLE);
		else if (!strcmp(visibility, "gone"))		SetVisibility(WIDGET_VISIBILITY_GONE);
	}
	if (TBStr state = info.node->GetValueString("state", nullptr))
	{
		if (strstr(state, "disabled"))
			SetState(WIDGET_STATE_DISABLED, true);
		else if (strstr(state, "selected"))
			SetState(WIDGET_STATE_SELECTED, true);
	}
	if (TBStr skin = info.node->GetValueString("skin", nullptr))
	{
		if (!g_tb_skin->GetSkinElement(skin))
			TBDebugPrint("Widget '%s' requesting invalid skin element '%s'\n",
						 GetClassName(), skin.CStr());
		SetSkinBg(skin);
	}
	if (TBNode *lp = info.node->GetNode("lp"))
	{
		LayoutParams layout_params;
		if (GetLayoutParams())
			layout_params = *GetLayoutParams();
		const TBDimensionConverter *dc = g_tb_skin->GetDimensionConverter();
		if (TBStr str = lp->GetValueString("width", nullptr))
			layout_params.SetWidth(dc->GetPxFromString(str, LayoutParams::UNSPECIFIED));
		if (TBStr str = lp->GetValueString("height", nullptr))
			layout_params.SetHeight(dc->GetPxFromString(str, LayoutParams::UNSPECIFIED));
		if (TBStr str = lp->GetValueString("min-width", nullptr))
			layout_params.min_w = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("max-width", nullptr))
			layout_params.max_w = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("pref-width", nullptr))
			layout_params.pref_w = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("min-height", nullptr))
			layout_params.min_h = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("max-height", nullptr))
			layout_params.max_h = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("pref-height", nullptr))
			layout_params.pref_h = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		SetLayoutParams(layout_params);
	}

	// Add the new widget to the hiearchy if not already added.
	if (!GetParent())
		info.target->AddChild(this, info.target->GetZInflate());

	// Read the font now when the widget is in the hiearchy so inheritance works.
	if (TBNode *font = info.node->GetNode("font"))
	{
		TBFontDescription fd = GetCalculatedFontDescription();
		if (TBStr size = font->GetValueString("size", nullptr))
		{
			int new_size = g_tb_skin->GetDimensionConverter()->GetPxFromString(size, fd.GetSize());
			fd.SetSize(new_size);
		}
		if (TBStr name = font->GetValueString("name", nullptr))
			fd.SetID(name);
		SetFontDescription(fd);
	}

	info.target->OnInflateChild(this);

	if (TBNode *rect_node = info.node->GetNode("rect"))
	{
		const TBDimensionConverter *dc = g_tb_skin->GetDimensionConverter();
		TBValue &val = rect_node->GetValue();
		if (val.GetArrayLength() == 4)
		{
			SetRect(TBRect(dc->GetPxFromValue(val.GetArray()->GetValue(0), 0),
				dc->GetPxFromValue(val.GetArray()->GetValue(1), 0),
				dc->GetPxFromValue(val.GetArray()->GetValue(2), 0),
				dc->GetPxFromValue(val.GetArray()->GetValue(3), 0)));
		}
	}
}

/** Create a named ID node, IFF id != def */
static void OptCreateID(TBNode * target, const char * name, const TBID & id, const TBID def = TBID())
{
	if (id != def) {
		TBNode * new_node = TBNode::Create(name);
#ifdef TB_RUNTIME_DEBUG_INFO
		if (id.debug_string.Length())
			new_node->GetValue().SetString(id.debug_string);
		else
#endif
			new_node->GetValue().SetInt(id);
		target->Add(new_node);
	}
}

/** Create a named int node, IF val != def */
static void OptCreateInt(TBNode * target, const char * name, const long val,
						 const long def = (long)0xdeadbeef)
{
	if (val != def || def == (long)0xdeadbeef) {
		TBNode * new_node = TBNode::Create(name);
		new_node->GetValue().SetInt(val);
		target->Add(new_node);
	}
}

/** Create a named float node, IF val != def */
static void OptCreateFloat(TBNode * target, const char * name, const double val,
						   const double def = (double)0xdeadbeef)
{
	if (val != def || def == (double)0xdeadbeef) {
		TBNode * new_node = TBNode::Create(name);
		new_node->GetValue().SetFloat(val);
		target->Add(new_node);
	}
}

/** Create a named string node, IF val != def */
static void OptCreateString(TBNode * target, const char * name, const TBStr & val,
							const TBStr def = TBStr())
{
	if (val && val.Length() && (!def || val == def)) {
		TBNode * new_node = TBNode::Create(name);
		new_node->GetValue().SetString(val);
		target->Add(new_node);
	}
}

struct MTEnum {
	const char *name;
	int val;
};

/** Create an enum node, IF val != def */
static void OptCreateEnum(TBNode * target, const char * name, int val, int def, MTEnum * enums, bool bitmask = false)
{
	if (val != def) {
		TBNode * new_node = TBNode::Create(name);
		TBStr s;
		for (MTEnum * e = enums; e->name; e++) {
			if ((bitmask && (e->val & val)) || (!bitmask && e->val == val)) {
				if (s.Length())
					s.Append(" ");
				s.Append(e->name);
			}
		}
		new_node->GetValue().SetString(std::move(s));
		target->Add(new_node);
	}
}

/** Create a rect node, IF val != def */
static void OptCreateRect(TBNode * target, const char * name, const TBRect & val, const TBRect & def = TBRect())
{
	if (!val.Equals(def)) {
		TBNode * new_node = TBNode::Create(name);
		TBStr s;
		s.SetFormatted("[%d x %d @ %d,%d]", val.w, val.h, val.x, val.y);
		new_node->GetValue().SetString(s);
		target->Add(new_node);
	}
}

void TBWidget::OnDeflate(const INFLATE_INFO &info)
{
	TBNode * node = info.node;
	OptCreateID(node, "id", GetID());
	OptCreateID(node, "group-id", GetGroupID());

	switch (m_sync_type) {
	case TBValue::TYPE_FLOAT:
		OptCreateFloat(node, "value", GetValueDouble(), 0);
		break;
	case TBValue::TYPE_STRING:
		OptCreateString(node, "value", GetText(), "");
		break;
	case TBValue::TYPE_INT:
		OptCreateInt(node, "value", GetValue());
		break;
	default:
		break;
	}

	if (TBNode *data_node = info.node->GetNode("data"))
		data.Copy(data_node->GetValue());

	OptCreateInt(node, "is-group-root", GetIsGroupRoot(), false);
	OptCreateInt(node, "is-focusable", GetIsFocusable(), false);
	OptCreateInt(node, "want-long-click", GetWantLongClick(), false);
	OptCreateInt(node, "ignore-input", GetIgnoreInput(), false);
	OptCreateFloat(node, "opacity", GetOpacity(), 1.);
	OptCreateString(node, "text", GetText(), "");

#if 0
	if (TBStr connection = info.node->GetValueStringRaw("connection", nullptr))
	{
		// If we already have a widget value with this name, just connect to it and the widget will
		// adjust its value to it. Otherwise create a new widget value, and give it the value we
		// got from the resource.
		if (TBWidgetValue *value = g_value_group.GetValue(connection))
			Connect(value);
		else if (TBWidgetValue *value = g_value_group.CreateValueIfNeeded(connection, m_sync_type))
		{
			value->SetFromWidget(this);
			Connect(value);
		}
	}
#endif

	MTEnum axis [] = {{"x", AXIS_X}, {"y", AXIS_Y}, {nullptr, 0}};
	OptCreateEnum(node, "axis", GetAxis(), AXIS_X, axis);

	MTEnum gravity [] = {{"left", WIDGET_GRAVITY_LEFT},
						 {"top", WIDGET_GRAVITY_TOP},
						 {"right", WIDGET_GRAVITY_RIGHT},
						 {"bottom", WIDGET_GRAVITY_BOTTOM},
						 {nullptr, 0}};
	if (GetGravity() == WIDGET_GRAVITY_ALL)
		OptCreateString(node, "gravity", "all");
	else
		OptCreateEnum(node, "gravity", GetGravity(), WIDGET_GRAVITY_NONE, gravity, true);

	MTEnum visibility [] = {{"visible", WIDGET_VISIBILITY_VISIBLE},
							{"invisible", WIDGET_VISIBILITY_INVISIBLE},
							{"gone", WIDGET_VISIBILITY_GONE},
							{nullptr, 0}};
	OptCreateEnum(node, "visibility", GetVisibility(), WIDGET_VISIBILITY_VISIBLE, visibility);

	MTEnum state [] = {{"disabled", WIDGET_STATE_DISABLED},
					   {"selected", WIDGET_STATE_SELECTED},
					   {"pressed", WIDGET_STATE_PRESSED},
					   {"hovered", WIDGET_STATE_HOVERED},
					   {"focused", WIDGET_STATE_FOCUSED},
					   {nullptr, 0}};
	OptCreateEnum(node, "state", GetStateRaw(), WIDGET_STATE_NONE, state, true);
	OptCreateEnum(node, "autostate", GetAutoState(), WIDGET_STATE_NONE, state, true);

	if (m_skin_bg)
		OptCreateID(node, "skin", m_skin_bg, node->GetName());

	if (0 && m_font_desc.GetFontFaceID() != 0) {
		OptCreateID(node, "font>name", m_font_desc.GetID());
		OptCreateInt(node, "font>size", m_font_desc.GetSize());
	}

	OptCreateRect(node, "rect", GetRect());

#if 0
	if (TBNode *lp = info.node->GetNode("lp"))
	{
		LayoutParams layout_params;
		if (GetLayoutParams())
			layout_params = *GetLayoutParams();
		const TBDimensionConverter *dc = g_tb_skin->GetDimensionConverter();
		if (TBStr str = lp->GetValueString("width", nullptr))
			layout_params.SetWidth(dc->GetPxFromString(str, LayoutParams::UNSPECIFIED));
		if (TBStr str = lp->GetValueString("height", nullptr))
			layout_params.SetHeight(dc->GetPxFromString(str, LayoutParams::UNSPECIFIED));
		if (TBStr str = lp->GetValueString("min-width", nullptr))
			layout_params.min_w = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("max-width", nullptr))
			layout_params.max_w = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("pref-width", nullptr))
			layout_params.pref_w = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("min-height", nullptr))
			layout_params.min_h = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("max-height", nullptr))
			layout_params.max_h = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		if (TBStr str = lp->GetValueString("pref-height", nullptr))
			layout_params.pref_h = dc->GetPxFromString(str, LayoutParams::UNSPECIFIED);
		SetLayoutParams(layout_params);
	}
#endif
}

TB_WIDGET_FACTORY(TBWindow, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}

TB_WIDGET_FACTORY(TBButton, TBValue::TYPE_INT, WIDGET_Z_BOTTOM) {}
void TBButton::OnInflate(const INFLATE_INFO &info)
{
	SetSqueezable(info.node->GetValueInt("squeezable", GetSqueezable()) ? true : false);
	SetAutoRepeat(info.node->GetValueInt("auto-repeat", GetAutoRepeat()) ? true : false);
	SetToggleMode(info.node->GetValueInt("toggle-mode", GetToggleMode()) ? true : false);
	TBWidget::OnInflate(info);
}

void TBButton::OnDeflate(const INFLATE_INFO &info)
{
	TBNode * node = info.node;
	TBWidget::OnDeflate(info);
	OptCreateInt(node, "squeezable", GetSqueezable(), false);
	OptCreateInt(node, "auto-repeat", GetAutoRepeat(), false);
	OptCreateInt(node, "toggle-mode", GetToggleMode(), false);
}

TB_WIDGET_FACTORY(TBInlineSelect, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
void TBInlineSelect::OnInflate(const INFLATE_INFO &info)
{
	int min = info.node->GetValueInt("min", GetMinValue());
	int max = info.node->GetValueInt("max", GetMaxValue());
	SetLimits(min, max);
	TBWidget::OnInflate(info);
}

TB_WIDGET_FACTORY(TBClickLabel, TBValue::TYPE_STRING, WIDGET_Z_BOTTOM) {}

TB_WIDGET_FACTORY(TBEditField, TBValue::TYPE_STRING, WIDGET_Z_TOP) {}
void TBEditField::OnInflate(const INFLATE_INFO &info)
{
	SetMultiline(info.node->GetValueInt("multiline", 0) ? true : false);
	SetStyling(info.node->GetValueInt("styling", 0) ? true : false);
	SetReadOnly(info.node->GetValueInt("readonly", 0) ? true : false);
	SetWrapping(info.node->GetValueInt("wrap", GetWrapping()) ? true : false);
	SetAdaptToContentSize(info.node->GetValueInt("adapt-to-content", GetAdaptToContentSize()) ? true : false);
	if (TBStr virtual_width = info.node->GetValueString("virtual-width", nullptr))
		SetVirtualWidth(g_tb_skin->GetDimensionConverter()->GetPxFromString(virtual_width, GetVirtualWidth()));
	if (TBStr text = info.node->GetValueString("placeholder", nullptr))
		SetPlaceholderText(text);
	if (TBStr type = info.node->GetValueString("type", nullptr))
	{
		if (stristr(type, "text"))			SetEditType(EDIT_TYPE_TEXT);
		else if (stristr(type, "search"))	SetEditType(EDIT_TYPE_SEARCH);
		else if (stristr(type, "password"))	SetEditType(EDIT_TYPE_PASSWORD);
		else if (stristr(type, "email"))	SetEditType(EDIT_TYPE_EMAIL);
		else if (stristr(type, "phone"))	SetEditType(EDIT_TYPE_PHONE);
		else if (stristr(type, "url"))		SetEditType(EDIT_TYPE_URL);
		else if (stristr(type, "number"))	SetEditType(EDIT_TYPE_NUMBER);
		else TBDebugPrint("TBEditField: Unknown type '%s'\n", type.CStr());
	}
	if (TBStr text_align = info.node->GetValueString("text-align", nullptr))
	{
		if (!strcmp(text_align, "left"))		SetTextAlign(TB_TEXT_ALIGN_LEFT);
		else if (!strcmp(text_align, "center"))	SetTextAlign(TB_TEXT_ALIGN_CENTER);
		else if (!strcmp(text_align, "right"))	SetTextAlign(TB_TEXT_ALIGN_RIGHT);
		else TBDebugPrint("TBEditField: Unknown text-align '%s'\n", text_align.CStr());
	}
	if (TBStr format = info.node->GetValueString("format", nullptr))
		SetFormat(std::move(format));
	TBWidget::OnInflate(info);
}

void TBEditField::OnDeflate(const INFLATE_INFO &info)
{
	TBNode * node = info.node;
	TBWidget::OnDeflate(info);
	OptCreateInt(node, "multiline", GetMultiline(), false);
	OptCreateInt(node, "styling", GetStyling(), false);
	OptCreateInt(node, "readonly", GetReadOnly(), false);
	OptCreateInt(node, "wrap", GetWrapping(), false);
	OptCreateInt(node, "adapt-to-content", GetAdaptToContentSize(), false);
	MTEnum type [] = {{"text", EDIT_TYPE_TEXT},
					   {"search", EDIT_TYPE_SEARCH},
					   {"password", EDIT_TYPE_PASSWORD},
					   {"email", EDIT_TYPE_EMAIL},
					   {"phone", EDIT_TYPE_PHONE},
					   {"url", EDIT_TYPE_URL},
					   {"number", EDIT_TYPE_NUMBER},
					   {nullptr, 0}};
	OptCreateEnum(node, "type", GetEditType(), EDIT_TYPE_TEXT, type, false);
	OptCreateString(node, "format", GetFormat());
}

TB_WIDGET_FACTORY(TBLayout, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
void TBLayout::OnInflate(const INFLATE_INFO &info)
{
	if (TBStr spacing = info.node->GetValueString("spacing", nullptr))
		SetSpacing(g_tb_skin->GetDimensionConverter()->GetPxFromString(spacing, SPACING_FROM_SKIN));
	SetGravity(WIDGET_GRAVITY_ALL);
	if (TBStr size = info.node->GetValueString("size", nullptr))
	{
		LAYOUT_SIZE ls = LAYOUT_SIZE_PREFERRED;
		if (strstr(size, "available"))
			ls = LAYOUT_SIZE_AVAILABLE;
		else if (strstr(size, "gravity"))
			ls = LAYOUT_SIZE_GRAVITY;
		else if (strstr(size, "preferred"))
			;
		else
			TBDebugPrint("TBLayout: Unknown size '%s'\n", size.CStr());
		SetLayoutSize(ls);
	}
	if (TBStr pos = info.node->GetValueString("position", nullptr))
	{
		LAYOUT_POSITION lp = LAYOUT_POSITION_CENTER;
		if (strstr(pos, "left") || strstr(pos, "top"))
			lp = LAYOUT_POSITION_LEFT_TOP;
		else if (strstr(pos, "right") || strstr(pos, "bottom"))
			lp = LAYOUT_POSITION_RIGHT_BOTTOM;
		else if (strstr(pos, "gravity"))
			lp = LAYOUT_POSITION_GRAVITY;
		else if (!strcmp(pos, "center"))
			;
		else
			TBDebugPrint("TBLayout: Unknown position '%s'\n", pos.CStr());
		SetLayoutPosition(lp);
	}
	if (TBStr pos = info.node->GetValueString("overflow", nullptr))
	{
		LAYOUT_OVERFLOW lo = LAYOUT_OVERFLOW_CLIP;
		if (strstr(pos, "scroll"))
			lo = LAYOUT_OVERFLOW_SCROLL;
		else if (strstr(pos, "clip"))
			;
		else
			TBDebugPrint("TBLayout: Unknown overflow '%s'\n", pos.CStr());
		SetLayoutOverflow(lo);
	}
	if (TBStr dist = info.node->GetValueString("distribution", nullptr))
	{
		LAYOUT_DISTRIBUTION ld = LAYOUT_DISTRIBUTION_PREFERRED;
		if (strstr(dist, "available"))
			ld = LAYOUT_DISTRIBUTION_AVAILABLE;
		else if (strstr(dist, "gravity"))
			ld = LAYOUT_DISTRIBUTION_GRAVITY;
		else if (strstr(dist, "preferred"))
			;
		else
			TBDebugPrint("TBLayout: Unknown distribution '%s'\n", dist.CStr());
		SetLayoutDistribution(ld);
	}
	if (TBStr dist = info.node->GetValueString("distribution-position", nullptr))
	{
		LAYOUT_DISTRIBUTION_POSITION ld = LAYOUT_DISTRIBUTION_POSITION_CENTER;
		if (strstr(dist, "left") || strstr(dist, "top"))
			ld = LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP;
		else if (strstr(dist, "right") || strstr(dist, "bottom"))
			ld = LAYOUT_DISTRIBUTION_POSITION_RIGHT_BOTTOM;
		else if (!strcmp(dist, "center"))
			;
		else
			TBDebugPrint("TBLayout: Unknown distribution-position '%s'\n", dist.CStr());
		SetLayoutDistributionPosition(ld);
	}
	TBWidget::OnInflate(info);
}

void TBLayout::OnDeflate(const INFLATE_INFO &info)
{
	TBNode * node = info.node;
	TBWidget::OnDeflate(info);

	OptCreateInt(node, "spacing", GetSpacing(), 0);

	MTEnum size [] = {{"preferred", LAYOUT_SIZE_PREFERRED},
					  {"available", LAYOUT_SIZE_AVAILABLE},
					  {"gravity", LAYOUT_SIZE_GRAVITY},
					  {nullptr, 0}};
	OptCreateEnum(node, "size", m_packed.layout_mode_size, -1 /*LAYOUT_SIZE_PREFERRED*/, size);

	MTEnum position [] = {{"center", LAYOUT_POSITION_CENTER},
						  {"left top", LAYOUT_POSITION_LEFT_TOP},
						  {"bottom right", LAYOUT_POSITION_RIGHT_BOTTOM},
						  {"gravity", LAYOUT_POSITION_GRAVITY},
						  {nullptr, 0}};
	OptCreateEnum(node, "position", m_packed.layout_mode_pos, -1 /*LAYOUT_POSITION_CENTER*/, position);

	MTEnum overflow [] = {{"scroll", LAYOUT_OVERFLOW_SCROLL},
						  {"clip", LAYOUT_OVERFLOW_CLIP},
						  {nullptr, 0}};
	OptCreateEnum(node, "overflow", m_packed.layout_mode_overflow, -1 /*LAYOUT_POSITION_CENTER*/, overflow);

	MTEnum distribution [] = {{"preferred", LAYOUT_DISTRIBUTION_PREFERRED},
							  {"available", LAYOUT_DISTRIBUTION_AVAILABLE},
							  {"gravity", LAYOUT_DISTRIBUTION_GRAVITY},
							  {nullptr, 0}};
	OptCreateEnum(node, "distribution", m_packed.layout_mode_dist,
				  -1 /*LAYOUT_POSITION_CENTER*/, distribution);

	MTEnum distribution_pos [] = {{"center", LAYOUT_DISTRIBUTION_POSITION_CENTER},
								  {"left top", LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP},
								  {"bottom right", LAYOUT_DISTRIBUTION_POSITION_RIGHT_BOTTOM},
								  {nullptr, 0}};
	OptCreateEnum(node, "distribution-position", m_packed.layout_mode_dist_pos,
				  -1 /*LAYOUT_POSITION_CENTER*/, distribution_pos);
}

TB_WIDGET_FACTORY(TBScrollContainer, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
void TBScrollContainer::OnInflate(const INFLATE_INFO &info)
{
	SetGravity(WIDGET_GRAVITY_ALL);
	SetAdaptContentSize(info.node->GetValueInt("adapt-content", GetAdaptContentSize()) ? true : false);
	SetAdaptToContentSize(info.node->GetValueInt("adapt-to-content", GetAdaptToContentSize()) ? true : false);
	if (TBStr mode = info.node->GetValueString("scroll-mode", nullptr))
	{
		if (!strcmp(mode, "xy"))			SetScrollMode(SCROLL_MODE_X_Y);
		else if (!strcmp(mode, "y"))		SetScrollMode(SCROLL_MODE_Y);
		else if (!strcmp(mode, "y-auto"))	SetScrollMode(SCROLL_MODE_Y_AUTO);
		else if (!strcmp(mode, "auto"))		SetScrollMode(SCROLL_MODE_X_AUTO_Y_AUTO);
		else if (!strcmp(mode, "off"))		SetScrollMode(SCROLL_MODE_OFF);
		else TBDebugPrint("TBScrollContainer: Unknown scroll-mode '%s'\n", mode.CStr());
	}
	TBWidget::OnInflate(info);
}

TB_WIDGET_FACTORY(TBTabContainer, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
void TBTabContainer::OnInflate(const INFLATE_INFO &info)
{
	TBWidget::OnInflate(info);

	if (TBStr align = info.node->GetValueString("align", nullptr))
	{
		if (!strcmp(align, "left"))			SetAlignment(TB_ALIGN_LEFT);
		else if (!strcmp(align, "top"))		SetAlignment(TB_ALIGN_TOP);
		else if (!strcmp(align, "right"))	SetAlignment(TB_ALIGN_RIGHT);
		else if (!strcmp(align, "bottom"))	SetAlignment(TB_ALIGN_BOTTOM);
		else TBDebugPrint("TBTabContainer: Unknown align '%s'\n", align.CStr());
	}
	// Allow additional attributes to be specified for the "tabs", "content" and "root" layouts by
	// calling OnInflate.
	if (TBNode *tabs = info.node->GetNode("tabs"))
	{
		// Inflate the tabs widgets into the tab layout.
		TBLayout *tab_layout = GetTabLayout();
		info.reader->LoadNodeTree(tab_layout, tabs);

		INFLATE_INFO inflate_info(info.reader, tab_layout->GetContentRoot(), tabs, TBValue::TYPE_NULL);
		tab_layout->OnInflate(inflate_info);
	}
	if (TBNode *tabs = info.node->GetNode("content"))
	{
		INFLATE_INFO inflate_info(info.reader, GetContentRoot(), tabs, TBValue::TYPE_NULL);
		GetContentRoot()->OnInflate(inflate_info);
	}
	if (TBNode *tabs = info.node->GetNode("root"))
	{
		INFLATE_INFO inflate_info(info.reader, &m_root_layout, tabs, TBValue::TYPE_NULL);
		m_root_layout.OnInflate(inflate_info);
	}
}

TB_WIDGET_FACTORY(TBScrollBar, TBValue::TYPE_FLOAT, WIDGET_Z_TOP) {}
void TBScrollBar::OnInflate(const INFLATE_INFO &info)
{
	TBStr axis = info.node->GetValueString("axis", "x");
	SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
	SetGravity(*axis == 'x' ? WIDGET_GRAVITY_LEFT_RIGHT : WIDGET_GRAVITY_TOP_BOTTOM);
	TBWidget::OnInflate(info);
}

template <typename VAL_T>
void TBSliderX<VAL_T>::OnInflate(const INFLATE_INFO &info)
{
	TBStr axis = info.node->GetValueString("axis", "x");
	SetAxis(*axis == 'x' ? AXIS_X : AXIS_Y);
	SetGravity(*axis == 'x' ? WIDGET_GRAVITY_LEFT_RIGHT : WIDGET_GRAVITY_TOP_BOTTOM);
	double min = (double)info.node->GetValueFloat("min", (float)GetMinValue());
	double max = (double)info.node->GetValueFloat("max", (float)GetMaxValue());
	SetLimits(min, max);
	double step = (double)info.node->GetValueFloat("step", (float)GetSmallStep());
	SetSmallStep(step);
	TBWidget::OnInflate(info);
}
TB_WIDGET_FACTORY(TBSlider, TBValue::TYPE_FLOAT, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBSliderInt, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBSliderLong, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
template class TBSliderX<double>;
template class TBSliderX<int>;
template class TBSliderX<long>;

void ReadItems(TBNode *node, TBGenericStringItemSource *target_source)
{
	// If there is a items node, loop through all its children and add
	// items to the target item source.
	if (TBNode *items = node->GetNode("items"))
	{
		for (TBNode *n = items->GetFirstChild(); n; n = n->GetNext())
		{
			if (strcmp(n->GetName(), "item") != 0 && strcmp(n->GetName(), "menu") != 0)
				continue;
			TBStr item_str = n->GetValueString("text", "");
			TBID item_id;
			if (TBNode *n_id = n->GetNode("id"))
				TBWidgetsReader::SetIDFromNode(item_id, n_id);

			TBGenericStringItem *item = new TBGenericStringItem(item_str, item_id);
			if (!item || !target_source->AddItem(item))
			{
				// Out of memory
				delete item;
				break;
			}
			if (strcmp(n->GetName(), "menu") == 0) {
				TBGenericStringItemSource * msource = new TBGenericStringItemSource();
				ReadItems(n, msource);
				item->sub_source = msource;
			}
		}
	}
}

TB_WIDGET_FACTORY(TBSelectList, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
void TBSelectList::OnInflate(const INFLATE_INFO &info)
{
	// Read items (if there is any) into the default source
	ReadItems(info.node, GetDefaultSource());
	TBWidget::OnInflate(info);
}

TB_WIDGET_FACTORY(TBSelectDropdown, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
void TBSelectDropdown::OnInflate(const INFLATE_INFO &info)
{
	// Read items (if there is any) into the default source
	ReadItems(info.node, GetDefaultSource());
	TBWidget::OnInflate(info);
}

TB_WIDGET_FACTORY(TBCheckBox, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBRadioButton, TBValue::TYPE_INT, WIDGET_Z_TOP) {}

TB_WIDGET_FACTORY(TBTextField, TBValue::TYPE_STRING, WIDGET_Z_TOP) {}
void TBTextField::OnInflate(const INFLATE_INFO &info)
{
	if (TBStr text_align = info.node->GetValueString("text-align", nullptr))
	{
		if (!strcmp(text_align, "left"))		SetTextAlign(TB_TEXT_ALIGN_LEFT);
		else if (!strcmp(text_align, "center"))	SetTextAlign(TB_TEXT_ALIGN_CENTER);
		else if (!strcmp(text_align, "right"))	SetTextAlign(TB_TEXT_ALIGN_RIGHT);
	}
	if (TBStr format = info.node->GetValueString("format", nullptr))
		SetFormat(std::move(format));
	TBWidget::OnInflate(info);
}
void TBTextField::OnDeflate(const INFLATE_INFO &info)
{
	TBNode * node = info.node;
	TBWidget::OnDeflate(info);
	OptCreateString(node, "format", GetFormat());
}

TB_WIDGET_FACTORY(TBSkinImage, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBSeparator, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBProgressSpinner, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBContainer, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBSectionHeader, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
TB_WIDGET_FACTORY(TBSection, TBValue::TYPE_INT, WIDGET_Z_TOP) {}

TB_WIDGET_FACTORY(TBToggleContainer, TBValue::TYPE_INT, WIDGET_Z_TOP) {}
void TBToggleContainer::OnInflate(const INFLATE_INFO &info)
{
	if (TBStr toggle = info.node->GetValueString("toggle", nullptr))
	{
		if (stristr(toggle, "enabled"))			SetToggle(TBToggleContainer::TOGGLE_ENABLED);
		else if (stristr(toggle, "opacity"))	SetToggle(TBToggleContainer::TOGGLE_OPACITY);
		else if (stristr(toggle, "expanded"))	SetToggle(TBToggleContainer::TOGGLE_EXPANDED);
	}
	SetInvert(info.node->GetValueInt("invert", GetInvert()) ? true : false);
	TBWidget::OnInflate(info);
}

#ifdef TB_IMAGE

TB_WIDGET_FACTORY(TBImageWidget, TBValue::TYPE_NULL, WIDGET_Z_TOP) {}
void TBImageWidget::OnInflate(const INFLATE_INFO &info)
{
	if (TBStr filename = info.node->GetValueString("filename", nullptr))
		SetImage(filename.CStr());
	SetAdaptTextColor(info.node->GetValueInt("adapt-text-color", false) ? true : false);
	TBWidget::OnInflate(info);
}

#endif // TB_IMAGE

// == TBWidgetFactory ===================================

// We can't use a linked list object since we don't know if its constructor
// would run before of after any widget factory constructor that add itself
// to it. Using a manual one way link list is very simple.
TBWidgetFactory *g_registered_factories = nullptr;

TBWidgetFactory::TBWidgetFactory(const char *name, TBValue::TYPE s)
	: name(name)
	, sync_type(s)
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

bool TBWidgetsReader::LoadFile(TBWidget *target, const TBStr & filename)
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
		CreateWidget(target, child);
}

bool TBWidgetsReader::DumpFile(TBWidget *source, const TBStr & filename)
{
	TBNode node;
	if (!CreateNode(&node, source) || !node.WriteFile(filename))
		return false;
	return true;
}

bool TBWidgetsReader::DumpData(TBWidget *source, TBStr & data)
{
	TBNode node;
	if (!CreateNode(&node, source) || !node.WriteNode(data))
		return false;
	return true;
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

bool TBWidgetsReader::CreateWidget(TBWidget *target, TBNode *node)
{
	// Find a widget creator from the node name
	TBWidgetFactory *wc = nullptr;
	for (wc = factories.GetFirst(); wc; wc = wc->GetNext())
		if (strcmp(node->GetName(), wc->name) == 0)
			break;
	if (!wc)
		return false;

	// Create the widget
	INFLATE_INFO info(this, target->GetContentRoot(), node, wc->sync_type);
	TBWidget *new_widget = wc->Create(&info);
	if (!new_widget)
		return false;

	// Read properties and add i to the hierarchy.
	new_widget->OnInflate(info);

	// If this assert is trigged, you probably forgot to call TBWidget::OnInflate from an overridden version.
	assert(new_widget->GetParent());

	// Iterate through all nodes and create widgets
	for (TBNode *n = node->GetFirstChild(); n; n = n->GetNext())
		CreateWidget(new_widget, n);

	if (node->GetValueInt("autofocus", 0))
		new_widget->SetFocus(WIDGET_FOCUS_REASON_UNKNOWN);

	return true;
}

bool TBWidgetsReader::CreateNode(TBNode *target, TBWidget *widget)
{
	// Find a widget creator from the node name
	TBWidgetFactory *wc = nullptr;
	TBValue::TYPE sync_type = TBValue::TYPE_NULL;
	for (wc = factories.GetFirst(); wc; wc = wc->GetNext())
		if (strcmp(widget->GetClassName(), wc->name) == 0)
			break;
	if (wc)
		sync_type = wc->sync_type;

	// Create the node
	TBNode *new_node = TBNode::Create(widget->GetClassName());
	if (!new_node)
		return false;
	INFLATE_INFO info(this, widget, new_node, sync_type);

	// Read properties and add i to the hierarchy.
	widget->OnDeflate(info);

	// Add the node to the node tree
	target->Add(new_node);

	// Iterate through all children and create their nodes
	for (TBWidget *w = widget->GetFirstChild(); w; w = w->GetNext())
		CreateNode(new_node, w);

	return true;
}

#pragma GCC diagnostic pop

} // namespace tb
