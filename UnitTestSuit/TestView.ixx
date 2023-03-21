module;

#include <string>
#include <format>
#include <iostream>
#include <vector>
#include <ranges>
#include <string_view>

export module Testing:TestView;

import :Interfaces;

export namespace Testing {

	export
	class TestViewBase : public TestViewInterface {
	public:

	private:
		std::vector<const TestViewInterface*> m_childs;
		TestViewInterface* m_parent;
		size_t m_indent;
		std::string m_indentString;

	public:
		inline TestViewBase() : TestViewBase(nullptr, std::string("\t")) {}
		inline TestViewBase(TestViewInterface* parent) : TestViewBase(parent, std::string("\t")) {}
		inline TestViewBase(std::string indentString) : TestViewBase(nullptr, indentString) {}
		inline TestViewBase(TestViewInterface* parent, std::string indentString) : m_parent(parent), m_indentString(indentString) {
			if (parent) {
				parent->registerSelf(this);
			}
		}

		~TestViewBase() {
			for (auto child : m_childs) {
				delete child;
			}
		}
	public:
		virtual TestViewInterface* parent() const override { return m_parent; }
		virtual void parent(TestViewInterface* parent) override {
			if (this == parent) {
				throw "TestViewInterface: Trying to set self as parent!";
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
		virtual TestViewInterface* clone(TestViewInterface* target) const override {
			if (!target) {
				return nullptr;
			}

			TestViewBase* asBase = static_cast<TestViewBase*>(target);

			asBase->m_indent = this->m_indent;
			asBase->m_indentString = this->m_indentString;

			return asBase;
		}

	protected:
		virtual void indent() override { indent(1); }
		virtual void indent(size_t count) override { m_indent += count; }
		virtual void unindent() override { unindent(1); }
		virtual void unindent(size_t count) override { m_indent = (m_indent > count ? (m_indent - count) : 0); }

		virtual size_t indentValue() const { return m_indent; }
		virtual std::string indentString() const { return m_indentString; }
	};

	export
	class TestViewConsole : public TestViewBase {
		typedef TestViewBase inherited;

		struct Entry {
			std::string data;
			ViewLevel level;
			size_t indent;
		};

	private:
		size_t m_entryIndex;
		std::string m_indentPrefix;
		std::vector<Entry> m_entries;

	public:
		inline TestViewConsole() : TestViewBase(nullptr, "\t"), m_entryIndex(0) {}
		inline TestViewConsole(TestViewInterface* parent) : TestViewBase(parent, "\t"), m_entryIndex(0) {}
		inline TestViewConsole(std::string indentValue) : TestViewBase(nullptr, indentValue), m_entryIndex(0) {}
		inline TestViewConsole(TestViewInterface* parent, std::string indentValue) : TestViewBase(parent, indentValue), m_entryIndex(0) {}

	public:
		void addMultilineEntry(const ViewLevel level, const std::string& multiline, const bool appendFirstLine = false, size_t maxLines = static_cast<size_t>(-1)) {
			const std::string_view lines{multiline};
			const std::string_view delim{"\n"};

			size_t linesAdded = 0;
			for (const auto line : std::views::split(lines, delim)) {
				if (linesAdded > maxLines) {
					break;
				}
				if (linesAdded == 0 && appendFirstLine) {
					append(level, std::string(line.begin(), line.end()), false, 1);
				} else {
					addEntry(level, std::string(line.begin(), line.end()), false, 1);
				}
				++linesAdded;
			}
		}

		void appendMultilineEntry(const ViewLevel level, const std::string& multiline, size_t maxLines = static_cast<size_t>(-1)) {
			addMultilineEntry(level, multiline, m_entryIndex != m_entries.size(), maxLines);
		}

		virtual void append(const ViewLevel level, const std::string& appendix, const bool multiline, size_t maxLines) override {
			if (m_entryIndex == m_entries.size()) {
				addEntry(level, appendix, multiline, maxLines);
			} else {
				if (multiline) {
					appendMultilineEntry(level, appendix, maxLines);
				} else {
					Entry& entry = m_entries.back();
					entry.data.append(appendix);
				}
			}
		}

		virtual void append(const ViewLevel level, std::string&& appendix, const bool multiline, size_t maxLines) override {
			if (m_entryIndex == m_entries.size()) {
				addEntry(level, appendix, multiline, maxLines);
			} else {
				if (multiline) {
					appendMultilineEntry(level, appendix, maxLines);
				} else {
					Entry& entry = m_entries.back();
					entry.data.append(appendix);
					if (entry.level < level) {
						entry.level = level;
					}
				}
			}
		}

		virtual void addEntry(const ViewLevel level, const std::string& entry, const bool multiline, size_t maxLines) override {
			if (multiline) {
				addMultilineEntry(level, entry, false, maxLines);
			} else {
				Entry e;
				e.level = level;
				e.data = std::string(m_indentPrefix);
				e.data = e.data.append(entry);
				e.indent = indentValue();
				m_entries.emplace_back(std::move(e));
			}
		}

		virtual void addEntry(const ViewLevel level, std::string&& entry, const bool multiline, size_t maxLines) override {
			if (multiline) {
				addMultilineEntry(level, entry, false, maxLines);
			} else {
				Entry e;
				e.level = level;
				e.data = std::string(m_indentPrefix);
				e.data = e.data.append(entry);
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

		virtual void print() override {
			while (m_entryIndex < m_entries.size()) {
				const Entry& entry = m_entries[m_entryIndex];
				outStream(entry.level) << std::format("{}\n", entry.data);
				++m_entryIndex;
			}
			flushStreams();
		}

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

		virtual void clear() override {
			m_entries.clear();
			m_indentPrefix.clear();
			m_entryIndex = 0;
		}

		virtual TestViewInterface* clone(TestViewInterface* target) const override {
			if (!target) {
				target = new TestViewConsole();
			}
			return inherited::clone(target);
		}
	public:
		static TestViewInterface* create() { return new TestViewConsole(); }
	};
}
