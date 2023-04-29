module;

//#pragma warning( push )
//#pragma warning( disable : 4355 4365 4625 4626 4820 5202 5026 5027 5039 5220 )
//#include <string>
//
//#if !defined(_HAS_CXX23) || !_HAS_CXX23
//#pragma warning( push )
//#pragma warning( disable : 5202 ) // warning C5202: a global module fragment can only contain preprocessor directives
//namespace std {
//	class stacktrace {
//	public:
//		static stacktrace current(size_t framesToSkip = 0) {
//			return {};
//		}
//	};
//
//	std::string to_string(stacktrace trace) {
//		return "C++23 required for stacktrace!";
//	}
//}
//#pragma warning( pop )
//#else 
//#include <stacktrace>
//#endif
//
//#include <format>
//#include <vector>
//#include <exception>
//#pragma warning( pop )

export module Testing:TestException;

import std;

export namespace Testing {

	export class BaseException : public std::exception {
		typedef std::exception inherited;

	public:
		BaseException() : inherited() {}
		BaseException(char const* what) : inherited(what) {}
		BaseException(const std::string& caused) : inherited(caused.c_str()) {}
		BaseException(const std::exception& caused) : inherited(caused) {}

		[[nodiscard]] virtual std::string reason() const {
			const std::stacktrace trace = where<2>();
			return std::format("{}: {}", inherited::what(), std::to_string(trace));
		};

		template<size_t t_StackToSkip = 0>
		[[nodiscard]] std::stacktrace where() const noexcept {
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

		[[nodiscard]] virtual std::stacktrace where() const noexcept {
			return std::stacktrace::current();
		}
	};

	export class CollectedException : public BaseException {
		typedef BaseException inherited;
	private:
		std::vector<BaseException> m_caused;
		std::string m_message;
	public:
		CollectedException(const std::string& message = {}) : m_caused(), m_message(message) {}
		CollectedException(const std::vector<BaseException>& caused, const std::string& message = {}) : m_caused(caused), m_message(message) {}

		virtual const char* name() const { return "CollectedException"; }

		[[nodiscard]] virtual std::string reason() const {
			std::string result = std::format("\n{}{}{}{}{}",
											 m_message.size() ? "Message:\t" : "" ,
											 m_message,
											 m_message.size() ? "\n" : "",
											 name(),
											 m_caused.empty() ? ": Execution skipped!" : ":");
			for (BaseException e : m_caused) {
				result.append(std::format("\n\nCaused by: {}", e.reason()));
			}
			return result;
		}

		[[nodiscard]] bool hasAny() const noexcept {
			return !m_caused.empty();
		}

		[[nodiscard]] virtual std::stacktrace where() const noexcept {
			return std::stacktrace::current();
		}
	};

	export class IgnoredException : public CollectedException {
		typedef CollectedException inherited;

	public:
		IgnoredException(const std::string& message = {}) : CollectedException(message) {}
		IgnoredException(const std::vector<BaseException>& caused, const std::string& message = {}) : CollectedException(caused, message) {}

		virtual const char* name() const override { return "IgnoredException"; }
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

		[[nodiscard]] virtual std::stacktrace where() const noexcept {
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
