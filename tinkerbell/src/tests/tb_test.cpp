// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_test.h"
#include "tb_system.h"

namespace tinkerbell {

#ifdef TB_UNIT_TESTING

uint32 test_settings;
int fail_line_nr;
const char *fail_file;
const char *fail_text;

TBLinkListOf<TBTestGroup> tests;

// == TBRegisterCall ==========================================================

TBRegisterCall::TBRegisterCall(TBTestGroup *test, TBCall *call)
	: call(call)
{
	if (strcmp(call->name(), "Setup") == 0)
		test->setup = call;
	else if (strcmp(call->name(), "Cleanup") == 0)
		test->cleanup = call;
	else
		test->calls.AddLast(call);
}

TBRegisterCall::~TBRegisterCall()
{
	if (call->linklist)
		call->linklist->Remove(call);
}

// == TBTestGroup =============================================================

TBTestGroup::TBTestGroup(const char *name)
	: name(name), setup(nullptr), cleanup(nullptr)
{
	tests.AddLast(this);
}

TBTestGroup::~TBTestGroup()
{
	tests.Remove(this);
}

const char *CallAndOutput(TBTestGroup *test, TBCall *call)
{
	fail_text = nullptr;
	call->exec();

	if (!fail_text)
		return fail_text;
	TBStr msg;
	msg.SetFormatted("FAIL: \"%s/%s\":\n"
					"  %s(%d): \"%s\"\n",
					test->name, call->name(),
					fail_file, fail_line_nr, fail_text);
	TBDebugOut(msg);
	return fail_text;
}

void OutputPass(TBTestGroup *test, const char *call_name)
{
	if (!(test_settings & TB_TEST_VERBOSE))
		return;
	TBStr msg;
	msg.SetFormatted("PASS: \"%s/%s\"\n",
					test->name, call_name);
	TBDebugOut(msg);
}

void TBRunTests(uint32 settings)
{
	test_settings = settings;
	int num_failed = 0;
	int num_passed = 0;

	TBDebugOut("Running tests...\n");

	TBLinkListOf<TBTestGroup>::Iterator i = tests.IterateForward();
	while (TBTestGroup *test = i.GetAndStep())
	{
		for (TBCall *call = test->calls.GetFirst(); call; call = call->GetNext())
		{
			// Execute test (and call setup and cleanup if available).
			int fail = 0;
			if (test->setup)
				fail = !!CallAndOutput(test, test->setup);
			if (!fail) // Only run if setup succeeded
			{
				fail |= !!CallAndOutput(test, call);
				if (test->cleanup)
					fail |= !!CallAndOutput(test, test->cleanup);
			}
			// Handle result
			if (fail)
				num_failed++;
			else
			{
				num_passed++;
				OutputPass(test, call->name());
			}
		}
	}

	TBStr msg;
	msg.SetFormatted("Test results: %d passed, %d failed.\n", num_passed, num_failed);
	TBDebugOut(msg);
	assert(num_failed == 0);
}

#endif // TB_UNIT_TESTING

}; // namespace tinkerbell
