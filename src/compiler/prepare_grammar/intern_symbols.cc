#include "compiler/prepare_grammar/intern_symbols.h"
#include <memory>
#include <vector>
#include "tree_sitter/compiler.h"
#include "compiler/prepared_grammar.h"
#include "compiler/rules/visitor.h"
#include "compiler/rules/named_symbol.h"
#include "compiler/rules/symbol.h"

namespace tree_sitter {
    using std::string;
    using rules::rule_ptr;
    using std::vector;
    using std::pair;
    using std::make_shared;

    namespace prepare_grammar {
        class InternSymbols : public rules::IdentityRuleFn {
            using rules::IdentityRuleFn::apply_to;

            rule_ptr apply_to(const rules::NamedSymbol *rule)  {
                for (size_t i = 0; i < grammar.rules.size(); i++)
                    if (grammar.rules[i].first == rule->name)
                        return make_shared<rules::Symbol>(i);
                missing_rule_name = rule->name;
                return rule_ptr();
            }

        public:
            explicit InternSymbols(const Grammar &grammar) : grammar(grammar) {}
            const Grammar grammar;
            string missing_rule_name;
        };

        pair<PreparedGrammar, const GrammarError *> intern_symbols(const Grammar &grammar) {
            InternSymbols interner(grammar);
            vector<pair<string, rule_ptr>> rules;

            for (auto &pair : grammar.rules) {
                auto new_rule = interner.apply(pair.second);
                if (!interner.missing_rule_name.empty())
                    return {
                        PreparedGrammar({}, {}),
                        new GrammarError(GrammarErrorTypeUndefinedSymbol,
                                         "Undefined rule '" + interner.missing_rule_name + "'")
                    };
                rules.push_back({ pair.first, new_rule });
            }

            return { PreparedGrammar(rules, {}), nullptr };
        }
    }
}