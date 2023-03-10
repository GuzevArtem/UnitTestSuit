module;

#include <string>
#include <vector>
#include <exception>

export module Testing:Interfaces;

import :TestException;
import :TestContext;

export namespace Testing {

	// TestView

	export
		class TestViewInterface {
		public:
			enum ViewLevel : uint8_t {
				invalid = 0,
				trace,
				info,
				warning,
				error,
			};

			virtual void indent() = 0;
			virtual void indent(size_t count) = 0;
			virtual void unindent() = 0;
			virtual void unindent(size_t count) = 0;

			virtual void append(const ViewLevel level, const std::string& appendix, const bool multiline = false) = 0;
			virtual void append(const ViewLevel level, std::string&& appendix, const bool multiline = false) = 0;
			virtual void addEntry(const ViewLevel level, const std::string& entry, const bool multiline = false) = 0;
			virtual void addEntry(const ViewLevel level, std::string&& entry, const bool multiline = false) = 0;

			virtual void print() = 0;

			virtual void clear() = 0;

			virtual TestViewInterface* parent() const = 0;
			virtual void parent(TestViewInterface* parent) = 0;
			virtual void removeSelf(const TestViewInterface* child) = 0;
			virtual void registerSelf(const TestViewInterface* child) = 0;
			virtual TestViewInterface* clone(TestViewInterface* target = nullptr) const = 0;
	};

	// UnitTest

	export
		class UnitTestInterface {
	public:
		virtual void run() = 0;

		virtual TestState getState() const = 0;

		constexpr virtual char const* name() const = 0;
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

		virtual void onTestStart(const UnitTestInterface* test, TestContext* ctx) = 0;
		virtual void onTestComplete(const UnitTestInterface* test, TestContext* ctx) = 0;
		virtual void onTestComplete(const UnitTestInterface* test, TestContext* ctx, const IgnoredException& e) = 0;
		virtual void onTestStop(const UnitTestInterface* test, TestContext* ctx, const TestStopException& e) = 0;
		virtual void onTestIgnore(const UnitTestInterface* test, TestContext* ctx, const IgnoredException& e) = 0;
		virtual void onTestFail(const UnitTestInterface* test, TestContext* ctx, const TestException& e) = 0;
		virtual void onTestFail(const UnitTestInterface* test, TestContext* ctx, const std::exception& e) = 0;

		virtual void beforeTest() = 0;
		virtual void afterTest() = 0;
		virtual void beforeAllTests() = 0;
		virtual void afterAllTests() = 0;

		constexpr TestClassInterface* self() { return this; };
		constexpr const TestClassInterface* self() const { return this; };
	};
}
