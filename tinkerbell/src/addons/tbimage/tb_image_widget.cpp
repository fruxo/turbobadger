// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "addons/tbimage/tb_image_widget.h"
#include "tb_widgets_reader.h"
#include "parser/TBNodeTree.h"

namespace tinkerbell {

TB_WIDGET_FACTORY(TBImageWidget, TBValue::TYPE_NULL, WIDGET_Z_TOP)
{
	if (const char *filename = info->node->GetValueString("filename", nullptr))
		widget->SetImage(filename);
}

// == TBImageWidget =======================================

PreferredSize TBImageWidget::GetPreferredSize()
{
	PreferredSize ps;
	ps.min_w = ps.max_w = ps.pref_w = m_image.Width();
	ps.min_h = ps.max_h = ps.pref_h = m_image.Height();
	return ps;
}

void TBImageWidget::OnPaint(const PaintProps &paint_props)
{
	if (TBBitmapFragment *fragment = m_image.GetBitmap())
		g_renderer->DrawBitmap(GetPaddingRect(), TBRect(0, 0, m_image.Width(), m_image.Height()), fragment);
}

}; // namespace tinkerbell
