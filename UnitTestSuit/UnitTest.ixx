module;

#include <iostream>
#include <format>
#include <functional>
#include <concepts>

export module UnitTest;

import TestContext;
import TestException;

export namespace Testing {
	template<typename T>
	concept TestContextType = std::derived_from<T, TestContext> || std::same_as<T, TestContext>;

	export
		class UnitTestInterface {
		public:
			virtual void run() = 0;

			virtual TestState getState() const = 0;
	};

	template<TestContextType ContextType>
	class UnitTestBase : public UnitTestInterface {
	private:
		char const* m_name;
		std::function<void(ContextType&)> m_testFunction;
		ContextType context;
	public:
		UnitTestBase(char const* name, std::function<void(ContextType&)> func) : m_name(name), m_testFunction(func) {}

	public:
		virtual void run() override {
			context.reset();
			try {
				std::cout << std::format("\tStarting test: [{}]\t", m_name);
				context.start();
				m_testFunction(context);
				context.finish();
				std::cout << std::format("COMPLETED\n", m_name);
			} catch (const TestException& e) {
				context.processTestException(e);
				std::cerr << std::format("FAIL:\t{}\n", e.reason());
			} catch (const std::exception& e) {
				context.processGeneralException(e);
				std::cerr << std::format("FAIL:\t{}\n", e.what());
			} catch (...) {
				context.terminate();
				std::cerr << "FAIL:\tUnknown error\n";
			}
		}

		virtual TestState getState() const override { return context.state(); }
	};

	export class UnitTest : public UnitTestBase<TestContext> {
	public:
		UnitTest(char const* name, std::function<void(TestContext&)> func) : UnitTestBase<TestContext>(name, func) {}
	};

	export
		template<typename Type>
	class UnitTestTyped : public UnitTestBase<TestContextTyped<Type>> {
	public:
		UnitTestTyped(char const* name, std::function<void(TestContextTyped<Type>&)> func) : UnitTestBase<TestContextTyped<Type>>(name, func) {}
	};
}

static_assert(sizeof(Testing::UnitTest) == sizeof(Testing::UnitTestTyped<uint32_t>));
