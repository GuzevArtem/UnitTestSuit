import std.compat;

import Testing;
import TypeParse;

#pragma warning( push )
#pragma warning( disable : 4100 ) // warning C4100: unreferenced formal parameter
namespace Testing {

	constexpr bool PRINT_IGNORED_EXCEPTIONS = false;

	template<typename T>
	class CustomException : public std::exception {
		using inherited = std::exception;
	public:
		T value;

		CustomException(T value)
			: inherited(utils::concat("CustomException", '<', helper::TypeParse<T>::name, '>', " with value ", '[', std::to_string(value), ']', '\n', std::to_string(std::stacktrace::current())).data()),
			value(value) {
		}
	};

	template<typename T>
	void functionToRaiseException(T arg) noexcept(false) {
		throw CustomException(arg);
	}

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
			Assert::forEach<true>(objects.begin(), objects.end(), [](Type& item) -> void {
				Assert::notEquals(item, -1);
			});
		}

		static void SampleTestStatic(TestContext& ctx) {
			Test::ignore();
		}

		virtual void registerTestMethods() override {
			this->addTest("SampleTest", &MyTestClass<Type>::SampleTest);
			this->addTest("SampleTestTyped", &MyTestClass<Type>::SampleTestTyped);
			this->addTest("SampleStaticTest", &MyTestClass<Type>::SampleTestStatic);

			this->addTest("FailTest", [](TestContext& ctx) -> void {
				Assert::fail("Test should fail!");
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
				Expected<AssertEqualsException<int, int>>::duringMatching(
					[](const AssertEqualsException<int, int>& actual) -> bool { return true; },
					[]() {
						Assert::equals(1, 0);
					}
				);
			});

			this->addTest("ExpectedExpectedException", [](TestContext& ctx) -> void {
				Expected<ExpectedFailedException<void, false>>::during([]() {
					Expected<>::during([]() {
						Assert::equals(0, 0);
					});
				});
			});


			this->addTest("ExpectedTemplatedOuterFunctionMatchingValueException", [](TestContextTyped<Type>& ctx) -> void {
				const Type passedValue{};
				try {
					Expected<CustomException<Type>>::duringMatching(
						[](const CustomException<Type>& exception) -> bool { return true; },
						&Testing::functionToRaiseException<Type>,
						passedValue
					);
				} catch (const std::exception& e) {
					Assert::fail(std::format("Unexpected exception captured!\n{}", e.what()));
				}

				Expected<ExpectedFailedException<CustomException<Type>>>::during(
					[passedValue]() {
						Expected<CustomException<Type>>::duringMatching(
							[passedValue](const CustomException<Type>& exception) -> bool { return false; },
							&Testing::functionToRaiseException<Type>,
							passedValue
						);
					}
				);
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
				Assert::anyOf<PRINT_IGNORED_EXCEPTIONS>(
					[&object]() { Assert::notZero(object); },
					[&object]() { Assert::notEquals(object, 0); },
					[&object]() { Assert::equals(object, 0); }
				);
			});

			this->addTest("TypedTestNor", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();
				Assert::noneOf<PRINT_IGNORED_EXCEPTIONS>(
					[&object]() { Assert::notZero(object); },
					[&object]() { Assert::notEquals(object, (Type)0); },
					[&object]() { Assert::equals(object, (Type)1); }
				);
			});

			this->addTest("TypedTestAnd", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();
				Assert::allOf<PRINT_IGNORED_EXCEPTIONS>(
					[&object]() { Assert::isZero(object); },
					[&object]() { Assert::notEquals(object, (Type)1); },
					[&object]() { Assert::equals(object, (Type)0); }
				);
			});

			this->addTest("TypedTestNand", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();
				Assert::notAllOf<PRINT_IGNORED_EXCEPTIONS>(
					[&object]() { Assert::notZero(object); },
					[&object]() { Assert::notEquals(object, (Type)0); },
					[&object]() { Assert::equals(object, (Type)1); }
				);
			});

			this->addTest("TypedTestWithBenchmarkIn", [](TestContextTyped<Type>& ctx) -> void {
				Type object = ctx.createTestObject();
				auto foo = [](Type& obj) mutable {
					std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<uint64_t>(10 * static_cast<uint64_t>(++obj))));
					Benchmark::doNotOptimizeAway(obj);
				};
				auto result = Benchmark::function(100, foo, object);
				ctx.log(std::format("100 iterations:   [{}]", result));
				result = Benchmark::function(foo, object);
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

	class TestClassDuplicatedNames : public TestClass<TestClassDuplicatedNames> {
		typedef TestClass<TestClassDuplicatedNames> inherited;
	private:
		bool allowNonUniqueTestNamesState;
		std::string actualErrorMessage;

	public:
		TestClassDuplicatedNames() : inherited("TestClassDuplicatedNames"), allowNonUniqueTestNamesState(true) {}

		virtual bool allowNonUniqueTestNames() const noexcept override { return allowNonUniqueTestNamesState; }

		virtual void registerTestMethods() override {
			this->addTest("TestWithSameName", [](TestContext& ctx) -> void {
				Assert::isTrue(true);
			});
			this->addTest("TestWithSameName", [](TestContext& ctx) -> void {
				Assert::isTrue(true);
			});
			allowNonUniqueTestNamesState = false;
			try {
				this->addTest("TestWithSameName", [](TestContext& ctx) -> void {
					Assert::fail("Test with same name Should not be registerable if allowNonUniqueTestNames() returns false");
				});
			} catch(const std::exception& e) {
				actualErrorMessage = std::string(e.what()); // save error message
				this->addTest("TestWithSameNameException", [this](TestContext& ctx) -> void {
					const std::string expectedErrorMessage = std::format("Test with name [TestWithSameName] already exists in TestClass [{}]", name());
					Assert::isTrue(actualErrorMessage.contains(expectedErrorMessage), std::format("Expecting exception that contains:\n{}\nbut received:\n{}", expectedErrorMessage, actualErrorMessage));
				});
			}
		}
	};

	template<size_t value>
	class MyArgumentedTestClass : public TestClassArgumented<MyArgumentedTestClass<value>, value> {
		typedef TestClassArgumented<MyArgumentedTestClass<value>, value> inherited;
	public:

		MyArgumentedTestClass() : inherited("MyArgumentedTestClass") {}

		virtual void registerTestMethods() override {
			this->addTest("ArgumentTest", [](TestContext& ctx) -> void {
				Assert::same<size_t>(value);
			});
		}
	};

	template<typename Type, size_t value>
	class MyArgumentedTypedTestClass : public TestClassArgumentedTyped<MyArgumentedTypedTestClass<Type, value>, Type, value > {
		typedef TestClassArgumentedTyped<MyArgumentedTypedTestClass<Type, value>, Type, value> inherited;
	public:

		MyArgumentedTypedTestClass() : inherited("MyArgumentedTypedTestClass") {}

		virtual void registerTestMethods() override {
			this->addTest("ArgumentTypedTest", [](TestContext& ctx) -> void {
				Assert::same<size_t>(value);
			});
		}
	};
}
#pragma warning( pop ) // C4100

int main() {
	Testing::TestSuit suit;
	suit.registerClass<Testing::MyTestClass<uint64_t>>();
	suit.registerClass<Testing::TestClassDuplicatedNames>();
	suit.registerMultipleClasses<Testing::MyTestClass, uint32_t, uint16_t, uint8_t, float> ();
	suit.registerArgumentedClass<Testing::MyArgumentedTestClass, 0, 1, 2, 3, 4, 5, 6, 7, 8>();
	suit.registerArgumentedClass<Testing::MyArgumentedTypedTestClass, int, 0, 1, 2, 3, 4, 5, 6, 7, 8>();
	suit.run();
}
