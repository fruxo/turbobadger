// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_test.h"
#include "tinkerbell.h"

#ifdef TB_UNIT_TESTING

using namespace tinkerbell;

TB_TEST_GROUP(tb_px)
{
	TBDimensionConverter dim_conv;

	TB_TEST(Init)
	{
		dim_conv.SetDPI(100, 200);
	}

	TB_TEST(set_from_string_unspecified)
	{
		TBPx px;
		px.SetFromString(dim_conv, "50");
		TB_VERIFY(px == 50);
	}
	TB_TEST(set_from_string_px)
	{
		TBPx px;
		px.SetFromString(dim_conv, "50px");
		TB_VERIFY(px == 50);
	}
	TB_TEST(set_from_string_dp)
	{
		TBPx px;
		px.SetFromString(dim_conv, "50dp");
		TB_VERIFY(px == 50 * 2);
	}
}

#endif // TB_UNIT_TESTING
