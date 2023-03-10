module;

#include <format>
#include <functional>
#include <concepts>

export module Testing:UnitTest;

import :Interfaces;
import :TestContext;
import :TestException;
import :Benchmark;
import :Assert;
import Helpers;
import Utils;

export namespace Testing {
	template<typename T>
	concept TestContextType = std::is_base_of_v<TestContext, T> || std::same_as<T, TestContext>;

	template<typename T, typename Return, typename ...Args>
	concept FunctorType = requires (T f, Args... args) {
		{ f(args...) } -> std::convertible_to<Return>;
	};

	template<TestContextType ContextType, FunctorType<void, ContextType&> functor>
		class UnitTestBase : public UnitTestInterface {
	private:
		char const* m_name;
		functor m_testFunction;
		ContextType* context;
		TestClassInterface* m_parent;

	public:
		constexpr UnitTestBase(char const* name, TestClassInterface* parent, functor func) : m_name(name), m_parent(parent), m_testFunction(func) {
			context = new ContextType(this);
		}

		virtual ~UnitTestBase() {
			delete context;
		}

	public:
		constexpr virtual char const* name() const override { return m_name; }

		virtual TestViewInterface* view() const override { return m_parent ? m_parent->view() : nullptr; }

		virtual void run() override {
			TestContext* ctx = static_cast<TestContext*>(context);
			ctx->reset();
			try {
				m_parent->onTestStart(this, context);
				ctx->start();
				m_testFunction(*context);
				ctx->finish();
				m_parent->onTestComplete(this, context);
			} catch (const TestStopException& e) {
				ctx->stop();
				m_parent->onTestStop(this, context, e);
			} catch (const IgnoredException& e) {
				if (e.hasAny()) {
					ctx->finish();
					m_parent->onTestComplete(this, context, e);
				} else {
					ctx->ignore();
					m_parent->onTestIgnore(this, context, e);
				}
			} catch (const TestException& e) {
				ctx->processException(e);
				m_parent->onTestFail(this, context, e);
			} catch (const AssertException& e) {
				const TestException& te = TestException(e.reason());
				ctx->processException(te);
				m_parent->onTestFail(this, context, te);
			} catch (const UnexpectedException& e) {
				const TestException& te = TestException(e.reason());
				ctx->processException(te);
				m_parent->onTestFail(this, context, te);
			} catch (const std::exception& e) {
				ctx->terminate(e);
				m_parent->onTestFail(this, context, e);
			}
		}

		virtual TestState getState() const override { return static_cast<TestContext*>(context)->state(); }
	};

	export
		template<FunctorType<void, TestContext&> functor = std::function<void(TestContext&)>>
	class UnitTest : public UnitTestBase<TestContext, functor> {
	public:
		constexpr UnitTest(char const* name, TestClassInterface* parent, functor func) : UnitTestBase<TestContext, functor>(name, parent, func) {}

		template<typename Function>
		static void instance(std::vector<UnitTestInterface*>& result, char const* name, TestClassInterface* parent, Function func) {
			result.emplace_back(new UnitTest(name, parent, func));
		}
	};

	export
		template<typename Type, FunctorType<void, TestContextTyped<Type>&> functor = std::function<void(TestContextTyped<Type>&)>>
	class UnitTestTyped : public UnitTestBase<TestContextTyped<Type>, functor> {
		typedef UnitTestBase<TestContextTyped<Type>, functor> inherited;
	public:
		constexpr UnitTestTyped(char const* name, TestClassInterface* parent, functor func) : UnitTestBase<TestContextTyped<Type>, functor>(name, parent, func) {}

		constexpr virtual char const* name() const override {
			char const* name_ptr = inherited::name();
			char const* type_name_ptr = helper::TypeParse<Type>::name;
			size_t name_length = utils::length(name_ptr);
			size_t type_name_length = utils::length(type_name_ptr);
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

		template<typename Function>
		static void instance(std::vector<UnitTestInterface*>& result, char const* name, TestClassInterface* parent, Function func) {
			result.emplace_back(new UnitTestTyped<Type>(name, parent, func));
		}
	};

	template<TestContextType ContextType, FunctorType<void, ContextType&> functor = std::function<void(ContextType&)>>
	struct runBenchmarked {
		functor func;
		TestViewInterface* view;

		inline runBenchmarked(functor _func, TestViewInterface* view) : func(_func), view(view) {}

		inline constexpr void operator()(ContextType& context) const {
			Benchmark::Result result = Benchmark::function(func, context);
			if (view) {
				view->addEntry(TestViewInterface::info, std::format("\tTest run time = [{:#t}]\t", result), true);
			}
		}
	};

	export
		template<FunctorType<void, TestContext&> functor = std::function<void(TestContext&)>>
	class BenchmarkUnitTest : public UnitTest<functor> {
	public:
		constexpr BenchmarkUnitTest(char const* name, TestClassInterface* parent, functor func) : UnitTest(name, parent, runBenchmarked<TestContext, functor>(func, parent ? parent->view() : nullptr)) {}
	};

	export
		template<typename Type, FunctorType<void, TestContextTyped<Type>&> functor = std::function<void(TestContextTyped<Type>&)>>
	class BenchmarkUnitTestTyped : public UnitTestTyped<Type, functor> {
	public:
		constexpr BenchmarkUnitTestTyped(char const* name, TestClassInterface* parent, functor func) : UnitTestTyped<Type>(name, parent, runBenchmarked<TestContextTyped<Type>, functor>(func, parent ? parent->view() : nullptr)) {}
	};
}
