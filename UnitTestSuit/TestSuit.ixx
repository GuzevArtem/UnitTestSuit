module;

#include <format>
#include <concepts>
#include <vector>
#include <algorithm>
#include <chrono>

export module Testing:TestSuit;

import :Interfaces;
import :TestView;
import :TestClass;

export namespace Testing {

	template<typename T>
	concept TestClassType = !std::same_as<T, TestClassInterface> && std::is_base_of_v<TestClassInterface, T>;


	template<typename T, auto... args, template<typename T, auto... args> typename ClassType>
	concept TestClassTemplateArgumentedType = !std::same_as<ClassType<T, args...>, TestClassInterface>&& std::is_base_of_v<TestClassInterface, ClassType<T, args...>>;

	export
	class TestSuit {
	public:
		constexpr TestSuit(TestViewInterface* view = TestViewConsole::create()) : m_view(view), m_registeredTestClasses(0), m_testClasses() {}
		constexpr virtual ~TestSuit() {
			for (TestClassInterface* testClass : m_testClasses) {
				delete testClass;
			}
			if (!m_view->parent()) {
				delete m_view;
			}
		}
	public:
		size_t m_errorLinesToPrint = 7;

	private:
		TestViewInterface* m_view;
		size_t m_registeredTestClasses;
		std::vector<TestClassInterface*> m_testClasses;

	public:
		void run() {
			size_t finishedClassCount = 0;

			size_t totalTestsCount = 0;
			size_t succeededTestsCount = 0;
			size_t failedTestsCount = 0;
			size_t ignoredTestsCount = 0;
			size_t stoppedTestsCount = 0;

			const auto start_time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
			const std::chrono::steady_clock::time_point start_at = std::chrono::steady_clock::now();
			for (TestClassInterface* cls : m_testClasses) {
				if (!cls) {
					throw "TestSuit::run() Trying to run (null) as TestClass!";
				}
				bool tearingDownStarted = false;
				try {
					cls->setUp();
					cls->beforeAllTests();
					cls->run();
					cls->afterAllTests();
					tearingDownStarted = true;
					cls->tearDown();
				} catch (const std::exception& e) {
					if (!tearingDownStarted) {
						try {
							cls->tearDown();
						} catch (const std::exception& e) {
							m_view->addEntry(ViewLevel::error, std::format("TearDown for class {} failed.\n {}", cls->name(), e.what()), true);
						}
					}
					if (!processTestException(e)) {
						m_view->addEntry(ViewLevel::error, std::format("Test execution for class {} failed.\n {}", cls->name(), e.what()), true);
						break;
					}
				}
				m_view->print();
				++finishedClassCount;
				totalTestsCount += cls->getTotalTestsCount();
				succeededTestsCount += cls->getCompletedTestsCount();
				failedTestsCount += cls->getFailedTestsCount();
				ignoredTestsCount += cls->getIgnoredTestsCount();
				stoppedTestsCount += cls->getStoppedTestsCount();
			}
			const std::chrono::nanoseconds total_time = std::chrono::steady_clock::now() - start_at;

			m_view->addEntry(ViewLevel::info, std::format("___________________________________________________________"));
			m_view->addEntry(ViewLevel::info, std::format("__________________ TEST RUN COMPLETED _____________________"));
			m_view->addEntry(ViewLevel::info, std::format("___________________________________________________________"));
			m_view->addEntry(ViewLevel::info, std::format(""));
			m_view->addEntry(ViewLevel::info, std::format("Finished tests execution for {}/{} classes.", finishedClassCount, m_testClasses.size()));

			m_view->addEntry(ViewLevel::info, std::format("Started at: {}", start_time));
			m_view->addEntry(ViewLevel::info, std::format("And took:   "));
			m_view->append(ViewLevel::info, std::format("{} ", std::chrono::duration_cast<std::chrono::hours>(total_time)));
			m_view->append(ViewLevel::info, std::format("{} ", std::chrono::duration_cast<std::chrono::minutes>(total_time) % std::chrono::hours(1)));
			m_view->append(ViewLevel::info, std::format("{} ", std::chrono::duration_cast<std::chrono::seconds>(total_time) % std::chrono::minutes(1)));
			m_view->append(ViewLevel::info, std::format("{} ", std::chrono::duration_cast<std::chrono::milliseconds>(total_time) % std::chrono::seconds(1)));
			m_view->indent();
			m_view->addEntry(ViewLevel::info, std::format("Total completed tests count: {}/{}", succeededTestsCount, totalTestsCount));
			m_view->addEntry(ViewLevel::info, std::format("Total failed tests count:    {}/{}", failedTestsCount, totalTestsCount));
			m_view->addEntry(ViewLevel::info, std::format("Total ignored tests count:   {}/{}", ignoredTestsCount, totalTestsCount));
			m_view->addEntry(ViewLevel::info, std::format("Total stopped tests count:   {}", stoppedTestsCount, totalTestsCount));
			m_view->unindent();
			m_view->addEntry(ViewLevel::info, std::format("___________________________________________________________"));
			m_view->addEntry(ViewLevel::info, std::format(""));
			m_view->print();

			for (TestClassInterface* cls : m_testClasses) {
				if (cls->countTestFailed() > 0) {
					forEachFailedTestOf(cls);
				}
			}
		}

		virtual void forEachFailedTestOf(TestClassInterface* cls) {
			m_view->addEntry(ViewLevel::info, std::format("Class \"{}\" has {} failed tests:", cls->name(), cls->countTestFailed()));
			m_view->indent();
			for (auto test : cls->getAllTests()) {
				if (test->getState() == TestState::Failed || test->getState() == TestState::Crashed) {
					m_view->addEntry(ViewLevel::info, std::format("Test \"{}\":\t", test->name()));
					m_view->indent();
					const std::string errorMessage = test->errorMessage();
					m_view->addEntry(ViewLevel::info, errorMessage, true, m_errorLinesToPrint);
					const std::string::difference_type allLines = std::count(errorMessage.begin(), errorMessage.end(), '\n');
					const auto skippedLines = allLines - m_errorLinesToPrint;
					if (m_errorLinesToPrint && skippedLines > 0) {
						m_view->addEntry(ViewLevel::info, std::format("... {} lines skipped ...", skippedLines));
					}
					m_view->unindent();
				}
			}
			m_view->unindent();
			m_view->print();
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
			m_registeredTestClasses++;
		}

		template<template <typename> typename TemplatedTestClass, typename ClassType, typename... ClassTypes>
		void registerMultipleClasses() {
			registerClass<TemplatedTestClass<ClassType>>();
			if constexpr (sizeof...(ClassTypes)) {
				registerMultipleClasses<TemplatedTestClass, ClassTypes...>();
			}
		}

		template<template <auto> typename ArgumentedTestClass, auto arg, auto... args>
		void registerArgumentedClass() {
			registerClass<ArgumentedTestClass<arg>>();
			if constexpr (sizeof...(args)) {
				registerArgumentedClass<ArgumentedTestClass, args...>();
			}
		}

		template<template <typename, auto> typename ArgumentedTemplatedTestClass, typename ClassType, auto arg, auto... args>
		void registerArgumentedClass() {
			registerClass<ArgumentedTemplatedTestClass<ClassType, arg>>();
			if constexpr (sizeof...(args)) {
				registerArgumentedClass<ArgumentedTemplatedTestClass, ClassType, args...>();
			}
		}

	};
}
