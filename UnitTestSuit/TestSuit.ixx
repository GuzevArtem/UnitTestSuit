module;

#include <format>
#include <concepts>
#include <vector>

export module Testing:TestSuit;

import :Interfaces;
import :TestView;
import :TestClass;

export namespace Testing {

	template<typename T>
	concept TestClassType = std::same_as<T, TestClassInterface> || std::is_base_of_v<TestClassInterface, T>;

	export
	class TestSuit {
	public:
		constexpr TestSuit(TestViewInterface* view = TestViewConsole::create()) : m_view(view), m_registeredTests(0), m_testClasses() {}
		constexpr virtual ~TestSuit() {
			for (TestClassInterface* testClass : m_testClasses) {
				delete testClass;
			}
			if (!m_view->parent()) {
				delete m_view;
			}
		}
	public:
		size_t m_errorLinesToPrint = 5;

	private:
		TestViewInterface* m_view;
		size_t m_registeredTests;
		std::vector<TestClassInterface*> m_testClasses;

	public:
		void run() {
			size_t finishedClassCount = 0;
			for (TestClassInterface* cls : m_testClasses) {
				if (!cls) {
					throw "TestSuit::run() Trying to run (null) as TestClass!";
				}
				try {
					cls->setUp();
					cls->beforeAllTests();
					cls->run();
					cls->afterAllTests();
					cls->tearDown();
				} catch (const std::exception& e) {
					if (!processTestException(e)) {
						m_view->addEntry(ViewLevel::error, std::format("Test execution for class {} failed.\n {}", cls->name(), e.what()), true);
						break;
					}
				}
				m_view->print();
				++finishedClassCount;
			}
			m_view->addEntry(ViewLevel::info, std::format("___________________________________________________________"));
			m_view->addEntry(ViewLevel::info, std::format("__________________ TEST RUN COMPLETED _____________________"));
			m_view->addEntry(ViewLevel::info, std::format("___________________________________________________________"));
			m_view->addEntry(ViewLevel::info, std::format(""));
			m_view->addEntry(ViewLevel::info, std::format("Finished tests execution for {}/{} classes.", finishedClassCount, m_testClasses.size()));
			m_view->addEntry(ViewLevel::info, std::format("___________________________________________________________"));
			m_view->addEntry(ViewLevel::info, std::format(""));
			m_view->print();

			for (TestClassInterface* cls : m_testClasses) {
				if (cls->countTestFailed() > 0) {
					m_view->addEntry(ViewLevel::info, std::format("Class \"{}\" has {} failed tests:", cls->name(), cls->countTestFailed()));
					m_view->indent();
					for (auto test : cls->getAllTests()) {
						if (test->getState() == TestState::Failed || test->getState() == TestState::Crashed) {
							m_view->addEntry(ViewLevel::info, std::format("Test \"{}\":\t", test->name()));
							m_view->indent();
							m_view->addEntry(ViewLevel::info, test->errorMessage(), true, m_errorLinesToPrint);
							m_view->unindent();
						}
					}
					m_view->unindent();
					m_view->print();
				}
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
				view->parent(m_view);
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
