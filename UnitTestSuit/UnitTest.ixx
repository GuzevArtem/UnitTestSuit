export module Testing.UnitTest;

import std;

import Testing.Interfaces;
import Testing.TestContext;
import Testing.TestException;
import Testing.Benchmark;
import Testing.Assert;
import TypeParse;
import Testing.Utils;

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
		std::string m_name;
		functor m_testFunction;
		ContextType* m_context;
		TestClassInterface* m_parent;
		std::string m_errorMessage;
	private:
		
	public:
		constexpr UnitTestBase(const std::string& name, TestClassInterface* parent, functor func) : m_name(name), m_parent(parent), m_testFunction(func) {
			m_context = new ContextType(this);
		}
		constexpr UnitTestBase(std::string&& name, TestClassInterface* parent, functor func) : m_name(name), m_parent(parent), m_testFunction(func) {
			m_context = new ContextType(this);
		}
		constexpr UnitTestBase(char const* name, TestClassInterface* parent, functor func) : UnitTestBase(std::string(name), parent, func) {}
		constexpr UnitTestBase(const std::string_view& name, TestClassInterface* parent, functor func) : UnitTestBase(std::string(name), parent, func) {}
		constexpr UnitTestBase(std::string_view&& name, TestClassInterface* parent, functor func) : UnitTestBase(std::string(name), parent, func) {}
		constexpr UnitTestBase(const utils::string_static& name, TestClassInterface* parent, functor func) : UnitTestBase(std::to_string(name), parent, func) {}
		constexpr UnitTestBase(utils::string_static&& name, TestClassInterface* parent, functor func) : UnitTestBase(std::to_string(name), parent, func) {}

		virtual ~UnitTestBase() {
			delete m_context;
		}

	public:
		constexpr virtual char const* name() const override { return m_name.c_str(); }

		virtual TestViewInterface* view() const override { return m_parent ? m_parent->view() : nullptr; }

		virtual void run() override {
			TestContext* ctx = static_cast<TestContext*>(m_context);
			ctx->reset();
			try {
				m_parent->onTestStart(this, m_context);
				ctx->start();
				m_testFunction(*m_context);
				ctx->finish();
				m_parent->onTestComplete(this, m_context);
			} catch (const TestStopException& e) {
				ctx->stop();
				m_parent->onTestStop(this, m_context, e);
				m_errorMessage = e.reason();
			} catch (const IgnoredException& e) {
				if (e.hasAny()) {
					ctx->finish();
					m_parent->onTestComplete(this, m_context, e);
				} else {
					ctx->ignore();
					m_parent->onTestIgnore(this, m_context, e);
					m_errorMessage = e.reason();
				}
			} catch (const TestException& e) {
				ctx->processException(e);
				m_parent->onTestFail(this, m_context, e);
				m_errorMessage = e.reason();
			} catch (const AssertException& e) {
				const TestException& te = TestException(e.reason());
				ctx->processException(te);
				m_parent->onTestFail(this, m_context, te);
				m_errorMessage = te.reason();
			} catch (const UnexpectedException& e) {
				const TestException& te = TestException(e.reason());
				ctx->processException(te);
				m_parent->onTestFail(this, m_context, te);
				m_errorMessage = te.reason();
			} catch (const std::exception& e) {
				ctx->terminate(e);
				m_parent->onTestFail(this, m_context, e);
				m_errorMessage = std::string(e.what());
			}
		}

		virtual TestState getState() const override { return static_cast<TestContext*>(m_context)->state(); }

		virtual std::string errorMessage() const override { return m_errorMessage;  };
	};

	export
		template<FunctorType<void, TestContext&> functor = std::function<void(TestContext&)>>
	class UnitTest : public UnitTestBase<TestContext, functor> {
	public:
		constexpr UnitTest(char const* name, TestClassInterface* parent, functor func) : UnitTestBase<TestContext, functor>(name, parent, func) {}
		constexpr UnitTest(const std::string_view & name, TestClassInterface* parent, functor func) : UnitTestBase<TestContext, functor>(name, parent, func) {}
		constexpr UnitTest(std::string_view&& name, TestClassInterface* parent, functor func) : UnitTestBase<TestContext, functor>(name, parent, func) {}
		constexpr UnitTest(const std::string& name, TestClassInterface* parent, functor func) : UnitTestBase<TestContext, functor>(name, parent, func) {}
		constexpr UnitTest(std::string&& name, TestClassInterface* parent, functor func) : UnitTestBase<TestContext, functor>(name, parent, func) {}
		constexpr UnitTest(const utils::string_static& name, TestClassInterface* parent, functor func) : UnitTestBase<TestContext, functor>(name, parent, func) {}
		constexpr UnitTest(utils::string_static&& name, TestClassInterface* parent, functor func) : UnitTestBase<TestContext, functor>(name, parent, func) {}

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
		constexpr UnitTestTyped(char const* name, TestClassInterface* parent, functor func) : UnitTestBase<TestContextTyped<Type>, functor>(std::move(utils::concat(name, '<', helper::TypeParse<Type>::name, '>')), parent, func) {}

		template<typename Function>
		static void instance(std::vector<UnitTestInterface*>& result, char const* name, TestClassInterface* parent, Function func) {
			result.emplace_back(new UnitTestTyped<Type>(name, parent, func));
		}
	};

	template<TestContextType ContextType, FunctorType<void, ContextType&> functor = std::function<void(ContextType&)>>
	struct runBenchmarked {
		functor m_func;
		TestViewInterface* m_view;
		size_t m_iterations;

		inline runBenchmarked(functor func, TestViewInterface* view, size_t iterations = 1) : m_func(func), m_view(view), m_iterations(iterations) {}

		inline constexpr void operator()(ContextType& context) const {
			auto result = Benchmark::function(m_iterations, m_func, context);
			if (m_view) {
				if (m_iterations == 1) {
					m_view->addEntry(ViewLevel::info, std::format("\tTest run time = [{:#t}]\t", result), true);
				} else {
					m_view->addEntry(ViewLevel::info, std::format("\tTest run = [{}]\n", result), true);
				}
			}
		}
	};

	export
		template<FunctorType<void, TestContext&> functor = std::function<void(TestContext&)>>
	class BenchmarkUnitTest : public UnitTest<functor> {
	public:
		constexpr BenchmarkUnitTest(char const* name, TestClassInterface* parent, functor func, size_t iterations = 1) : UnitTest<>(name, parent, runBenchmarked<TestContext, functor>(func, parent ? parent->view() : nullptr, iterations)) {}

		template<typename Function>
		static void instance(std::vector<UnitTestInterface*>& result, char const* name, TestClassInterface* parent, Function func, size_t iterations = 1) {
			result.emplace_back(new BenchmarkUnitTest(name, parent, func, iterations));
		}
	};

	export
		template<typename Type, FunctorType<void, TestContextTyped<Type>&> functor = std::function<void(TestContextTyped<Type>&)>>
	class BenchmarkUnitTestTyped : public UnitTestTyped<Type, functor> {
	public:
		constexpr BenchmarkUnitTestTyped(char const* name, TestClassInterface* parent, functor func, size_t iterations = 1) : UnitTestTyped<Type>(name, parent, runBenchmarked<TestContextTyped<Type>, functor>(func, parent ? parent->view() : nullptr, iterations)) {}

		template<typename Function>
		static void instance(std::vector<UnitTestInterface*>& result, char const* name, TestClassInterface* parent, Function func, size_t iterations = 1) {
			result.emplace_back(new BenchmarkUnitTestTyped<Type>(name, parent, func, iterations));
		}
	};
}
