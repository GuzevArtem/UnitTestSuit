module;

#include <exception>
#include <string>
#include <iostream>

export module TestException;

export namespace Testing {
	export class TestException : std::exception {
		typedef std::exception inherited;
	public:

		[[nodiscard]] virtual std::string reason() const {
			return {};
		}

		virtual void print() const {
			std::cerr << reason() << '\n';
		}
	};
}
