module;

#include <format>
#include <vector>
#include <exception>
#include <string>
#include <unordered_map>

export module TestException;

namespace Testing {
	class TestException;
}

template<> struct std::hash<Testing::TestException*> {
	std::size_t operator()(Testing::TestException* const& te) const noexcept {
		return static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(te));
	}
};

export namespace Testing {

	export class TestException : std::exception {
		typedef std::exception inherited;
	private:
		std::string m_errorMessage;

	public:
		TestException(const std::string& what) : inherited(), m_errorMessage(what) {}

		[[nodiscard]] virtual std::string reason() const {
			return m_errorMessage;
		}

		[[nodiscard]] virtual char const* what() const override {
			return m_errorMessage.c_str();
		}
	};

	export class UnexpectedException : public std::exception {
		typedef std::exception inherited;
	public:
		UnexpectedException(const std::exception& e) : inherited(e) {};

		[[nodiscard]] virtual std::string reason() const {
			return std::format("UnexpectedException: caused by {}", inherited::what());
		}
	};

	export class IgnoredException : public std::exception {
		typedef std::exception inherited;
	private:
		std::vector<TestException> m_caused;

	public:
		IgnoredException() : m_caused() {}
		IgnoredException(const std::vector<TestException>& caused) : m_caused(caused) {}

		[[nodiscard]] virtual std::string reason() const {
			std::string result = std::format("IgnoredException{}", m_caused.empty() ? ": Execution skipped!" : ":");
			for (TestException e : m_caused) {
				result.append(std::format("\n\tCaused by: {}", e.reason()));
			}
			return result;
		}
	};

	export class TestStopException : public std::exception {
		typedef std::exception inherited;
	private:

	public:
		TestStopException() : inherited("Test execuion was stopped by user.") {}

		[[nodiscard]] virtual std::string reason() const {
			return std::format("TestStopException: {}", inherited::what());
		}
	};
}
