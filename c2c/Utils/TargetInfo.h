/* Copyright 2013-2022 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTILS_TARGET_INFO_H
#define UTILS_TARGET_INFO_H

#include <string>
#include <assert.h>

#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ADT/None.h>

namespace C2 {

class TargetInfo {
public:
	virtual ~TargetInfo();

    enum Arch { ARCH_UNKNOWN, ARCH_I686, ARCH_ARM, ARCH_X86_64, ARCH_ARM_64, ARCH_RISCV_32 };
    enum System { SYS_UNKNOWN, SYS_LINUX, SYS_DARWIN, SYS_CYGWIN };
    enum Vendor { VENDOR_UNKNOWN, VENDOR_APPLE };
    enum Abi { ABI_UNKNOWN, ABI_GNU, ABI_GNUEABI, ABI_MACHO, ABI_WIN32, ABI_RV32G };

    Arch arch;
    System sys;
    Vendor vendor;
    Abi abi;

    unsigned intWidth;
    static void getNative(TargetInfo& info);
    static bool fromString(TargetInfo& info, const std::string& triple);

    bool isValidClobber(llvm::StringRef Name) const;
    bool isValidGCCRegisterName(llvm::StringRef Name) const;

  struct GCCRegAlias {
    const char * const Aliases[5];
    const char * const Register;
  };

  struct AddlRegName {
    const char * const Names[5];
    const unsigned RegNum;
  };

  struct ConstraintInfo {
	  enum {
		  CI_None = 0x00,
		  CI_AllowsMemory = 0x01,
		  CI_AllowsRegister = 0x02,
		  CI_ReadWrite = 0x04,         // "+r" output constraint (read and write).
		  CI_HasMatchingInput = 0x08,  // This output operand has a matching input.
		  CI_ImmediateConstant = 0x10, // This operand must be an immediate constant
		  CI_EarlyClobber = 0x20,      // "&" output constraint (early clobber).
		};
		unsigned Flags;
		int TiedOperand;
		struct {
		  int Min;
		  int Max;
		} ImmRange;
		llvm::SmallSet<int, 4> ImmSet;

		std::string ConstraintStr;  // constraint: "=rm"
		std::string Name;           // Operand name: [foo] with no []'s.
	public:
	  ConstraintInfo(llvm::StringRef ConstraintStr, llvm::StringRef Name)
        : Flags(0), TiedOperand(-1), ConstraintStr(ConstraintStr.str()),
          Name(Name.str()) {
      ImmRange.Min = ImmRange.Max = 0;
    }

    const std::string &getConstraintStr() const { return ConstraintStr; }
    const std::string &getName() const { return Name; }
    bool isReadWrite() const { return (Flags & CI_ReadWrite) != 0; }
    bool earlyClobber() { return (Flags & CI_EarlyClobber) != 0; }
    bool allowsRegister() const { return (Flags & CI_AllowsRegister) != 0; }
    bool allowsMemory() const { return (Flags & CI_AllowsMemory) != 0; }

    /// \brief Return true if this output operand has a matching
    /// (tied) input operand.
    bool hasMatchingInput() const { return (Flags & CI_HasMatchingInput) != 0; }

    /// \brief Return true if this input operand is a matching
    /// constraint that ties it to an output operand.
    ///
    /// If this returns true then getTiedOperand will indicate which output
    /// operand this is tied to.
    bool hasTiedOperand() const { return TiedOperand != -1; }
    unsigned getTiedOperand() const {
      assert(hasTiedOperand() && "Has no tied operand!");
      return (unsigned)TiedOperand;
    }

    bool requiresImmediateConstant() const {
      return (Flags & CI_ImmediateConstant) != 0;
    }
#if 0
    bool isValidAsmImmediate(const llvm::APInt &Value) const {
      return (Value.sge(ImmRange.Min) && Value.sle(ImmRange.Max)) ||
             ImmSet.count(Value.getZExtValue()) != 0;
    }
#endif

	    void setIsReadWrite() { Flags |= CI_ReadWrite; }
		void setEarlyClobber() { Flags |= CI_EarlyClobber; }
		void setAllowsMemory() { Flags |= CI_AllowsMemory; }
		void setAllowsRegister() { Flags |= CI_AllowsRegister; }
		void setHasMatchingInput() { Flags |= CI_HasMatchingInput; }
		void setRequiresImmediate(int Min, int Max) {
		  Flags |= CI_ImmediateConstant;
		  ImmRange.Min = Min;
		  ImmRange.Max = Max;
		}
#if 0
		void setRequiresImmediate(llvm::ArrayRef<int> Exacts) {
		  Flags |= CI_ImmediateConstant;
		  for (int Exact : Exacts)
			ImmSet.insert(Exact);
		}
#endif
		void setRequiresImmediate(int Exact) {
		  Flags |= CI_ImmediateConstant;
		  ImmSet.insert(Exact);
		}
		void setRequiresImmediate() {
		  Flags |= CI_ImmediateConstant;
		  ImmRange.Min = INT_MIN;
		  ImmRange.Max = INT_MAX;
		}

		/// \brief Indicate that this is an input operand that is tied to
		/// the specified output operand.
		///
		/// Copy over the various constraint information from the output.
		void setTiedOperand(unsigned N, ConstraintInfo &Output) {
		  Output.setHasMatchingInput();
		  Flags = Output.Flags;
		  TiedOperand = N;
		  // Don't copy Name or constraint string.
		}
    };

    bool validateOutputConstraint(ConstraintInfo& Info) const;
    //bool validateInputConstraint(MutableArrayRef<ConstaintInfo> OutputConstraints, ConstraintInfo& Info) const;
    bool validateInputConstraint(ConstraintInfo& Info) const;
private:
    void init();
#if 0
protected:
	TargetInfo(const llvm::Triple& T);

	virtual llvm::ArrayRef<const char *> getGCCRegNames() const = 0;
	virtual llvm::ArrayRef<GCCRegAlias> getGCCRegAliases() const = 0;
	virtual llvm::ArrayRef<AddlRegName> getGCCAddlRegNames() const {
		return llvm::None;
	}
private:
	llvm::Triple Triple;
#endif
};

const char* Str(const TargetInfo& info);
const char* Str(TargetInfo::Arch mach);
const char* Str(TargetInfo::Vendor vendor);
const char* Str(TargetInfo::System sys);
const char* Str(TargetInfo::Abi abi);

}

#endif

