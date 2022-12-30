#include <iostream>

import Testing;

namespace Testing {

	template<typename Type>
	class MyTestClass : public TestClassTyped<MyTestClass<Type>, Type> {
		typedef TestClassTyped<MyTestClass<Type>, Type> inherited;
	public:

		MyTestClass() : inherited("MyTestClass", {

		new UnitTest("Creation", [](TestContext& ctx) -> void {
			uint64_t object = ctx.createTestObject<uint64_t>();

			Assert::isZero(object);
			Assert::same<uint64_t>(object);
			Assert::equals(0, object);
			Assert::notEquals(1, object);
		}),

		new UnitTestTyped<Type>("TypedTest", [](TestContextTyped<Type>& ctx) -> void {
			//Do test;
		}),

		}) {}
	};

}

int main() {
	Testing::TestSuit::registerClass<Testing::MyTestClass<uint64_t>>();

	Testing::TestSuit::instance()->run();
}

