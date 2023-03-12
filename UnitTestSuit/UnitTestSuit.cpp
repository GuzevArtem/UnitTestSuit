#include <format>
#include <chrono>

import Testing;

namespace Testing {

	constexpr bool PRINT_IGNORED_EXCEPTIONS = false;

	template<typename Type>
	class MyTestClass : public TestClassTyped<MyTestClass, Type> {
		typedef TestClassTyped<MyTestClass, Type> inherited;
	public:

		MyTestClass() : inherited("MyTestClass") {}

		void SampleTest(TestContext& ctx) {
			auto objects = ctx.createTestObjectsArray<uint64_t, 5>();
			for (size_t i = 0; i < 5; ++i) {
				objects[i] = { i };
			}
			Assert::forEach(objects.begin(), objects.end(), [](uint64_t& item) -> void {
				Assert::notEquals(item, -1);
			});
		}

		void SampleTestTyped(TestContextTyped<Type>& ctx) {
			auto objects = ctx.createTestObjectsArray<5>();
			for (size_t i = 0; i < 5; ++i) {
				objects[i] = { (Type)i };
			}
			Assert::forEach(objects.begin(), objects.end(), [](Type& item) -> void {
				Assert::notEquals(item, -1);
			});
		}

		static void SampleTestStatic(TestContext& ctx) {
			Test::ignore();
		}

		constexpr virtual void registerTestMethods() override {
			this->addTest("SampleTest", &MyTestClass<Type>::SampleTest);
			this->addTest("SampleTestTyped", &MyTestClass<Type>::SampleTestTyped);
			this->addTest("SampleStaticTest", &MyTestClass<Type>::SampleTestStatic);

			this->addTest("FailTest", [](TestContext& ctx) -> void {
				Assert::Fail("Test should fail!");
			});

			this->addTest("StopTest", [](TestContext& ctx) -> void {
				Test::stop();
			});

			this->addTest("ExpectedException", [](TestContext& ctx) -> void {
				Expected<AssertEqualsException<int, int>>::during([]() {
					Assert::equals(1, 0);
				});
			});

			this->addTest("ExpectedMatchedException", [](TestContext& ctx) -> void {
				Expected<AssertEqualsException<int, int>>::during([]() {
					Assert::equals(1, 0);
				}, [](const AssertEqualsException<int, int>& actual) -> bool {
					return true;
				});
			});

			this->addTest("ExpectedExpectedException", [](TestContext& ctx) -> void {
				Expected<ExpectedFailedException<void>>::during([]() {
					Expected<>::during([]() {
						Assert::equals(0, 0);
					});
				});
			});

			this->addTest("Creation", [](TestContext& ctx) -> void {
				uint64_t object = ctx.createTestObject<uint64_t>();

				Assert::isZero(object);
				Assert::same<uint64_t>(object);
				Assert::notEquals(object, 1);
				Assert::equals(object, 0);
			});

			this->addTest("TypedTestOr", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();
				Assert::Or<PRINT_IGNORED_EXCEPTIONS>(
					[&object]() { Assert::notZero(object); },
					[&object]() { Assert::notEquals(object, 0); },
					[&object]() { Assert::equals(object, 0); }
				);
			});

			this->addTest("TypedTestNor", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();
				Assert::Nor<PRINT_IGNORED_EXCEPTIONS>(
					[&object]() { Assert::notZero(object); },
					[&object]() { Assert::notEquals(object, (Type)0); },
					[&object]() { Assert::equals(object, (Type)1); }
				);
			});

			this->addTest("TypedTestAnd", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();
				Assert::And<PRINT_IGNORED_EXCEPTIONS>(
					[&object]() { Assert::isZero(object); },
					[&object]() { Assert::notEquals(object, (Type)1); },
					[&object]() { Assert::equals(object, (Type)0); }
				);
			});

			this->addTest("TypedTestNand", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();
				Assert::Nand<PRINT_IGNORED_EXCEPTIONS>(
					[&object]() { Assert::notZero(object); },
					[&object]() { Assert::notEquals(object, (Type)0); },
					[&object]() { Assert::equals(object, (Type)1); }
				);
			});

			this->addTest("TypedTestWithBenchmarkIn", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();
				auto foo = [](Type& obj) mutable {
					std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<uint64_t>(10 * (++obj))));
					Benchmark::doNotOptimizeAway(obj);
				};
				auto result = Benchmark::function(100, foo, object);
				//std::cout << std::format("100 iterations:   [{}]\n", result);
				ctx.log(std::format("100 iterations:   [{}]", result));
				result = Benchmark::function(foo, object);
				//std::cout << std::format("single iteration: [{}]\n", result);
				ctx.log(std::format("single iteration: [{}]", result));
			});

			this->addBenchmarkTest("BenchmarkedTypedTest", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();

				Assert::isZero(object);
				Assert::same<Type>(object);
				Assert::notEquals(object, (Type)1);
				Assert::equals(object, (Type)0);
			}, 5);
		}
	};
}

int main() {
	Testing::TestSuit suit;
	suit.registerClass<Testing::MyTestClass<uint64_t>>();
	suit.registerMultipleClasses<Testing::MyTestClass, uint32_t, uint16_t, uint8_t, float> ();
	suit.run();
}
