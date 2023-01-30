module;

#include <iostream>
#include <format>
#include <functional>
#include <concepts>

export module UnitTest;

import TestContext;
import TestException;
import Benchmark;
import Assert;

export namespace Testing {
	template<typename T>
	concept TestContextType = std::is_base_of_v<TestContext, T> || std::same_as<T, TestContext>;

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
		ContextType* context;
	public:
		UnitTestBase(char const* name, std::function<void(ContextType&)>& func) : context(new ContextType()), m_name(name), m_testFunction(func) {}
		virtual ~UnitTestBase() {
			delete context;
		}
	public:
		virtual void run() override {
			TestContext* ctx = static_cast<TestContext*>(context);
			ctx->reset();
			try {
				std::cout << std::format("\tStarting test: [{}]\t", m_name);
				ctx->start();
				m_testFunction(*context);
				ctx->finish();
				std::cout << std::format("COMPLETED\n", m_name);
			} catch (const TestStopException& e) {
				ctx->stop();
				std::cerr << std::format("STOPPED:\t{}\n", e.reason());
			} catch (const IgnoredException& e) {
				if (e.hasAny()) {
					ctx->finish();
					std::cerr << std::format("COMPLETED\t{}\n", e.reason());
				} else {
					ctx->ignore();
					std::cerr << std::format("IGNORED:\t{}\n", e.reason());
				}
			} catch (const TestException& e) {
				ctx->processException(e);
				std::cerr << std::format("FAIL:\t{}\n", e.reason());
			} catch (const AssertException& e) {
				ctx->processException(TestException(e.reason()));
				std::cerr << std::format("FAIL:\t{}\n", e.reason());
			} catch (const UnexpectedException& e) {
				ctx->processException(TestException(e.reason()));
				std::cerr << std::format("FAIL:\t{}\n", e.reason());
			} catch (const std::exception& e) {
				ctx->terminate(e);
				std::cerr << std::format("FAIL:\t{}\n", e.what());
			}
		}

		virtual TestState getState() const override { return static_cast<TestContext*>(context)->state(); }
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

	template<TestContextType ContextType>
	struct runBenchmarked {
		std::function<void(ContextType&)> func;

		inline void operator()(ContextType& context) const {
			Benchmark::Result result = Benchmark::function(func, context);
			std::cout << std::format("\tTest run time = [{:#t}]\t", result);
		}
	};

	export class BenchmarkUnitTest : public UnitTest {
	public:
		BenchmarkUnitTest(char const* name, std::function<void(TestContext&)> func) : UnitTest(name, runBenchmarked<TestContext>(func)) {}
	};

	export
		template<typename Type>
	class BenchmarkUnitTestTyped : public UnitTestTyped<Type> {
	public:
		BenchmarkUnitTestTyped(char const* name, std::function<void(TestContextTyped<Type>&)> func) : UnitTestTyped<Type>(name, runBenchmarked<TestContextTyped<Type>>(func)) {}
	};
}

static_assert(sizeof(Testing::UnitTest) == sizeof(Testing::UnitTestTyped<uint32_t>));
