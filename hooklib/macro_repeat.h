#pragma once
#ifndef _MACRO_REPEAT_
#define _MACRO_REPEAT_
/*使用方法


#define __tparam(n, ...) ""#n","
#define __tparam_end(n, ...) ""#n"\n"

#define repeat(n, ...) __repeat(n,__tparam,__tparam_end)

int _tmain(int argc, _TCHAR* argv[])
{
	printf( _T("%s\n"),__repeat(8,__tparam,__tparam_end));
	printf( _T("%s\n"),__for_each_number(repeat));
	system("pause");
	return 0;
}


输出:
1,2,3,4,5,6,7,8

1
1,2
1,2,3
1,2,3,4
1,2,3,4,5
1,2,3,4,5,6
1,2,3,4,5,6,7
1,2,3,4,5,6,7,8
1,2,3,4,5,6,7,8,9
1,2,3,4,5,6,7,8,9,10
1,2,3,4,5,6,7,8,9,10,11
1,2,3,4,5,6,7,8,9,10,11,12
1,2,3,4,5,6,7,8,9,10,11,12,13
1,2,3,4,5,6,7,8,9,10,11,12,13,14
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
*/
//////////////////////////////////////////////////////////////////////////
// 基础工具宏定义
#define __for_each_number(v, ...) \
    v(0, __VA_ARGS__) \
    v(1, __VA_ARGS__) \
    v(2, __VA_ARGS__) \
    v(3, __VA_ARGS__) \
    v(4, __VA_ARGS__) \
    v(5, __VA_ARGS__) \
    v(6, __VA_ARGS__) \
    v(7, __VA_ARGS__) \
    v(8, __VA_ARGS__) \
    v(9, __VA_ARGS__) \
    v(10, __VA_ARGS__) \
    v(11, __VA_ARGS__) \
    v(12, __VA_ARGS__) \
    v(13, __VA_ARGS__) \
    v(14, __VA_ARGS__) \
    v(15, __VA_ARGS__)
 
// 数值减 1 的常数
#define __cntdec_0 0
#define __cntdec_1 0
#define __cntdec_2 1
#define __cntdec_3 2
#define __cntdec_4 3
#define __cntdec_5 4
#define __cntdec_6 5
#define __cntdec_7 6
#define __cntdec_8 7
#define __cntdec_9 8
#define __cntdec_10 9
#define __cntdec_11 10
#define __cntdec_12 11
#define __cntdec_13 12
#define __cntdec_14 13
#define __cntdec_15 14
 
#define __cntdec(n) __cntdec_##n
// 连接两个符号
#define __connect2(x, y) x##y
#define __connect(x, y) __connect2(x, y)
 
// 生成不同个数的顺序符号
#define __repeat_0(m, ...)
#define __repeat_1(m, ...)    __repeat_0(m, __VA_ARGS__)  m(1, __VA_ARGS__)
#define __repeat_2(m, ...)    __repeat_1(m, __VA_ARGS__)  m(2, __VA_ARGS__)
#define __repeat_3(m, ...)    __repeat_2(m, __VA_ARGS__)  m(3, __VA_ARGS__)
#define __repeat_4(m, ...)    __repeat_3(m, __VA_ARGS__)  m(4, __VA_ARGS__)
#define __repeat_5(m, ...)    __repeat_4(m, __VA_ARGS__)  m(5, __VA_ARGS__)
#define __repeat_6(m, ...)    __repeat_5(m, __VA_ARGS__)  m(6, __VA_ARGS__)
#define __repeat_7(m, ...)    __repeat_6(m, __VA_ARGS__)  m(7, __VA_ARGS__)
#define __repeat_8(m, ...)    __repeat_7(m, __VA_ARGS__)  m(8, __VA_ARGS__)
#define __repeat_9(m, ...)    __repeat_8(m, __VA_ARGS__)  m(9, __VA_ARGS__)
#define __repeat_10(m, ...) __repeat_9(m, __VA_ARGS__)  m(10, __VA_ARGS__)
#define __repeat_11(m, ...) __repeat_10(m, __VA_ARGS__)  m(11, __VA_ARGS__)
#define __repeat_12(m, ...) __repeat_11(m, __VA_ARGS__)  m(12, __VA_ARGS__)
#define __repeat_13(m, ...) __repeat_12(m, __VA_ARGS__)  m(13, __VA_ARGS__)
#define __repeat_14(m, ...) __repeat_13(m, __VA_ARGS__)  m(14, __VA_ARGS__)
#define __repeat_15(m, ...) __repeat_14(m, __VA_ARGS__)  m(15, __VA_ARGS__)
 
#define __last_repeat_0(m, ...)
#define __last_repeat_1(m, ...)    m(1, __VA_ARGS__)
#define __last_repeat_2(m, ...)    m(2, __VA_ARGS__)
#define __last_repeat_3(m, ...)    m(3, __VA_ARGS__)
#define __last_repeat_4(m, ...)    m(4, __VA_ARGS__)
#define __last_repeat_5(m, ...)    m(5, __VA_ARGS__)
#define __last_repeat_6(m, ...)    m(6, __VA_ARGS__)
#define __last_repeat_7(m, ...)    m(7, __VA_ARGS__)
#define __last_repeat_8(m, ...)    m(8, __VA_ARGS__)
#define __last_repeat_9(m, ...)    m(9, __VA_ARGS__)
#define __last_repeat_10(m, ...) m(10, __VA_ARGS__)
#define __last_repeat_11(m, ...)  m(11, __VA_ARGS__)
#define __last_repeat_12(m, ...)  m(12, __VA_ARGS__)
#define __last_repeat_13(m, ...)  m(13, __VA_ARGS__)
#define __last_repeat_14(m, ...)  m(14, __VA_ARGS__)
#define __last_repeat_15(m, ...)  m(15, __VA_ARGS__)
 
#define __repeat(n, m_begin, m_end, ...) __connect(__repeat_, __cntdec(n))(m_begin, __VA_ARGS__) __connect(__last_repeat_, n)(m_end, __VA_ARGS__)

#endif //_MACRO_REPEAT_