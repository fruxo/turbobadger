// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_renderer.h"

namespace tinkerbell {

void TBRenderer::InvokeContextLost()
{
	for (TBRendererListener *listener = m_listeners.GetFirst(); listener; listener = listener->GetNext())
		listener->OnContextLost();
}

void TBRenderer::InvokeContextRestored()
{
	for (TBRendererListener *listener = m_listeners.GetFirst(); listener; listener = listener->GetNext())
		listener->OnContextRestored();
}

}; // namespace tinkerbell
