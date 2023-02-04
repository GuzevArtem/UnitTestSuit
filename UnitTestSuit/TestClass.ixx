module;

#include <format>
#include <iostream>
#include <string>
#include <vector>
#include <map>

export module TestClass;

import UnitTest;
import TypeParse;

/*export namespace Testing {
	class TestSuit;
}*/

export namespace Testing {

	export
	class TestClassInterface {
	public:
		constexpr virtual char const* name() const = 0;

		constexpr virtual void setUp() {};
		constexpr virtual void tearDown() {};

		virtual void run() = 0;

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

	template<typename Self/*, TestSuit* suit_t*/>
	class TestClassInner : public TestClassInterface {
	private:
		char const* m_name;
		TestData m_data;
		std::vector<UnitTestInterface*> m_unitTests;
	public:
		constexpr TestClassInner(char const* name, std::vector<UnitTestInterface*> tests) : m_name(name), m_unitTests(tests) {}

		constexpr virtual ~TestClassInner() {
			for (UnitTestInterface* test : m_unitTests) {
				delete test;
			}
		}
	public:
		constexpr virtual char const* name() const { return m_name; }

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
			std::cout << std::format("Class [{}] results:\n\tCompleted tests: {}/{}\n\tFailed tests: {}/{}\n\tIgnored tests: {}/{}\n", name(), m_data.succeededTestsCount, m_data.totalTestsCount, m_data.failedTestsCount, m_data.totalTestsCount, m_data.ignoredTestsCount, m_data.totalTestsCount);
		}
	};

	export
		template<typename Self/*, TestSuit* suit_t*/>
	class TestClass : public TestClassInner<Self/*, suit_t*/> {
		typedef TestClassInner<Self/*, suit_t*/> inherited;
	public:
		constexpr TestClass(char const* name, std::vector<UnitTestInterface*> tests) : inherited(name, tests) {}
	};

	export
		template<typename Self, typename Type/*, TestSuit* suit_t*/>
	class TestClassTyped : public TestClassInner<Self/*, suit_t*/> {
		typedef TestClassInner<Self/*, suit_t*/> inherited;
	public:
		constexpr TestClassTyped(char const* name, std::vector<UnitTestInterface*> tests) : inherited(name, tests) {}
	};
}
