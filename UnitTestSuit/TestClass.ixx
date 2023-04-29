module;

//#pragma warning( push )
//#pragma warning( disable : 4355 4365 4625 4626 4820 5202 5026 5027 5039 5220 )
//#include <format>
//#include <string>
//#include <string_view>
//#include <array>
//#include <vector>
//#include <map>
//#include <exception>
//#pragma warning( pop )

export module Testing:TestClass;

import std;

import :Interfaces;
import :TestView;
import :UnitTest;
import :TestContext;
import :TestException;
import TypeParse;
import Testing.Utils;

export namespace Testing {

	template<typename T>
	struct is_static_function_pointer : std::bool_constant<false> {};

	template<typename Return, typename ...Args>
	struct is_static_function_pointer<Return(*)(Args...)> : std::bool_constant<true> {};

	struct TestData {
		size_t totalTestsCount = 0;
		size_t startedTestsCount = 0;
		size_t succeededTestsCount = 0;
		size_t failedTestsCount = 0;
		size_t stoppedTestsCount = 0;
		size_t ignoredTestsCount = 0;

		constexpr void reset() {
			totalTestsCount = 0;
			startedTestsCount = 0;
			succeededTestsCount = 0;
			failedTestsCount = 0;
			stoppedTestsCount = 0;
			ignoredTestsCount = 0;
		}
	};

	template<typename Self>
		class TestClassInner : public TestClassInterface {
	private:
		mutable TestViewInterface* m_view;

		std::string m_name;
		TestData m_data;
		mutable std::vector<UnitTestInterface*> m_unitTests;
	public:
		constexpr TestClassInner(const char* name) : m_name(name), m_view(nullptr) {}
		constexpr TestClassInner(const std::string_view& name) : m_name(name), m_view(nullptr) {}
		constexpr TestClassInner(std::string_view&& name) : m_name(name), m_view(nullptr) {}
		constexpr TestClassInner(const std::string& name) : m_name(name), m_view(nullptr) {}
		constexpr TestClassInner(std::string&& name) : m_name(name), m_view(nullptr) {}
		constexpr TestClassInner(const utils::string_static& name) : m_name(std::to_string(name)), m_view(nullptr) {}
		constexpr TestClassInner(utils::string_static&& name) : m_name(std::to_string(name)), m_view(nullptr) {}

		constexpr virtual ~TestClassInner() {
			for (UnitTestInterface* test : m_unitTests) {
				delete test;
			}
		}

		virtual void setView(TestViewInterface* view) override {
			if (m_view) {
				m_view->update();
				if (!m_view->parent()) {
					delete m_view;
				}
			}
			if (view) {
				m_view = view;
			}
		}
		virtual TestViewInterface* view() const override {
			if (!m_view) {
				m_view = new TestViewConsole();
			}
			return m_view;
		}


		virtual void throwIfNotUniqueTestName(char const* name) noexcept(false) override {
			if (testExists(name)) {
				const std::string error_message = std::format("Test with name [{}] already exists in TestClass [{}].", name, this->name());
				throw std::runtime_error(error_message);
			}
		}

		constexpr virtual bool testExists(char const* name) override {
			if (m_view) {
				for (auto* test : getAllTests()) {
					if (utils::str_equal(name, test->name())) {
						m_view->addEntry(ViewLevel::error, std::format("Already exists! Registering test [{}] for TestClass [{}].", name, this->name()));
						return true;
					}
				}
			} else {
				std::cerr << std::format("Already exists! Registering test [{}] for TestClass [{}].", name, this->name()) << std::endl;
			}
			return false;
		}

	public:
		virtual size_t getTotalTestsCount() const override { return m_data.totalTestsCount; }
		virtual size_t getStartedTestsCount() const override { return m_data.startedTestsCount; }
		virtual size_t getCompletedTestsCount() const override { return m_data.succeededTestsCount; }
		virtual size_t getFailedTestsCount() const override { return m_data.failedTestsCount; }
		virtual size_t getIgnoredTestsCount() const override { return m_data.ignoredTestsCount; }
		virtual size_t getStoppedTestsCount() const override { return m_data.stoppedTestsCount; }

	private:
		virtual void onTestStart(const UnitTestInterface* test, TestContextInterface* ctx) override {
			view()->addEntry(ViewLevel::info, std::format("Starting test: [{}]", test->name()));
		}
		virtual void onTestComplete(const UnitTestInterface* test, TestContextInterface* ctx) override {
			view()->append(ViewLevel::info, "\t");
			view()->append(ViewLevel::info, std::format("COMPLETED"));
		}
		virtual void onTestComplete(const UnitTestInterface* test, TestContextInterface* ctx, const IgnoredException& e) override {
			view()->append(ViewLevel::info, "\t");
			view()->append(ViewLevel::info, std::format("COMPLETED:"));
			view()->append(ViewLevel::info, "\t");
			view()->append(ViewLevel::info, std::format("IGNORING: "));
			view()->startBlock();
			view()->append(ViewLevel::warning, std::format("{}", e.reason()), true);
			view()->endBlock();
		}
		virtual void onTestStop(const UnitTestInterface* test, TestContextInterface* ctx, const TestStopException& e) override {
			view()->append(ViewLevel::info, "\t");
			view()->append(ViewLevel::warning, std::format("STOPPED: "));
			view()->startBlock();
			view()->append(ViewLevel::warning, std::format("{}", e.reason()), true);
			view()->endBlock();
		}
		virtual void onTestIgnore(const UnitTestInterface* test, TestContextInterface* ctx, const IgnoredException& e) override {
			view()->append(ViewLevel::info, "\t");
			view()->append(ViewLevel::warning, std::format("IGNORED: "));
			view()->startBlock();
			view()->append(ViewLevel::warning, std::format("{}", e.reason()), true);
			view()->endBlock();
		}
		virtual void onTestFail(const UnitTestInterface* test, TestContextInterface* ctx, const TestException& te) override {
			view()->append(ViewLevel::info, "\t");
			view()->append(ViewLevel::error, std::format("FAIL: "));
			view()->startBlock();
			view()->append(ViewLevel::error, std::format("{}", te.reason()), true);
			view()->endBlock();
		}
		virtual void onTestFail(const UnitTestInterface* test, TestContextInterface* ctx, const std::exception& e) override {
			view()->append(ViewLevel::info, "\t");
			view()->append(ViewLevel::error, std::format("FAIL: "));
			view()->startBlock();
			view()->append(ViewLevel::error, std::format("{}", e.what()), true);
			view()->endBlock();
		}

	public:
		constexpr virtual char const* name() const { return m_name.c_str(); }

		virtual std::vector<UnitTestInterface*>& getAllTests() const override { return m_unitTests; }

		virtual void processTestResult(UnitTestInterface* test) {
			switch (test->getState()) {
				case Stopped:
					m_data.startedTestsCount++;
					m_data.stoppedTestsCount++;
					m_data.succeededTestsCount++;
					break;
				case Ignored:
					m_data.startedTestsCount++;
					m_data.ignoredTestsCount++;
					break;
				case Crashed:
					m_data.startedTestsCount++;
					m_data.failedTestsCount++;
					break;
				case Failed:
					m_data.startedTestsCount++;
					m_data.failedTestsCount++;
					break;
				case Suceeded:
					m_data.startedTestsCount++;
					m_data.succeededTestsCount++;
					break;
				case Started:
					m_data.startedTestsCount++;
					break;
				case NeverStarted:
					break;
				default:
					break;
			}
		}

		virtual void beforeTest() override {
			view()->onBeforeTest();
		}

		virtual void afterTest() override {
			view()->onAfterTest();
			view()->update();
		}

		virtual void beforeAllTests() override {
			view()->addEntry(ViewLevel::info, std::format("Starting test class: [{}]", name()));
			view()->onBeforeAllTests();
		}

		virtual void afterAllTests() override {
			view()->onAfterAllTests();
			printSummary();
		}

		virtual void printSummary() const override {
			view()->addEntry(ViewLevel::info, std::format("-----------------------------------------------------------"));
			view()->addEntry(ViewLevel::info, std::format("Class [{}] results:", name()));
			view()->startBlock();
			view()->addEntry(ViewLevel::info, std::format("Completed tests: {}/{}", m_data.succeededTestsCount, m_data.totalTestsCount));
			view()->addEntry(ViewLevel::info, std::format("Failed tests: {}/{}", m_data.failedTestsCount, m_data.totalTestsCount));
			view()->addEntry(ViewLevel::info, std::format("Ignored tests: {}/{}", m_data.ignoredTestsCount, m_data.totalTestsCount));
			view()->addEntry(ViewLevel::info, std::format("Stopped tests: {}", m_data.stoppedTestsCount, m_data.totalTestsCount));
			view()->endBlock();
			view()->addEntry(ViewLevel::info, std::format("==========================================================="));
			view()->update();
		}

		virtual size_t countTestFailed() const override {
			return m_data.failedTestsCount;
		}

		virtual void run() override {
			m_data.reset();
			m_data.totalTestsCount = m_unitTests.size();
			for (UnitTestInterface* test : m_unitTests) {
				beforeTest();
				test->run();
				processTestResult(test);
				afterTest();
			}
		}
	};

	export
		template<typename Self>
	class TestClass : public TestClassInner<Self> {
		typedef TestClassInner<Self> inherited;
	public:
		constexpr TestClass(char const* name) : inherited(name) {}
		constexpr TestClass(const std::string_view& name) : inherited(name) {}
		constexpr TestClass(std::string_view&& name) : inherited(name) {}
		constexpr TestClass(const std::string& name) : inherited(name) {}
		constexpr TestClass(std::string&& name) : inherited(name) {}
		constexpr TestClass(const utils::string_static& name) : inherited(name) {}
		constexpr TestClass(utils::string_static&& name) : inherited(name) {}

		template<typename Function>
		void addTest(char const* name, Function func) {
			inherited::checkTestNameUnique(name);

			if constexpr (is_static_function_pointer<Function>::value) {
				utils::FunctionStaticWrapper<void, TestContext&> function((void(*)(TestContext&))(func));
				UnitTest<utils::FunctionStaticWrapper<void, TestContext&>>::instance(inherited::getAllTests(), name, this, function);
			} else if constexpr (std::is_member_function_pointer_v<Function>) {
				utils::FunctionWrapper<Self, void, TestContext&> function(static_cast<const Self*>(this), (void(Self::*)(TestContext &))(func));
				UnitTest<utils::FunctionWrapper<Self, void, TestContext&>>::instance(inherited::getAllTests(), name, this, function);
			} else {
				UnitTest<Function>::instance(inherited::getAllTests(), name, this, func);
			}
		}

		template<typename Function>
		void addBenchmarkTest(char const* name, Function func, size_t iterations = 1) {
			inherited::checkTestNameUnique(name);

			if constexpr (is_static_function_pointer<Function>::value) {
				utils::FunctionStaticWrapper<void, TestContext&> function((void(*)(TestContext&))(func));
				BenchmarkUnitTest<utils::FunctionStaticWrapper<void, TestContext&>>::instance(inherited::getAllTests(), name, this, function, iterations);
			} else if constexpr (std::is_member_function_pointer_v<Function>) {
				utils::FunctionWrapper<Self, void, TestContext&> function(static_cast<const Self*>(this), (void(Self::*)(TestContext&))(func));
				BenchmarkUnitTest<utils::FunctionWrapper<Self, void, TestContext&>>::instance(inherited::getAllTests(), name, this, function, iterations);
			} else {
				BenchmarkUnitTest<Function>::instance(inherited::getAllTests(), name, this, func, iterations);
			}
		}
	};

	export
		template<template<typename> typename Self, typename Type>
	class TestClassTyped : public TestClassInner<Self<Type>> {
		typedef TestClassInner<Self<Type>> inherited;
	public:
		constexpr TestClassTyped(char const* name) : inherited(std::move(utils::concat(name, '<', helper::TypeParse<Type>::name, '>'))) {}
		constexpr TestClassTyped(const std::string_view& name) : TestClassTyped(name.data()) {}
		constexpr TestClassTyped(std::string_view&& name) : TestClassTyped(name.data()) {}
		constexpr TestClassTyped(const std::string& name) : TestClassTyped(name.data()) {}
		constexpr TestClassTyped(std::string&& name) : TestClassTyped(name.data()) {}
		constexpr TestClassTyped(const utils::string_static& name) : TestClassTyped(name.data()) {}
		constexpr TestClassTyped(utils::string_static&& name) : TestClassTyped(name.data()) {}

		template<typename Function>
		void addTest(char const* name, Function func) {
			inherited::checkTestNameUnique(name);

			if constexpr (is_static_function_pointer<Function>::value) {
				utils::FunctionStaticWrapper<void, TestContextTyped<Type>&> function((void(*)(TestContextTyped<Type>&))(func));
				UnitTestTyped<Type, utils::FunctionStaticWrapper<void, TestContextTyped<Type>&>>::instance(inherited::getAllTests(), name, this, function);
			} else if constexpr (std::is_member_function_pointer_v<Function>) {
				utils::FunctionWrapper<Self<Type>, void, TestContextTyped<Type>&> function(static_cast<const Self<Type>*>(this), (void(Self<Type>::*)(TestContextTyped<Type> &))(func));
				UnitTestTyped<Type, utils::FunctionWrapper<Self<Type>, void, TestContextTyped<Type>&>>::instance(inherited::getAllTests(), name, this, function);
			} else {
				UnitTestTyped<Type, Function>::instance(inherited::getAllTests(), name, this, func);
			}
		}

		template<typename Function>
		void addBenchmarkTest(char const* name, Function func, size_t iterations = 1) {
			inherited::checkTestNameUnique(name);

			if constexpr (is_static_function_pointer<Function>::value) {
				utils::FunctionStaticWrapper<void, TestContextTyped<Type>&> function((void(*)(TestContextTyped<Type>&))(func));
				BenchmarkUnitTestTyped<Type, utils::FunctionStaticWrapper<void, TestContextTyped<Type>&>>::instance(inherited::getAllTests(), name, this, function, iterations);
			} else if constexpr (std::is_member_function_pointer_v<Function>) {
				utils::FunctionWrapper<Self<Type>, void, TestContextTyped<Type>&> function(static_cast<const Self<Type>*>(this), (void(Self<Type>::*)(TestContextTyped<Type> &))(func));
				BenchmarkUnitTestTyped<Type, utils::FunctionWrapper<Self<Type>, void, TestContextTyped<Type>&>>::instance(inherited::getAllTests(), name, this, function, iterations);
			} else {
				BenchmarkUnitTestTyped<Type, Function>::instance(inherited::getAllTests(), name, this, func, iterations);
			}
		}
	};

	export
		template<template<auto> typename Self, auto t_Arg>
	class TestClassArgumented : public TestClass<Self<t_Arg>> {
		typedef TestClass<Self<t_Arg>> inherited;
	public:
		constexpr TestClassArgumented(char const* name) : inherited(std::move(utils::concat(name, "<(", helper::TypeParse<decltype(t_Arg)>::name, ')', t_Arg, '>'))) {}
		constexpr TestClassArgumented(const std::string_view& name) : TestClassArgumented(name.data()) {}
		constexpr TestClassArgumented(std::string_view&& name) : TestClassArgumented(name.data()) {}
		constexpr TestClassArgumented(const std::string& name) : TestClassArgumented(name.data()) {}
		constexpr TestClassArgumented(std::string&& name) : TestClassArgumented(name.data()) {}
		constexpr TestClassArgumented(const utils::string_static& name) : TestClassArgumented(name.data()) {}
		constexpr TestClassArgumented(utils::string_static&& name) : TestClassArgumented(name.data()) {}
	};

	export
		template<template<typename, auto> typename Self, typename Type, auto t_Arg>
	class TestClassArgumentedTyped : public TestClassInner<Self<Type, t_Arg>> {
		typedef TestClassInner<Self<Type, t_Arg>> inherited;
	public:
		constexpr TestClassArgumentedTyped(char const* name) : inherited(std::move(utils::concat(name, '<', helper::TypeParse<Type>::name, ", (", helper::TypeParse<decltype(t_Arg)>::name, ')', t_Arg, '>'))) {}
		constexpr TestClassArgumentedTyped(const std::string_view& name) : TestClassArgumentedTyped(name.data()) {}
		constexpr TestClassArgumentedTyped(std::string_view&& name) : TestClassArgumentedTyped(name.data()) {}
		constexpr TestClassArgumentedTyped(const std::string& name) : TestClassArgumentedTyped(name.data()) {}
		constexpr TestClassArgumentedTyped(std::string&& name) : TestClassArgumentedTyped(name.data()) {}
		constexpr TestClassArgumentedTyped(const utils::string_static& name) : TestClassArgumentedTyped(name.data()) {}
		constexpr TestClassArgumentedTyped(utils::string_static&& name) : TestClassArgumentedTyped(name.data()) {}

		template<typename Function>
		void addTest(char const* name, Function func) {
			inherited::checkTestNameUnique(name);

			if constexpr (is_static_function_pointer<Function>::value) {
				utils::FunctionStaticWrapper<void, TestContextTyped<Type>&> function((void(*)(TestContextTyped<Type>&))(func));
				UnitTestTyped<Type, utils::FunctionStaticWrapper<void, TestContextTyped<Type>&>>::instance(inherited::getAllTests(), name, this, function);
			} else if constexpr (std::is_member_function_pointer_v<Function>) {
				utils::FunctionWrapper<Self<Type, t_Arg>, void, TestContextTyped<Type>&> function(static_cast<const Self<Type, t_Arg>*>(this), (void(Self<Type, t_Arg>::*)(TestContextTyped<Type> &))(func));
				UnitTestTyped<Type, utils::FunctionWrapper<Self<Type, t_Arg>, void, TestContextTyped<Type>&>>::instance(inherited::getAllTests(), name, this, function);
			} else {
				UnitTestTyped<Type, Function>::instance(inherited::getAllTests(), name, this, func);
			}
		}

		template<typename Function>
		void addBenchmarkTest(char const* name, Function func, size_t iterations = 1) {
			inherited::checkTestNameUnique(name);

			if constexpr (is_static_function_pointer<Function>::value) {
				utils::FunctionStaticWrapper<void, TestContextTyped<Type>&> function((void(*)(TestContextTyped<Type>&))(func));
				BenchmarkUnitTestTyped<Type, utils::FunctionStaticWrapper<void, TestContextTyped<Type>&>>::instance(inherited::getAllTests(), name, this, function, iterations);
			} else if constexpr (std::is_member_function_pointer_v<Function>) {
				utils::FunctionWrapper<Self<Type, t_Arg>, void, TestContextTyped<Type>&> function(static_cast<const Self<Type, t_Arg>*>(this), (void(Self<Type, t_Arg>::*)(TestContextTyped<Type> &))(func));
				BenchmarkUnitTestTyped<Type, utils::FunctionWrapper<Self<Type, t_Arg>, void, TestContextTyped<Type>&>>::instance(inherited::getAllTests(), name, this, function, iterations);
			} else {
				BenchmarkUnitTestTyped<Type, Function>::instance(inherited::getAllTests(), name, this, func, iterations);
			}
		}
	};
}
