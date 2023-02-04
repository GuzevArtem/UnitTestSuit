module;

#include <exception>
#include <vector>
#include <array>
#include <any>

export module TestContext;

import TestException;

export namespace Testing {

	export enum TestState {
		NeverStarted = 0,
		Started,
		Suceeded,
		Failed,
		Crashed,
		Stopped,
		Ignored
	};

	export class TestContext {
	private:
		TestState m_state;
		std::vector<std::any> m_controlledObjects;

	public:
		[[nodiscard]] constexpr TestState state() const { return m_state; };
		constexpr void state(TestState state) { m_state = state; };

	public:
		template<typename T>
		constexpr T createTestObject() { return {}; }

		template<typename T, class... Args >
		constexpr T* createTestObjectPointed(Args&&... args) {
			T* obj = &m_controlledObjects.emplace_back(std::make_any<T>(std::forward<Args>(args)...));
			return obj;
		}

		template<typename T, size_t Count>
		constexpr std::array<T, Count> createTestObjectsArray() { return {}; }

	public:

		virtual void reset() {
			m_state = NeverStarted;
		}

		virtual void start() {
			m_state = Started;
		}

		virtual void stop() {
			m_state = Stopped;
		}

		virtual void finish() {
			m_state = Suceeded;
		}

		virtual void ignore() {
			m_state = Ignored;
		}

		virtual void processException(const TestException& e) {
			m_state = Failed;
		}

		virtual void processException(const std::exception& e) {
			m_state = Crashed;
		}

		virtual void terminate(const std::exception& e) {
			m_state = Crashed;
		}
	};

	export
		template<typename Type>
	class TestContextTyped : public TestContext {
	public:
		constexpr Type createTestObject() { return {}; }

		template<size_t Count>
		constexpr std::array<Type, Count> createTestObjectsArray() { return {}; }

		template<class... Args >
		constexpr Type* createTestObjectPointed(Args&&... args) {
			Type* obj = &m_controlledObjects.emplace_back(std::make_any<Type>(std::forward<Args>(args)...));
			return obj;
		}
	};
}
