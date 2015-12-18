---
layout: post
title: 현대 C++로 Python의 enumerate 만들어보기
---

- [C++11으로 파이썬의 range 흉내내기](/2015/12/03/python-range-c++11/)를 먼저 읽어 볼 것을 추천합니다.
- 기본적인 우측값 참조에 대한 개념은 있다고 가정합니다.
- 경고: 매우 긴 글입니다.

<!-- <span style="font-size: 1.5rem; vertical-align: -0.25rem">☞</span>  -->

파이썬 내장 함수 중 `enumerate`가 있다. [이전 글](/2015/12/03/python-range-c++11/)에서 유도 변수(induction variable)에 대해 설명했다. 이 파이썬 함수는 유도 변수 없이도 루프에서 순환마다 하나씩 증가하는 인덱스를 쓸 수 있게 해준다. [함수 원형](https://docs.python.org/3/library/functions.html#enumerate)과 사용 예는 이러하다.

<pre>
<b>enumerate</b>(<i>iterable</i>, <i>start=0</i>)
</pre>

{% highlight bash %}
$ python3
>>> A = ["foo", "bar"]
>>> for i, e in enumerate(A, 1):
...     print("{}: {}".format(i, e))
...
1: foo
2: bar
>>>
{% endhighlight %}

유도 변수를 피할 수 있다면 피하는 것이 좋다는 건 이전 글에서 잘 설명하였다. 그런데 위 예 같이 순환마다 인덱스를 얻고 싶다면 결국 유도 변수를 써야 할 것 같다. 다행히 파이썬에는 `enumerate`로 현재 순환 내에서만 유효한 인덱스 변수, `i`를 얻어낼 수 있다. 파이썬을 부러워만 할 필요 없다. C++11/14로 `enumerate`를 얼마든지 만들 수 있다.

### 1. C++ enumerate 요구조건 정하기

개념적으로 C++ `enumerate`는 범위 기반 for 루프에서 주어진 컨테이너의 원소와 하나씩 증가하는 인덱스 변수를 순환마다 함께 돌려준다. 언뜻 간단해 보이지만 `enumerate`는 고려할 점이 정말 많다. `enumerate`가 C++ 코드에서 어떻게 쓰일지부터 생각한다.

아마도 가장 기본적인 예는 아래처럼 될 것이고 출력도 파이썬의 결과와 같을 것이다.

{% highlight C++ linenos=table%}
std::vector<std::string> A = {"foo", "bar"};
// 유도 변수를 이용한 방법
// for (size_t i = 0; i < A.size(); ++i)
//   std::cout << i << ": " << A[i] << '\n';
//
// Python의 "for i, e in enumerate(A, 1)"과 비슷하게
for (std::pair<size_t, std::string&> p : enumerate(A, 1))
  std::cout << p.first << ": " << p.second << '\n';
{% endhighlight %}
{% include code_caption.html caption="가장 기본적인 C++ enumerate 사용 예" %}

이 외에도 다양한 형태로 `enumerate`를 사용할 수 있을 것이다. 자세한 논의를 하려면 다음 두 가지를 생각해야 한다:

- 순환마다 얻는 `std::pair<>`에 해당하는 인덱스-원소 쌍 타입.
- 함수 인수(argument) `A` 자리에 허용되는 컨테이너 타입.

#### `enumerate`의 인덱스-원소 쌍의 타입

파이썬은 동적 언어이고 복수 반환 값이 허용되므로[^mutiret] 우아하게 `for i, e in enumerate(A)` 같이 표현된다. 가엾은 C++은 어쩔 수 없이 `std::pair`나 `std::tuple` 같은 것으로 두 개의 다른 타입 변수를 묶어야 한다. 직접 이런 클래스를 만들 수도 있을 것이다. 이 구현에서는 간단한 `std::pair`를 선택한다.

[^mutiret]: 복수 반환 값은 C++에서는 원래 안 되지만, C++11의 `std::tuple`과 `std::tie`로 [대략 흉내](http://en.cppreference.com/w/cpp/utility/tuple/tie)를 낼 수 있다. 안타깝게도 `tie` 묘수를 범위 기반 for 루프에는 쓸 수가 없다.

인덱스 타입은 플랫폼에서 최대 객체 크기까지 표현할 수 있는 <a href="http://en.cppreference.com/w/cpp/types/size_t"><code>size_t</code></a>으로 골랐다. STL 컨테이너는 별도로 `container::size_type`이라는 타입을 두는데 `size_t`와 [사실상  같다](http://stackoverflow.com/questions/918567/size-t-vs-containersize-type). `size_t`는 배열 인덱스로도 안전하게 쓰이므로 이 값을 쓰는 것이 타당해 보인다. 파이썬의 `enumerate`는 시작 오프셋으로 음수 값도 줄 수 있다. 음수까지 지원하고 싶으면 부호 있는 `size_t`인  POSIX의 <a href="http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_types.h.html"><code>ssize_t</code></a>로 선언하면 될 것이다.

코드 1에서 벡터 `A`의 원소를 참조자 형태, `string&`로 받았다. 벡터 `A`가 상수형이 아니므로 참조자로 루프를 돌면서 직접 원솟값을 변경할 수 있다. 읽기만 한다면 `const string&`으로 받으면 좋다. 아니면 그냥 `string`으로 복사 받아 원 벡터에 영향 없이 변경할 수도 있다. 원소는 값, 상수 참조, - 컨테이너가 수정을 허용한다는 조건 하에 - 일반 참조가 모두 가능해야 한다. 그런데 값 형태는 얼마든지 참조자로부터 복사해서 얻을 수 있다. 결과적으로 참조자 또는 상수 참조자로만 받아도 된다.
파이썬과 다르게 이런 부분에선 C++의 자유도가 확실히 높다. 물론 실수와 어려움의 대가가 따르지만.[^proxy]

[^proxy]: `vector<bool>` 같이 프락시(proxy) 반복자를 이용하는 컨테이너에서는 `bool&`로 받을 수 없다. 이 프락시 반복자는 널리 알려진 [악명 높은 문제](http://www.gotw.ca/publications/mill09.htm)로 지금 프락시 반복자를 [표준에 넣는 노력](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0022r1.html)도 진행 중이다.

정리하자면:

- 반복자(iterator)의 인덱스 타입은 `size_t`.
- 원소 타입은 상수 참조자, 때에 따라 참조자가 되며 이 둘은 `std::pair<IndexType, ElementType>`으로 묶는다.
`ElementType`<small>(이하 `ElemType`)</small>의 상수 여부는 일반적으로 입력 컨테이너의 상수 성질이 결정한다.[^constkey]

[^constkey]: 예외가 있다. 대표적으로 `std::map`, `std::unordered_map` 같은 키/값 쌍을 저장하는 연관 자료 구조는 상수 반복자가 아니어도 키는 항상 상수이다. `std::map<string, int>::iterator`의 타입은 `pair<const string, int>`이다. 상수형인 이유는 키를 변경하려면 컨테이너 내부 자료 구조도 반드시 바뀌어야 하기 때문이다.

여기서 `for (pair<size_t, ElemType> e : enumerate(A))` 대신에 간편한 `for (auto e : enumerate(A))`로 쓰면 안 되냐라고 물을 수 있다. 당연히 된다. 다만 이럴 때는 `ElemType`에 해당하는 부분을 맘대로 조절할 수 없게 된다. 테스트 케이스에서 `auto` 사용 예를 볼 것이다.

#### 어떤 컨테이너가 가능한가? 배열은 되는지?

코드 1에서는 간단하게 `vector<string>`를 컨테이너로 했는데, `A`에 올 수 있는 타입은 어떤 것일까? 가능한 타입의 상한선은 명확하다: _범위 기반 for 루프에 쓸 수 있는 모든 것이다._
범위 기반 루프의 [정의](/2015/12/03/python-range-c++11#range-for-definition)를 상기하며 하나씩 생각해보자. 최종 목표는 `enumerate`가 이 모든 것을 지원하는 것이다.

1. STL 자료구조 중 컨테이너 어댑터인 `stack`, `queue`, `priority_queue`를 제외한 [모든 컨테이너](http://en.cppreference.com/w/cpp/container), 예를 들어, `list`, `unordered_map`을 범위 기반 루프에서 쓸 수 있다. 이 두 그룹의 표면적 차이는 컨테이너의 `begin()`과 `end()` 메서드를 지원 유무이다. 일례로 `stack`은 `begin()` 메서드가 없을뿐더러 전역 함수 `std::begin` 역시 `stack` 타입을 받을 수 없다.  엄밀히 말하면 [forward iterator](http://en.cppreference.com/w/cpp/concept/ForwardIterator)가 있는 컨테이너는 범위 기반 for 루프에서 쓸 수 있다.

2. C/C++ 일반 배열도 범위 기반 for 루프에서 쓸 수 있다. 전역 컨테이너 접근 함수 <code><a href="http://en.cppreference.com/w/cpp/iterator/begin">std::begin</a></code>과 <code><a href="http://en.cppreference.com/w/cpp/iterator/end">std::end</a></code>도 배열을 받을 수 있다. 힌트를 주자면 `std::begin`과 `std::end`는 이번 구현에서 중추적인 역할을 한다. 두 함수의 원형과 사용법을 꼭 보고 자주 쓸 것을 권장한다.

3. 컨테이너 `A`는 `const` 타입도 될 수 있다. 이럴 때는 순회할 때 `A`의 원소 변경을 막아야 한다. 일반적으로 `volatile`을 포함한 [cv qualifiers](http://en.cppreference.com/w/cpp/language/cv)가 허용된다. C++ 컨테이너에서 `volatile`을 쓰는 일은 거의 없긴 할 것이다.

4. 이전 글에서 만든 `range`처럼 사용자 정의 타입도 조건만 맞는다면 범위 기반 for 루프에 쓸 수 있다. 반드시 STL 컨테이너나 `std::iterator` 인터페이스를 따를 필요가 없다.

5. `A` 같은 변수를 거치지 않고 컨테이너를 `enumerate` 내에서 정의할 수 있다. 함수 리턴값으로도 바로 받을 수 있다. 이 부분에서 이해하기 참 까다로운 C++11의 우측값 참조(rvalue reference), 포워딩/유니버셜 레퍼런스(뒤에서 설명), 퍼펙트 포워딩 같은 것들이 나온다. 이 글에서도 대략 이야기할 것이다.

6. C++11부터 지원되는 초기화 리스트([initializer list](http://en.cppreference.com/w/cpp/utility/initializer_list))도 범위 기반 루프에 쓸 수 있다.

이상적으로 `enumerate`도 이 모든 것을 지원하면 좋다. 한번에 모든 것을 구현하기 어려우니 하나씩 구현할 것이다.


### 2. 테스트 케이스 작성

앞서 설계한 요구 조건을 검사할 수 있는 테스트 케이스부터 만든다. 각각의 예상 결과도 함께 나열하였다.

#### 테스트 케이스 1

{% highlight C++ linenos=table%}
// 1. Forward iterator를 얻을 수 있는 STL 컨테이너
cout << "[TEST 1] vector<string>\n";
vector<string> A = {"foo", "bar", "baz"};
// 참조자로 직접 A 내용 수정.
for (pair<size_t, string&> p : enumerate(A))
  cout << p.first << ": " << (p.second += p.second) << '\n';

// 수정 내역 확인: 벡터 원소를 상수 참조자로 받음.
for (pair<size_t, const string&> p : enumerate(A))
  cout << p.first << ": " << p.second << '\n';

// 백터 원소를 복사해서 값으로 받음. A는 영향 없음.
for (pair<size_t, string> p : enumerate(A))
  cout << p.first << ": " << (p.second += p.second) << '\n';

// auto로도 받을 수 있음: p의 타입은 pair<size_t, string&>
for (auto p : enumerate(A))
  cout << p.first << ": " << p.second << '\n';
{% endhighlight %}

<pre>
[TEST 1] vector&lt;string&gt;
0: foofoo
1: barbar
2: bazbaz
0: foofoo
1: barbar
2: bazbaz
0: foofoofoofoo
1: barbarbarbar
2: bazbazbazbaz
0: foofoo
1: barbar
2: bazbaz
</pre>

명시적인 `pair<size_t, ElemType>` 대신에 `auto`를 쓴 예도 포함했다. 어떤 타입으로 추론되었는지 알려면 C++ 자체의 `typeid`를 쓸 수도 있는데 참조자와 cv 한정자가 생략되는 터라 boost의 `type_id_with_cvr`를 써야 정확한 타입을 알 수 있다. 자세한 내용은 [Effective Modern C++](http://shop.oreilly.com/product/0636920033707.do)의 Item 4를 참조하면 좋다.

#### 테스트 케이스 2

{% highlight C++ linenos=table%}
// 2. 일반 배열 예
cout << "[TEST 2] array\n";
string C[] = {"foo", "bar", "baz"};
// auto&&로 받는 것이 범위 기반 for 문에서 일반적이고 효율적인 방법
// p 타입: pair<size_t, string&>&&, 원소 타입이 string&로 추론
for (auto&& p : enumerate(C, 100))
  cout << p.first << ": " << (p.second += p.second) << '\n';
for (auto&& p : enumerate(C, 100))
  cout << p.first << ": " << p.second << '\n';
{% endhighlight %}

<pre>
[TEST 2] array
100: foofoo
101: barbar
102: bazbaz
100: foofoo
101: barbar
102: bazbaz
</pre>

라인 6을 보면 `auto&& p`로 받아 쓰고 있다. 자세한 설명[^rangefor]은 생략하고 요약하자면, `auto&&`로 받는 것이 범위 기반 for 문에서 일반적이면서 안전하고 효율적인 방법이다.
그냥 `for (auto p : enumerate(A))`로 쓰면 `pair<..>` 부분이 복사되는 오버헤드가 있으나 `pair` 자체는 가볍게 복사되므로 의도적으로 `auto&&`를 처음부터 쓰지는 않았다. 라인 6과 8에서 p의 타입은 우측값 참조, `pair<size_t, string&>&&`가 되며 `ElemType`은 참조자로 받기로 설계했으니 `string&` 타입으로 추론될 것이다.

[^rangefor]: 스택오버플로우의 [답변](http://stackoverflow.com/questions/15927033/what-is-the-correct-way-of-using-c11s-range-based-for)을 참고하면 좋다. 아예 C++ 표준으로 `for (elem : range)` 처럼 쓰면 자동으로 `for (auto&& elem : range)`가 되도록 하자는 [제안도 논의 중](http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2014/n3853.htm)이다.


#### 테스트 케이스 3
{% highlight C++ linenos=table%}
// 3. const 예제
cout << "[TEST 3] const\n";
const string E[] = {"foo", "bar", "baz"};
// decltype(p) == pair<size_t, string const&>&&
// p 자체는 상수가 아니므로 인덱스값은 변경 가능, 배열 값은 수정 불가.
for (auto&& p : enumerate(E))
  cout << (p.first += 1) << ": " << p.second << '\n';
{% endhighlight %}

<pre>
[TEST 3] const
1: foo
2: bar
3: baz
</pre>

`E`는 상수 문자열 배열이므로 `pair`의 `ElemType`은 상수 참조자, `const string&`로 얻어질 것이다. 그러나 p 자체는 상수가 아니므로 인덱스는 변경 가능하다.

#### 테스트 케이스 4

{% highlight C++ linenos=table%}
// 4. 앞서 구현한 range 사용 예
cout << "[TEST 4] range\n";
auto&& D = range(100, 103);
// decltype(p) == pair<size_t, int>&&
for (auto&& p : enumerate(D))
  cout << p.first << ": " << p.second << '\n';
{% endhighlight %}

<pre>
[TEST 4] range
0: 100
1: 101
2: 102
</pre>

앞서 만든 C++ `range` 객체를 `D`로 받아 쓰는 테스트이다. 유의할 점은 원소 타입이 우리가 설계했던 참조자 형태가 아니라 `int` 값이라는 점이다. 그 이유는 `range_iterator`의 `operator*`가 `int`를 [반환하기 때문](/2015/12/03/python-range-c++11#range-iterator-class)이다. 이 글 뒤에서 이 부분에 대해 더 자세히 논의할 것이다.

#### 테스트 케이스 5
{% highlight C++ linenos=table%}
// 5. 변수를 거치지 않고 직접 사용
cout << "[TEST 5] in-place through rvalue reference\n";
for (auto&& p : enumerate(range(100, 103)))
  cout << p.first << ": " << p.second << '\n';

// decltype(p) == pair<size_t, string&>&&
for (auto&& p : enumerate(vector<string>{"foo", "bar", "baz"}))
  cout << p.first << ": " << (p.second += p.second) << '\n';

// 함수 반환값 직접 사용
auto create = []()->vector<string> { return {"foo", "bar", "baz"}; };
// decltype(p) == pair<size_t, string&>&&
for (auto&& p : enumerate(create()))
  cout << p.first << ": " << p.second << '\n';
{% endhighlight %}

<pre>
[TEST 5] in-place through rvalue reference
0: 100
1: 101
2: 102
0: foofoo
1: barbar
2: bazbaz
0: foo
1: bar
2: baz
</pre>

테스트 5는 컨테이너나 배열을 명시적인 변수를 거치지 않고 임시 변수를 바로 사용하는 예이다. C++11부터 도입된 우측값 참조, 무브 시맨틱(move semantics), 퍼펙트 포워딩(perfect forwarding)으로 이제 이럴 때도 효율적으로 구현할 수 있다.

#### 테스트 케이스 6
{% highlight C++ linenos=table%}
// 6. 초기화 리스트
cout << "[TEST 6] initializer list\n";
for (auto&& p : enumerate({"foo", "bar", "baz"}))
  cout << p.first << ": " << p.second << '\n';
{% endhighlight %}

<pre>
[TEST 6] initializer list
0: foo
1: bar
2: baz
</pre>

초기화 리스트 덕에 C++에서도 파이썬처럼 `{"foo", "bar"}`로 `vector<string>`을 초기화할 수 있다. 예전에는 일일이 `push_back`를 불러야만 했다.

참조자, 상수 컨테이너, 배열, 우측값 등 여러 사항을 고려하니 테스트 케이스가 짧지 않았다. 코딩할 시간이다.

### 3. `enumerate` 함수 인터페이스 구현

`enumerate`는 임의의 컨테이너 타입을 받는 템플릿 함수로 설계한다. `range`와 비슷하게 `enumerate` 함수 얼개를 만들어보자.

{% highlight C++ linenos=table%}
template<typename C /* 컨테이너 타입 */>
// 좌측값 참조자만 받는 원형으로 테스트 케이스 1~4까지만 실행가능.
enumerate_impl<C> enumerate(C& container, size_t start = 0) {
  return enumerate_impl{/* TODO: arguments... */}
}
{% endhighlight %}
{% include code_caption.html caption="enumerate 함수 얼개" %}

거의 팩토리 함수이므로 입력 인자를 받아 구현 객체인 `enumerate_impl`을 돌려준다.
라인 3을 보면 일반적인 참조자, `C& container`로 받았다. 그래야 입력 컨테이너의 불필요한 복사를 막고, `C`가 비상수형일 때 원소 변경도 할 수 있다. `C`가 상수형이면 어떻게 될까? 템플릿 인수 연역 법칙(template argument deduction)에 따르면[^deduct], `C`가 `const vector<string>` 같은 상수형이면 `C& container` 역시 상수형 참조자로 넘어간다. 직관적이다. 굳이 `enumerate(const C& container)` 버전을 오버로드할 필요가 없다. 물론 오버로드할 수는 있고, `C`가 상수형이면 `const` 함수가 먼저 선택된다.
테스트 케이스 1~4까지는 이걸로 해결될 것이다.

[^deduct]: 템플릿 인수 연역의 대부분은 직관적이지만 몇몇 규칙은 반드시 알아야 템플릿 프로그래밍을 제대로 할 수 있다. 대표적으로 만약 <code>enumerate(T container)</code>처럼 값으로 쓰인다면, <code>T</code>에 붙어있는 <code>const</code>와 <code>volatile</code> 속성, 다시 말해, cv 한정자(<a href="http://en.cppreference.com/w/cpp/language/cv">cv qualifiers</a>), 그리고 참조자 기호(`&`, `&&`)는 무시되는 법칙이 있다.  [레퍼런스](http://en.cppreference.com/w/cpp/language/template_argument_deduction)는 읽기가 어려우므로 [요약 슬라이드](http://www.aristeia.com/TalkNotes/C++TypeDeductionandWhyYouCareCppCon2014.pdf){:target="_blank"}를 추천한다.

[^ignorecv]: `template<typename T> void foo(T t);`에서 `t`는 복사로 만들어진 새로운 객체이다. 그런데 굳이 `T`의 `const` 같은 성질까진 가져올 필요 없다는 이유에서 이런 법칙이 나왔다. 반면, 참조자나 포인터 형일 때는 cv 한정자를 유지한다. 법칙만 보면 왜 갸우뚱한데 곰곰히 생각하면 그럴듯한 이유가 있긴 하다.

이미 현대 C++을 잘 아시는 분, 혹은 Effective Modern C++ 책을 읽으신 분은 이것만으로는 테스트 케이스 5가 안 됨을 알 것이다. `enumerate`가 지금 좌측값 참조(lvalue reference)만 받을 수 있어서 우측값(rvalue)인 함수 반환 값이나 임시 객체를 함수 인자로 받을 수 없다. 이럴 때는 우측값 참조(rvalue reference), 더 나가 포워딩/유니버셜 레퍼런스(forwarding/universal reference)를 써야 하는데 이 문제는 마지막에 생각할 것이다.
C++11의 아마도 가장 큰 변화인 우측값 참조와 무브 시맨틱(move semantics)은 C++를 잘하시는 분들도 어려움을 많이 겪는다. 일단 복잡한 템플릿 이야기는 잠시 접어 두고 `enumerate` 구현에 집중한다.

기억을 되살리고자 `enumerate`가 범위 기반 루프에서 어떻게 확장되는지 복습하자.

<p><div class="highlight"><pre style="padding: .25rem .5rem;">
for ( <span class="k"><i>std::pair<size_t, string&> p</i></span> : <span class="k"><i>enumerate(A, 1)</i></span> ) <span class="k"><i>foo(p);</i></span>
</pre></div></p>

<p><div class="highlight"><pre style="padding: .25rem .5rem;">
{
  <span class="c1">// __range의 타입은 enumerate_impl 클래스</span>
  auto&&  __range = <span class="k"><i>enumerate(A, 1)</i></span>;
  <span class="c1">// __begin, __end의 타입은 enumerate_iterator 클래스</span>
  for ( auto __begin = <span class="mi">__range.begin()</span>,
             __end = <span class="mi">__range.end()</span>;
        __begin != __end;
        ++__begin ) {
    <span class="k"><i>std::pair<size_t, string&> p</i></span> = *__begin;
    <span class="k"><i>foo(p);</i></span>
  }
}
</pre></div></p>
{% include code_caption.html caption="범위 기반 for 루프의 확장" %}

구현할 것은 `range` 구현과 비슷하게 아래와 같다:

- `enumerate_impl` 클래스: `begin()`과 `end()` 메서드 구현.
- `enumerate_iterator` 클래스: `!=`, `++`, `*` 연산자 구현.


### 4. `enumerate_impl` 클래스 구현

`enumerate_impl`은 주어진 컨테이너로부터 시작과 끝 반복자를 반환해야 한다. 이 클래스를 어떻게 설계해야 할까?

<p><center><i>잠시 생각할 시간을 가져보세요...</i></center></p>

아마도 가장 좋은 방법은 이미 힌트를 준 <code><a href="http://en.cppreference.com/w/cpp/iterator/begin">std::begin</a></code>과 <code><a href="http://en.cppreference.com/w/cpp/iterator/end">std::end</a></code>를 쓰는 것이다.
`enumerate`에서 받은 컨테이너 변수를 참조자로 가지면서(복사 방지) 이 두 전역 함수를 호출한다. 다른 방법으로는 `enumerate`가 직접 컨테이너의 시작과 끝을 구해 `enumerate_impl`에 넘겨주는 방법도 있다. 하지만 전자의 방식이 더 효과적인데, 그 이유는:

- `enumerate_impl` 클래스의 멤버 변수로 컨테이너 참조자와 시작 오프셋만 있으면 되고,
- `std::begin`과 `std::end`의 도움으로 상수형 컨테이너뿐만 아니라 일반 배열도 한 번에 처리된다.

아직 테스트 케이스 5와 6의 우측값과 초기화 리스트는 고려하지 않았음을 기억하고 구현한다.

{% highlight C++ linenos=table%}
template<typename C /* 컨테이너 타입 */>
class enumerate_impl {
  C& container_; // 컨테이너를 참조자로 보관
  const size_t start_;

public:
  enumerate_impl(C& container, size_t start) :
    container_{container}, start_{start} {}

  enumerate_iterator<C /* 아니면 다른 타입?? */> begin() {
    return {std::begin(container_), start_};
  }

  enumerate_iterator<C> end() {
    return {std::end(container_), 0 /* 중요하지 않음 */};
  }
};

enumerate_impl<C> enumerate(C& container, size_t start = 0) {
  return {container, start};
}
{% endhighlight %}
{% include code_caption.html caption="첫 번째 enumerate_impl 구현" %}


템플릿 인자는 `enumerate`와 같이 컨테이너 타입 `C`만 받도록 했다. 생성자는 `enumerate`에서 받은 컨테이너를 그대로 전달받아 참조자로 저장했고, 오프셋 변수인 `start`는 더는 변경할 일이 없으므로 상수로 했다.

이 클래스의 핵심인 `enumerate_imple::begin/end`는 `std::begin/end` 함수로부터 `container_`의 시작/끝 반복자를 각각 얻는다. 여기에 인덱스 변수 값을 더해 `enumerate_iterator` 생성자를 부르도록 만들었다. 이처럼 구현이 간단한 이유는 전역 `std::begin/end` 덕분이다. 이 함수들은 [여러 오버로드 형태](http://en.cppreference.com/w/cpp/iterator/begin)로 지원되어 상수형 컨테이너와 배열을 모두 처리한다. 이 오버로드된 원형들을 자세히 알아보자.

<a name="begin-overload"></a>
{% highlight C++ %}
// C++14 표준 24.7 Range access 참고
template <class C> auto begin(C& c) -> decltype(c.begin());
template <class C> auto begin(const C& c) -> decltype(c.begin());
template <class T, size_t N> constexpr T* begin(T (&array)[N]) noexcept;
{% endhighlight %}
{% include code_caption.html caption="<code>std::begin</code>의 함수 오버로드들" %}

첫 번째 함수 원형은 자명하다. 두 번째를 보면 `const C&`를 받지만, 반환형은 첫 번째와 같이 여전히 `c.begin()`이다. 상수 반복자를 돌려주는 `c.cbegin()`을 써야 할 것 같은데 아니다.
 C++14 표준의 컨테이너 요구 조건(§23.2.1)에 따르면, `c.begin()`은 `c`가 상수형일 때 `C::const_iterator`를 반환하게 되어있다. 굳이 `std::cbegin/cend`로 구분해 호출할 필요가 없다. 테스트 케이스 3이 자연스럽게 해결될 것이다.

> 사소한 점 하나: `enumerate`의 설계 기준을 STL 컨테이너 조건이 아니라 범위 기반 for 루프의 조건에 따랐다. 그래서 컨테이너 `C`의 `begin/end`가 반드시 `C::iterator` 또는 `C::const_iterator` 타입을 반환해야 하는 건 아니다. 그냥 `begin/end`만 지원되면 된다. <!-- 혹시나 상수형 `C`임에도 불구 `begin/end`가 수정 가능한 반복자를 돌려주면 어쩌나라는 걱정도 필요 없다. `C`가 상수형인데 내부 상태를 반복자로 고치려는 것은 컴파일 오류이니까. 컨테이너 `C`는 `begin/end` 인터페이스만 맞추면 범위 기반 for 루프나 `enumerate`에서 쓸 수 있다. -->

마지막 원형은 일반 배열을 처리한다. 배열은 메서드가 없으므로 이 구현을 `C::begin/end`에 의존했다면 더 복잡해졌을 것이다. `int A[10]` 같은 배열의 타입을 `int*`로 생각하기 쉬우나 정확한 타입은 `int[10]`이고, 컴파일러가 자동으로 `T`를 `int`로, `N`을 10으로 맞춰준다. 배열의 마지막 위치도 이 `N` 값으로 알 수 있다.

전역 보조 함수는 아주 요긴하다. C++17부터는 `std::size`, `std::empty`, `std::data` 같은 다양한 [컨테이너 접근 함수](http://en.cppreference.com/w/cpp/iterator/size)가 지원된다. 파이썬의 `iter`, `size` 같은 [내장 함수](https://docs.python.org/3/library/functions.html)를 보는 것 같다.


### 5. `enumerate_iterator` 클래스 구현

역시 핵심은 `enumerate_iterator` 클래스이다. 타입만 신경 안 쓴다면 구현 자체는 `range_iterator`만큼이나 무척 간단하다. 왜냐면 `enumerate_impl::begin/end`에서 호출된 `std::begin/end`가 상수/배열까지 모두 다룰 수 있는 컨테이너의 반복자를 얻어주었기 때문이다. 이 반복자에서 원소 참조자를 마지막으로 얻고 인덱스 변수를 `pair`로 묶어서 돌려주면 될 것이다.

문제는 템플릿 타입 설정이다. 만들어 보면 컨테이너 반복자인 `IterType`과 컨테이너 원소 참조자인 `ElemType`을 정하는 것이 어렵다. 자세한 타입 정의는 뒤에서 하고 기본적인 구현부터 해본다. 참고로 타입 정의는 `typedef` 대신 C++11부터 가능해진 `using`를 이용하는 것이 여러모로 더 좋다.

{% highlight C++ linenos=table%}
template<typename C /* 컨테이너 타입 */>
class enumerate_iterator {
  using IndexType = size_t;
  using IterType = ??; // C::iterator? C::const_iterator?
  using ElemType = ??; // 원소에 대한 참조자: 예: string&
  using PairType = std::pair<IndexType, ElemType>;

  IterType it_;
  size_t index_;

public:
  enumerate_iterator(IterType it, size_t index) :
    it_{it}, index_{index} {}

  enumerate_iterator& operator++() {
    ++it_;
    ++index_;
    return *this;
  }

  bool operator!=(const enumerate_iterator& rhs) {
    return it_ != rhs.it_;
  }

  PairType operator*() {
    return {index_, *it_};
  }
};
{% endhighlight %}
{% include code_caption.html caption="enumerate_iterator 구현: 자세한 타입 선언은 뒤에서" %}

쉬운 부분부터 빨리 살펴보면, 라인 12의 생성자는 `enumerate_impl`로부터 받은 반복자와 인덱스를 받아 저장한다.
라인 15부터 23까지의 `operator++`, `operator!=`의 구현 역시 간단하다.

라인 25의 `operator*`가 비로소 프로그래머가 만나게 될 `pair<size_t, ElemType>`를 만든다. 라인 26 처럼 입력 컨테이너 반복자(iterator) `it_`에 `*` 연산자를 호출해 원소에 대한 참조자 또는 상수 참조자를 얻는다.

> 디테일한 이야기 하나: 지금까지 반복자의 `*` 연산자가 참조자를 반환한다고 가정을 했다. C++14 표준(§24.2.2)은 `iterator`의 요구 조건으로 `*` 연산은 명시하는데, 반환형은 정하지 않는다(unspecified). 이미 테스트 케이스 4처럼 `range`가 참조자가 아닌 값을 반환한 것을 보았다. 엄밀히 말하자면 `pair<size_t, ElemType>`에서 `ElemType`은 참조자 형이 아니라 "입력 컨테이너 반복자의 `*` 연산자가 반환하는 타입"으로 정의할 수 있다.

`IterType` 타입을 이제 제대로 따져보자. `IterType`은 `enumerate_impl::begin/end`로부터 `std::begin/end`의 반환형과 같고, 결과적으로 `vector<string>::begin/end`의 반환형이 된다.
쉽게 생각하고자 `C`를 `vector<string>`으로 생각하면, `IterType`은 상수형 벡터일 때는 `C::const_iterator`이고 그 외는 `C::iterator`이다.

그래서 `IterType`을 어떻게 정의해야 할까? `C::iterator`로 늘 설정해버리면 상수 벡터가 들어오면 컴파일 오류가 난다. 그렇다고 `C::const_iterator`로 정의하면 일반 벡터의 내부 원소 수정을 못 하게 된다. 아래처럼 조건부로 타입을 골라야 할 것이다.

{% highlight C++ %}
using IterType = C::const_iterator; // C가 상수형이면
                 C::iterator;       // C가 상수형이 아니면
{% endhighlight %}

이런 컴파일 시간에 타입에 따른 분기는 <a href="http://en.cppreference.com/w/cpp/header/type_traits"><code>type_traits</code></a> 도움으로 할 수는 있다. 좀 복잡해진다.

{% highlight C++ %}
// C++11:
using IterType = typename conditional<is_const<C>::value,
                   typename C::const_iterator,
                   typename C::iterator>::type;
// C++14/17:
using IterType = conditional_t<is_const_v<C>,
                   typename C::const_iterator,
                   typename C::iterator>;
{% endhighlight %}

<!-- `std::is_const`는 주어진 타입의 `const` 여부에 따라 참/거짓을 `value`로 가진다. `std::conditional`은 첫번째 템플릿 인자의 참/거짓 유무로 뒤 두 타입 중 하나를 고른다. -->
처음 보는 함수들은 찾아보면 그리 어려운 코드는 아니다. 템플릿 인수 연역 법칙과 템플릿 특수화(specialization)로 컴파일 시간에 타입 특성에 따른 여러 작업을 할 수 있다. 여하간 `type_traits`을 쓰면 상수/비상수 반복자를 `C`에 따라 컴파일 시간에 선택적으로 정의할 수 있다.

이제 문제가 해결된 것인가? 잊고 있던 것이 있었다. C/C++ 배열! 배열이 `C::iterator` 같은 내부 타입이 있을리 없다. 앞서 만든 `range`도 `iterator`라는 내부 타입을 정의하지 않았다. `range_iterator`를 `std::iterator`로부터 상속받으면 되지만 범위 기반 for 루프는 그것까지 요구하지 않았다. 불필요하게 제한 조건을 더하고 싶지 않다. `IterType`을 이 두 가지까지 담으려면 오버로드와 `type_traits`로 되긴 되지만 코드 양이 늘어난다. 뭔가 다른 우아한 해법이 있을 것이다. 실마리는 `std::begin/end`의 반환형으로부터 유추하는 것이다.

C++11의 새로운 기능에 어느 정도 익숙한 분이라면 대략 답을 떠올릴 수 있다. 바로 `auto`의 짝인 `decltype`을 이용하는 것이다. `is_const`, `conditional` 같은 건 죄다 삽질이었다.


### 6. `decltype`을 이용한 `enumerate_iterator::IterType`

<a href="http://en.cppreference.com/w/cpp/language/decltype"><code>decltype</code></a>은 주어진 이름이나 식의 타입을 구하는 C++11부터 추가된 지정자이다. `decltype`도 자세하게 파고들면 머리가 아픈데 타입을 알려주는 매직으로 생각하자. 간단한 사용 예는 [여기서](https://msdn.microsoft.com/en-us/library/dd537655.aspx) 살펴보면 좋다.

`IterType` 정의 문제를 다시 정리하면 이렇다:

- 클래스 `enumerator_iterator<C>`의 템플릿 인자로 주어진 컨테이너 타입 `C`로부터 `std::begin()` 또는 `std::end()`의 반환형을 얻어내 `IterType`으로 정의하라.

쉬워 보인다. 하지만 실제로 해보면 굉장한 난관이 있다. 일단 다음은 당연하게도 컴파일이 안 된다.

{% highlight C++ %}
using IterType = decltype(std::begin(C)); // 컴파일 오류
{% endhighlight %}

`decltype`은 온전한 수식(expression)을 받아 그 타입을 알려준다. 위 코드는 `begin` 함수 인자로 그냥 타입 `C`만 넘겨주니 올바른 수식이 아니다. 어떻게 해서든 타입이 `C`인 값을 줘야 한다. 이런 시도를 해볼 수 있을 것이다.

{% highlight C++ linenos=table %}

// C의 기본 생성자 호출로 값을 만듬
// std::begin(const C&) 오버로드가 불림
// 테스트 케이스 1, 라인 5에서 컴파일 오류:
//  "pair<size_t, const string&>"에서 "pair<size_t, string&>"으로 변환 불가
using IterType = decltype(std::begin(C{}));
{% endhighlight %}
{% include code_caption.html caption="첫 번째 <code>IterType</code> 정의 시도: 상수 반환자 및 공통 생성자 부재로 컴파일 오류" %}

아이디어는 타입 `C`로부터 객체를 만드는 것이다. 여기서 객체 생성 비용 걱정은 하지 않아도 된다. `decltype`이 받는 수식은 _unevaluated operands_, _평가되지 않는 피연산자_ 이므로 괜찮다. 비슷한 예로 `sizeof`가 있다. 이 방법에도 심각한 문제 두 개가 있다.

위 코드를 보면 `C{}`로 임시 객체 즉, 우측값을 만들어 `std::begin`의 인수로 한다. `std::begin` 함수는 여러 오버로드가 있으므로 이 중 하나가 선택될 것이다. 그런데 [코드 5](#begin-overload)를 보면 `std::begin/end`는 우측값을 받는 오버로드가 없다. `std::begin(C& c)` 또는 `std::begin(const C& c)` 중에 하나가 불릴 것이다. 이 때, 컴파일러는 인수 `C{}`를 상수형인 `const C&`로 변환하여 상수형 오버로드를 부르고 만다. 그 이유는 우측값이 좌측값 참조로 변환이 필요할 때는 반드시 상수 좌측값 참조로만 변환이 [허용되기 때문이다](http://stackoverflow.com/questions/34176219/why-rvalue-reference-argument-matches-to-const-reference-in-overload-resolution). 생각해보면 합리적인 규칙이다. 우측값은 자신이 사용되는 수식에서만 유효한 임시 값이므로 상수 참조자로 받는 것이 안전할 것이다. 다음 예로부터 이 규칙을 확인할 수 있다.

{% highlight C++ linenos=table %}
struct X { int foo; };
const X& r1 = X{}; // Okay
      X& r2 = X{}; // Clang error: non-const lvalue reference to type 'X'
                   // cannot bind to a temporary of type 'X'
void f(const X& r) { cout << "const X&\n"; }
void f(      X& r) { cout << "X&\n"; }
f(X{}); // "const X&" 출력
{% endhighlight %}
{% include code_caption.html caption="오류 내용: 비상수 좌측값 참조자를 임시 변수로 초기화할 수 없다." %}

정리하면 `std::begin(C{})`는 `std::begin(const C&)`로 오버로드 결정되어 `C`가 상수/비상수 성질에 상관없이 항상 상수 반복자만 반환되는 불상사가 벌어진다. 테스트 케이스 1에서 컨테이너 원소를 직접 수정하는 코드는 컴파일 오류를 낼 수밖에 없다. `std::begin(C&&)` 버전이 있었다면 이런 문제가 없을 것이다. 왜 표준이 이렇게 설계했는지 확실히 [이해는 못 하였다](http://stackoverflow.com/questions/26959953/stdbegin-and-r-values).

> <code>std::begin</code>과 <code>C::begin</code>의 행동은 같아야 할 것이다. 위에서 <code>decltype(std::begin(C{}))</code> 타입이 항상 상수 반복자임을 확인했다. 만약 항상 <code>C::begin</code> 메서드가 있다면 <code>using IterType = decltype(C{}.begin())</code>로도 쓸 수 있다. 여기서는 <code>C</code>의 상수 성질에 따라 상수 또는 비상수형 반복자를 얻을 수 있다. 우리가 원하는 행동을 한다. 아쉽게도 이 방법은 배열 코드를 처리할 수 없다. <code>std::begin</code>과 <code>C::begin</code>의 행동은 이런 미묘한 차이가 있다. 나는 개선이 필요하다고 생각한다.

대안으로 `new`를 써보자.

{% highlight C++ linenos=table %}
// Clang warning: expression with side effects has no effect
// in an unevaluated context [-Wunevaluated-expression]
// 테스트 케이스 1은 통과하지만 2~4는 생성자를 찾을 수 없다는 컴파일 오류.
using IterType = decltype(std::begin(*(new C{})));

f(*(new X{})); // "X&" 출력!
{% endhighlight %}
{% include code_caption.html caption="두 번째 <code>IterType</code> 정의 시도:  생성자 관련 문제는 아직 미해결" %}

신기하게도 `new`를 이용하면 비상수형 참조자로도 변환이 된다. 앞서 설명한 unevaluated operands 관련 컴파일러 경고가 있지만 테스트 케이스 1은 컴파일된다. 하지만 두 번째 문제를 만나게 된다. `new C{}` 부분을 보면 늘 `C`의 기본 생성자만 호출한다. 배열이나 앞서 구현한 `range`에서는 기본 생성자를 찾지 못 하니까 여전히 컴파일 오류가 난다.

아, 타입만 가지고 `IterType`을 구하는 것은 포기해야만 하는가? 좌절하지 말고 다시 한번 "unevaluated operands" 특성을 활용해보자.

`decltype`이 받는 수식은 실행되지 않는다. 이 말은 구현이 없는 함수도 그 선언만 있고 문법만 맞는다면 쓸 수 있다는 뜻이다. 지금 생성자 부분에서 막혀있으니 어떤 타입이라도 만들어주는 가상 팩토리 함수가 있다고 하면 어떨까? 그러면 생성자 호출을 피할 수 있을 것이다.

{% highlight C++ linenos=table %}
// 구현 없이 decltype에서 쓸 함수 원형
template<typename T> T fake_create();

// 생성자 불일치 컴파일 오류는 없지만 코드 8과 같은 오류 발생
using IterType = decltype(std::begin(fake_create<C>()));
// 드디어 컴파일 성공!
using IterType = decltype(std::begin(fake_create<C&>()));
{% endhighlight %}
{% include code_caption.html caption="세 번째 <code>IterType</code> 정의 시도: 성공" %}

마침내 방법을 찾았다. `fake_create`는 주어진 타입을 만들어주는 가짜 팩토리 함수이다. 그러니 이 타입을 그대로 반환형으로만 써주면 된다. 이런 함수가 있다고 가정하고 쓰니까 생성자를 직접 안 불러도 된다. 이 테크닉으로 임의의 타입 `C`에 대해 공통적인 생성자가 없어도 `C` 타입의 값을 만들 수 있다. 코드 8에서 자세히 이야기 했듯이 `std::begin` 함수의 오버로드 특성으로 라인 5처럼 `C` 타입을 넘겨주면 임시 값인 `C&&`이 되고 `const C&` 함수가 또다시 불린다. 라인 7처럼 `C&` 타입을 쓰면 말끔히 해결된다.

쉬울 것 같았던 문제는 예상과 달리 정말 어려웠다.

`fake_create` 같은 함수 선언을 직접 만들자니 덜 깔끔하다. 분명 C++ 표준에 이와 비슷한 일을 하는 함수가 있을 것이다.
찾아보면 결국 있는데 C++11부터 추가된 <a href="http://en.cppreference.com/w/cpp/utility/declval"><code>std::declval</code></a> 템플릿 함수가 그것이다.


### 7. `std::declval`의 도움으로 `enumerate_iterator::IterType` 완성하기

`std::declval`의 작동 원리는 앞서 만든 `fake_create`와 거의 같다. 평가되지 않는 문맥 특성을 적극 활용하여 공통의 생성자가 없는 상황에서도 값으로 만들어 쓸 수 있게 한다.
`std::declval` 구현은 아래와 같다.

<div class="highlight"><pre><span class="k">template</span>&lt;<span class="k">class</span> T&gt; <span class="k">typename</span> add_rvalue_reference&lt;T&gt;::type <b>declval</b>();</pre></div>

`fake_create`와 비교하면 반환형만 다르다. `std::declval`은 주어진 타입 `T`에 우측값 참조 기호인 `&&`를 더 하는 <a href="https://github.com/llvm-mirror/libcxx/blob/master/include/type_traits#L1030"><code>add_rvalue_reference</code></a>를 써서 반환형으로 한다.
`X` 타입을 넣으면 `X&&`가 반환된다. `fake_create`에서는 이때 `X`가 반환되지만 임시 값으로 인식되므로 우측값 참조로 명시적 캐스팅하는 것이 더 나을 것이다.

그런데 `IterType`은 `fake_create<C&>`에서 보듯 `C&` 타입을 넣었다. 만약 `std::declval<C&>`으로 쓰면 어떻게 될까?
주어진 타입에 `&&`를 더해야 하니까 `C& &&`가 될 것이다. 프로그래머는 이런 참조자의 참조자를 쓸 수 없다. 해봤자 컴파일 오류다.
컴파일러는 템플릿 확장 시 이런 상황을 다뤄야 한다. C++11에서 이 규칙이 명확하게 정의되었다. 바로 *reference collapsing rule*, *참조자 중첩 규칙*이다. 이미 현대 C++을 공부하신 분들이라면 알고 있는 법칙일 것이다.

이 규칙은 간단하다. 주어진 타입에 `&` 또는 `&&`가 있고, 추가로 `&` 또는 `&&`를 덧붙일 때 그 결과를 정한다. 다음의 네 가지 경우가 있다.

- <span style="white-space: pre">`add_lvalue_reference<C&> ` → `C&   &` → `C&`</span>
- <span style="white-space: pre">`add_lvalue_reference<C&&>` → `C&&  &` → `C&`</span>
- <span style="white-space: pre">`add_rvalue_reference<C&> ` → `C&  &&` → `C&`</span>
- <span style="white-space: pre">`add_rvalue_reference<C&&>` → `C&& &&` → <b>`C&&`</b></span>

보다시피 `&`가 하나라도 있으면 `&`가 이긴다. `std::declval<C&>`는 `fake_create<C&>`와 같이 `C&` 타입으로 결정된다. 따라서 `fake_create` 대신에 `declval`만 써도 된다.

{% highlight C++ %}
using IterType = decltype(std::begin(std::declval<C&>()));
{% endhighlight %}
{% include code_caption.html caption="드디어 완성된 <code>IterType</code> 정의" %}

너무 지나치게 `IterType` 문제에 집착하기는 했다. 그런데 쭉 과정을 보면 알겠지만 사소한 내용 하나라도 제대로 이해하지 못 했다면 컴파일 오류를 이해하기도 어려웠을 것이고 간결한 타입 정의도 어려웠을 것이다.

어려움 속에서 `IterType` 정의에 성공했으니 `ElemType`은 쉽게 된다. 여러 방법으로 할 수 있다.

{% highlight C++ linenos=table%}
template<typename C>
class enumerate_iterator {
  ....
  using IterType  = decltype(std::begin(std::declval<C&>()));
  using ElemType  = decltype(*std::begin(std::declval<C&>()));
  ....

  IterType it_;
  size_t index_;

  // it_ 변수를 이용할 수도 있다.
  using ElemType = decltype(*it_);
  ....
{% endhighlight %}
{% include code_caption.html caption="ElemType 정의" %}

`std::begin/end`에서 얻은 반복자에 `*` 연산자를 부르면 그게 바로 `ElemType`이 된다. 아니면 이 클래스에서는 `IterType it_` 변수가 있으므로 라인 12의 방법도 유효하다.

잠깐, 라인 12를 보니 특정 타입의 변수가 있으니 무척 쉽게 정의된다. 그렇다면 왜 굳이 `IterType`을 타입 `C`로부터만 유추하려고 했을까? `enumerate_impl` 클래스를 보면 이미 `C` 타입의 변수 `container_`가 있지 않은가? 다음과 같은 구현도 될 것이다.

{% highlight C++ linenos=table%}
// 컨테이너 타입과 함께 Iter/Elem 타입도 같이 받음
template<typename C, typename IterType, typename ElemType>
class enumerate_iterator {
  ....
};

template<typename C /* 컨테이너 타입 */>
class enumerate_impl {
  C& container_;
  const size_t start_;

  // container_ 변수로 간단히 정의
  using IterType = decltype(std::begin(container_));
  using ElemType = decltype(*std::begin(container_));

  enumerate_iterator<C, IterType, ElemType> begin() {
    return {std::begin(container_), start_};
  }
  ....  
{% endhighlight %}
{% include code_caption.html caption="<code>container_</code>를 직접 써서 타입 추론" %}

`enumerate_impl` 클래스 안에서 `container_`를 직접 써서 타입을 정의하니 사실 훨씬 간단하긴 하다. 그렇다고 지금까지 거쳐온 과정이 모두 무의인 것은 아니다. `container_` 같은 변수가 없는 상황에서 타입만 가지고 작업해야 할 때가 분명 있을 것이다. 우리는 이미 유용한 테크닉과 여러 규칙을 배웠으니 그 의미는 크다.

이렇게 `IterType`과 `ElemType`을 성공적으로 정의함으로써 테스트 케이스 1~4는 이제 작동하게 된다. 남은 과제는 테스트 케이스 5와 6이다.


### 8. `enumerate`의 포워딩(유니버셜) 레퍼런스화 하기

코드 2의 `enumerate`는 좌측값 참조만 받도록 설계했고, 코드 4를 봐도 `enumerate_impl`의 멤버 변수는 `C& container_`로 참조자 형태로 있다. 이 코드로는 테스트 케이스 5를 컴파일할 수 없다. 임시 변수는 상수 좌측값 참조로는 변환되어도 그냥 좌측값 참조로는 안 되기 때문이다. 우측값을 받는 함수를 추가해야 할 것이다.

{% highlight C++ linenos=table%}
// 기존의 좌측값 참조 버전
template<typename C>
enumerate_impl<C> enumerate(C& container, size_t start = 0) {
  return enumerate_impl{container, start};
}

// 우측값(이 아니고 포워딩 레퍼런스이지만) 버전
template<typename C>
enumerate_impl<C> enumerate(C&& container, size_t start = 0) {
  return enumerate_impl{/* TODO: */, start};
}
{% endhighlight %}
{% include code_caption.html caption="enumerate의 우측값 참조 오버로드" %}

아마 이런 식으로 시도를 할 것인데 문제가 있다. 우측값 버전을 오버로드 해야 하니까 코드가 두 배로 는다. 더 큰 문제는 두 개 이상의 인자를 받을 때다. 함수 인자마다 좌측값 또는 우측값의 두 경우의 수가 있으므로 필요한 함수 오버로드 개수가 기하급수적으로 늘어난다.

이런 문제를 막고자 뛰어나신 C++ 전문가들께서 `T&&`에 우측값 참조 말고도 다른 의미를 부여하셨다. `T&&`가 조건에 따라 `T&`, 또는 `T&&`가 될 수 있도록 하였다. 
안타깝게도 C++11/14까지 이런 `T&&`의 다른 의미에 해당하는 이름이 없었다. Effective Modern C++의 저자, Scott Meyers씨는 이런 상황을 가엾이 여겨 친히 이름을 지었으니 바로 _유니버셜 레퍼런스_, _universal reference_ 이다. 우리 말로 번역하자면 만능 참조 정도가 적당할 것이다.

이윽고 C++ 표준 위원들도 이름 명명의 필요성을 제기되어 논의를 거쳤는데, 유니버셜 레퍼런스 대신 _포워딩 레퍼런스_, _forwarding reference_ 를 공식적인 이름으로 [공포하였다](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4164.pdf).[^forwardref] 추후 확정될 C++17부터 포함된다. 우리 말로는 전달 참조자면 어떨까 한다.

[^forwardref]: Scott Meyers씨도 포워딩 레퍼런스 이름에 동의했다고 하니 Effective Modern C++의 추후 개정판에서는 유니버셜 레퍼런스 이름이 바뀔 것이다. 다만 C++ 업계에서 은퇴한다고 하시니 아쉬울 뿐...

포워딩 레퍼런스를 자세히 설명하자면 또 하나의 글이 필요할 정도니 [이 글](https://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers)이나 [이 슬라이드](http://www.aristeia.com/TalkNotes/C++TypeDeductionandWhyYouCareCppCon2014.pdf)를 참고하기 바란다. 간단한 예만 살펴보자.

{% highlight C++ linenos=table%}
// C++17(n4567) §14.8.2.1
template <class T> int f(T&& heisenreference);
template <class T> int g(const T&&);
int i;
int n1 = f(i); // calls f<int&>(int&)
int n2 = f(0); // calls f<int>(int&&)
int n3 = g(i); // error: would call g<int>(const int&&), which
               // would bind an rvalue reference to an lvalue
{% endhighlight %}
{% include code_caption.html caption="포워딩 레퍼런스: 하이젠레퍼런스?!" %}

템플릿 함수 `f`의 `T&&` 인자는 두 가지 의미로 해석된다.
라인 5에서 변수 `i`, 그러니까 주소를 얻을 수 있는 좌측값을 넣으면 `int&&`가 아닌 `int&`로 추론된다. `T` 역시 `int&`가 된다.
반면, 라인 6의 0처럼 주소를 얻을 수 없는 임시/우측값은 `int&&`로, `T`는 `int`로 추론된다. 
이처럼 `f(T&&)` 원형 하나로 좌/우측값을 모두 해결한다.
주의할 점은 반드시 `T&&` 형태로 템플릿 타입 연역이 일어나야 한다는 것이다. `const` 같은 것이 붙어버리면 포워딩 레퍼런스로 작동하지 않는다.
`enumerate`도 포워딩 레퍼런스를 인자로 하는 함수 하나만 있어도 된다.

{% highlight C++ linenos=table%}
// 좌/우측값을 모두 아우르는 포워딩 레퍼런스 버전: 이거 하나면 됨.
template<typename C>
enumerate_impl<C> enumerate(C&& container, size_t start = 0) {
  return enumerate_impl{std::forward<C>(container), start};
}
{% endhighlight %}
{% include code_caption.html caption="enumerate의 우측값 참조 오버로드" %}

아직 현대 C++에 익숙하지 않다면 <a href="http://en.cppreference.com/w/cpp/utility/forward"><code>std::forward</code></a>가 좀 생소할 것이다. 이것도 자세히 설명하자면 복잡한데 간단히 설명하면, 포워딩 레퍼런스로 받은 인자를 다른 함수로 그대로 전달하고 싶다면 `forward`를 쓴다고 외워도 된다. 

> `forward` 역할은 캐스팅이다. 캐스팅이 필요한 이유는 함수 호출 시 인수를 우측값으로 초기화해도 함수 인자 자체는 주소를 얻을 수 있는 좌측값라서 명시적으로 다시 우측값 참조자로 변환하는 것이다. 관심 있는 분은 C++ 표준 정의(§20.2.4)와 그걸 그대로 코드로 [구현한 것](https://github.com/llvm-mirror/libcxx/blob/master/include/type_traits#L2136)을 봐도 좋다. Modern Effective C++의 내용에 의하면 `forward`는 인수가 우측값으로 초기화되었을 때만 _조건부로_ 우측값으로 캐스트한다고 설명한다.


마지막으로 `enumerate_impl` 구현을 손봐야한다. 클래스 멤버 변수로 `C& container_`를 가졌는데 `C`가 포워딩 레퍼런스가 되었으니 관련된 부분을 고쳐야 한다. 수정한 부분을 볼드체로 강조했다.

<table class="highlighttable"><tr><td class="linenos"><div class="linenodiv"><pre><code class="language-c--" data-lang="c++"> 1
 2
 3
 4
 5
 6
 7
 8
 9
10
</code></pre></div></td><td class="code"><div class="highlight"><pre><span class="k">template</span><span class="o">&lt;</span><span class="k">typename</span> <span class="n">C</span> <span class="cm">/* 컨테이너 타입 */</span><span class="o">&gt;</span>
<span class="k">class</span> <span class="nc">enumerate_impl</span> <span class="p">{</span>
  <span class="n" style="color: darkred; font-weight: bold">C</span> <span class="n" style="color: darkred; font-weight: bold">container_</span><span class="p">;  </span><span class="c1">// 좌측값일 때는 C&, 우측값일 때는 C로 됨</span>
  <span class="k">const</span> <span class="kt">size_t</span> <span class="n">start_</span><span class="p">;</span>

<span class="k">public</span><span class="o">:</span>
  <span class="n">enumerate_impl</span><span class="p">(</span><span class="n"  style="color: darkred; font-weight: bold">C&amp;&amp; container</span><span class="p">,</span> <span class="kt">size_t</span> <span class="n">start</span><span class="p">)</span> <span class="o">:</span>
    <span class="n">container_</span><span class="p">{</span><span class="n"  style="color: darkred; font-weight: bold">std::forward&lt;C&gt;(container)}</span>, <span class="n">start_</span><span class="p">{</span><span class="n">start</span><span class="p">}</span> <span class="p">{}</span>

  <span class="c1">// 이전 코드와 같음...</span>
</pre></div>
</td></tr></table>

{% include code_caption.html caption="포워딩 레퍼런스를 고려한 enumerate_impl 구현" %}

사용자가 `enumerate`를 좌측값 또는 우측값으로 사용할 때를 생각하자. 코드 15의 예와 정확하게 같게 작동한다.

- 테스트 케이스 1~4: 좌측값이 코드 16에 있는 `enumerate` 함수 인수로 넘어오면 `C&& container` 인자는 포워딩 레퍼런스 규칙에 따라 `C&` 타입이 되고, `C` 타입 역시 `C&`가 된다. 코드 17의 `enumerate_impl`는 `C&` 템플릿 인자를 받으므로 `container_` 멤버 변수의 타입은 `C&`이 된다. `enumerate_impl`의 생성자에서는 이제 좌측 참조자를 단순 복사만 하는 것이다.

- 테스트 케이스 5: 우측값이 `enumerate` 함수 인수로 넘어오면 `C&& container` 인자는 우측값 참조가 되고 `C` 타입은 그냥 `C`로 남게 된다. 마찬가지로 `enumerate_impl`의 `container_` 멤버 변수 역시 참조자가 아닌 값 형태의 `C` 타입이 된다. 여기서 재밌는 일이 벌어진다. 코드 17의 라인 8에서 `container_` 멤버 변수는 이동 생성자(move constructor)로 초기화된다. 사용자가 넘겨준 `container` 인자가 `std::forward`에 의해 우측값 참조로 캐스팅되기 때문이다. 복사 생성자는 불리지 않는다. 디버거나 타입 확인을 해보면 이 내용을 확인할 수 있다.


주의할 점은 코드 17에서 라인 3을 `C&& container_` 형태로 선언하면 컴파일은 되어도 우측값 테스트 케이스가 오작동한다. 멤버 변수도 포워딩 레퍼런스로 선언해야 할 것 같지만 아니다. 멤버 변수의 타입 `C`는 별도로 템플릿 인수 연역이 일어나는 것이 아니라 이미 연역된 타입을 그대로 받아 쓴다. `C&& container_`로 선언했다면 이건 그냥 우측값 참조가 되고 만다. 생성자에서도 단순한 참조자 간의 복사만 이뤄진다. 범위 기반 for 루프의 확장된 코드를 보면 왜 이것이 문제가 되는지 알 수 있다.

{% highlight C++ linenos=table%}
// 범위 기반 for 루프의 확장
// for (auto p : enumerate(vector<int>{42, 17})) { ... }
{
  // 임시로 만들어진 벡터는 우측값으로 전달 
  auto&&  __range = enumerate(vector<int>{42, 17});
  // enumerate_impl::container_의 타입이 C&&라면
  // 임시 벡터는 이 시점에서 파괴되어 더 이상 유효하지 않음.
  for ( auto __begin = __range.begin(),
             __end = __range.end();
  ....
}
{% endhighlight %}
{% include code_caption.html caption="임시 컨테이너를 사용하는 예" %}

위 코드처럼 확장되었을 때, 만약 `C&& enumerate_impl::container_`로 선언이 되었다면 이 변수는 단순히 우측값 참조를 값으로 보관할 뿐이다. 그래서 라인 5의 문(statement)이 완료되면 이 임시 벡터는 파괴된다. 이후의 `__range.begin()` 호출은 정의되지 않은 행동이다. `enumerate_impl`은 우측값으로 넘어온 임시 컨테이너를 범위 기반 루프의 스코프(라인 3~11) 동안 반드시 보관해야 한다.

보다시피 포워딩 레퍼런스는 이런 오묘한 두 의미를 아슬아슬하게 다루고 있다. 템플릿 자체가 이미 여러 타입을 하나로 표현하는 건데, 한 단계 더 깊은 중첩의 의미가 있으니 이해하기 더 어렵게 되었다. 차라리 우측값 참조와 헷갈리지 않게 포워딩 레퍼런스를 다른 기호로 썼다면 어땠을까라는 생각도 든다.

지금까지 테스트 케이스 1~5를 완벽하게 처리할 수 있는 `enumerate`를 만들었다.    

### 9. 마지막 테스트 케이스: 초기화 리스트 처리하기

지금까지 달려오느라 너무 힘들었습니다. 이건 쉽습니다. 연습 문제로 남기겠습니다.

### 10. 결론

`enumerate`는 유도 변수를 피하면서 인덱스 변수를 쓸 수 있도록 한다. 실제 구현은 코드는 60줄 정도로 짧지만 도달하는 과정 동안 생각해야 할 점이 무척 많았다. 이번 구현에서 배웠으면 좋을 것을 마지막으로 정리해보자.

- 범위 기반 for 루프의 정의와 사용할 수 있는 컨테이너와 배열의 조건.
- `std::begin/end`의 유용함.
- 우측값과 상수 좌측값 참조 사이의 캐스팅 여부.
- `decltype`은 평가되지 않는 피연산자를 받고 이를 응용한 `std::declval`.
- 레퍼런스 중첩 법칙.
- 우측값과 형태는 같아도 그 의미가 다른 포워딩 레퍼런스와 실제 응용.

코드는 [여기에서](/assets/2016/python_like_enumerate/python_like.cpp) 받아 볼 수 있고 온라인 실행도 [할 수 있다](http://coliru.stacked-crooked.com/a/9c0ff54963ef1c11).

`enumerate`를 이리저리 시도해보면서 코딩하면 제대로 작동하는 코드를 아주 어렵지 않게 찾을 수는 있다. 문제는 "어 되네? 왜 되는 거지? 이건 또 왜 안 되지?" 라는 의문이 그대로 남았다는 것. 코딩 도중 컴파일 오류가 나도 그 의미를 완전히 이해하지 못 했다. 이 글을 쓰면서 비로소 모든 과정을 이해하려고 했고 그 결과 이렇게 긴 글이 나오고 말았다.

현대 C++는 많은 분에게 고통을 안겨준다. 하지만 어려운 점이 있으니 배우는 묘미도 있는 법이다. 이해하기 어렵더라도 치밀하게 설계된 표준에 경외감을 표한다.

[^cv]: `volatile`도 포함되는데 사실 컨테이너 같은 C++ 자료구조에 `volatile` 쓸 일은 거의 없을 것이다. C++에서 `const`와 `volatile`은 흔히 [cv type qualifiers/한정자](http://en.cppreference.com/w/cpp/language/cv)로 분류되고 템플릿 및 `auto` [타입 연역/추론](http://www.aristeia.com/TalkNotes/C++TypeDeductionandWhyYouCareCppCon2014.pdf){:target="_blank"}에서 특별한 역할을 하기도 한다.
