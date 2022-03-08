#include <bits/stdc++.h>
#include "json_parse.hpp"
using namespace std;

void test_double_auxiliary(double expect, string_view json_str)
{
	auto res = parse_json(json_str);
	string res_string = stringify(res);
	cout << "原浮点数是" << expect << " 解析后的字符串是" << res_string << endl;
}
void test_double()
{
	cout << "测试double: " << endl;
	test_double_auxiliary(0.0, "0.0");
	test_double_auxiliary(1.0, "1.0");
	test_double_auxiliary(-0.0, "-0.0");
	test_double_auxiliary(-1.0, "-1");
	test_double_auxiliary(1.5, "1.5");
	test_double_auxiliary(-1.5, "-1.5");
	test_double_auxiliary(3.1416, "3.1416");
	test_double_auxiliary(1E10, "1E10");
	test_double_auxiliary(1e10, "1e10");
	test_double_auxiliary(1E+10, "1E+10");
	test_double_auxiliary(1E-10, "1E-10");
	test_double_auxiliary(-1E10, "-1E10");
	test_double_auxiliary(-1e10, "-1e10");
	test_double_auxiliary(-1E+10, "-1E+10");
	test_double_auxiliary(-1E-10, "-1E-10");
	test_double_auxiliary(1.234E+10, "1.234E+10");
	test_double_auxiliary(1.234E-10, "1.234E-10");
	test_double_auxiliary(1.234E+10, "1.234E+10");
	test_double_auxiliary(1.234E-10, "1.234E-10");
	test_double_auxiliary(0.0, "1e-10000"); // must underflow
	test_double_auxiliary(std::numeric_limits<double>::max(),std::to_string(std::numeric_limits<double>::max()));
	test_double_auxiliary(std::numeric_limits<double>::min(),std::to_string(std::numeric_limits<double>::min()));
	test_double_auxiliary(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	test_double_auxiliary( 4.9406564584124654e-324, "4.9406564584124654e-324" ); /* minimum denormal */
	test_double_auxiliary(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	test_double_auxiliary( 2.2250738585072009e-308, "2.2250738585072009e-308" );  /* Max subnormal double */
	test_double_auxiliary(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	test_double_auxiliary( 2.2250738585072014e-308, "2.2250738585072014e-308" );  /* Min normal positive double */
	test_double_auxiliary(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	test_double_auxiliary( 1.7976931348623157e+308, "1.7976931348623157e+308" );  /* Max double */
	test_double_auxiliary(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}



void test_bool()
{
	cout << "测试bool: " << endl;

	auto res = parse_json("true");
	string res_string = stringify(res);
	cout << "期望的结果是true" << "  实际的结果是" << res_string << endl;

	res = parse_json("false");
	res_string = stringify(res);
	cout << "期望的结果是false" << "  实际的结果是" << res_string << endl;
}

void test_null()
{
	cout << "测试null: " << endl;

	auto res = parse_json("null");
	string res_string = stringify(res);
	cout << "期望的结果是null" << " 实际的结果是" << res_string << endl;
}


void test_string_auxiliary(string_view expect, string_view json_str)
{
	auto res = parse_json(json_str);
	string res_string = stringify(res);
	cout << "期望的结果是" << expect << " 实际的结果是" << res_string << endl;
}
void test_string()
{
	cout << "测试string: " << endl;
	test_string_auxiliary(R"("")", "\"\"");
	test_string_auxiliary(R"(""Hello")", "\"Hello\"");
	test_string_auxiliary(R"("Hello\nWorld")", "\"Hello\nWorld\"");
	test_string_auxiliary(R"("\" \\ / \b \f \n \r \t")", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}


void test_array_auxiliary(string_view expect, string_view json_str)
{
	auto res = parse_json(json_str);
	string res_string = stringify(res);;

	cout << "期望的结果是" <<expect << " 实际的结果是" << endl << res_string << endl;
}
void test_array()
{
	cout << "测试array: " << endl;
	test_array_auxiliary("[]", "[]");
	test_array_auxiliary("[ null , false ,true, 123.0, \"abc\"]", "[ null , false ,true, 123.0, \"abc\"]");
	test_array_auxiliary("[true,[true,false],true]", "[true,[true,false],true]");
	test_array_auxiliary(R"(["Hello","Wo\trld" ,0.9,true])", R"(["Hello","Wo\trld" ,0.9,true])");
	test_array_auxiliary("[ [  ] , [ 0  ] , [ 0 , 1  ] , [ 0 , 1 , 2  ]  ]", "[ [  ] , [ 0  ] , [ 0 , 1  ] , [ 0 , 1 , 2  ]  ]");
}


void test_object_auxiliary(string_view expect, string_view json_str)
{
	auto res = parse_json(json_str);
	string res_string = stringify(res);

	cout << "期望的结果是" << endl << expect << endl << " 实际的结果是" << endl << res_string << endl;
}
void test_object()
{
	cout << "测试object: " << endl;
	test_object_auxiliary(R"({"a":1.0})", R"({"a":1.0})");

	string str =
R"(
{
 "n" : null ,
 "f" : false ,
 "t" : true ,
 "i" : 123 ,
 "s" : "abc",
 "a" : [ 1, 2, 3  ],
 "o" : { "1" : 1, "2" : 2, "3" : 3  }
}
)";
	test_object_auxiliary(str, str);
}

void test_exception_aux(string_view json_str)
{
	Value res;
	try { res = parse_json(json_str); }
	catch (const exception& e) { cout << e.what() << endl; return; }
	cout << "未抛出异常，结果为" << stringify(res) << endl;
}
void test_exception()
{
	test_exception_aux("");
	test_exception_aux(" ");
	test_exception_aux("nul");
	test_exception_aux("tr");
	test_exception_aux("fals");
	test_exception_aux("???");
	test_exception_aux("true 18.2");

	test_exception_aux("2022e1000"); // todo: 数字溢出
	test_exception_aux("-2022e1000");

	test_exception_aux(R"({"num : 123})"); // 字符串缺失右引号
	test_exception_aux("\"abc");

	test_exception_aux(R"("num" : 123})");
	test_exception_aux(R"({"num" : 123)");

	test_exception_aux(R"(456, 123])");
	test_exception_aux(R"([456, 123)");

	test_exception_aux(R"("\v")"); // 错误的转义字符
	test_exception_aux(R"("\'")");
	test_exception_aux(R"("\0")");
	test_exception_aux(R"("\x12")");

	test_exception_aux("\"\x01\""); // todo: 非法字符的判断
	test_exception_aux("\"\x1F\"");
}


int main()
{
	test_null();
	test_bool();
	test_double();
	test_string();
	test_array();
	test_object();
	test_exception();
}
