// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_SKIN_H
#define TB_SKIN_H

#include "tinkerbell.h"
#include "tb_renderer.h"
#include "tb_bitmap_fragment.h"
#include "tb_hashtable.h"
#include "tb_linklist.h"

namespace tinkerbell {

class TBNode;
class TBSkinConditionContext;

/** Used for some values in TBSkinElement if they has not been specified in the skin. */
#define SKIN_VALUE_NOT_SPECIFIED -1

/** Default spacing for layout. */
#define SKIN_DEFAULT_SPACING 5

/** Skin state types (may be combined).
	NOTE: This should exactly match WIDGET_STATE in tb_widgets.h! */
enum SKIN_STATE {
	SKIN_STATE_NONE			= 0,
	SKIN_STATE_DISABLED		= 1,
	SKIN_STATE_FOCUSED		= 2,
	SKIN_STATE_PRESSED		= 4,
	SKIN_STATE_SELECTED		= 8,
	SKIN_STATE_HOVERED		= 16
};
#define NUM_SKIN_STATES 6
#define SKIN_STATE_ALL 0xffffffff

/** Type of painting that should be done for a TBSkinElement. */
enum SKIN_ELEMENT_TYPE {
	SKIN_ELEMENT_TYPE_STRETCH_BOX,
	SKIN_ELEMENT_TYPE_STRETCH_IMAGE,
	SKIN_ELEMENT_TYPE_TILE,
	SKIN_ELEMENT_TYPE_IMAGE
};

/** TBSkinCondition checks if a condition is true for a given TBSkinConditionContext.
	This is used to apply different state elements depending on what is currently
	painting the skin. */

class TBSkinCondition : public TBLinkOf<TBSkinCondition>
{
public:
	/** Defines which target(s) relative to the context that should be tested for the condition. */
	enum TARGET {
		TARGET_THIS,			///< The object painting the skin.
		TARGET_PARENT,			///< The parent of the object painting the skin.
		TARGET_ANCESTORS		///< All ancestors of the object painting the skin.
	};

	/** Defines which property in the context that should be checked. */
	enum PROPERTY {
		PROPERTY_SKIN,				///< The background skin id.
		PROPERTY_WINDOW_ACTIVE,		///< The window is active (no value required).
		PROPERTY_AXIS,				///< The axis of the content (x or y)
		PROPERTY_ALIGN,				///< The alignment.
		PROPERTY_ID,				///< The id.
		PROPERTY_STATE,				///< The state is set.
		PROPERTY_HOVER,				///< Focus is on the target or any child (no value required).
		PROPERTY_CAPTURE,			///< Capture is on the target or any child (no value required).
		PROPERTY_FOCUS,				///< Focus is on the target or any child (no value required).
	};

	/** Defines if the condition tested should be equal or not for the condition to be true. */
	enum TEST {
		TEST_EQUAL,		///< Value should be equal for condition to be true.
		TEST_NOT_EQUAL	///< Value should not be equal for condition to be true.
	};

	TBSkinCondition(TARGET target, PROPERTY prop, const TBID &value, TEST test);

	/** Return true if the condition is true for the given context. */
	bool GetCondition(TBSkinConditionContext &context) const;
private:
	TARGET m_target;
	PROPERTY m_prop;
	TBID m_value;
	TEST m_test;
};

/** TBSkinConditionContext checks if a condition is true. It is passed to skin painting functions
	so different state elements can be applied depending on the current situation of the context.
	F.ex a widget may change appearance if it's under a parent with a certain skin. */

class TBSkinConditionContext
{
public:
	/** Return true if the given target and property equals the given value. */
	virtual bool GetCondition(TBSkinCondition::TARGET target, TBSkinCondition::PROPERTY prop, const TBID &value) = 0;
};

/** TBSkinElementState has a skin element id that should be used if its state and condition
	matches that which is being painted.
*/

class TBSkinElementState : public TBLinkOf<TBSkinElementState>
{
public:
	bool IsMatch(uint32 state, TBSkinConditionContext &context) const;
	bool IsExactMatch(uint32 state, TBSkinConditionContext &context) const;
	TBID element_id;
	uint32 state;
	TBLinkListOf<TBSkinCondition> conditions;
};

/** List of state elements in a TBSkinElement. */

class TBSkinElementStateList
{
public:
	~TBSkinElementStateList();

	TBSkinElementState *GetStateElement(uint32 state, TBSkinConditionContext &context) const;
	TBSkinElementState *GetStateElementExactMatch(uint32 state, TBSkinConditionContext &context) const;

	bool HasStateElements() const { return m_state_elements.HasLinks(); }
	const TBSkinElementState *GetFirstElement() const { return m_state_elements.GetFirst(); }

	void Load(TBNode *n);
private:
	TBLinkListOf<TBSkinElementState> m_state_elements;
};

/** Skin element.
	Contains a bitmap fragment (or nullptr) and info specifying how it should be painted.
	Also contains padding and other look-specific widget properties. */
class TBSkinElement
{
public:
	TBSkinElement();
	~TBSkinElement();
public:
	// Skin properties
	TBID id;			///< ID of the skin element
	TBStr name;			///< Name of the skin element, f.ex "TBSelectDropdown.arrow"
	TBStr bitmap_file;	///< File name of the bitmap (might be empty)
	TBBitmapFragment *bitmap;///< Bitmap fragment containing the graphics, or nullptr.
	uint8 cut;			///< How the bitmap should be sliced using StretchBox
	int16 expand;		///< How much the skin should expand outside the widgets rect.
	SKIN_ELEMENT_TYPE type;///< Skin element type
	bool is_painting;	///< If the skin is being painted
	int8 padding_left;	///< Left padding for any content in the element
	int8 padding_top;	///< Top padding for any content in the element
	int8 padding_right;	///< Right padding for any content in the element
	int8 padding_bottom;///< Bottom padding for any content in the element
	int16 min_width;	///< Minimum width or SKIN_VALUE_NOT_SPECIFIED
	int16 min_height;	///< Minimum height or SKIN_VALUE_NOT_SPECIFIED
	int16 max_width;	///< Maximum width or SKIN_VALUE_NOT_SPECIFIED
	int16 max_height;	///< Maximum height or SKIN_VALUE_NOT_SPECIFIED
	int8 spacing;		///< Spacing used on layout. SKIN_DEFAULT_SPACING by default.
	int8 content_ofs_x;	///< X offset of the content in the widget.
	int8 content_ofs_y;	///< Y offset of the content in the widget.
	int8 img_position_x;///< Horizontal position for type image. 0-100 (left to right).
	int8 img_position_y;///< Vertical position for type image. 0-100 (top to bottom).
	int8 img_ofs_x;		///< X offset for type image. Relative to the x position (img_position_x)
	int8 img_ofs_y;		///< Y offset for type image. Relative to the y position (img_position_y)
	float opacity;		///< Opacity that should be used for the whole widget (0.f-1.f).
	TBColor text_color;

	/** List of override elements (See TBSkin::PaintSkin) */
	TBSkinElementStateList m_override_elements;

	/** List of child elements (See TBSkin::PaintSkin) */
	TBSkinElementStateList m_child_elements;

	/** List of overlay elements (See TBSkin::PaintSkin) */
	TBSkinElementStateList m_overlay_elements;

	/** Check if there's a exact or partial match for the given state in either
		override, child or overlay element list. */
	bool HasState(uint32 state, TBSkinConditionContext &context)
								{ return	m_override_elements.GetStateElement(state, context) ||
											m_child_elements.GetStateElement(state, context) ||
											m_overlay_elements.GetStateElement(state, context); }
};

/** TBSkin contains a list of TBSkinElement. */
class TBSkin : private TBRendererListener
{
public:
	TBSkin();
	~TBSkin();

	/** Load the skin file and the bitmaps it refers to.
		If override_skin_file is specified, it will also load that skin and use it
		as override skin.
		Returns true on success, and all bitmaps referred to also loaded successfully. */
	bool Load(const char *skin_file, const char *override_skin_file = nullptr);

	/** Unload all bitmaps used in this skin. */
	void UnloadBitmaps();

	/** Reload all bitmaps used in this skin. Calls UnloadBitmaps first to ensure no bitmaps
		are loaded before loading new ones. */
	bool ReloadBitmaps();

	/** Get the skin element with the given id.
		It will return a skin element from the override_skin (if set and there
		is a match).
		Returns nullptr if there's no match. */
	TBSkinElement *GetSkinElement(const TBID &skin_id) const;

	/** Get the default text color for all skin elements */
	TBColor GetDefaultTextColor() const { return m_default_text_color; }

	/** Paint the skin at dst_rect.

		Override skin:
		-It will first try with the override_skin (if set).

		Override elements:
		-If there is a override element with the exact matching state, it will paint
		 the override *instead* if the base skin. If no exact match was found, it will
		 check for a partial match and paint that *instead* of the base skin.

		Child elements:
		-It will paint *all* child elements that match the current state ("all" can be specified
		 as state so it will always be painted). The elements are painted in the order they are
		 specified in the skin.

		Special elements:
		-There's some special generic skin elements used by TBWidget (see TBWidget::SetSkinBg)

		Overlay elements:
		-Overlay elements are painted separately, from PaintSkinOverlay (when all sibling
		 widgets has been painted). As with child elements, all overlay elements that match
		 the current state will be painted in the order they are specified in the skin.

		Return the skin element used (after following override elements or override skins),
		or nullptr if no skin element was found matching the skin_id. */
	TBSkinElement *PaintSkin(const TBRect &dst_rect, const TBID &skin_id, uint32 state, TBSkinConditionContext &context);

	/** Paint the skin at dst_rect. Just like the PaintSkin above, but takes a specific
		skin element instead of looking it up from the id. */
	TBSkinElement *PaintSkin(const TBRect &dst_rect, TBSkinElement *element, uint32 state, TBSkinConditionContext &context);

	/** Paint the overlay elements for the given skin element and state. */
	void PaintSkinOverlay(const TBRect &dst_rect, TBSkinElement *element, uint32 state, TBSkinConditionContext &context);

#ifdef _DEBUG
	/** Render the skin bitmaps on screen, to analyze fragment positioning. */
	void Debug();
#endif

	// Implementing TBRendererListener
	virtual void OnContextLost();
	virtual void OnContextRestored();
private:
	TBHashTableAutoDeleteOf<TBSkinElement> m_elements;	///< All skin elements for this skin.
	TBSkin *m_parent_skin;								///< Parent skin (set to the default skin for for the override skins)
	TBSkin *m_override_skin;							///< Override skin (or nullptr)
	TBBitmapFragmentManager m_frag_manager;				///< Fragment manager (not used for override skins)
	TBColor m_default_text_color;						///< Default text color for all skin elements
	bool ReloadBitmapsInternal();
	void PaintElement(const TBRect &dst_rect, TBSkinElement *element);
	void PaintElementImage(const TBRect &dst_rect, TBSkinElement *element);
	void PaintElementTile(const TBRect &dst_rect, TBSkinElement *element);
	void PaintElementStretchImage(const TBRect &dst_rect, TBSkinElement *element);
	void PaintElementStretchBox(const TBRect &dst_rect, TBSkinElement *element);
};

}; // namespace tinkerbell

#endif // TB_SKIN_H
