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

	public:
		TestException(const std::string& what) : inherited(what.c_str()) {}

		[[nodiscard]] virtual std::string reason() const {
			return std::string(inherited::what());
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
		std::string m_message;
	public:
		IgnoredException(const std::string& message = {}) : m_message(message), m_caused() {}
		IgnoredException(const std::vector<BaseException>& caused, const std::string& message = {}) : m_message(message), m_caused(caused) {}

		[[nodiscard]] virtual std::string reason() const {
			std::string result = std::format("\n{}{}{}IgnoredException{}", m_message.size() ? "Message:\t" : "" , m_message, m_message.size() ? "\n" : "", m_caused.empty() ? ": Execution skipped!" : ":");
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
		TestStopException(char const* what) : inherited(what) {}
		TestStopException(std::string message) : inherited(message) {}

		[[nodiscard]] virtual std::string reason() const {
			const char* message = inherited::what();
			if (message) {
				return std::format("TestStopException: {}", inherited::what());
			} else {
				return std::string("TestStopException");
			}
		}

		[[nodiscard]] virtual std::stacktrace where() const {
			return std::stacktrace::current();
		}
	};

	export
	struct Test {
		static void ignore(std::string reason = {}) noexcept(false) {
			throw IgnoredException(reason);
		}

		static void stop() noexcept(false) {
			throw TestStopException();
		}

		static void stop(const char* reason) noexcept(false) {
			throw TestStopException(reason);
		}

		static void stop(std::string reason) noexcept(false) {
			throw TestStopException(reason);
		}
	};
}
