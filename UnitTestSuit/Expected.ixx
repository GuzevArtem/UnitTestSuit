module;

#include <exception>
#include <string>
#include <format>

export module Expected;

import TestException;
import TypeParse;

export namespace Testing {

	export
	template<typename ExceptionType>
	class ExpectedFailedException : public TestException {

	private:
		std::exception* m_caused;

	public:
		[[nodiscard]] virtual std::string reason() const override {
			return m_caused ? std::format("ExpectedFailedException: expeted \"{}\", but raised {}", TypeParse<ExceptionType>::name, m_caused->what()) : std::format("ExpectedFailedException: ");
		}
	};

	export
	template<typename ExceptionType>
	class Expected {
	public:
		template<typename Function>
		void during(Function& f) {
			try {
				f();
			} catch (ExceptionType e) {
				return;
			} catch (std::exception e) {
				throw new ExpectedFailedException<ExceptionType>(e);
			}
			throw new ExpectedFailedException<ExceptionType>();
		}
	};
}
