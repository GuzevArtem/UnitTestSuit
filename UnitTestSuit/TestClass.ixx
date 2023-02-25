module;

#include <format>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <map>

export module TestClass;

import UnitTest;
import TestContext;
import TypeParse;

namespace {
	int constexpr length(const char* str) {
		return *str ? 1 + length(str + 1) : 0;
	}
}

export namespace Testing {

	export
	class TestClassInterface {
	public:
		constexpr virtual char const* name() const = 0;

		virtual std::vector<UnitTestInterface*>& getAllTests() = 0;

		constexpr virtual void setUp() {};
		constexpr virtual void tearDown() {};

		virtual void run() = 0;
		virtual void registerTestMethods() = 0;

		virtual void printData() const = 0;

		constexpr TestClassInterface* self() { return this; };
		constexpr const TestClassInterface* self() const { return this; };
	};

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

	public:
		constexpr virtual char const* name() const { return m_name; }

		virtual std::vector<UnitTestInterface*>& getAllTests() override { return m_unitTests; }

		virtual void run() override {
			m_data.reset();
			m_data.totalTestsCount = m_unitTests.size();
			for (UnitTestInterface* test : m_unitTests) {
				test->run();
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
				//TODO: update view per test
			}
			//TODO: update view per class
		}

		virtual void printData() const override {
			std::cout << std::format("Class [{}] results:\n\tCompleted tests: {}/{}\n\tFailed tests: {}/{}\n\tIgnored tests: {}/{}\n", this->name(), m_data.succeededTestsCount, m_data.totalTestsCount, m_data.failedTestsCount, m_data.totalTestsCount, m_data.ignoredTestsCount, m_data.totalTestsCount);
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
			if constexpr (std::is_member_function_pointer_v<Function>) {
				FunctionWrapper<Self, void, TestContext&> function(const_cast<Self*>(static_cast<const Self*>(this)), (void(__cdecl Self::*)(TestContext &))(func));
				UnitTestFunction::instance(inherited::getAllTests(), name, function);
			} else {
				UnitTestFunction::instance(inherited::getAllTests(), name, func);
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
			if constexpr (std::is_member_function_pointer_v<Function>) {
				FunctionTypedWrapper<Type, Self, void, TestContextTyped<Type>&> function(const_cast<Self<Type>*>(static_cast<const Self<Type>*>(this)), (void(__cdecl Self<Type>::*)(TestContextTyped<Type> &))(func));
				UnitTestTypedFunction<Type>::instance(inherited::getAllTests(), name, function);
			} else {
				UnitTestTypedFunction<Type>::instance(inherited::getAllTests(), name, func);
			}
		}

		constexpr virtual char const* name() const {
			char const* name_ptr = inherited::name();
			char const* type_name_ptr = TypeParse<Type>::name;
			size_t name_length = ::length(name_ptr);
			size_t type_name_length = ::length(type_name_ptr);
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
