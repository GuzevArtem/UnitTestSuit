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

		virtual bool printErrors() const override {
			bool result = false;
			for (const TestViewInterface* child : m_childs) {
				if (child->printErrors()) {
					result = true;
				}
			}
			return result;
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
		void addMultilineEntry(const ViewLevel level, const std::string& multiline, const bool appendFirstLine = false) {
			const std::string_view lines{multiline};
			const std::string_view delim{"\n"};

			bool isFirstLine = true;
			for (const auto line : std::views::split(lines, delim)) {
				if (isFirstLine && appendFirstLine) {
					isFirstLine = false;
					append(level, std::string(line.begin(), line.end()), false);
				} else {
					addEntry(level, std::string(line.begin(), line.end()), false);
				}
			}
		}

		void appendMultilineEntry(const ViewLevel level, const std::string& multiline) {
			addMultilineEntry(level, multiline, m_entryIndex != m_entries.size());
		}

		virtual void append(const ViewLevel level, const std::string& appendix, const bool multiline) override {
			if (m_entryIndex == m_entries.size()) {
				addEntry(level, appendix, multiline);
			} else {
				if (multiline) {
					appendMultilineEntry(level, appendix);
				} else {
					Entry& entry = m_entries.back();
					entry.data.append(appendix);
				}
			}
		}

		virtual void append(const ViewLevel level, std::string&& appendix, const bool multiline) override {
			if (m_entryIndex == m_entries.size()) {
				addEntry(level, appendix, multiline);
			} else {
				if (multiline) {
					appendMultilineEntry(level, appendix);
				} else {
					Entry& entry = m_entries.back();
					entry.data.append(appendix);
					if (entry.level < level) {
						entry.level = level;
					}
				}
			}
		}

		virtual void addEntry(const ViewLevel level, const std::string& entry, const bool multiline) override {
			if (multiline) {
				addMultilineEntry(level, entry);
			} else {
				Entry e;
				e.level = level;
				e.data = std::string(m_indentPrefix);
				e.data = e.data.append(entry);
				e.indent = indentValue();
				m_entries.emplace_back(std::move(e));
			}
		}

		virtual void addEntry(const ViewLevel level, std::string&& entry, const bool multiline) override {
			if (multiline) {
				addMultilineEntry(level, entry);
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

		virtual void print() override {
			while (m_entryIndex < m_entries.size()) {
				const Entry& entry = m_entries[m_entryIndex];
				outStream(entry.level) << std::format("{}\n", entry.data);
				++m_entryIndex;
			}
		}

		virtual bool printErrors() const override {
			bool result = inherited::printErrors();

			if (m_entries.empty()) {
				return result;
			}

#pragma message("Made `linesOfCurrentIndentIfError` adjustable?") 
			constexpr size_t linesOfCurrentIndentIfError = 5;

			std::vector<Entry> buff;
			{
				bool skip_next_pop = false;
				size_t prev_indent;
				size_t error_count = linesOfCurrentIndentIfError;
				size_t skipped_error_lines = 0;
				auto errorFound = [&buff, &prev_indent, &skipped_error_lines, &skip_next_pop, &error_count](const Entry& e) -> void {
					if (error_count == 0) {
						++skipped_error_lines;
						return;
					}
					if (!skip_next_pop && prev_indent == e.indent && buff.size()) {
						buff.pop_back();
					}
					skip_next_pop = true;
					if (prev_indent == e.indent) {
						--error_count;
					} else {
						prev_indent = e.indent;
						error_count = linesOfCurrentIndentIfError;
					}
					buff.push_back(e);
				};
				auto normalFound = [&buff, &error_count, &prev_indent, &skip_next_pop, &linesOfCurrentIndentIfError](const Entry& e) -> void {
					error_count = linesOfCurrentIndentIfError;
					if (skip_next_pop) {
						skip_next_pop = false;
					}
					buff.pop_back();
					buff.emplace_back(e);
					prev_indent = e.indent;
				};

				auto processEntry = [&result , &prev_indent, &buff, &skipped_error_lines, &errorFound, &normalFound, &linesOfCurrentIndentIfError](const Entry& e) -> void {
					if (buff.empty()) {
						prev_indent = e.indent;
						if (e.level >= ViewLevel::error) {
							result = true;
							errorFound(e);
						} else {
							buff.push_back(e);
						}
						return;
					}

					if (e.level >= ViewLevel::error) {
						result = true;
						errorFound(e);
					} else {
						if (skipped_error_lines > 0) {
							Entry skippedData;
							skippedData.indent = prev_indent;
							skippedData.level = ViewLevel::error;
							skippedData.data = std::format("\t\t... {} lines skipped ...", skipped_error_lines);
							if (linesOfCurrentIndentIfError > 0) {
								buff.pop_back();
							}
							buff.push_back(skippedData);
						}
						if (e.indent > prev_indent) {
							buff.push_back(e);
							prev_indent = e.indent;
						} else {
							if (skipped_error_lines > 0) {
								buff.emplace_back(Entry{}); //add dummy to be pop
							}
							normalFound(e);
						}
						skipped_error_lines = 0;
					}
				};

				for (const Entry& e : m_entries) {
					processEntry(e);
				}
			}

			if (!result) {
				// no errors
				return false;
			}
			{ // rollback unrequired entries
				Entry e = buff.back();
				while (e.level < ViewLevel::error) {
					buff.pop_back();
					if (buff.empty()) {
						return false;
					}
					e = buff.back();
				}
			}

			for (const Entry& entry : buff) {
				outStream(ViewLevel::error) << std::format("{}\n", entry.data);
			}
			return true;
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
