#include "Clang/Attributes.h"
#include "Clang/AttrSubjectMatchRules.h"
#include "Clang/IdentifierTable.h"
#include <llvm/ADT/StringSwitch.h>
using namespace c2lang;

int c2lang::hasAttribute(AttrSyntax Syntax, const IdentifierInfo *Scope,
                        const IdentifierInfo *Attr, const TargetInfo &Target,
                        const LangOptions &LangOpts) {
  StringRef Name = Attr->getName();
  // Normalize the attribute name, __foo__ becomes foo.
  if (Name.size() >= 4 && Name.startswith("__") && Name.endswith("__"))
    Name = Name.substr(2, Name.size() - 4);

#include "Clang/AttrHasAttributeImpl.inc"

  return 0;
}

const char *attr::getSubjectMatchRuleSpelling(attr::SubjectMatchRule Rule) {
  switch (Rule) {
#define ATTR_MATCH_RULE(NAME, SPELLING, IsAbstract)                            \
  case attr::NAME:                                                             \
    return SPELLING;
#include "Clang/AttrSubMatchRulesList.inc"
  }
  llvm_unreachable("Invalid subject match rule");
}
