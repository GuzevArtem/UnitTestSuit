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

	template<typename T, typename Return, typename ...Args>
	concept FunctorType = requires (T f, Args... args) {
		{ f(args...) } -> std::convertible_to<Return>;
	};

	export
	class UnitTestInterface {
	public:
		virtual void run() = 0;

		virtual TestState getState() const = 0;
	};

	template<TestContextType ContextType, FunctorType<void, ContextType&> functor>
		class UnitTestBase : public UnitTestInterface {
	private:
		char const* m_name;
		functor m_testFunction;
		ContextType* context;
	public:
		constexpr UnitTestBase(char const* name, functor func) : context(new ContextType()), m_name(name), m_testFunction(func) {}
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

	export
		template<FunctorType<void, TestContext&> functor = std::function<void(TestContext&)>>
	class UnitTest : public UnitTestBase<TestContext, functor> {
	public:
		constexpr UnitTest(char const* name, functor func) : UnitTestBase<TestContext, functor>(name, func) {}
	};

	export
		template<typename Type, FunctorType<void, TestContextTyped<Type>&> functor = std::function<void(TestContextTyped<Type>&)>>
	class UnitTestTyped : public UnitTestBase<TestContextTyped<Type>, functor> {
	public:
		constexpr UnitTestTyped(char const* name, functor func) : UnitTestBase<TestContextTyped<Type>, functor>(name, func) {}
	};

	template<TestContextType ContextType, FunctorType<void, ContextType&> functor = std::function<void(ContextType&)>>
	struct runBenchmarked {
		functor func;

		inline constexpr void operator()(ContextType& context) const {
			Benchmark::Result result = Benchmark::function(func, context);
			std::cout << std::format("\tTest run time = [{:#t}]\t", result);
		}
	};

	export
		template<FunctorType<void, TestContext&> functor = std::function<void(TestContext&)>>
	class BenchmarkUnitTest : public UnitTest<functor> {
	public:
		constexpr BenchmarkUnitTest(char const* name, functor func) : UnitTest(name, runBenchmarked<TestContext, functor>(func)) {}
	};

	export
		template<typename Type, FunctorType<void, TestContextTyped<Type>&> functor = std::function<void(TestContextTyped<Type>&)>>
	class BenchmarkUnitTestTyped : public UnitTestTyped<Type, functor> {
	public:
		constexpr BenchmarkUnitTestTyped(char const* name, functor func) : UnitTestTyped<Type>(name, runBenchmarked<TestContextTyped<Type>, functor>(func)) {}
	};
}
