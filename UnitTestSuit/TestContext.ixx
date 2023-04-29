module;

//#pragma warning( push )
//#pragma warning( disable : 4355 4365 4625 4626 4820 5202 5026 5027 5039 5220 )
//#include <string>
//#include <exception>
//#include <vector>
//#include <array>
//#include <any>
//#include <type_traits>
//#pragma warning( pop )

export module Testing:TestContext;

import std;

import :Interfaces;
import :TestException;

export namespace Testing {

	export class TestContext : public TestContextInterface {
	private:
		mutable UnitTestInterface* m_owner;
		std::vector<std::any> m_controlledObjects;
		TestState m_state;

	public:
		inline TestContext(UnitTestInterface* owner) : m_owner(owner) {}

	public:
		[[nodiscard]] virtual constexpr TestState state() const override { return m_state; }
		virtual constexpr void state(TestState state) override { m_state = state; }

	public:
		template<typename T, typename... Args>
		constexpr auto createTestObject(Args&&... args) const -> std::enable_if_t<std::is_constructible_v<T, Args...>, T> {
			return T(std::forward<Args>(args)...);
		}
		template<typename T, typename... Args>
		constexpr auto createTestObject(Args&&... args) const -> std::enable_if_t<!std::is_constructible_v<T, Args...> && std::is_constructible_v<T>, T> {
			return this->createTestObject<T>();
		}
		template<typename T>
		constexpr auto createTestObject() const -> std::enable_if_t<std::is_constructible_v<T>, T> {
			return T();
		}

		template<typename T, typename... Args >
		constexpr T* createTestObjectPointed(Args&&... args) {
			T* obj = &m_controlledObjects.emplace_back(std::make_any<T>(std::forward<Args>(args)...));
			return obj;
		}

		template<typename T, size_t Count>
		constexpr std::array<T, Count> createTestObjectsArray() { return {}; }
		template<typename T, size_t Count>
		constexpr std::array<const T, Count> createTestObjectsArray() const { return {}; }

	public:

		virtual void reset() override {
			m_state = NeverStarted;
		}

		virtual void start() override {
			m_state = Started;
		}

		virtual void stop() override {
			m_state = Stopped;
		}

		virtual void finish() override {
			m_state = Suceeded;
		}

		virtual void ignore() override {
			m_state = Ignored;
		}

		virtual void processException(const TestException&) override {
			m_state = Failed;
		}

		virtual void processException(const std::exception&) override {
			m_state = Crashed;
		}

		virtual void terminate(const std::exception&) override {
			m_state = Crashed;
		}

	public:
		virtual void log(std::string&& data, bool immediate = true) const override {
			if (!m_owner) {
				throw std::exception("Invalid TestContext! Owner must be not null!");
			}
			if (m_owner->view()) {
				m_owner->view()->addEntry(ViewLevel::info, data);
				if (immediate) {
					m_owner->view()->update();
				}
			}
		}
		virtual void log(const std::string& data, bool immediate = true) const override {
			if (!m_owner) {
				throw std::exception("Invalid TestContext! Owner must be not null!");
			}
			if (m_owner->view()) {
				m_owner->view()->addEntry(ViewLevel::info, data);
				if (immediate) {
					m_owner->view()->update();
				}
			}
		}
	};

	export
		template<typename Type>
	class TestContextTyped : public TestContext {

	public:
		inline TestContextTyped(UnitTestInterface* owner) : TestContext(owner) {}

	public:
		template<typename... Args>
		constexpr auto createTestObject(Args&&... args) const -> std::enable_if_t<std::is_constructible_v<Type, Args...>, Type>
		{
			return Type(std::forward<Args>(args)...);
		}
		template<typename... Args>
		constexpr auto createTestObject(Args&&... args) const -> std::enable_if_t<!std::is_constructible_v<Type, Args...> && std::is_constructible_v<Type>, Type>
		{
			return this->createTestObject();
		}
		constexpr auto createTestObject() const -> std::enable_if_t<std::is_constructible_v<Type>, Type> {
			return Type();
		}

		template<size_t Count>
		constexpr std::array<Type, Count> createTestObjectsArray() { return {}; }
		template<size_t Count>
		constexpr std::array<const Type, Count> createTestObjectsArray() const { return {}; }

		template<class... Args >
		constexpr Type* createTestObjectPointed(Args&&... args) {
			Type* obj = &m_controlledObjects.emplace_back(std::make_any<Type>(std::forward<Args>(args)...));
			return obj;
		}
	};
}
