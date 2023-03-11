module;

#include <format>
#include <vector>
#include <exception>
#include <string>
#include <stacktrace>

export module Testing:TestException;

export namespace Testing {

	export size_t InitialStackTraceDepth = 8;

	export class BaseException : public std::exception {
		typedef std::exception inherited;

	public:
		BaseException() : inherited() {}
		BaseException(char const* what) : inherited(what) {}
		BaseException(const std::string& caused) : inherited(caused.c_str()) {}
		BaseException(const std::exception& caused) : inherited(caused) {}

		[[nodiscard]] virtual std::string reason() const {
			return std::format("{}: {}", inherited::what(), std::to_string(where<2>()));
		};

		template<size_t t_StackToSkip = 0>
		[[nodiscard]] std::stacktrace where() const {
			return std::stacktrace::current(t_StackToSkip);
		}
	};

	export class TestException : public BaseException {
		typedef BaseException inherited;
	private:
		std::string m_errorMessage;

	public:
		TestException(const std::string& what) : inherited(what.c_str()), m_errorMessage(what) {}

		[[nodiscard]] virtual std::string reason() const {
			return m_errorMessage;
		}

		[[nodiscard]] virtual char const* what() const override {
			return m_errorMessage.c_str();
		}
	};

	export class UnexpectedException : public BaseException {
		typedef BaseException inherited;
	public:
		UnexpectedException(const BaseException& e) : inherited(e) {};

		[[nodiscard]] virtual std::string reason() const {
			return std::format("UnexpectedException: caused by {}", inherited::what());
		}

		[[nodiscard]] virtual std::stacktrace where() const {
			return std::stacktrace::current();
		}
	};

	export class IgnoredException : public BaseException {
		typedef BaseException inherited;
	private:
		std::vector<BaseException> m_caused;

	public:
		IgnoredException() : m_caused() {}
		IgnoredException(const std::vector<BaseException>& caused) : m_caused(caused) {}

		[[nodiscard]] virtual std::string reason() const {
			std::string result = std::format("\nIgnoredException{}", m_caused.empty() ? ": Execution skipped!" : ":");
			for (BaseException e : m_caused) {
				result.append(std::format("\nCaused by: {}", e.reason()));
			}
			return result;
		}

		[[nodiscard]] bool hasAny() const {
			return !m_caused.empty();
		}

		[[nodiscard]] virtual std::stacktrace where() const {
			return std::stacktrace::current();
		}
	};

	export class TestStopException : public BaseException {
		typedef BaseException inherited;
	private:

	public:
		TestStopException() : inherited("Test execution was stopped by user.") {}

		[[nodiscard]] virtual std::string reason() const {
			return std::format("TestStopException: {}", inherited::what());
		}

		[[nodiscard]] virtual std::stacktrace where() const {
			return std::stacktrace::current();
		}
	};

	export
	struct Test {
		static void ignore() noexcept(false) {
			throw IgnoredException();
		}

		static void stop() noexcept(false) {
			throw TestStopException();
		}
	};
}
