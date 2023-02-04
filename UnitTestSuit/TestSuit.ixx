module;

#include <iostream>
#include <format>
#include <concepts>
#include <vector>

export module TestSuit;

import TestClass;

export namespace Testing {

	template<typename T>
	concept TestClassType = std::same_as<T, TestClassInterface> || std::is_base_of_v<TestClassInterface, T>;

	export
	class TestSuit {
	
	public:
		constexpr TestSuit() : m_registeredTests(0), m_testClasses() {}
		constexpr virtual ~TestSuit() {
			for (TestClassInterface* testClass : m_testClasses) {
				delete testClass;
			}
		}

	private:
		size_t m_registeredTests;
		std::vector<TestClassInterface*> m_testClasses;

	public:
		void run() {
			for (TestClassInterface* cls : m_testClasses) {
				try {
					std::cout << std::format("Starting test class: [{}]\n", cls->name());
					cls->setUp();
					cls->run();
					cls->tearDown();
					cls->printData();
				} catch (const std::exception& e) {
					if (!processTestException(e)) {
						std::cerr << std::format("Test execution for class {} failed.\n {}", cls->name(), e.what());
						break;
					}
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
			m_testClasses.emplace_back(static_cast<TestClassInterface*>(cls->self()));
			m_registeredTests++;
		}
	};
}
