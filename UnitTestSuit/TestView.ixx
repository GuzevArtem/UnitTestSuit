export module Testing:TestView;

import std;

import :Interfaces;

export namespace Testing {

	export
	class TestViewBase : public TestViewInterface {
	private:
		std::vector<const TestViewInterface*> m_childs;
		TestViewInterface* m_parent;

	public:
		inline TestViewBase(TestViewInterface* parent) : m_parent(parent) {
			if (parent) {
				parent->registerSelf(this);
			}
		}
		virtual ~TestViewBase() noexcept {
			for (auto child : m_childs) {
				delete child;
			}
		}

	public:
		[[nodiscard]]
		virtual TestViewInterface* parent() const override { return m_parent; }
		virtual void parent(TestViewInterface* parent) override {
			if (this == parent) {
				throw std::exception("TestViewInterface: Trying to set self as parent!");
			}

			if (m_parent) {
				parent->removeSelf(this);
			}
			if (parent) {
				parent->registerSelf(this);
			}
			m_parent = parent;
		}
		virtual void removeSelf(const TestViewInterface* child) {
			if (!child) {
				return;
			}
			if (auto it = std::find(m_childs.begin(), m_childs.end(), child); it != m_childs.end()) {
				m_childs.erase(it);
			}
		}
		virtual void registerSelf(const TestViewInterface* child) override {
			if (child != nullptr && m_childs.end() == std::find(m_childs.begin(), m_childs.end(), child)) {
				m_childs.push_back(child);
			}
		}
		[[nodiscard]]
		virtual TestViewInterface* clone(TestViewInterface* target) const override {
			// do not clone parent
			return dynamic_cast<TestViewBase*>(target);
		}

	public:
		virtual void onBeforeTest() override {}
		virtual void onAfterTest() override {}
		virtual void onBeforeAllTests() override {}
		virtual void onAfterAllTests() override {}

		virtual void startBlock() override {}
		virtual void endBlock() override {}

	public:
		virtual bool needUpdate() const override { return true; }
	};

	export
	class TestViewTextBase : public TestViewBase {
		typedef TestViewBase inherited;

	private:
		size_t m_indent;
		std::string m_indentString;

	public:
		inline TestViewTextBase() : TestViewTextBase(nullptr, std::string("\t")) {}
		inline TestViewTextBase(TestViewInterface* parent) : TestViewTextBase(parent, std::string("\t")) {}
		inline TestViewTextBase(std::string indentString) : TestViewTextBase(nullptr, indentString) {}
		inline TestViewTextBase(TestViewInterface* parent, std::string indentString) : inherited(parent), m_indentString(indentString), m_indent(0) {}

	public:
		[[nodiscard]]
		virtual TestViewInterface* clone(TestViewInterface* target) const override {
			TestViewInterface* asBase = inherited::clone(target);
			TestViewTextBase* asTextBase = dynamic_cast<TestViewTextBase*>(asBase);

			if (!asTextBase) {
				return nullptr;
			}

			asTextBase->m_indent = this->m_indent;
			asTextBase->m_indentString = this->m_indentString;

			return asTextBase;
		}

	protected:
		virtual void indent() { indent(1); }
		virtual void indent(size_t count) { m_indent += count; }
		virtual void unindent() { unindent(1); }
		virtual void unindent(size_t count) { m_indent = (m_indent > count ? (m_indent - count) : 0); }

		[[nodiscard]]
		virtual size_t indentValue() const { return m_indent; }
		[[nodiscard]]
		virtual std::string indentString() const { return m_indentString; }

	public:
		virtual void onBeforeTest() override {
			inherited::onBeforeTest();
			startBlock();
		}
		virtual void onAfterTest() override {
			endBlock();
			inherited::onAfterTest();
		}

		virtual void onBeforeAllTests() override {
			inherited::onBeforeAllTests();
			//startBlock();
		}
		virtual void onAfterAllTests() override {
			//endBlock();
			inherited::onAfterAllTests();
		}

		virtual void startBlock() {
			inherited::startBlock();
			indent();
		}
		virtual void endBlock() {
			unindent();
			inherited::endBlock();
		}
	};

	export
	class TestViewConsole : public TestViewTextBase {
		typedef TestViewTextBase inherited;

	private:
		struct Entry {
			std::string data;
			size_t indent;
			ViewLevel level;
		};

	private:
		size_t m_entryIndex;
		std::string m_indentPrefix;
		std::vector<Entry> m_entries;

	public:
		inline TestViewConsole() : inherited(nullptr, "\t"), m_entryIndex(0) {}
		inline TestViewConsole(TestViewInterface* parent) : inherited(parent, "\t"), m_entryIndex(0) {}
		inline TestViewConsole(std::string indentValue) : inherited(nullptr, indentValue), m_entryIndex(0) {}
		inline TestViewConsole(TestViewInterface* parent, std::string indentValue) : inherited(parent, indentValue), m_entryIndex(0) {}

	public:
		virtual TestViewInterface* clone(TestViewInterface* target) const override {
			TestViewConsole* asConsole = dynamic_cast<TestViewConsole*>(target);
			if (!asConsole) {
				asConsole = new TestViewConsole();
			}
			return inherited::clone(asConsole);
		}

	private:
		void addMultilineEntry(const ViewLevel level, const std::string& multiline, const bool appendFirstLine = false, size_t maxLines = static_cast<size_t>(-1)) {
			size_t linesAdded = 0;
			for (std::string::const_iterator beg = multiline.begin(), it = multiline.begin(), end = multiline.end();; ++it) {
				if (it != end && *it != '\n') {
					continue; // walk to next delimiter or end of line
				}
				if (linesAdded == 0 && appendFirstLine) {
					append(level, std::string(beg, it), false, 1);
				} else {
					addEntry(level, std::string(beg, it), false, 1);
				}
				++linesAdded;
				if (linesAdded > maxLines) {
					break;
				}
				beg = it;
				if (beg == end) {
					break;
				}
				++beg;
			}
		}

		void appendMultilineEntry(const ViewLevel level, const std::string& multiline, size_t maxLines = static_cast<size_t>(-1)) {
			addMultilineEntry(level, multiline, m_entryIndex != m_entries.size(), maxLines);
		}

	public:
		virtual void append(const ViewLevel level, const std::string& data, const bool multiline = false, size_t maxLines = static_cast<size_t>(-1)) override {
			if (m_entryIndex == m_entries.size()) {
				addEntry(level, data, multiline, maxLines);
			} else {
				if (multiline) {
					appendMultilineEntry(level, data, maxLines);
				} else {
					Entry& e = m_entries.back();
					e.data.append(data);
					if (e.level < level) {
						e.level = level;
					}
				}
			}
		}

		virtual void addEntry(const ViewLevel level, const std::string& data, const bool multiline = false, size_t maxLines = static_cast<size_t>(-1)) override {
			if (multiline) {
				addMultilineEntry(level, data, false, maxLines);
			} else {
				Entry e;
				e.level = level;
				e.data = std::string(m_indentPrefix);
				e.data = e.data.append(data);
				e.indent = indentValue();
				m_entries.emplace_back(std::move(e));
			}
		}

		std::ostream& outStream(ViewLevel level) const {
			switch (level) {
				case ViewLevel::invalid:
				case ViewLevel::error:
					return std::cerr;
				case ViewLevel::trace:
				case ViewLevel::info:
				case ViewLevel::warning:
				default:
					return std::cout;
			}
		}

		void flushStreams() const {
			std::flush(std::cout);
			std::flush(std::cerr);
		}

	public:
		[[nodiscard]]
		virtual bool needUpdate() const {
			return !m_entries.empty() && m_entryIndex < m_entries.size();
		}
		virtual void update() override {
			while (needUpdate()) {
				const Entry& entry = m_entries[m_entryIndex];
				outStream(entry.level) << std::format("{}\n", entry.data);
				++m_entryIndex;
			}
			flushStreams();
		}

		virtual void clear() override {
			m_entries.clear();
			m_indentPrefix.clear();
			m_entryIndex = 0;
		}

	protected:
		using inherited::indent; // prevent function 'hiding'
		using inherited::unindent; // prevent function 'hiding'

		virtual void indent(size_t count) override { 
			inherited::indent(count);
			for (size_t i = 0; i < count; ++i) {
				m_indentPrefix.append(indentString());
			}
		}

		virtual void unindent(size_t count) override {
			inherited::unindent(count);
			m_indentPrefix.clear();
			for (size_t i = 0; i < indentValue(); ++i) {
				m_indentPrefix.append(indentString());
			}
		}
	};
}
