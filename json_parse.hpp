// json
// Created by kiki on 2021/10/13.12:06
#pragma once
#include <string>
#include <string_view>
#include <charconv>
#include <map>
#include <vector>
#include <variant>
#include <type_traits>
#include <algorithm>
#include <exception>
#include <cmath>
using std:: variant, std::monostate, std::visit, std::get;
using std::is_convertible_v, std::remove_reference_t, std::remove_const_t, std::is_same_v;
using std::string, std::string_view, std::map, std::vector, std::move;
using std::isdigit, std::numeric_limits, std::min, std::all_of;
using std::exception;

class JsonException : public exception
{
private:
	string message; // todo: size_t err_index;指示出错的位置。
public:
	explicit JsonException(string mes) : message(move(mes)) { }
	[[nodiscard]] const char* what() const noexcept override{ return message.data(); }
};


class Value;
using Array = vector<Value>;
using Object = map<string, Value>;

class Value
{
private:
	variant<monostate, double, bool, string, Array, Object> value; // std::monostate模拟空类型

public:
	Value() = default; // variant的默认构造函数调用第一个选项类型的默认构造函数。即：模拟空类型monostate
	explicit Value(double num) : value(num) { } // fixme: 大整数转化为double时精度损失严重，考虑使用pair<long, double>
	explicit Value(bool b) : value(b) { }
	explicit Value(string&& str) : value(move(str)) { }
	explicit Value(Array&& arr) : value(move(arr)) { }
	explicit Value(Object&& obj) : value(move(obj)) { }
public:
	Value(const Value& val) = delete; // 禁止拷贝，只能移动
	Value& operator= (const Value& val) = delete;
	Value(Value&& val)  noexcept : value(move(val.value)){ }
	Value& operator= (Value&& val) noexcept { if (this != &val) { value = move(val.value); } return *this; }

	friend string stringify(const Value& root, int indent_count);
};

/* 将json字符串转化为树，树的节点类型为 class Value */
Value parse_json_aux(string_view& sv)
{
	auto skip =	// 跳过空格
	[&sv]() -> void
	{
		auto iter = sv.cbegin();
		while (iter != sv.cend() && isspace(*iter)) { ++iter; } // isspace检查给定的字符是否是空白字符。空白字符有: ' ', '\f', '\n', '\r', '\t', '\v'
		sv.remove_prefix(iter-sv.cbegin());
	};

	auto parse_number = // fixme : 使用std::from_chars
	[&sv]() -> double
	{
		double val = 0; //数字的值

		int sign = 1; // 正负号
		int scale = 0; // 小数点后几位
		int sub_sign = 1; // e的符号
		int sub_scale = 0; // e后面的小数点位数

		auto iter = sv.cbegin();

		if (*iter == '-') { sign = -1; ++iter; }
		else if (*iter == '+' || *iter == '0') { ++iter; }

		//if (iter == sv.cend() || !(isdigit(*iter) || *iter == '.')) { throw JsonException("invalid number"); }

		while(iter != sv.cend() && isdigit(*iter))
		{
			if (val > (numeric_limits<double>::max() -(*iter - '0')) / 10) { throw JsonException("number overflow"); }
			val = val * 10.0 + *iter - '0';
			++iter;
		}

		if (iter != sv.cend() && *iter == '.')
		{
			++iter; // 跳过'.'
			if (iter == sv.end() || !isdigit(*iter)) { throw JsonException("invalid number"); }

			while(iter != sv.cend() && isdigit(*iter))
			{
				val = val * 10.0 + *iter - '0';
				--scale;
				++iter;
			}
		}

		if ( iter != sv.cend() && (*iter == 'e' || *iter == 'E') )
		{
			++iter; // 跳过'e'或'E'
			if (iter != sv.cend() && *iter == '+') { ++iter; }
			if (iter != sv.cend() && *iter == '-') { sub_sign = -1; ++iter; }
			if (iter == sv.end() || !isdigit(*iter)) { throw JsonException("invalid number"); }

			while (iter != sv.cend() && isdigit(*iter))
			{
				if (sub_scale > (numeric_limits<double>::max() -(*iter - '0')) / 10) { throw JsonException("number overflow"); }
				sub_scale = sub_scale * 10 + *iter - '0';
				++iter;
			}
		}

		sv.remove_prefix(iter-sv.cbegin());

		val = sign * val * pow(10.0, scale + sub_sign * sub_scale); // fixme: pow函数返回错误、最终结果溢出返回错误

		return val;
	};

	auto parse_bool =
	[&sv]() -> bool
	{
		auto iter = sv.cbegin();
		if (*iter == 't')
		{
			if (iter + 3 < sv.cend() && sv.substr(0, 4) == "true")
			{
				sv.remove_prefix(4);
				return true;
			}
			else { throw JsonException("error type: true"); }
		}
		else // *iter == 'f'
		{
			if (iter + 4 < sv.cend() && sv.substr(0, 5) == "false")
			{
				sv.remove_prefix(5);
				return false;
			}
			else { throw JsonException("error type: false"); }
		}
	};

	auto parse_null =
	[&sv]() -> void
	{
		auto iter = sv.cbegin();
		if (iter + 3 < sv.cend() && sv.substr(0, 4) == "null")
		{
			sv.remove_prefix(4);
			return;
		}
		throw JsonException("error type: null");
	};

	auto parse_string =
	[&sv]() -> string
	{
		string str;

		auto iter = sv.cbegin() + 1; // 跳过'\"'
		for (; iter != sv.cend() && *iter != '\"'; ++iter)
		{
			if (*iter == '\\')
			{
				++iter;	if (iter == sv.cend()) { throw JsonException("type string: missing escape character"); }
				switch (*iter)
				{
					case '\"': str.push_back('\"'); break;
					case '/': str.push_back('/'); break;
					case '\\': str.push_back('\\'); break;
					case 'b': str.push_back('\b'); break;
					case 'f': str.push_back('\f'); break;
					case 'n': str.push_back('\n'); break;
					case 'r': str.push_back('\r'); break;
					case 't': str.push_back('\t'); break;
					default: throw JsonException("type string: unknown escape character");
				}
			}
			else { str.push_back(*iter); }
		}

		if (iter == sv.cend()) { throw JsonException("type string: missing right quotation marks"); } // 字符串缺失右引号
		sv.remove_prefix(++iter - sv.begin()); //跳过字符串结尾的'\"'

		return str;
	};

	auto parse_array =
	[&sv, &skip]()
	{
		Array array;
		sv.remove_prefix(1); // 跳过'['
		skip();

		while (!sv.empty() && *sv.begin() != ']')
		{
			if (*sv.begin() == ',') {  sv.remove_prefix(1); skip(); }
			array.emplace_back( parse_json_aux(sv) );
			skip();
		}

		if (sv.empty()) { throw JsonException("type array : missing rignt bracket"); }

		sv.remove_prefix(1); //跳过数组结尾的 ]

		return array;
	};

	auto parse_object =
	[&sv, &skip, &parse_string]()
	{
		Object object;

		sv.remove_prefix(1); skip(); //跳过 { 及其后的空白

		while (!sv.empty() && *sv.begin() != '}')
		{
			if (*sv.begin() == '\"')
			{
				string key;
				try{ key = parse_string(); }  catch(...) { throw JsonException("type object: invalid key"); }
				skip(); //跳过 : 之前的空格
				if (sv.empty() || *sv.begin() != ':') {  throw JsonException("type object: missing colon between key and value"); }
				sv.remove_prefix(1); skip(); //跳过 : 及其后的空格
				object[key] = parse_json_aux(sv);
			}
			else if (*sv.begin() == ',') { sv.remove_prefix(1); skip(); } //跳过,及它之后的空白
			else
			{
				skip();
			}
		}

		if (sv.empty()) { throw JsonException("type object: missing right brace"); }

		sv.remove_prefix(1); //跳过'}'

		return object;
	};


	skip();
	if (sv.empty()) { throw JsonException("json_str is empty, expect a value"); }

	char ch = sv[0];
	switch (ch)
	{
		case 'n' : parse_null(); return {}; // null
		case 't' : case 'f' : return Value(parse_bool()); // bool
		case '\"' : return Value(parse_string()); // 字符串
		case '[' : return Value(parse_array());
		case '{' : return Value(parse_object());
		default:
			if (ch == '-' || ch == '+' || (ch >= '0' && ch <= '9')) {  return Value(parse_number()); } // 数字
			else { throw JsonException("unknown type: invalid value"); }
	}
}
#include <iostream>
using namespace std;
Value parse_json(string_view sv)
{
	auto value= parse_json_aux(sv);
	if (!sv.empty() && !all_of(sv.cbegin(), sv.cend(), [](char ch){ return isspace(ch); }) )
		{ throw JsonException("Multiple roots appear in the syntax tree"); }

	return value;
}

/* 传入树的头节点，将树转化为json字符串 (stringify). indent_count为初始缩进格数 */
string stringify(const Value& root, int indent_count = 0)
{
	string out; // 最终返回的结果
	auto write_indent = [](string& str, int count = 0) { while (count--) { str.push_back('\t'); } };
	auto write_string = [](string& to, const string& from)
	{
		to.push_back('\"');
		for (char ch: from)
		{
			switch (ch)
			{
				case '\\' : to.push_back('\\'); to.push_back('\\'); break;
				case '\"' : to.push_back('\\'); to.push_back('\"'); break;
				case '\b' : to.push_back('\\'); to.push_back('b'); break;
				case '\f' : to.push_back('\\'); to.push_back('f'); break;
				case '\n' : to.push_back('\\'); to.push_back('n'); break;
				case '\r' : to.push_back('\\'); to.push_back('r'); break;
				case '\t' : to.push_back('\\'); to.push_back('t'); break;
				default: to.push_back(ch); break;
			}
		}
		to.push_back('\"');
	};

	auto stringify_aux = [&](auto& val)
	{
		if constexpr(is_convertible_v<decltype(val), string>)
		{
			write_string(out, val);
		}
		else if constexpr(is_same_v<remove_const_t<remove_reference_t<decltype(val)>>, double>)
		{
			char buf[64];
			snprintf(buf, sizeof(buf), "%.17g", val);
			out += buf;
		}
		else if constexpr(is_convertible_v<decltype(val), bool>)
		{
			out += val ? "true" : "false";
		}
		else if constexpr(is_convertible_v<decltype(val), Array>)
		{
			write_indent(out, indent_count); out.push_back('['); if (val.empty()) { out.push_back(']'); return; }

			bool once = true;
			for (auto& value : val)
			{
				if (once) { once = false; }
				else{ out.push_back(','); }

				out.push_back('\n'); write_indent(out, indent_count);

				out.append(stringify(value, indent_count + 1));
			}

			out.push_back('\n');

			write_indent(out, indent_count); out.push_back(']');
		}
		else if constexpr(is_convertible_v<decltype(val), Object>)
		{
			out.push_back('{'); if (val.empty()) { out.push_back('}'); return; }

			bool once = true;
			for (auto& pair : val)
			{
				if (once) { once = false; }
				else{ out.push_back(','); }

				out.push_back('\n'); write_indent(out, indent_count);

				write_string(out, pair.first);
				out.push_back(':');
				out.append(stringify(pair.second, indent_count + 1));
			}

			out.push_back('\n'); write_indent(out, indent_count);
			out.push_back('}');
		}
		else
		{
			out += "null";
		}
	};

	visit(stringify_aux, root.value);

	return out;
}