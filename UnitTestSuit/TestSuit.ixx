module;

#include <format>
#include <concepts>
#include <vector>

export module TestSuit;

import Interfaces;
import TestView;
import TestClass;

export namespace Testing {

	template<typename T>
	concept TestClassType = std::same_as<T, TestClassInterface> || std::is_base_of_v<TestClassInterface, T>;

	export
	class TestSuit {
	public:
		constexpr TestSuit(TestViewInterface* view = TestViewConsole::instance()) : m_view(view), m_registeredTests(0), m_testClasses() {}
		constexpr virtual ~TestSuit() {
			for (TestClassInterface* testClass : m_testClasses) {
				delete testClass;
			}
			if (!m_view->parent()) {
				delete m_view;
			}
		}

	private:
		TestViewInterface* m_view;
		size_t m_registeredTests;
		std::vector<TestClassInterface*> m_testClasses;

	public:
		void run() {
			for (TestClassInterface* cls : m_testClasses) {
				try {
					cls->setUp();
					cls->beforeAllTests();
					cls->run();
					cls->afterAllTests();
					cls->tearDown();
				} catch (const std::exception& e) {
					if (!processTestException(e)) {
						m_view->addEntry(TestViewInterface::error, std::format("Test execution for class {} failed.\n {}", cls->name(), e.what()), true);
						break;
					}
				}
				m_view->print();
			}
		}

		/*
		* Return false if exception is fatal and class execution should be terminated.
		*/
		constexpr bool processTestException(const std::exception& e) const {
			return false;
		}

		template<TestClassType T, typename... Args>
		void registerClass(Args&&... args) {
			T* cls = new T(std::forward<Args>(args)...);
			if (m_view) {
				TestViewInterface* view = m_view->clone();
				view->parent(view);
				cls->setView(view);
			}
			cls->registerTestMethods();
			m_testClasses.emplace_back(static_cast<TestClassInterface*>(cls->self()));
			m_registeredTests++;
		}

		template<template <typename T> typename TemplatedTestClass, typename ClassType, typename... ClassTypes>
		void registerMultipleClasses() {
			registerClass<TemplatedTestClass<ClassType>>();
			if constexpr (sizeof...(ClassTypes)) {
				registerMultipleClasses<TemplatedTestClass, ClassTypes...>();
			}
		}
	};
}
