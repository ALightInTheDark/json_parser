# json解析器

这是一个基于c++17的json解析器，使用string_view遍历和处理json字符串; 使用std::variant作为语法树的节点；使用编译期if和type_traits判断variant中存储的数据类型，将json语法树转化为json字符串。



## JSON语法格式

```cpp
https://www.json.org/json-zh.html
https://www.json.org/fatfree.html

JSON中，值是以下数据类型：
数字，布尔， NUll，字符串，数组，对象

object:
{
"name": "Jack (\"Bee\") Nimble",
"format": {
"type":       "rect",
"width":      1920,
"height":     1080,
"interlace":  false,
"frame rate": 24
}
}

array:
["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]

[
[0, -1, 0],
[1, 0, 0],
[0, 0, 1]
]
```



## 关于string_view

```cpp
// 前向移动视图起点 n 个字符。
constexpr void remove_prefix(size_type n);


std::string str = "   trim me";
std::string_view v = str;
v.remove_prefix(std::min(v.find_first_not_of(" "), v.size())); // 'trim me'

// 反向移动视图终点 n 个字符。
constexpr void remove_suffix(size_type n);

char arr[] = {'a', 'b', 'c', 'd', '\0', '\0', '\0'};
std::string_view v(arr, sizeof arr);
auto trim_pos = v.find('\0');
if(trim_pos != v.npos)
v.remove_suffix(v.size() - trim_pos); // 'abcd'
```



```cpp
#include <string_view>
#include <string>
#include <iostream>
using namespace std;

int main()
{
	string str("");
	cout << str.size() << endl; // 0
	cout << boolalpha << str.empty() << endl; // true
	cout << str[0] << endl; // 无输出

	string_view sv = "";
	cout << sv.size() << endl; // 0
	cout << boolalpha << sv.empty() << endl; // true
	cout << sv[0] << endl; // 无输出

	sv = "123";
	for (auto iter = sv.cbegin(); iter != sv.cend(); iter = sv.cbegin())
	{
		cout << *iter << " ";
		sv.remove_prefix(1); //  输出1 2 3
	}
	cout << endl;

	char arr[] = {'1', '2', '3', '\0', '4', '5', '6'};
	sv = arr;
	for (char ch : sv) { cout << ch << " "; } cout << endl; // 输出1 2 3
	cout << "sv.size(): " << sv.size() << endl; // 3

	str = arr;
	cout << str; cout << " str.size(): " << str.size() << endl; // 输出1 2 3 str.size(): 3
}
```



## 关于variant

std::variant可以持有任何类型的值；当前值的类型已知；variant可以被其他的类继承。我们可以使用variant处理不同类型的数据，并且不需要公共基类和指针。

variant的大小为所有可能的底层类型中最大值 加 一个记录当前类型的固定内存开销。variant不会分配堆内存。

```cpp
using Array = vector<unique_ptr<Value>>;
using Object = map<string, std::unique_ptr<Value>>;
variant<monostate, double, bool, string, Array, Object> value;
底层类型中最大的为map，占48字节。variant的大小为48+8 = 56字节
```

使用variant可以实现多态，这种方法最大的优势是不需要使用指针，没有了 new 和 delete 可能会减少很大开销（`vector<variant>`在堆内存上连续存储），也不会出现访问已释放内存或内存泄露等问题。然而，variant的大小都是所有可能的类型中最大的，当不同内置类型的大小差距很大时，使用variant会浪费很多内存。



## 关于_v

type_traits中带`_v`的萃取类，可以直接获取不带`_v`的萃取类的value。

```cpp
template<_From, _To> 
inline const bool is_convertible_v = is_convertible<_From, _To>::value 
```

