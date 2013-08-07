// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_renderer.h"

namespace tinkerbell {

void TBRenderer::InvokeContextLost()
{
	TBLinkListOf<TBRendererListener>::Iterator iter = m_listeners.IterateForward();
	while (TBRendererListener *listener = iter.GetAndStep())
		listener->OnContextLost();
}

void TBRenderer::InvokeContextRestored()
{
	TBLinkListOf<TBRendererListener>::Iterator iter = m_listeners.IterateForward();
	while (TBRendererListener *listener = iter.GetAndStep())
		listener->OnContextRestored();
}

}; // namespace tinkerbell
