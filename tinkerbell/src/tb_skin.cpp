// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_skin.h"
#include "tb_system.h"
#include "parser/TBNodeTree.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

namespace tinkerbell {

TBStr FilenameToPath(const char *filename)
{
	const char *filename_start = filename;
	while (const char *next = strpbrk(filename, "\\/"))
		filename = next + 1;

	if (filename_start == filename) // Filename contained no path
		return "./";
	return TBStr(filename_start, filename - filename_start);
}

TBColor StringToColor(const char *str)
{
	int r, g, b, a;
	int len = strlen(str);
	if (len == 9 && sscanf(str, "#%2x%2x%2x%2x", &r, &g, &b, &a) == 4)	// rrggbbaa
		return TBColor(r, g, b, a);
	if (len == 7 && sscanf(str, "#%2x%2x%2x", &r, &g, &b) == 3)			// rrggbb
		return TBColor(r, g, b);
	if (len == 5 && sscanf(str, "#%1x%1x%1x%1x", &r, &g, &b, &a) == 4)	// rgba
		return TBColor(r + (r << 4), g + (g << 4), b + (b << 4), a + (a << 4));
	if (len == 4 && sscanf(str, "#%1x%1x%1x", &r, &g, &b) == 3)			// rgb
		return TBColor(r + (r << 4), g + (g << 4), b + (b << 4));
	return TBColor();
}

// == TBSkin ================================================================

TBSkin::TBSkin()
	: m_parent_skin(nullptr)
	, m_override_skin(nullptr)
{
	assert(FilenameToPath("foo.txt").Equals("./"));
	assert(FilenameToPath("Path/subpath/foo.txt").Equals("Path/subpath/"));
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

	TBStr skin_path = FilenameToPath(skin_file);

	if (const char *color = node.GetValueString("defaults>text-color", nullptr))
		m_default_text_color = StringToColor(color);

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
				e->bitmap_file.Append(skin_path);
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
			e->opacity = n->GetValueFloat("opacity", 1.f);

			if (const char *color = n->GetValueString("text-color", nullptr))
				e->text_color = StringToColor(color);
			else
				e->text_color = m_default_text_color;

			const char *type = n->GetValueString("type", "StretchBox");
			if (strcmp(type, "Image") == 0)
				e->type = SKIN_ELEMENT_TYPE_IMAGE;
			else if (strcmp(type, "Stretch Image") == 0)
				e->type = SKIN_ELEMENT_TYPE_STRETCH_IMAGE;
			else if (strcmp(type, "Tile") == 0)
				e->type = SKIN_ELEMENT_TYPE_TILE;
			else
				e->type = SKIN_ELEMENT_TYPE_STRETCH_BOX;

			// Create all state elements
			e->m_override_elements.Load(n->GetNode("overrides"));
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
		success = m_frag_manager.CreateBitmaps();

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
			bool dedicated_map = element->type == SKIN_ELEMENT_TYPE_TILE;
			element->bitmap = frag_man->CreateNewBitmapFragment(element->bitmap_file, dedicated_map);
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

TBSkinElement *TBSkin::PaintSkin(const TBRect &dst_rect, const TBID &skin_id, uint32 state)
{
	return PaintSkin(dst_rect, GetSkinElement(skin_id), state);
}

TBSkinElement *TBSkin::PaintSkin(const TBRect &dst_rect, TBSkinElement *element, uint32 state)
{
	if (!element || element->is_painting)
		return nullptr;

	// Avoid potential endless recursion in evil skins
	element->is_painting = true;

	// Return the override if we have one.
	TBSkinElement *return_element = element;

	// If there's any override for this state, paint it. Otherwise paint the standard skin element.
	TBSkinElementState *override_state = element->m_override_elements.GetStateElement(state);
	if (override_state)
		return_element = PaintSkin(dst_rect, override_state->element_id, state);
	else
		PaintElement(dst_rect, element);

	// Paint all child element with exact match
	if (element->m_child_elements.HasStateElements())
	{
		for(int i = 0; i < NUM_SKIN_STATES; i++)
		{
			uint32 mask = 1 << i;
			if (state & mask)
				if (TBSkinElementState *child_state = element->m_child_elements.GetStateElementExactMatch(state & mask))
					PaintSkin(dst_rect, child_state->element_id, state & mask);
		}
	}

	element->is_painting = false;
	return return_element;
}

void TBSkin::PaintSkinOverlay(const TBRect &dst_rect, TBSkinElement *element, uint32 state)
{
	if (!element || element->is_painting)
		return;

	// Avoid potential endless recursion in evil skins
	element->is_painting = true;

	// Paint all overlay element with exact match
	if (element->m_overlay_elements.HasStateElements())
	{
		for(int i = 0; i < NUM_SKIN_STATES; i++)
		{
			uint32 mask = 1 << i;
			if (state & mask)
				if (TBSkinElementState *child_state = element->m_overlay_elements.GetStateElementExactMatch(state & mask))
					PaintSkin(dst_rect, child_state->element_id, state & mask);
		}
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
	else
		PaintElementStretchBox(dst_rect, element);
}

void TBSkin::PaintElementImage(const TBRect &dst_rect, TBSkinElement *element)
{
	TBRect src_rect(0, 0, element->bitmap->Width(), element->bitmap->Height());
	TBRect rect(dst_rect.x + (dst_rect.w - src_rect.w) / 2,
				dst_rect.y + (dst_rect.h - src_rect.h) / 2,
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

void TBSkin::PaintElementStretchBox(const TBRect &dst_rect, TBSkinElement *element)
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
	if (rect.w > dst_cut_w * 2 && rect.h > dst_cut_h * 2)
		g_renderer->DrawBitmap(TBRect(rect.x + dst_cut_w, rect.y + dst_cut_h, rect.w - dst_cut_w * 2, rect.h - dst_cut_h * 2), TBRect(cut, cut, bw - cut * 2, bh - cut * 2), element->bitmap);
}

#ifdef _DEBUG
void TBSkin::Debug()
{
	m_frag_manager.Debug();
}
#endif // _DEBUG

// == TBSkinElement =========================================================

TBSkinElement::TBSkinElement()
	: bitmap(nullptr), cut(0), expand(0), type(SKIN_ELEMENT_TYPE_STRETCH_BOX), is_painting(false)
	, padding_left(0), padding_top(0), padding_right(0), padding_bottom(0)
	, min_width(SKIN_VALUE_NOT_SPECIFIED), min_height(SKIN_VALUE_NOT_SPECIFIED)
	, max_width(SKIN_VALUE_NOT_SPECIFIED), max_height(SKIN_VALUE_NOT_SPECIFIED)
	, spacing(SKIN_DEFAULT_SPACING)
	, content_ofs_x(0), content_ofs_y(0)
	, opacity(1.f)
{
}

TBSkinElement::~TBSkinElement()
{
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

TBSkinElementState *TBSkinElementStateList::GetStateElement(uint32 state)
{
	if (!state)
		return nullptr;
	// First try to get a state element with a exact match to the current state
	if (TBSkinElementState *element_state = GetStateElementExactMatch(state))
		return element_state;
	// No exact state match. Get a state with a partly match
	for(int i = 0; i < NUM_SKIN_STATES; i++)
	{
		uint32 mask = 1 << i;
		if (state & mask)
			if (TBSkinElementState *element_state = GetStateElementExactMatch(state & mask))
				return element_state;
	}
	return nullptr;
}

TBSkinElementState *TBSkinElementStateList::GetStateElementExactMatch(uint32 state)
{
	TBSkinElementState *state_element = m_state_elements.GetFirst();
	while (state_element)
	{
		if (state_element->state == state)
			return state_element;
		state_element = state_element->GetNext();
	}
	return nullptr;
}

void TBSkinElementStateList::Load(TBNode *n)
{
	if (!n)
		return;

	TBNode *element_node = n->GetFirstChild();
	while (element_node)
	{
		TBSkinElementState *state = new TBSkinElementState;
		if (!state)
			return;

		state->state = 0;
		state->element_id.Set(element_node->GetValue().GetString());

		if (TBNode *state_node = element_node->GetNode("state"))
		{
			const char *state_str = state_node->GetValue().GetString();
			if (strstr(state_str, "disabled"))	state->state |= SKIN_STATE_DISABLED;
			if (strstr(state_str, "focused"))	state->state |= SKIN_STATE_FOCUSED;
			if (strstr(state_str, "pressed"))	state->state |= SKIN_STATE_PRESSED;
			if (strstr(state_str, "selected"))	state->state |= SKIN_STATE_SELECTED;
			if (strstr(state_str, "hovered"))	state->state |= SKIN_STATE_HOVERED;
		}

		m_state_elements.AddLast(state);
		element_node = element_node->GetNext();
	}
}

}; // namespace tinkerbell
