#ifndef OPERATION_KINDS_H
#define OPERATION_KINDS_H

namespace C2 {

// TODO cleanup for C2
enum BinaryOperatorKind {
  // Operators listed in order of precedence.
  // Note that additions to this should also update the StmtVisitor class.
  BO_PtrMemD, BO_PtrMemI,       // [C++ 5.5] Pointer-to-member operators.
  BO_Mul, BO_Div, BO_Rem,       // [C99 6.5.5] Multiplicative operators.
  BO_Add, BO_Sub,               // [C99 6.5.6] Additive operators.
  BO_Shl, BO_Shr,               // [C99 6.5.7] Bitwise shift operators.
  BO_LT, BO_GT, BO_LE, BO_GE,   // [C99 6.5.8] Relational operators.
  BO_EQ, BO_NE,                 // [C99 6.5.9] Equality operators.
  BO_And,                       // [C99 6.5.10] Bitwise AND operator.
  BO_Xor,                       // [C99 6.5.11] Bitwise XOR operator.
  BO_Or,                        // [C99 6.5.12] Bitwise OR operator.
  BO_LAnd,                      // [C99 6.5.13] Logical AND operator.
  BO_LOr,                       // [C99 6.5.14] Logical OR operator.
  BO_Assign, BO_MulAssign,      // [C99 6.5.16] Assignment operators.
  BO_DivAssign, BO_RemAssign,
  BO_AddAssign, BO_SubAssign,
  BO_ShlAssign, BO_ShrAssign,
  BO_AndAssign, BO_XorAssign,
  BO_OrAssign,
  BO_Comma                      // [C99 6.5.17] Comma operator.
};

enum UnaryOperatorKind {
  // Note that additions to this should also update the StmtVisitor class.
  UO_PostInc, UO_PostDec, // [C99 6.5.2.4] Postfix increment and decrement
  UO_PreInc, UO_PreDec,   // [C99 6.5.3.1] Prefix increment and decrement
  UO_AddrOf, UO_Deref,    // [C99 6.5.3.2] Address and indirection
  UO_Plus, UO_Minus,      // [C99 6.5.3.3] Unary arithmetic
  UO_Not, UO_LNot,        // [C99 6.5.3.3] Unary arithmetic
  UO_Real, UO_Imag,       // "__real expr"/"__imag expr" Extension.
  UO_Extension            // __extension__ marker.
};

}

#endif

