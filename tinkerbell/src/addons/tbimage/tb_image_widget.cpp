// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2014, Emil Segerås  ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "addons/tbimage/tb_image_widget.h"
#include "tb_widgets_reader.h"
#include "tb_node_tree.h"

namespace tinkerbell {

TB_WIDGET_FACTORY(TBImageWidget, TBValue::TYPE_NULL, WIDGET_Z_TOP)
{
	if (const char *filename = info->node->GetValueString("filename", nullptr))
		widget->SetImage(filename);
}

// == TBImageWidget =======================================

PreferredSize TBImageWidget::OnCalculatePreferredContentSize(const SizeConstraints &constraints)
{
	return PreferredSize(m_image.Width(), m_image.Height());
}

void TBImageWidget::OnPaint(const PaintProps &paint_props)
{
	if (TBBitmapFragment *fragment = m_image.GetBitmap())
		g_renderer->DrawBitmap(GetPaddingRect(), TBRect(0, 0, m_image.Width(), m_image.Height()), fragment);
}

}; // namespace tinkerbell
