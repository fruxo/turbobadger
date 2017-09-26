// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "image/tb_image_widget.h"
#include "tb_widgets_reader.h"
#include "tb_node_tree.h"
#include "tb_system.h"

#ifdef TB_IMAGE

namespace tb {

PreferredSize TBImageWidget::OnCalculatePreferredContentSize(const SizeConstraints & /*constraints*/)
{
	//TBDebugPrint("PCS: %d x %d\n", m_image.Width(), m_image.Height());
	return PreferredSize(m_image.Width(), m_image.Height());
}

void TBImageWidget::OnPaint(const PaintProps &paint_props)
{
	if (TBBitmapFragment *fragment = m_image.GetBitmap()) {
		if (m_adapt_text_color)
			g_renderer->DrawBitmapColored(GetPaddingRect(), TBRect(0, 0, m_image.Width(), m_image.Height()),
										  paint_props.text_color, fragment);
		else
			g_renderer->DrawBitmap(GetPaddingRect(), TBRect(0, 0, m_image.Width(), m_image.Height()), fragment);
	}
}


} // namespace tb

#endif // TB_IMAGE
