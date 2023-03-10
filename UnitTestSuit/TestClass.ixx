module;

#include <format>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <exception>

export module TestClass;

import Interfaces;
import TestView;
import UnitTest;
import TestContext;
import TestException;
import TypeParse;
import Utils;

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
		TestViewInterface* m_view = TestViewConsole::create();

		char const* m_name;
		TestData m_data;
		std::vector<UnitTestInterface*> m_unitTests;
	public:
		constexpr TestClassInner(char const* name) : m_name(name) {}

		constexpr virtual ~TestClassInner() {
			for (UnitTestInterface* test : m_unitTests) {
				delete test;
			}
		}

		virtual void setView(TestViewInterface* view) {
			if (m_view) {
				m_view->print();
				if (!m_view->parent()) {
					delete m_view;
				}
			}
			if (view) {
				m_view = view;
			}
		}
		virtual TestViewInterface* view() const { return m_view; }

	private:
		virtual void onTestStart(const UnitTestInterface* test, TestContext* ctx) override {
			m_view->addEntry(TestViewInterface::info, std::format("Starting test: [{}]", test->name()));
		}
		virtual void onTestComplete(const UnitTestInterface* test, TestContext* ctx) override {
			m_view->append(TestViewInterface::info, "\t");
			m_view->append(TestViewInterface::info, std::format("COMPLETED"));
		}
		virtual void onTestComplete(const UnitTestInterface* test, TestContext* ctx, const IgnoredException& e) override {
			m_view->append(TestViewInterface::info, "\t");
			m_view->append(TestViewInterface::info, std::format("COMPLETED:"));
			m_view->append(TestViewInterface::info, "\t");
			m_view->append(TestViewInterface::info, std::format("IGNORING: "));
			m_view->indent();
			m_view->append(TestViewInterface::warning, std::format("{}", e.reason()), true);
			m_view->unindent();
		}
		virtual void onTestStop(const UnitTestInterface* test, TestContext* ctx, const TestStopException& e) override {
			m_view->append(TestViewInterface::info, "\t");
			m_view->append(TestViewInterface::warning, std::format("STOPPED: "));
			m_view->indent();
			m_view->append(TestViewInterface::warning, std::format("{}", e.reason()), true);
			m_view->unindent();
		}
		virtual void onTestIgnore(const UnitTestInterface* test, TestContext* ctx, const IgnoredException& e) override {
			m_view->append(TestViewInterface::info, "\t");
			m_view->append(TestViewInterface::warning, std::format("IGNORED: "));
			m_view->indent();
			m_view->append(TestViewInterface::warning, std::format("{}", e.reason()), true);
			m_view->unindent();
		}
		virtual void onTestFail(const UnitTestInterface* test, TestContext* ctx, const TestException& te) override {
			m_view->append(TestViewInterface::info, "\t");
			m_view->append(TestViewInterface::error, std::format("FAIL: "));
			m_view->indent();
			m_view->append(TestViewInterface::error, std::format("{}", te.reason()), true);
			m_view->unindent();
		}
		virtual void onTestFail(const UnitTestInterface* test, TestContext* ctx, const std::exception& e) override {
			m_view->append(TestViewInterface::info, "\t");
			m_view->append(TestViewInterface::error, std::format("FAIL: "));
			m_view->indent();
			m_view->append(TestViewInterface::error, std::format("{}", e.what()), true);
			m_view->unindent();
		}

	public:
		constexpr virtual char const* name() const { return m_name; }

		virtual std::vector<UnitTestInterface*>& getAllTests() override { return m_unitTests; }

		virtual void processTestResult(UnitTestInterface* test) {
			switch (test->getState()) {
				case Stopped:
					m_data.startedTestsCount++;
					m_data.stoppedTestsCount++;
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
			m_view->indent();
		}

		virtual void afterTest() override {
			m_view->unindent();
			m_view->print();
		}

		virtual void beforeAllTests() override {
			m_view->addEntry(TestViewInterface::info, std::format("Starting test class: [{}]", name()));
		}

		virtual void afterAllTests() override {
			m_view->addEntry(TestViewInterface::info, std::format("-----------------------------------------------------------"));
			m_view->addEntry(TestViewInterface::info, std::format("Class [{}] results:", name()));
			m_view->indent();
			m_view->addEntry(TestViewInterface::info, std::format("Completed tests: {}/{}", m_data.succeededTestsCount, m_data.totalTestsCount));
			m_view->addEntry(TestViewInterface::info, std::format("Failed tests: {}/{}", m_data.failedTestsCount, m_data.totalTestsCount));
			m_view->addEntry(TestViewInterface::info, std::format("Ignored tests: {}/{}", m_data.ignoredTestsCount, m_data.totalTestsCount));
			m_view->unindent();
			m_view->addEntry(TestViewInterface::info, std::format("==========================================================="));
			m_view->print();
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

		template<typename Function>
		void addTest(char const* name, Function func) {
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
	};

	export
		template<template<typename> typename Self, typename Type>
	class TestClassTyped : public TestClassInner<Self<Type>> {
		typedef TestClassInner<Self<Type>> inherited;
	public:
		constexpr TestClassTyped(char const* name) : inherited(name) {}

		template<typename Function>
		void addTest(char const* name, Function func) {
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

		constexpr virtual char const* name() const override {
			char const* name_ptr = inherited::name();
			char const* type_name_ptr = TypeParse<Type>::name;
			size_t name_length = utils::length(name_ptr);
			size_t type_name_length = utils::length(type_name_ptr);
			std::array<char, 256> result_holder;
			for (size_t i = 0; i < name_length; ++i) {
				result_holder[i] = name_ptr[i];
			}
			result_holder[name_length] = '<';
			for (size_t i = 0; i < type_name_length; ++i) {
				result_holder[name_length + i + 1] = type_name_ptr[i];
			}
			result_holder[name_length + type_name_length + 1] = '>';
			result_holder[name_length + type_name_length + 2] = '\0';
			return result_holder.data();
		}
	};
}
