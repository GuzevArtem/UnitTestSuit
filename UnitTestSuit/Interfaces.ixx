module;

#include <string>
#include <vector>
#include <exception>

export module Testing:Interfaces;

import :TestException;

export namespace Testing {

	// TestView
	enum ViewLevel : uint8_t {
		invalid = 0,
		trace,
		info,
		warning,
		error,
	};

	export
		class TestViewInterface {
		public:
			virtual void indent() = 0;
			virtual void indent(size_t count) = 0;
			virtual void unindent() = 0;
			virtual void unindent(size_t count) = 0;

			virtual void append(const ViewLevel level, const std::string& appendix, const bool multiline = false, size_t maxLines = static_cast<size_t>(-1)) = 0;
			virtual void append(const ViewLevel level, std::string&& appendix, const bool multiline = false, size_t maxLines = static_cast<size_t>(-1)) = 0;
			virtual void addEntry(const ViewLevel level, const std::string& entry, const bool multiline = false, size_t maxLines = static_cast<size_t>(- 1)) = 0;
			virtual void addEntry(const ViewLevel level, std::string&& entry, const bool multiline = false, size_t maxLines = static_cast<size_t>(-1)) = 0;

			virtual void print() = 0;

			virtual void clear() = 0;

			virtual TestViewInterface* parent() const = 0;
			virtual void parent(TestViewInterface* parent) = 0;
			virtual void removeSelf(const TestViewInterface* child) = 0;
			virtual void registerSelf(const TestViewInterface* child) = 0;
			virtual TestViewInterface* clone(TestViewInterface* target = nullptr) const = 0;
	};

	// TestContext
	export enum TestState : uint8_t {
		NeverStarted = 0,
		Started,
		Suceeded,
		Failed,
		Crashed,
		Stopped,
		Ignored
	};

	export
		class TestContextInterface {
	public:
		[[nodiscard]] virtual constexpr TestState state() const = 0;
		virtual constexpr void state(TestState state) = 0;

	public:
		virtual void reset() = 0;
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void finish() = 0;
		virtual void ignore() = 0;
		virtual void processException(const TestException& e) = 0;
		virtual void processException(const std::exception& e) = 0;
		virtual void terminate(const std::exception& e) = 0;

	public:
		virtual void log(std::string&& data, bool immidiate = true) const = 0;
		virtual void log(const std::string& data, bool immidiate = true) const = 0;
	};

	// UnitTest

	export
		class UnitTestInterface {
	public:
		virtual void run() = 0;

		virtual TestState getState() const = 0;

		constexpr virtual char const* name() const = 0;

		virtual std::string errorMessage() const = 0;

		virtual TestViewInterface* view() const = 0;
	};

	// TestClass

	export
		class TestClassInterface {
	public:
		constexpr virtual char const* name() const = 0;

		virtual std::vector<UnitTestInterface*>& getAllTests() = 0;

		constexpr virtual void setUp() {};
		constexpr virtual void tearDown() {};

		virtual void setView(TestViewInterface* view) = 0;
		virtual TestViewInterface* view() const = 0;

		virtual void run() = 0;
		virtual void registerTestMethods() = 0;

		virtual void onTestStart(const UnitTestInterface* test, TestContextInterface* ctx) = 0;
		virtual void onTestComplete(const UnitTestInterface* test, TestContextInterface* ctx) = 0;
		virtual void onTestComplete(const UnitTestInterface* test, TestContextInterface* ctx, const IgnoredException& e) = 0;
		virtual void onTestStop(const UnitTestInterface* test, TestContextInterface* ctx, const TestStopException& e) = 0;
		virtual void onTestIgnore(const UnitTestInterface* test, TestContextInterface* ctx, const IgnoredException& e) = 0;
		virtual void onTestFail(const UnitTestInterface* test, TestContextInterface* ctx, const TestException& e) = 0;
		virtual void onTestFail(const UnitTestInterface* test, TestContextInterface* ctx, const std::exception& e) = 0;

		virtual void beforeTest() = 0;
		virtual void afterTest() = 0;
		virtual void beforeAllTests() = 0;
		virtual void afterAllTests() = 0;

		virtual void printSummary() const = 0;

		virtual size_t countTestFailed() const = 0;

		constexpr TestClassInterface* self() { return this; };
		constexpr const TestClassInterface* self() const { return this; };
	};
}

export namespace std {
	[[nodiscard]]
	constexpr string to_string(const Testing::ViewLevel& value) {
		switch (value) {
			case Testing::ViewLevel::invalid:
				return string("invalid");
			case Testing::ViewLevel::trace:
				return string("trace");
			case Testing::ViewLevel::info:
				return string("info");
			case Testing::ViewLevel::warning:
				return string("warning");
			case Testing::ViewLevel::error:
				return string("error");
			default:
				return string("undefined");
		}
	}
}
