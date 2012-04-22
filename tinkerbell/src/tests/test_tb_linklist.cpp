// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_test.h"
#include "tb_linklist.h"

#ifdef TB_UNIT_TESTING

using namespace tinkerbell;

TB_TEST_GROUP(tb_linklist)
{
	class Apple : public TBLinkOf<Apple>
	{
	public:
		Apple(int id) : id(id) {}
		int id;
	};

	TBLinkListOf<Apple> list;

	bool SetupList(int num_apples)
	{
		list.DeleteAll();
		for (int i = 0; i < num_apples; i++)
			list.AddLast(new Apple(i + 1));
		return list.CountItems() == num_apples;
	}

	TB_TEST(iteration_while_delete_all)
	{
		TB_VERIFY(SetupList(3));

		TBLinkListOf<Apple>::Iterator iterator = list.IterateForward();
		while (Apple *apple = iterator.GetAndStep())
		{
			// Lets pretend we do something with apple
			// that trig deletion of all apples.
			list.DeleteAll();
		}
		// success already, if we didn't crash
		TB_VERIFY(list.CountItems() == 0);
	}

	TB_TEST(iteration_while_delete)
	{
		TB_VERIFY(SetupList(3));

		TBLinkListOf<Apple>::Iterator iterator = list.IterateForward();
		while (Apple *apple = iterator.GetAndStep())
		{
			// Lets pretend we do something with apple
			// that trig deletion of both apple and next apple!
			if (apple->GetNext())
				list.Delete(apple->GetNext());
			list.Delete(apple);
		}
		// success already, if we didn't crash
		TB_VERIFY(list.CountItems() == 0);
	}

	TB_TEST(forward_iterator)
	{
		TB_VERIFY(SetupList(3));

		TBLinkListOf<Apple>::Iterator i = list.IterateForward();
		TB_VERIFY(i.Get()->id == 1);

		TB_VERIFY(i.GetAndStep()->id == 1);
		TB_VERIFY(i.GetAndStep()->id == 2);
		TB_VERIFY(i.GetAndStep()->id == 3);
		TB_VERIFY(i.Get() == nullptr);

		i.Reset();
		TB_VERIFY(i.Get()->id == 1);
	}

	TB_TEST(backward_iterator)
	{
		TB_VERIFY(SetupList(3));

		TBLinkListOf<Apple>::Iterator i = list.IterateBackward();
		TB_VERIFY(i.Get()->id == 3);

		TB_VERIFY(i.GetAndStep()->id == 3);
		TB_VERIFY(i.GetAndStep()->id == 2);
		TB_VERIFY(i.GetAndStep()->id == 1);
		TB_VERIFY(i.Get() == nullptr);

		i.Reset();
		TB_VERIFY(i.Get()->id == 3);
	}

	TB_TEST(multiple_iterators_assign)
	{
		TB_VERIFY(SetupList(3));

		TBLinkListOf<Apple>::Iterator iA = list.IterateForward();
		TBLinkListOf<Apple>::Iterator iB = list.IterateBackward();

		TB_VERIFY(iA.Get()->id == 1);
		TB_VERIFY(iB.Get()->id == 3);

		iA = iB;
		TB_VERIFY(iA.GetAndStep()->id == 3);
		TB_VERIFY(iA.GetAndStep()->id == 2);
		TB_VERIFY(iA.GetAndStep()->id == 1);
	}

	TB_TEST(multiple_iterators_assign_swap_list)
	{
		TB_VERIFY(SetupList(3));
		TBLinkListOf<Apple> other_list;
		other_list.AddLast(new Apple(42));

		TBLinkListOf<Apple>::Iterator iA = list.IterateForward();
		TBLinkListOf<Apple>::Iterator iB = other_list.IterateForward();

		TB_VERIFY(iA.Get()->id == 1);
		TB_VERIFY(iB.Get()->id == 42);

		iA = iB;
		TB_VERIFY(iA.GetAndStep()->id == 42);
		TB_VERIFY(iA.Get() == nullptr);
	}
}

#endif // TB_UNIT_TESTING
