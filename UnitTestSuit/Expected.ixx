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
	class ExpectedFailedException : public std::exception {
		typedef std::exception inherited;
	public:
		ExpectedFailedException(std::exception* caused = nullptr) : inherited(caused ? caused->what() : "No exception raised.") {}

		[[nodiscard]] virtual std::string reason() const override {
			return inherited::what()
				? std::format("ExpectedFailedException: expeted \"{}\", but raised {}.", static_cast<char const*>(TypeParse<ExceptionType>::name), inherited::what())
				: std::format("ExpectedFailedException: {}", inherited::what());
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
				return;	//we got what we want
			} catch (std::exception e) {
				throw new ExpectedFailedException<ExceptionType>(e);
			}
			throw new ExpectedFailedException<ExceptionType>();
		}
	};
}
