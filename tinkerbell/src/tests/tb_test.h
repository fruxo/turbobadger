// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_TEST_H
#define TB_TEST_H

/**
	This file contains a very simple unit testing framework.

	There are more capable unit testing frameworks available (that
	might be more suitable if you need extensive testing of your target
	application.

	I've chosen to not use any other framework for tinkerbells internal
	testing to minimize dependences.

	How to define a single test:
	---------------------------

	TB_TEST_GROUP(groupname)
	{
		TB_TEST(testname)
		{
			// Here goes test code and calls
			// to TB_VERIFY, TB_PASS, TB_FAIL
			TB_VERIFY(1);
		}
	}

	How to define multiple tests, with data, setup and cleanup:
	----------------------------------------------------------

	TB_TEST_GROUP(groupname)
	{
		// Here goes the data
		TBStr str;

		// Here goes methods with access to data
		bool is_str_empty() { return str.IsEmpty(); }

		// A test with name Setup will be called before each test.
		// If it fail, no other tests will be called (not even Cleanup).
		TB_TEST(Setup)
		{
			// Setup
		}

		// The actual test code. Will be called if Setup passed.
		TB_TEST(test_something_1)
		{
			// Test 1
		}

		// Another test code. Will be called if Setup passed.
		TB_TEST(test_something_2)
		{
			// Test 2
		}

		// A test with name Cleanup will be called after each test.
		// Will be called even if the test failed.
		TB_TEST(Cleanup)
		{
			// Cleanup
		}
	}
*/

#include "tb_types.h"
#include "tb_linklist.h"

namespace tinkerbell {

/** Setting for TBRunTests to print out more information. */
#define TB_TEST_VERBOSE				1

#ifdef _DEBUG
#define TB_UNIT_TESTING
#endif

#ifdef TB_UNIT_TESTING

/** Run the tests */
void TBRunTests(uint32 settings = TB_TEST_VERBOSE);

/** Verify that the expression is true and fail if it isn't. */
#define TB_VERIFY(expr) { fail_line_nr = __LINE__; fail_file = __FILE__; if (!(expr)) { fail_text = (#expr); return; } }

/** End the test with a pass. */
#define TB_PASS() return;

/** End the test with a description why it failed. */
#define TB_FAIL(error) { fail_line_nr = __LINE__; fail_file = __FILE__; fail_text = error; return; }

/** TBCall is used to execute callbacks for tests in TBTestGroup. */
class TBCall : public TBLinkOf<TBCall>
{
public:
	/** return the name of the call */
	virtual const char *name() = 0;
	/** execute the test code */
	virtual void exec() = 0;
};

/** TBTestGroup has a collection of callbacks for tests, and optional Setup and Cleanup calls. */
class TBTestGroup : public TBLinkOf<TBTestGroup>
{
public:
	TBTestGroup(const char *name);
	~TBTestGroup();
public:
	const char *name;		///< Test group name.
	TBCall *setup;			///< Setup call, or nullptr.
	TBCall *cleanup;		///< Cleanup call, or nullptr.
	TBLinkListOf<TBCall> calls;///< All test calls to call.
};

/** TBRegisterCall is used for registering calls on TBTestGroup .*/
class TBRegisterCall
{
public:
	TBRegisterCall(TBTestGroup *test, TBCall *call);
	~TBRegisterCall();
private:
	TBCall *call;
};

#define TB_TEST_GROUP(name) \
	namespace testgroup_##name \
	{ \
		class TheGroup : public TBTestGroup \
		{ \
		public: \
			TheGroup() : TBTestGroup(#name) {} \
		}; \
		TheGroup the_group_obj; \
	} \
	namespace testgroup_##name \

#define TB_TEST(callname) \
			class CallObj##callname : public TBCall \
			{ \
			public: \
				virtual const char *name(); \
				virtual void exec(); \
			}; \
			CallObj##callname callname; \
			TBRegisterCall callname##reg(&the_group_obj, &callname); \
			const char *CallObj##callname::name() { return #callname; } \
			void CallObj##callname::exec()

// Internal globals
extern uint32 test_settings;	///< Settings, as sent to TBRunTests
extern int fail_line_nr;		///< Fail line number
extern const char *fail_file;	///< Fail file name
extern const char *fail_text;	///< Fail text description

#else

inline void TBRunTests(uint32 settings = TB_TEST_VERBOSE) {}

#endif

}; // namespace tinkerbell

#endif // TB_TEST_H
