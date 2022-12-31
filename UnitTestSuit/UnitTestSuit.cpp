#include <iostream>

import Testing;

namespace Testing {

	constexpr bool PRINT_IGNORED_EXCEPTIONS = true;

	template<typename Type>
	class MyTestClass : public TestClassTyped<MyTestClass<Type>, Type> {
		typedef TestClassTyped<MyTestClass<Type>, Type> inherited;
	public:

		MyTestClass() : inherited("MyTestClass", { 

		new UnitTest("Creation", [](TestContext& ctx) -> void {
			uint64_t object = ctx.createTestObject<uint64_t>();

			Assert::isZero(object);
			Assert::same<uint64_t>(object);
			Assert::notEquals(object, 1);
			Assert::equals(object, 0);
		}),

		new UnitTestTyped<Type>("TypedTestOr", [](TestContextTyped<Type>& ctx) -> void {
			Type object = ctx.createTestObject();
			Assert::Or<PRINT_IGNORED_EXCEPTIONS>(
				[&object]() { Assert::notZero(object); },
				[&object]() { Assert::notEquals(object, 0); },
				[&object]() { Assert::equals(object, 0); }
				);
		}),

		new UnitTestTyped<Type>("TypedTestNor", [](TestContextTyped<Type>& ctx) -> void {
			Type object = ctx.createTestObject();
			Assert::Nor<PRINT_IGNORED_EXCEPTIONS>(
				[&object]() { Assert::notZero(object); },
				[&object]() { Assert::notEquals(object, 0); },
				[&object]() { Assert::equals(object, 1); }
			);
		}),

		new UnitTestTyped<Type>("TypedTestAnd", [](TestContextTyped<Type>& ctx) -> void {
			Type object = ctx.createTestObject();
			Assert::And<PRINT_IGNORED_EXCEPTIONS>(
				[&object]() { Assert::isZero(object); },
				[&object]() { Assert::notEquals(object, 1); },
				[&object]() { Assert::equals(object, 0); }
			);
		}),

			new UnitTestTyped<Type>("TypedTestNand", [](TestContextTyped<Type>& ctx) -> void {
			Type object = ctx.createTestObject();
			Assert::Nand<PRINT_IGNORED_EXCEPTIONS>(
				[&object]() { Assert::notZero(object); },
				[&object]() { Assert::notEquals(object, 0); },
				[&object]() { Assert::equals(object, 1); }
			);
		}),
	}) {}
};

}

int main() {
	Testing::TestSuit::registerClass<Testing::MyTestClass<uint64_t>>();

	Testing::TestSuit::instance()->run();
}
