// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_skin.h"
#include "tb_system.h"
#include "tb_tempbuffer.h"
#include "parser/TBNodeTree.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

namespace tinkerbell {

// == Util functions ==========================================================

uint32 StringToState(const char *state_str)
{
	uint32 state = 0;
	if (strstr(state_str, "all"))		state |= SKIN_STATE_ALL;
	if (strstr(state_str, "disabled"))	state |= SKIN_STATE_DISABLED;
	if (strstr(state_str, "focused"))	state |= SKIN_STATE_FOCUSED;
	if (strstr(state_str, "pressed"))	state |= SKIN_STATE_PRESSED;
	if (strstr(state_str, "selected"))	state |= SKIN_STATE_SELECTED;
	if (strstr(state_str, "hovered"))	state |= SKIN_STATE_HOVERED;
	return state;
}

TBSkinCondition::TARGET StringToTarget(const char *target_str)
{
	TBSkinCondition::TARGET target = TBSkinCondition::TARGET_THIS;
	if (strcmp(target_str, "this") == 0)
		target = TBSkinCondition::TARGET_THIS;
	else if (strcmp(target_str, "parent") == 0)
		target = TBSkinCondition::TARGET_PARENT;
	else if (strcmp(target_str, "ancestors") == 0)
		target = TBSkinCondition::TARGET_ANCESTORS;
	else
		TBDebugOut("Skin error: Unknown target in condition!\n");
	return target;
}

TBSkinCondition::PROPERTY StringToProperty(const char *prop_str)
{
	TBSkinCondition::PROPERTY prop = TBSkinCondition::PROPERTY_SKIN;
	if (strcmp(prop_str, "skin") == 0)
		prop = TBSkinCondition::PROPERTY_SKIN;
	else if (strcmp(prop_str, "window active") == 0)
		prop = TBSkinCondition::PROPERTY_WINDOW_ACTIVE;
	else if (strcmp(prop_str, "axis") == 0)
		prop = TBSkinCondition::PROPERTY_AXIS;
	else if (strcmp(prop_str, "align") == 0)
		prop = TBSkinCondition::PROPERTY_ALIGN;
	else if (strcmp(prop_str, "id") == 0)
		prop = TBSkinCondition::PROPERTY_ID;
	else if (strcmp(prop_str, "state") == 0)
		prop = TBSkinCondition::PROPERTY_STATE;
	else if (strcmp(prop_str, "hover") == 0)
		prop = TBSkinCondition::PROPERTY_HOVER;
	else if (strcmp(prop_str, "capture") == 0)
		prop = TBSkinCondition::PROPERTY_CAPTURE;
	else if (strcmp(prop_str, "focus") == 0)
		prop = TBSkinCondition::PROPERTY_FOCUS;
	else
		prop = TBSkinCondition::PROPERTY_CUSTOM;
	return prop;
}

// == TBSkinCondition =======================================================

TBSkinCondition::TBSkinCondition(TARGET target, PROPERTY prop, const TBID &custom_prop, const TBID &value, TEST test)
	: m_target(target)
	, m_test(test)
{
	m_info.prop = prop;
	m_info.custom_prop = custom_prop;
	m_info.value = value;
}

bool TBSkinCondition::GetCondition(TBSkinConditionContext &context) const
{
	bool equal = context.GetCondition(m_target, m_info);
	return equal == (m_test == TEST_EQUAL);
}

// == TBSkin ================================================================

TBSkin::TBSkin()
	: m_parent_skin(nullptr)
	, m_override_skin(nullptr)
{
	g_renderer->AddListener(this);

	// Avoid filtering artifacts at edges when we draw fragments stretched.
	m_frag_manager.SetAddBorder(true);
}

bool TBSkin::Load(const char *skin_file, const char *override_skin_file)
{
	if (override_skin_file)
	{
		// Create and load the override skin first.
		m_override_skin = new TBSkin;
		if (m_override_skin)
		{
			m_override_skin->m_parent_skin = this;
			m_override_skin->Load(override_skin_file);
		}
	}

	TBNode node;
	if (!node.ReadFile(skin_file))
		return false;

	TBTempBuffer skin_path;
	if (!skin_path.AppendPath(skin_file))
		return false;

	if (const char *color = node.GetValueString("defaults>text-color", nullptr))
		m_default_text_color.SetFromString(color, strlen(color));

	// Iterate through all elements nodes and add skin elements
	TBNode *elements = node.GetNode("elements");
	if (elements)
	{
		TBNode *n = elements->GetFirstChild();
		while (n)
		{
			while (TBNode *clone = n->GetNode("clone"))
			{
				n->Remove(clone);

				TBNode *clone_source = elements->GetNode(clone->GetValue().GetString());
				if (clone_source)
					n->CloneChildren(clone_source);

				delete clone;
			}
			TBSkinElement *e = new TBSkinElement;
			if (!e)
				return false;

			const char *bitmap = n->GetValueString("bitmap", nullptr);
			if (bitmap)
			{
				e->bitmap_file.Append(skin_path.GetData());
				e->bitmap_file.Append(bitmap);
			}

			e->cut = n->GetValueInt("cut", 0);
			e->expand = n->GetValueInt("expand", 0);
			e->name.Set(n->GetName());
			e->id.Set(n->GetName());

			if (TBNode *padding_node = n->GetNode("padding"))
			{
				TBValue &val = padding_node->GetValue();
				if (val.GetArrayLength() == 4)
				{
					e->padding_left = val.GetArray()->GetValue(0)->GetInt();
					e->padding_top = val.GetArray()->GetValue(1)->GetInt();
					e->padding_right = val.GetArray()->GetValue(2)->GetInt();
					e->padding_bottom = val.GetArray()->GetValue(3)->GetInt();
				}
				else if (val.GetArrayLength() == 2)
				{
					e->padding_left = e->padding_right = val.GetArray()->GetValue(0)->GetInt();
					e->padding_top = e->padding_bottom = val.GetArray()->GetValue(1)->GetInt();
				}
				else
					e->padding_left = e->padding_top = e->padding_right = e->padding_bottom = val.GetInt();
			}

			e->min_width = n->GetValueInt("min-width", SKIN_VALUE_NOT_SPECIFIED);
			e->min_height = n->GetValueInt("min-height", SKIN_VALUE_NOT_SPECIFIED);
			e->max_width = n->GetValueInt("max-width", SKIN_VALUE_NOT_SPECIFIED);
			e->max_height = n->GetValueInt("max-height", SKIN_VALUE_NOT_SPECIFIED);
			e->spacing = n->GetValueInt("spacing", SKIN_DEFAULT_SPACING);
			e->content_ofs_x = n->GetValueInt("content-ofs-x", 0);
			e->content_ofs_y = n->GetValueInt("content-ofs-y", 0);
			e->img_position_x = n->GetValueInt("img-position-x", 50);
			e->img_position_y = n->GetValueInt("img-position-y", 50);
			e->img_ofs_x = n->GetValueInt("img-ofs-x", 0);
			e->img_ofs_y = n->GetValueInt("img-ofs-y", 0);
			e->opacity = n->GetValueFloat("opacity", 1.f);

			if (const char *color = n->GetValueString("text-color", nullptr))
				e->text_color.SetFromString(color, strlen(color));
			else
				e->text_color = m_default_text_color;

			const char *type = n->GetValueString("type", "StretchBox");
			if (strcmp(type, "Image") == 0)
				e->type = SKIN_ELEMENT_TYPE_IMAGE;
			else if (strcmp(type, "Stretch Image") == 0)
				e->type = SKIN_ELEMENT_TYPE_STRETCH_IMAGE;
			else if (strcmp(type, "Tile") == 0)
				e->type = SKIN_ELEMENT_TYPE_TILE;
			else if (strcmp(type, "StretchBorder") == 0)
				e->type = SKIN_ELEMENT_TYPE_STRETCH_BORDER;
			else
				e->type = SKIN_ELEMENT_TYPE_STRETCH_BOX;

			// Create all state elements
			e->m_override_elements.Load(n->GetNode("overrides"));
			e->m_strong_override_elements.Load(n->GetNode("strong-overrides"));
			e->m_child_elements.Load(n->GetNode("children"));
			e->m_overlay_elements.Load(n->GetNode("overlay"));

#ifdef _DEBUG
			if (TBSkinElement *other = GetSkinElement(TBID(e->name)))
			{
				assert(other->name.Equals(e->name)); // You have a duplicate in the skin!
				assert(!other->name.Equals(e->name)); // You have two different skin names which hashes collide!
			}
#endif
			m_elements.Add(e->id, e);
			n = n->GetNext();
		}
	}
	if (m_parent_skin)
		return true; // ReloadBitmaps() is done by parent skin.
	return ReloadBitmaps();
}

void TBSkin::UnloadBitmaps()
{
	if (m_override_skin)
		m_override_skin->UnloadBitmaps();

	// Unset all bitmap pointers.
	TBHashTableIteratorOf<TBSkinElement> it(&m_elements);
	while (TBSkinElement *element = it.GetNextContent())
		element->bitmap = nullptr;

	// Clear all fragments and bitmaps.
	m_frag_manager.Clear();
}

bool TBSkin::ReloadBitmaps()
{
	UnloadBitmaps();
	bool success = ReloadBitmapsInternal();
	// Create all bitmaps for the bitmap fragment maps
	if (success)
		success = m_frag_manager.ValidateBitmaps();

#ifdef _DEBUG
	TBStr info;
	info.SetFormatted("Skin loaded using %d bitmaps.\n", m_frag_manager.GetNumMaps());
	TBDebugOut(info);
#endif
	return success;
}

bool TBSkin::ReloadBitmapsInternal()
{
	// Load all bitmap files into new bitmap fragments.
	// Let override skins use the same fragment manager as the parent
	// skin so we will pack their fragments into the same maps.
	TBBitmapFragmentManager *frag_man = m_parent_skin ? &m_parent_skin->m_frag_manager : &m_frag_manager;
	bool success = true;
	TBHashTableIteratorOf<TBSkinElement> it(&m_elements);
	while (TBSkinElement *element = it.GetNextContent())
	{
		if (!element->bitmap_file.IsEmpty())
		{
			// FIX: dedicated_map is not needed for all backends (only deprecated fixed function GL)
			bool dedicated_map = element->type == SKIN_ELEMENT_TYPE_TILE;
			element->bitmap = frag_man->GetFragmentFromFile(element->bitmap_file, dedicated_map);
			if (!element->bitmap)
				success = false;
		}
	}
	if (m_override_skin && !m_override_skin->ReloadBitmapsInternal())
		success = false;
	return success;
}

TBSkin::~TBSkin()
{
	delete m_override_skin;
	g_renderer->RemoveListener(this);
}

TBSkinElement *TBSkin::GetSkinElement(const TBID &skin_id) const
{
	if (!skin_id)
		return nullptr;

	// First see if the override skin can paint this element
	if (m_override_skin)
	{
		if (TBSkinElement *element = m_override_skin->GetSkinElement(skin_id))
			return element;
	}

	return m_elements.Get(skin_id);
}

TBSkinElement *TBSkin::GetSkinElementStrongOverride(const TBID &skin_id, uint32 state, TBSkinConditionContext &context) const
{
	if (TBSkinElement *skin_element = GetSkinElement(skin_id))
	{
		// Avoid eternal recursion when overrides refer to elements referring back.
		if (skin_element->is_getting)
			return nullptr;
		skin_element->is_getting = true;

		// Check if there's any strong overrides for this element with the given state.
		TBSkinElementState *override_state = skin_element->m_strong_override_elements.GetStateElement(state, context);
		if (override_state)
		{
			if (TBSkinElement *override_element = GetSkinElementStrongOverride(override_state->element_id, state, context))
			{
				skin_element->is_getting = false;
				return override_element;
			}
		}

		skin_element->is_getting = false;
		return skin_element;
	}
	return nullptr;
}

TBSkinElement *TBSkin::PaintSkin(const TBRect &dst_rect, const TBID &skin_id, uint32 state, TBSkinConditionContext &context)
{
	return PaintSkin(dst_rect, GetSkinElement(skin_id), state, context);
}

TBSkinElement *TBSkin::PaintSkin(const TBRect &dst_rect, TBSkinElement *element, uint32 state, TBSkinConditionContext &context)
{
	if (!element || element->is_painting)
		return nullptr;

	// Avoid potential endless recursion in evil skins
	element->is_painting = true;

	// Return the override if we have one.
	TBSkinElement *return_element = element;

	// If there's any override for this state, paint it. Otherwise paint the standard skin element.
	TBSkinElementState *override_state = element->m_override_elements.GetStateElement(state, context);
	if (override_state)
		return_element = PaintSkin(dst_rect, override_state->element_id, state, context);
	else
		PaintElement(dst_rect, element);

	// Paint all child elements that matches the state (or should be painted for all states)
	if (element->m_child_elements.HasStateElements())
	{
		const TBSkinElementState *state_element = element->m_child_elements.GetFirstElement();
		while (state_element)
		{
			if (state_element->IsMatch(state, context))
				PaintSkin(dst_rect, state_element->element_id, state_element->state & state, context);
			state_element = state_element->GetNext();
		}
	}

	element->is_painting = false;
	return return_element;
}

void TBSkin::PaintSkinOverlay(const TBRect &dst_rect, TBSkinElement *element, uint32 state, TBSkinConditionContext &context)
{
	if (!element || element->is_painting)
		return;

	// Avoid potential endless recursion in evil skins
	element->is_painting = true;

	// Paint all overlay elements that matches the state (or should be painted for all states)
	const TBSkinElementState *state_element = element->m_overlay_elements.GetFirstElement();
	while (state_element)
	{
		if (state_element->IsMatch(state, context))
			PaintSkin(dst_rect, state_element->element_id, state_element->state & state, context);
		state_element = state_element->GetNext();
	}

	element->is_painting = false;
}

void TBSkin::PaintElement(const TBRect &dst_rect, TBSkinElement *element)
{
	if (!element->bitmap)
		return;
	if (element->type == SKIN_ELEMENT_TYPE_IMAGE)
		PaintElementImage(dst_rect, element);
	else if (element->type == SKIN_ELEMENT_TYPE_TILE)
		PaintElementTile(dst_rect, element);
	else if (element->type == SKIN_ELEMENT_TYPE_STRETCH_IMAGE || element->cut == 0)
		PaintElementStretchImage(dst_rect, element);
	else if (element->type == SKIN_ELEMENT_TYPE_STRETCH_BORDER)
		PaintElementStretchBox(dst_rect, element, false);
	else
		PaintElementStretchBox(dst_rect, element, true);
}

void TBSkin::PaintElementImage(const TBRect &dst_rect, TBSkinElement *element)
{
	TBRect src_rect(0, 0, element->bitmap->Width(), element->bitmap->Height());
	TBRect rect(dst_rect.x + element->img_ofs_x + (dst_rect.w - src_rect.w) * element->img_position_x / 100,
				dst_rect.y + element->img_ofs_y + (dst_rect.h - src_rect.h) * element->img_position_y / 100,
				src_rect.w, src_rect.h);
	g_renderer->DrawBitmap(rect, src_rect, element->bitmap);
}

void TBSkin::PaintElementTile(const TBRect &dst_rect, TBSkinElement *element)
{
	g_renderer->DrawBitmapTile(dst_rect, element->bitmap->GetBitmap());
}

void TBSkin::PaintElementStretchImage(const TBRect &dst_rect, TBSkinElement *element)
{
	if (dst_rect.IsEmpty())
		return;
	TBRect rect = dst_rect.Expand(element->expand, element->expand);
	g_renderer->DrawBitmap(rect, TBRect(0, 0, element->bitmap->Width(), element->bitmap->Height()), element->bitmap);
}

void TBSkin::PaintElementStretchBox(const TBRect &dst_rect, TBSkinElement *element, bool fill_center)
{
	if (dst_rect.IsEmpty())
		return;

	TBRect rect = dst_rect.Expand(element->expand, element->expand);

	// Stretch the dst_cut (if rect is smaller than the skin size)
	// FIX: the expand should also be stretched!
	int cut = element->cut;
	int dst_cut_w = MIN(cut, rect.w / 2);
	int dst_cut_h = MIN(cut, rect.h / 2);
	int bw = element->bitmap->Width();
	int bh = element->bitmap->Height();

	// Corners
	g_renderer->DrawBitmap(TBRect(rect.x, rect.y, dst_cut_w, dst_cut_h), TBRect(0, 0, cut, cut), element->bitmap);
	g_renderer->DrawBitmap(TBRect(rect.x + rect.w - dst_cut_w, rect.y, dst_cut_w, dst_cut_h), TBRect(bw - cut, 0, cut, cut), element->bitmap);
	g_renderer->DrawBitmap(TBRect(rect.x, rect.y + rect.h - dst_cut_h, dst_cut_w, dst_cut_h), TBRect(0, bh - cut, cut, cut), element->bitmap);
	g_renderer->DrawBitmap(TBRect(rect.x + rect.w - dst_cut_w, rect.y + rect.h - dst_cut_h, dst_cut_w, dst_cut_h), TBRect(bw - cut, bh - cut, cut, cut), element->bitmap);

	// Left & right edge
	if (rect.h > dst_cut_h * 2)
	{
		g_renderer->DrawBitmap(TBRect(rect.x, rect.y + dst_cut_h, dst_cut_w, rect.h - dst_cut_h * 2), TBRect(0, cut, cut, bh - cut * 2), element->bitmap);
		g_renderer->DrawBitmap(TBRect(rect.x + rect.w - dst_cut_w, rect.y + dst_cut_h, dst_cut_w, rect.h - dst_cut_h * 2), TBRect(bw - cut, cut, cut, bh - cut * 2), element->bitmap);
	}

	// Top & bottom edge
	if (rect.w > dst_cut_w * 2)
	{
		g_renderer->DrawBitmap(TBRect(rect.x + dst_cut_w, rect.y, rect.w - dst_cut_w * 2, dst_cut_h), TBRect(cut, 0, bw - cut * 2, cut), element->bitmap);
		g_renderer->DrawBitmap(TBRect(rect.x + dst_cut_w, rect.y + rect.h - dst_cut_h, rect.w - dst_cut_w * 2, dst_cut_h), TBRect(cut, bh - cut, bw - cut * 2, cut), element->bitmap);
	}

	// Center
	if (fill_center && rect.w > dst_cut_w * 2 && rect.h > dst_cut_h * 2)
		g_renderer->DrawBitmap(TBRect(rect.x + dst_cut_w, rect.y + dst_cut_h, rect.w - dst_cut_w * 2, rect.h - dst_cut_h * 2), TBRect(cut, cut, bw - cut * 2, bh - cut * 2), element->bitmap);
}

#ifdef _DEBUG
void TBSkin::Debug()
{
	m_frag_manager.Debug();
}
#endif // _DEBUG

void TBSkin::OnContextLost()
{
	// We could simply do: m_frag_manager.DeleteBitmaps() and then all bitmaps
	// would be recreated automatically when needed. But because it's easy,
	// we unload everything so we save some memory (by not keeping any image
	// data around).
	// Override skins are handled recursively, so ignore skins having a parent skin.
	if (!m_parent_skin)
		UnloadBitmaps();
}

void TBSkin::OnContextRestored()
{
	// Reload bitmaps (since we unloaded everything in OnContextLost())
	// Override skins are handled recursively, so ignore skins having a parent skin.
	if (!m_parent_skin)
		ReloadBitmaps();
}

// == TBSkinElement =========================================================

TBSkinElement::TBSkinElement()
	: bitmap(nullptr), cut(0), expand(0), type(SKIN_ELEMENT_TYPE_STRETCH_BOX)
	, is_painting(false), is_getting(false)
	, padding_left(0), padding_top(0), padding_right(0), padding_bottom(0)
	, min_width(SKIN_VALUE_NOT_SPECIFIED), min_height(SKIN_VALUE_NOT_SPECIFIED)
	, max_width(SKIN_VALUE_NOT_SPECIFIED), max_height(SKIN_VALUE_NOT_SPECIFIED)
	, spacing(SKIN_DEFAULT_SPACING)
	, content_ofs_x(0), content_ofs_y(0)
	, img_position_x(50), img_position_y(50), img_ofs_x(0), img_ofs_y(0)
	, opacity(1.f)
{
}

TBSkinElement::~TBSkinElement()
{
}

// == TBSkinElementState ====================================================

bool TBSkinElementState::IsMatch(uint32 state, TBSkinConditionContext &context) const
{
	if (((state & this->state) || this->state == SKIN_STATE_ALL))
	{
		for (TBSkinCondition *condition = conditions.GetFirst(); condition; condition = condition->GetNext())
			if (!condition->GetCondition(context))
				return false;
		return true;
	}
	return false;
}

bool TBSkinElementState::IsExactMatch(uint32 state, TBSkinConditionContext &context) const
{
	if (state == this->state || this->state == SKIN_STATE_ALL)
	{
		for (TBSkinCondition *condition = conditions.GetFirst(); condition; condition = condition->GetNext())
			if (!condition->GetCondition(context))
				return false;
		return true;
	}
	return false;
}

// == TBSkinElement =========================================================

TBSkinElementStateList::~TBSkinElementStateList()
{
	while (TBSkinElementState *state = m_state_elements.GetFirst())
	{
		m_state_elements.Remove(state);
		delete state;
	}
}

TBSkinElementState *TBSkinElementStateList::GetStateElement(uint32 state, TBSkinConditionContext &context) const
{
	// First try to get a state element with a exact match to the current state
	if (TBSkinElementState *element_state = GetStateElementExactMatch(state, context))
		return element_state;
	// No exact state match. Get a state with a partly match if there is one.
	TBSkinElementState *state_element = m_state_elements.GetFirst();
	while (state_element)
	{
		if (state_element->IsMatch(state, context))
			return state_element;
		state_element = state_element->GetNext();
	}
	return nullptr;
}

TBSkinElementState *TBSkinElementStateList::GetStateElementExactMatch(uint32 state, TBSkinConditionContext &context) const
{
	TBSkinElementState *state_element = m_state_elements.GetFirst();
	while (state_element)
	{
		if (state_element->IsExactMatch(state, context))
			return state_element;
		state_element = state_element->GetNext();
	}
	return nullptr;
}

void TBSkinElementStateList::Load(TBNode *n)
{
	if (!n)
		return;

	// For each node, create a new state element.
	TBNode *element_node = n->GetFirstChild();
	while (element_node)
	{
		TBSkinElementState *state = new TBSkinElementState;
		if (!state)
			return;

		// By default, a state element applies to all combinations of states
		state->state = SKIN_STATE_ALL;
		state->element_id.Set(element_node->GetValue().GetString());

		// Loop through all nodes, read state and create all found conditions.
		for (TBNode *condition_node = element_node->GetFirstChild(); condition_node; condition_node = condition_node->GetNext())
		{
			if (strcmp(condition_node->GetName(), "state") == 0)
				state->state = StringToState(condition_node->GetValue().GetString());
			else if (strcmp(condition_node->GetName(), "condition") == 0)
			{
				TBSkinCondition::TARGET target = StringToTarget(condition_node->GetValueString("target", ""));

				const char *prop_str = condition_node->GetValueString("property", "");
				TBSkinCondition::PROPERTY prop = StringToProperty(prop_str);
				TBID custom_prop;
				if (prop == TBSkinCondition::PROPERTY_CUSTOM)
					custom_prop.Set(prop_str);

				TBID value;
				if (TBNode *value_n = condition_node->GetNode("value"))
				{
					// Set the it to number or string. If it's a state, we must first convert the
					// state string to the uint32 state combo.
					if (prop == TBSkinCondition::PROPERTY_STATE)
						value.Set(StringToState(value_n->GetValue().GetString()));
					else if (value_n->GetValue().IsString())
						value.Set(value_n->GetValue().GetString());
					else
						value.Set(value_n->GetValue().GetInt());
				}

				TBSkinCondition::TEST test = TBSkinCondition::TEST_EQUAL;
				if (TBSkinCondition *condition = new TBSkinCondition(target, prop, custom_prop, value, test))
					state->conditions.AddLast(condition);
			}
		}

		// State is reado to add
		m_state_elements.AddLast(state);
		element_node = element_node->GetNext();
	}
}

}; // namespace tinkerbell
