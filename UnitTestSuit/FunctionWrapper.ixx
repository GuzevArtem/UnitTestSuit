module;

#include <utility>
#include <concepts>

export module Utils:FunctionWrapper;

export namespace utils {

	export
		template<typename Return, typename ...Args>
	class FunctionStaticWrapper {
	private:
		typedef FunctionStaticWrapper<Return, Args...> Self;
		Return(*m_closure)(Args...);

	public:
		template<typename X>
		inline FunctionStaticWrapper(Return(X::* func)(Args...)) : m_closure(func) {}
		inline FunctionStaticWrapper(Return(*func)(Args...)) : m_closure(func) {}

		inline FunctionStaticWrapper(const Self&) = default;
		inline FunctionStaticWrapper(Self&&) = default;

		Self& operator= (const Self&) = default;
		Self& operator= (Self&&) = default;

	public:
		Return operator() (Args... args) const {
			(*m_closure)(std::forward<Args>(args)...);
		}
	};

	export
		template<typename ThisT, typename Return, typename ...Args>
	class FunctionWrapper {
	private:
		typedef FunctionWrapper<ThisT, Return, Args...> Self;

		mutable ThisT* m_this;
		Return(ThisT::* m_closure)(Args...);

	public:
		inline FunctionWrapper(ThisT* caller, Return(ThisT::* func)(Args...)) {
			m_this = caller;
			m_closure = func;
		}

		inline FunctionWrapper(const ThisT* caller, Return(ThisT::* func)(Args...)) : FunctionWrapper(const_cast<ThisT*>(caller), func) {}

		inline FunctionWrapper(const Self&) = default;
		inline FunctionWrapper(Self&&) = default;

		Self& operator= (const Self&) = default;
		Self& operator= (Self&&) = default;

	public:
		Return operator() (Args... args) const {
			if constexpr (std::same_as<Return, void>) {
				(m_this->*m_closure)(std::forward<Args>(args)...);
			} else {
				return (m_this->*m_closure)(std::forward<Args>(args)...);
			}
		}
	};
}
