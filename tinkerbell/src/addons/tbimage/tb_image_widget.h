// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_IMAGE_WIDGET_H
#define TB_IMAGE_WIDGET_H

#include "tb_widgets.h"
#include "addons/tbimage/tb_image_manager.h"

namespace tinkerbell {

/** TBImageWidget is a widget showing a image loaded by TBImageManager,
	constrained in size to its skin.
	If you need to show a image from the skin, you can use TBSkinImage. */

class TBImageWidget : public TBWidget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBImageWidget", TBWidget);

	TBImageWidget() {}

	void SetImage(const TBImage &image) { m_image = image; }
	void SetImage(const char *filename) { m_image = g_image_manager->GetImage(filename); }

	virtual PreferredSize GetPreferredSize();

	virtual void OnPaint(const PaintProps &paint_props);
private:
	TBImage m_image;
};

}; // namespace tinkerbell

#endif // TB_IMAGE_WIDGET_H
