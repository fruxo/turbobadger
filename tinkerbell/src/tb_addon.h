// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_ADDON_H
#define TB_ADDON_H

#include "tb_linklist.h"
#include "tb_widgets.h"

namespace tinkerbell {

/** TBAddon provides a simple API with Init/Shutdown callbacks that will
	be called during init_tinkerbell and shutdown_tinkerbell. */

class TBAddon : public TBLinkOf<TBAddon>
{
public:
	/** Called during init_tinkerbell after tinkerbell core has been initiated. */
	virtual bool Init() = 0;

	/** Called during shutdown_tinkerbell before tinkerbell core has been shut down. */
	virtual void Shutdown() = 0;

	virtual ~TBAddon() {}
};

/** TBAddonFactory is ment to be subclassed to create a TBAddon, by
	being declared as a global object. It will then register itself
	so the addon is created, initiated during init_tinkerbell and
	destroyed during shutdown_tinkerbell. */
class TBAddonFactory
{
public:
	TBAddonFactory();

	virtual TBAddon *Create() = 0;

	TBAddonFactory *next;	///< Next registered addon factory.
};

/** Init addons. */
bool TBInitAddons();

/** Shutdown and delete addons. */
void TBShutdownAddons();

/** This macro creates a new TBAddonFactory for the given class name. */
#define TB_ADDON_FACTORY(classname) \
	class classname##AddonFactory : public TBAddonFactory \
	{ \
	public: \
		virtual TBAddon *Create() \
		{ \
			return new classname(); \
		} \
	}; \
	static classname##AddonFactory classname##_af;

}; // namespace tinkerbell

#endif // TB_ADDON_H
