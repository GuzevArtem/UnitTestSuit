module;

#include <iostream>
#include <format>
#include <vector>
#include <concepts>

export module TestSuit;

import TestClass;

export namespace Testing {
	template<typename T>
	concept TestClassType = std::same_as<T, TestClassBase> || std::is_base_of_v<TestClassBase, T>;

	export class TestSuit {
	
	public:
		virtual ~TestSuit() {
			for (auto testClass : m_testClasses) {
				delete testClass;
			}
		}

	private:
		std::vector<TestClassBase*> m_testClasses;
	
	public:

		[[nodiscard]] static TestSuit* instance() {
			static TestSuit* ts = new TestSuit();
			return ts;
		}

	public:
		void run() {
			for (TestClassBase* cls : m_testClasses) {
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
		bool processTestException(const std::exception& e) {
			return false;
		}

		template<TestClassType T, typename... Args>
		static void registerClass(Args&&... args) {
			TestSuit::instance()->_registerClass<T>(std::forward<Args>(args)...);
		}

	protected:
		template<TestClassType T, typename... Args>
		void _registerClass(Args&&... args) {
			T* cls = new T(std::forward<Args>(args)...);
			m_testClasses.push_back(cls);
		}
	};
}