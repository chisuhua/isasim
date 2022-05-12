#pragma once

// macro primitives

#define COMMA ,
#define REM(...) __VA_ARGS__
#define EAT(...)

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define CONCAT_(x, ...) x ## __VA_ARGS__
#define CONCAT(x, ...) CONCAT_(x, __VA_ARGS__)

#define COUNTOF(arr) (sizeof(arr) / sizeof(arr[0]))

// conditional macro

#define IIF_0(x, y) y
#define IIF_1(x, y) x
#define IIF(c) CONCAT(IIF_, c)

#define PAIR_FIRST(a, b) a
#define PAIR_SECOND(a, b) b

// pair macros

#define PAIR(x) REM x
#define PAIR_HEAD_(x, ...) PAIR(x)
#define PAIR_PROBE_(...) (__VA_ARGS__),
#define PAIR_L_(...) PAIR_HEAD_(__VA_ARGS__)
#define PAIR_L(x) PAIR_L_(PAIR_PROBE_ x,)
#define PAIR_R(x) EAT x

// separator macros

#define SEP_COMMA() ,
#define SEP_SEMICOLON() ;
#define SEP_PLUS() +
#define SEP_AND() &
#define SEP_OR() |
#define SEP_COLON() :
#define SEP_SPACE() /**/
#define SEP_LESS() <
#define SEP_GREATER() >
#define SEP_ANDL() &&
#define SEP_ORL() ||

// MAKE_UNIQUE macro

#define MAKE_UNIQUE(x) CONCAT(x, __COUNTER__)

// increment macro

#define INC(x) INC_ ## x
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8
#define INC_8 9
#define INC_9 10
#define INC_10 11
#define INC_11 12
#define INC_12 13
#define INC_13 14
#define INC_14 15
#define INC_15 16
#define INC_16 17
#define INC_17 18
#define INC_18 19
#define INC_19 20
#define INC_20 21
#define INC_21 22
#define INC_22 23
#define INC_23 24
#define INC_24 25
#define INC_25 26
#define INC_26 27
#define INC_27 28
#define INC_28 29
#define INC_29 30
#define INC_30 31
#define INC_31 32
#define INC_32 33
#define INC_33 34
#define INC_34 35
#define INC_35 36
#define INC_36 37
#define INC_37 38
#define INC_38 39
#define INC_39 40
#define INC_40 41
#define INC_41 42
#define INC_42 43
#define INC_43 44
#define INC_44 45
#define INC_45 46
#define INC_46 47
#define INC_47 48
#define INC_48 49
#define INC_49 50
#define INC_50 51
#define INC_51 52
#define INC_52 53
#define INC_53 54
#define INC_54 55
#define INC_55 56
#define INC_56 57
#define INC_57 58
#define INC_58 59
#define INC_59 60
#define INC_60 61
#define INC_61 62
#define INC_62 63
#define INC_63 64

// NARG macro

#define NARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,_11,_12,_13,_14,_15,_16, \
                  _17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32, \
                  _33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48, \
                  _49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63, N, ...) N

#define NARG_R() 63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48, \
                    47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32, \
                    31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16, \
                    15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define NARG_(...) NARG_N(__VA_ARGS__) 
#define NARG(...)  NARG_(__VA_ARGS__, NARG_R())

// FOR_EACH macro

#define FOR_EA1(idx, func, arg, sep, ...)      func(arg, idx, __VA_ARGS__)
#define FOR_EA2(idx, func, arg, sep, x, ...)   func(arg, idx, x) sep() FOR_EA1(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA3(idx, func, arg, sep, x, ...)   func(arg, idx, x) sep() FOR_EA2(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA4(idx, func, arg, sep, x, ...)   func(arg, idx, x) sep() FOR_EA3(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA5(idx, func, arg, sep, x, ...)   func(arg, idx, x) sep() FOR_EA4(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA6(idx, func, arg, sep, x, ...)   func(arg, idx, x) sep() FOR_EA5(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA7(idx, func, arg, sep, x, ...)   func(arg, idx, x) sep() FOR_EA6(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA8(idx, func, arg, sep, x, ...)   func(arg, idx, x) sep() FOR_EA7(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA9(idx, func, arg, sep, x, ...)   func(arg, idx, x) sep() FOR_EA8(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA10(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA9(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA11(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA10(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA12(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA11(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA13(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA12(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA14(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA13(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA15(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA14(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA16(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA15(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA17(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA16(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA18(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA17(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA19(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA18(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA20(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA19(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA21(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA20(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA22(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA21(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA23(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA22(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA24(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA23(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA25(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA24(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA26(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA25(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA27(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA26(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA28(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA27(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA29(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA28(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA30(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA29(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA31(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA30(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA32(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA31(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA33(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA32(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA34(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA33(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA35(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA34(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA36(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA35(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA37(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA36(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA38(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA37(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA39(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA38(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA40(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA39(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA41(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA40(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA42(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA41(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA43(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA42(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA44(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA43(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA45(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA44(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA46(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA45(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA47(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA46(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA48(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA47(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA49(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA48(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA50(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA49(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA51(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA50(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA52(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA51(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA53(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA52(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA54(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA53(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA55(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA54(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA56(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA55(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA57(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA56(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA58(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA57(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA59(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA58(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA60(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA59(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA61(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA60(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA62(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA61(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA63(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA62(INC(idx), func, arg, sep, __VA_ARGS__)
#define FOR_EA64(idx, func, arg, sep, x, ...)  func(arg, idx, x) sep() FOR_EA63(INC(idx), func, arg, sep, __VA_ARGS__)

#define FOR_EA(N, func, arg, sep, ...) CONCAT(FOR_EA, N)(0, func, arg, sep, __VA_ARGS__)
#define FOR_EACH(func, arg, sep, ...) FOR_EA(NARG(__VA_ARGS__), func, arg, sep, __VA_ARGS__)

// REVERSE_FOR_EACH macro

#define REVERSE_FOR_EA1(func, arg, sep, ...)      func(arg, 0, __VA_ARGS__)
#define REVERSE_FOR_EA2(func, arg, sep, x, ...)   REVERSE_FOR_EA1(func, arg, sep, __VA_ARGS__) sep() func(arg, 1, x)
#define REVERSE_FOR_EA3(func, arg, sep, x, ...)   REVERSE_FOR_EA2(func, arg, sep, __VA_ARGS__) sep() func(arg, 2, x)
#define REVERSE_FOR_EA4(func, arg, sep, x, ...)   REVERSE_FOR_EA3(func, arg, sep, __VA_ARGS__) sep() func(arg, 3, x)
#define REVERSE_FOR_EA5(func, arg, sep, x, ...)   REVERSE_FOR_EA4(func, arg, sep, __VA_ARGS__) sep() func(arg, 4, x)
#define REVERSE_FOR_EA6(func, arg, sep, x, ...)   REVERSE_FOR_EA5(func, arg, sep, __VA_ARGS__) sep() func(arg, 5, x)
#define REVERSE_FOR_EA7(func, arg, sep, x, ...)   REVERSE_FOR_EA6(func, arg, sep, __VA_ARGS__) sep() func(arg, 6, x)
#define REVERSE_FOR_EA8(func, arg, sep, x, ...)   REVERSE_FOR_EA7(func, arg, sep, __VA_ARGS__) sep() func(arg, 7, x)
#define REVERSE_FOR_EA9(func, arg, sep, x, ...)   REVERSE_FOR_EA8(func, arg, sep, __VA_ARGS__) sep() func(arg, 8, x)
#define REVERSE_FOR_EA10(func, arg, sep, x, ...)  REVERSE_FOR_EA9(func, arg, sep, __VA_ARGS__) sep() func(arg, 9, x)
#define REVERSE_FOR_EA11(func, arg, sep, x, ...)  REVERSE_FOR_EA10(func, arg, sep, __VA_ARGS__) sep() func(arg, 10, x)
#define REVERSE_FOR_EA12(func, arg, sep, x, ...)  REVERSE_FOR_EA11(func, arg, sep, __VA_ARGS__) sep() func(arg, 11, x)
#define REVERSE_FOR_EA13(func, arg, sep, x, ...)  REVERSE_FOR_EA12(func, arg, sep, __VA_ARGS__) sep() func(arg, 12, x)
#define REVERSE_FOR_EA14(func, arg, sep, x, ...)  REVERSE_FOR_EA13(func, arg, sep, __VA_ARGS__) sep() func(arg, 13, x)
#define REVERSE_FOR_EA15(func, arg, sep, x, ...)  REVERSE_FOR_EA14(func, arg, sep, __VA_ARGS__) sep() func(arg, 14, x)
#define REVERSE_FOR_EA16(func, arg, sep, x, ...)  REVERSE_FOR_EA15(func, arg, sep, __VA_ARGS__) sep() func(arg, 15, x)
#define REVERSE_FOR_EA17(func, arg, sep, x, ...)  REVERSE_FOR_EA16(func, arg, sep, __VA_ARGS__) sep() func(arg, 16, x)
#define REVERSE_FOR_EA18(func, arg, sep, x, ...)  REVERSE_FOR_EA17(func, arg, sep, __VA_ARGS__) sep() func(arg, 17, x)
#define REVERSE_FOR_EA19(func, arg, sep, x, ...)  REVERSE_FOR_EA18(func, arg, sep, __VA_ARGS__) sep() func(arg, 18, x)
#define REVERSE_FOR_EA20(func, arg, sep, x, ...)  REVERSE_FOR_EA19(func, arg, sep, __VA_ARGS__) sep() func(arg, 19, x)
#define REVERSE_FOR_EA21(func, arg, sep, x, ...)  REVERSE_FOR_EA20(func, arg, sep, __VA_ARGS__) sep() func(arg, 20, x)
#define REVERSE_FOR_EA22(func, arg, sep, x, ...)  REVERSE_FOR_EA21(func, arg, sep, __VA_ARGS__) sep() func(arg, 21, x)
#define REVERSE_FOR_EA23(func, arg, sep, x, ...)  REVERSE_FOR_EA22(func, arg, sep, __VA_ARGS__) sep() func(arg, 22, x)
#define REVERSE_FOR_EA24(func, arg, sep, x, ...)  REVERSE_FOR_EA23(func, arg, sep, __VA_ARGS__) sep() func(arg, 23, x)
#define REVERSE_FOR_EA25(func, arg, sep, x, ...)  REVERSE_FOR_EA24(func, arg, sep, __VA_ARGS__) sep() func(arg, 24, x)
#define REVERSE_FOR_EA26(func, arg, sep, x, ...)  REVERSE_FOR_EA25(func, arg, sep, __VA_ARGS__) sep() func(arg, 25, x)
#define REVERSE_FOR_EA27(func, arg, sep, x, ...)  REVERSE_FOR_EA26(func, arg, sep, __VA_ARGS__) sep() func(arg, 26, x)
#define REVERSE_FOR_EA28(func, arg, sep, x, ...)  REVERSE_FOR_EA27(func, arg, sep, __VA_ARGS__) sep() func(arg, 27, x)
#define REVERSE_FOR_EA29(func, arg, sep, x, ...)  REVERSE_FOR_EA28(func, arg, sep, __VA_ARGS__) sep() func(arg, 28, x)
#define REVERSE_FOR_EA30(func, arg, sep, x, ...)  REVERSE_FOR_EA29(func, arg, sep, __VA_ARGS__) sep() func(arg, 29, x)
#define REVERSE_FOR_EA31(func, arg, sep, x, ...)  REVERSE_FOR_EA30(func, arg, sep, __VA_ARGS__) sep() func(arg, 30, x)
#define REVERSE_FOR_EA32(func, arg, sep, x, ...)  REVERSE_FOR_EA31(func, arg, sep, __VA_ARGS__) sep() func(arg, 31, x)
#define REVERSE_FOR_EA33(func, arg, sep, x, ...)  REVERSE_FOR_EA32(func, arg, sep, __VA_ARGS__) sep() func(arg, 32, x)
#define REVERSE_FOR_EA34(func, arg, sep, x, ...)  REVERSE_FOR_EA33(func, arg, sep, __VA_ARGS__) sep() func(arg, 33, x)
#define REVERSE_FOR_EA35(func, arg, sep, x, ...)  REVERSE_FOR_EA34(func, arg, sep, __VA_ARGS__) sep() func(arg, 34, x)
#define REVERSE_FOR_EA36(func, arg, sep, x, ...)  REVERSE_FOR_EA35(func, arg, sep, __VA_ARGS__) sep() func(arg, 35, x)
#define REVERSE_FOR_EA37(func, arg, sep, x, ...)  REVERSE_FOR_EA36(func, arg, sep, __VA_ARGS__) sep() func(arg, 36, x)
#define REVERSE_FOR_EA38(func, arg, sep, x, ...)  REVERSE_FOR_EA37(func, arg, sep, __VA_ARGS__) sep() func(arg, 37, x)
#define REVERSE_FOR_EA39(func, arg, sep, x, ...)  REVERSE_FOR_EA38(func, arg, sep, __VA_ARGS__) sep() func(arg, 38, x)
#define REVERSE_FOR_EA40(func, arg, sep, x, ...)  REVERSE_FOR_EA39(func, arg, sep, __VA_ARGS__) sep() func(arg, 39, x)
#define REVERSE_FOR_EA41(func, arg, sep, x, ...)  REVERSE_FOR_EA40(func, arg, sep, __VA_ARGS__) sep() func(arg, 40, x)
#define REVERSE_FOR_EA42(func, arg, sep, x, ...)  REVERSE_FOR_EA41(func, arg, sep, __VA_ARGS__) sep() func(arg, 41, x)
#define REVERSE_FOR_EA43(func, arg, sep, x, ...)  REVERSE_FOR_EA42(func, arg, sep, __VA_ARGS__) sep() func(arg, 42, x)
#define REVERSE_FOR_EA44(func, arg, sep, x, ...)  REVERSE_FOR_EA43(func, arg, sep, __VA_ARGS__) sep() func(arg, 43, x)
#define REVERSE_FOR_EA45(func, arg, sep, x, ...)  REVERSE_FOR_EA44(func, arg, sep, __VA_ARGS__) sep() func(arg, 44, x)
#define REVERSE_FOR_EA46(func, arg, sep, x, ...)  REVERSE_FOR_EA45(func, arg, sep, __VA_ARGS__) sep() func(arg, 45, x)
#define REVERSE_FOR_EA47(func, arg, sep, x, ...)  REVERSE_FOR_EA46(func, arg, sep, __VA_ARGS__) sep() func(arg, 46, x)
#define REVERSE_FOR_EA48(func, arg, sep, x, ...)  REVERSE_FOR_EA47(func, arg, sep, __VA_ARGS__) sep() func(arg, 47, x)
#define REVERSE_FOR_EA49(func, arg, sep, x, ...)  REVERSE_FOR_EA48(func, arg, sep, __VA_ARGS__) sep() func(arg, 48, x)
#define REVERSE_FOR_EA50(func, arg, sep, x, ...)  REVERSE_FOR_EA49(func, arg, sep, __VA_ARGS__) sep() func(arg, 49, x)
#define REVERSE_FOR_EA51(func, arg, sep, x, ...)  REVERSE_FOR_EA50(func, arg, sep, __VA_ARGS__) sep() func(arg, 50, x)
#define REVERSE_FOR_EA52(func, arg, sep, x, ...)  REVERSE_FOR_EA51(func, arg, sep, __VA_ARGS__) sep() func(arg, 51, x)
#define REVERSE_FOR_EA53(func, arg, sep, x, ...)  REVERSE_FOR_EA52(func, arg, sep, __VA_ARGS__) sep() func(arg, 52, x)
#define REVERSE_FOR_EA54(func, arg, sep, x, ...)  REVERSE_FOR_EA53(func, arg, sep, __VA_ARGS__) sep() func(arg, 53, x)
#define REVERSE_FOR_EA55(func, arg, sep, x, ...)  REVERSE_FOR_EA54(func, arg, sep, __VA_ARGS__) sep() func(arg, 54, x)
#define REVERSE_FOR_EA56(func, arg, sep, x, ...)  REVERSE_FOR_EA55(func, arg, sep, __VA_ARGS__) sep() func(arg, 55, x)
#define REVERSE_FOR_EA57(func, arg, sep, x, ...)  REVERSE_FOR_EA56(func, arg, sep, __VA_ARGS__) sep() func(arg, 56, x)
#define REVERSE_FOR_EA58(func, arg, sep, x, ...)  REVERSE_FOR_EA57(func, arg, sep, __VA_ARGS__) sep() func(arg, 57, x)
#define REVERSE_FOR_EA59(func, arg, sep, x, ...)  REVERSE_FOR_EA58(func, arg, sep, __VA_ARGS__) sep() func(arg, 58, x)
#define REVERSE_FOR_EA60(func, arg, sep, x, ...)  REVERSE_FOR_EA59(func, arg, sep, __VA_ARGS__) sep() func(arg, 59, x)
#define REVERSE_FOR_EA61(func, arg, sep, x, ...)  REVERSE_FOR_EA60(func, arg, sep, __VA_ARGS__) sep() func(arg, 60, x)
#define REVERSE_FOR_EA62(func, arg, sep, x, ...)  REVERSE_FOR_EA61(func, arg, sep, __VA_ARGS__) sep() func(arg, 61, x)
#define REVERSE_FOR_EA63(func, arg, sep, x, ...)  REVERSE_FOR_EA62(func, arg, sep, __VA_ARGS__) sep() func(arg, 62, x)
#define REVERSE_FOR_EA64(func, arg, sep, x, ...)  REVERSE_FOR_EA63(func, arg, sep, __VA_ARGS__) sep() func(arg, 63, x)

#define REVERSE_FOR_EA(N, func, arg, sep, ...) CONCAT(REVERSE_FOR_EA, N)(func, arg, sep, __VA_ARGS__)
#define REVERSE_FOR_EACH(func, arg, sep, ...) REVERSE_FOR_EA(NARG(__VA_ARGS__), func, arg, sep, __VA_ARGS__)

#define FIRST_ARG_(N, ...) N
#define FIRST_ARG(...) FIRST_ARG_(__VA_ARGS__, ignore)

#define VA_ARGS_MAP(x) x(0) \
                          x(0, 1) \
                          x(0, 1, 2) \
                          x(0, 1, 2, 3)\
                          x(0, 1, 2, 3, 4) \
                          x(0, 1, 2, 3, 4, 5) \
                          x(0, 1, 2, 3, 4, 5, 6) \
                          x(0, 1, 2, 3, 4, 5, 6, 7) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30) \
                          x(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31)
