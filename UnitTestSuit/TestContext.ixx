export module Testing.TestContext;

import std;

import Testing.Interfaces;
import Testing.TestException;

export namespace Testing {

	export struct ControlledObjectProxy {
		constexpr virtual ~ControlledObjectProxy() noexcept {}
	};

	export
	template<typename T>
	struct ControlledObject : public ControlledObjectProxy {
		T* value;

		template<typename... Args>
		constexpr ControlledObject(Args... args) { value = new T(args...); }
		constexpr ControlledObject(const ControlledObject& other) : value(other.value) {}
		constexpr ControlledObject(ControlledObject&& other) : value(std::move(other.value)) {}

		constexpr virtual ~ControlledObject() {
			delete value;
		}

		constexpr ControlledObject& operator=(const ControlledObject& other) = default;
		constexpr ControlledObject& operator=(ControlledObject&& other) = default;

		constexpr ControlledObject& operator=(const T*& other) {
			value = other;
			return *this;
		}
		constexpr ControlledObject& operator=(T*&& other) {
			value = other;
			return *this;
		}
	};

	export class TestContext : public TestContextInterface {
	private:
		mutable UnitTestInterface* m_owner;
		std::vector<ControlledObjectProxy*> m_controlledObjects;
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
			return static_cast<ControlledObject<T>*>(m_controlledObjects.push_back(new ControlledObject<T>(args...)))->value;
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
			return static_cast<ControlledObject<Type>*>(m_controlledObjects.push_back(new ControlledObject<Type>(args...)))->value;
		}
	};
}
