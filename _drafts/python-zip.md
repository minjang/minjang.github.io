---
layout: post
title: 가변 인수 템플릿으로 Python의 zip 만들어보기
---
현대 C++로 파이썬 흉내내기 시리즈:

1. [C++11으로 파이썬의 range 흉내내기](/2015/12/03/python-range-c++11/)
2. [현대 C++로 Python의 enumerate 만들어보기](/2016/01/07/python-enumerate-modern-c++/)

일러두기:

- C++ enumerate 보다는 아마도 쉽습니다 😂
- 예제 코드는 clang 3.6 이상에서 제대로 컴파일 됩니다. 리눅스에서는 libc++ 라이브러리를 사용해야 합니다. Visual Studio 2015 Update 1에서는 컴파일이 안 되는데 윈도우에서도 clang을 설치하면 됩니다. 

이전 두 글에서 유도 변수 없는 루프를 가능하게 하는 파이썬과 유사한 `range`와 `enumerate`를 만들었다. 이번에는 파이썬의 <a href="https://docs.python.org/3/library/functions.html#zip"><code>zip</code></a>에 해당하는 C++ 구현을 만들고자 한다. `zip`은 순회 가능한 객체 여러 개를 받아 동시에 이들을 순회할 수 있도록 한다. 사용 예는 이러하다.

<pre>
<b>zip</b>(<i>*iterables</i>)
</pre>

{% highlight bash %}
$ python3
>>> x = ["e", "p"]
>>> y = [2.718, 3.141]
>>> z = ["foo", "bar", "baz"]
>>> for t in zip(x, y, z):  # t는 tuple 타입
...     print("({}, {}, {})".format(t[0], t[1], t[2]))
...
(e, 2.718, foo)
(p, 3.141, bar)
>>>
>>> for i, (t0, t1) in enumerate(zip(x, y)):
...     print("{}: ({}, {})".format(i, t0, t1))
...
0: (e, 2.718)
1: (p, 3.141)
{% endhighlight %}
{% include code_caption.html caption="Python zip 예제: 여러 컨테이너를 묶어서 동시에 순환할 수 있다." %}

파이썬을 잘 몰라도 직관적으로 이해가는 코드이다. 파이썬의 `zip`은 리스트 `x`, `y`, `z`의 원소들로 이루어진 튜플(tuple)을 순환마다 돌려준다. `i` 번째 튜플 원소는 역시 직관적으로 `zip` 함수의 `i` 번째 인수에서 얻어낸 원소이다. 파이썬의 튜플은 배열처럼  접근할 수 있다. 예에서 배열 `z`의 길이가 다른 배열 보다 길지만 가장 짧은 배열의 길이까지만 순환되었음을 알 수 있다. 쉽게 예측 가능한 행동이다.

두 번째 코드는 `enumerate`와 같이 쓴 예제인데 튜플을 바로 `t0`, `t1` 변수로 풀었다(unpack). 이런 튜플을 낱개 변수로 푸는 언팩은 C++에서 안 되겠지만, 현대 C++로 `zip`과 거의 흡사한 기능을 역시 만들 수 있다.

앞 두 글에서 현대 C++의 여러 기능 - 범위 기반 for 루프, `std::begin/end`, 우측값 참조와 포워딩 레퍼런스, 무브 시맨틱과 퍼펙트 포워딩 - 을 최대한 활용했다. 이번 글은 C++11에 추가된 가변 인수 템플릿, variadic template을 쓸 것이다. 더 나가 C++14에서 선 보인 `decltype(auto)`, C++17에 도입된 희한한 문법도 활용한다.

### 1. C++ zip 요구조건 정하기
C++로 `zip`을 만든다면 아마 이렇게 만들어 쓸 수 있을 것이다. 그 결과는 코드 1의 파이썬 예와 같을 것이다.

{% highlight C++ linenos=table%}
vector<string> x = {"e", "p"};
vector<double> y = {2.718, 3.141};
vector<string> z = {"foo", "bar", "baz"};
// for (auto&& t : zip(x, y, z)) 또는
for (std::tuple<string&, double&, string&> t : zip(x, y, z))
  cout << "(" << get<0>(t) << ", " << get<1>(t) << ", " << get<2>(t) << ")\n";
// for (auto&& p : enumerate(zip(x, y))) 또는
for (pair<size_t, tuple<string&, double&>> &&p : enumerate(zip(x, y)))
  cout << p.first << ": (" << get<0>(p.second) << ", " << get<1>(p.second) << ")\n";
{% endhighlight %}
{% include code_caption.html caption="파이썬 코드 예를 C++로 쓴다면 아마 이렇게?" %}

파이썬과 비슷하게 순환마다 원소 참조자(또는 원소 값[^element])들의 튜플을 돌려주도록 만들 것이다.
C++11부터 <a href="http://en.cppreference.com/w/cpp/utility/tuple"><code>std::tuple</code></a>이 지원된다. C++ 튜플은 두 타입만 가질 수 있었던 `std::pair`를 일반화한 것으로 미리 정해진 개수 만큼의 서로 다른 타입을 가지는 템플릿 클래스이다.

[^element]: [앞 글](/2016/01/07/python-enumerate-modern-c++/#detail1)에서 설명했듯이 정확하게는 "컨테이너 반복자의 `*` 연산자가 돌려주는 타입"이지만 그냥 참조자로 가정할 것이다. 값이라고 해도 복사가 가벼운 값이일테니.

코드 2의 라인 5를 보면 `tuple<string&, double&, string&>`으로 선언했다. C++ 튜플은 컴파일 시간에 그 크기가 결정되는 템플릿 기반 자료구조라서 배열 처럼 접근이 안 된다. 예제에서 보다시피 <a href="http://en.cppreference.com/w/cpp/utility/tuple/get"><code>std::get<0>(t)</code></a> 같은 좀 못 생긴 문법으로 튜플 값을 읽어야 한다. `get<index>`에서 `index`는 반드시 상수 값이 되어야 한다. 배열처럼 루프에서 인덱스 변수를 이용하는 코드는 컴파일 오류이다. 뒤에서 살펴보겠지만 모든 튜플 원소를 순회하며 작업을 하려면 몇 가지 테크닉이 필요하다.

C++ `zip`이 받을 수 있는 컨테이너 타입은 앞 글에서 설명한 `enumerate`와 거의 같다. 범위 기반 for 루프가 받을 수 있는 모든 것: (1) STL 컨테이너, (2) 일반 배열, (3) `begin`/`end` 메서드를 노출하는 사용자 컨테이너를 `zip`에서도 쓸 수 있다. 초기화 리스트는 좀 골치 아프다. 이것만 무시하면 `enumerate`가 받을 수 있는 것을 모두 `zip`에서 쓸 수 있다.

파이썬의 `zip`이 가변 인수를 받듯이 C++ `zip` 또한 가변 인수를 허용한다. 구현은 `printf`에서 보아왔던 안전하지 않은 C 기반의 [가변 인수](http://en.cppreference.com/w/cpp/language/variadic_arguments)가 아닌 C++11부터 추가된 [가변 인수 템플릿(variadic template)](http://en.cppreference.com/w/cpp/language/parameter_pack)을 쓴다.[^variadic]

[^variadic]: Variadic을 가변 *인자*로 번역하기도 하는데 가변 인수로 보는 것이 더 타당하다. 일단, 함수 `f(int x)`가 있다면 `x`는 함수 `f`의 *인자*, *parameter*이다. 이 함수를 `f(42)`처럼 부를 때 42는 이 함수의 *인수*, *argument*이다. 인자와 인수에 대한 번역에는 합의가 있는 편이다. Variadic은 가변적인 *argument*를 받는다고 [보통 정의된다](https://en.wikipedia.org/wiki/Variadic_function). 따라서 variadic template도 가변 *인수* 템플릿으로 번역하는 것이 옳다. 템플릿을 쓰는 입장에서 가변적인 인수를 넣어주니 맞는 말이다. 가변 인수 템플릿과 자주 등장하는 용어가 parameter pack, 인자 묶음이다. 이것은 가변 인수 템플릿를 실제로 구현할 때 다루는 것으로 함수 내에서 보는 것은 이제 인자가 되기 떄문이다. Variadic template을 가변 인수 템플릿으로 번역하는 쪽이 더 일관성이 높다.

정리하면 C++ `zip`의 요구조건은 다음과 같다:

- 가변 인수 템플릿으로 구현.
- `std::tuple`로 입력 컨테이너들의 원소 집합을 받는다.
- 범위 기반 for 루프 또는 C++ `enumerate`가 받을 수 있는 컨테이너는 받을 수 있다. 단, 초기화 리스트는 나중에 고려하기로 한다.

별 내용 없다. 그냥 가변 인수와 튜플을 쓰겠다는 것이다.


### 2. 테스트 케이스

`enumerate`에서 이미 상수, 일반 배열 등은 자세히 다뤘으므로 여기서는 축약된 테스트만 만들자. 앞에서 만든 `range`, `enumerate`와 프락시 반복자(proxy iterator)를 쓰는 `vector<bool>`도 테스트에 추가한다. 테스트 케이스와 예상 결과는 다음과 같다. 

{% highlight C++ linenos=table%}
vector<bool> A   = {   false,    false,  true};
size_t B[]       = { 0, 0, 0, 0}; // 길이가 다름
vector<string> C = {    "pi",      "e", "ans"};
const double D[] = {3.141592, 2.718281,    42};

cout << std::boolalpha;

// 임의의 tuple/pair가 cout으로 출력되도록 만들었다고 가정
// pair<t1, t2>
//  => cout << "<" << t1 << ", " << t2 << ">"
// tuple<t1, t2, ..., tn>
//  => cout << "(" << t1 << ", " << ... << ")";

// A, B, C는 수정 가능, range도 같이 사용
// std::size는 C++1z/17부터 지원
// decltype(t) == tuple<proxy, size_t&, string&, size_t>&&
for (auto&& t : zip(A, B, C, range<size_t>(0, std::size(A)))) {
  cout << "Before: " << t << endl;
  get<0>(t) = get<0>(t) ^ true;
  get<1>(t) = get<3>(t);
  get<2>(t) = '\"' + get<2>(t) + '\"';
}

// enumerate와 같이 사용
// decltype(p) == pair<size_t,
//   tuple<proxy, size_t&, string&, const double&>>&&
for (auto&& p : enumerate(zip(A, B, C, D))) {
  cout << "After: " << p << endl;
}

// zip, enumerate, range 모두 같이 사용
// decltype(t) == tuple<pair<size_t, int>>&&
for (auto&& t : zip(enumerate(range(11, 14)))) {
  cout << t << endl;
}
{% endhighlight %}

{% highlight bash %}
Before: (false, 0, pi, 0)
Before: (false, 0, e, 1)
Before: (true, 0, ans, 2)
After: <0, (true, 0, "pi", 3.14159)>
After: <1, (true, 1, "e", 2.71828)>
After: <2, (false, 2, "ans", 42)>
(<0, 11>)
(<1, 12>)
(<2, 13>)
{% endhighlight %}
{% include code_caption.html caption="C++ <code>zip</code> 테스트 케이스와 예상 결과" %}

튜플 출력 코드를 생략하고자 `std::pair`와 `std::tuple`을 `cout`으로 출력하는 코드가 있다고 가정한다. 뒤에서 가변 인수 템플릿을 설명하면서 이 코드를 배울 것이다.

튜플 원소 값을 변경할 때는 별도로 `set` 같은 함수로 하는 것이 아니라 `std::get`이 참조자를 반환하므로 바로 여기에다 쓰면 된다. 보다시피 상수 배열이 아닌 이상 값을 바꿀 수 있어야 한다.

주목할 것은 `vector<bool>`, `range`, `enumerate`의 사용 예이다. `vector<string>`을 범위 기반 for 루프나 `enumerate`에 넣으면 순환마다 얻는 원소가 `string&` 참조자 형태이다. 그러나 `vector<bool>`은 `bool&`이 아닌 별도의 객체, 프락시(proxy, 대리자) 반복자를 만들어 값으로 반환된다. `bool`은 1비트만 있어도 되므로 1 바이트에 여러 `bool` 값을 모아 저장한다. 그래서 바로 `bool&`을 반환할 수 없고 프락시 반복자를 임시로 만들어 반환한다. 임시 값이므로 이것을 (비상수) 참조자로는 반환할 수 없으므로 값으로 돌려준다. 실제 라인 17의 튜플 타입을 디버거나 boost의 `type_id_with_cvr`로 확인하면 `bool&` 대신 내부 구현체를 볼 수 있다.

`range`도 프락시 반복자를 쓰는셈이다. 루프에서 `size_t&` 참조자 형태로 원소를 얻지 않았다. `range_iterator`가 순환마다 만들어지고 여기서 `size_t`를 바로 값으로 반환했다. 라인 33의 `enumerate`도 마찬가지이다. `pair<size_t, int>`를 즉석에서 만든 뒤 값으로 돌려주므로 프락시 반복자 구조이다. 왜 이 이야기를 하냐면, `zip` 구현시 작은 타입 캐스팅 실수를 한다면 프락시 반복자를 제대로 다룰 수 없기 때문이다.


### 3. 두 컨테이너만 받을 수 있는 `zip`을 만들기

바로 가변 인수를 받는 `zip`은 가변 인수 템플릿에 능숙하지 않은 이상 어렵다. 두 컨테이너만 받을 수 있는 `zip`을 먼저 만들고 이를 가변 인수 템플릿화하는 것이 좋을 것이다. `zip` 구현은 앞서 만든 `enumerate`와 같은 구조이고 코드도 간단하다. 앞 글에서 힘들게 설명했던 포워딩 레퍼런스, `decltype`, `std::declval` 등을 그대로 활용한다. `zip`과 `zip_impl` 구현은 이렇게 할 수 있을 것이다.

{% highlight C++ linenos=table%}
template<typename C1, typename C2>
zip_impl<C1, C2> zip(C1&& c1, C2&& c2) {
  return {std::forward<C1>(c1), std::forward<C2>(c2)};
}
{% endhighlight %}
{% include code_caption.html caption="두 컨테이너만 받는 <code>zip</code> 구현" %}

두 입력 컨테이너를 받을 때 좌측값과 우측값을 모두 받을 수 있는 포워딩 레퍼런스를 썼고 `std::forward`로 `zip_impl` 생성자로 이 값을 완벽하게 전달한다.

{% highlight C++ linenos=table%}
template<typename C1, typename C2>
class zip_impl {
  std::tuple<C1, C2> containers_;

public:
  zip_impl(C1&& c1, C2&& c2) : 
    containers_{std::forward<C1>(c1), std::forward<C2>(c2)} {}

  zip_iterator<C1, C2> begin() {
    return {std::begin(std::get<0>(containers_)),
            std::begin(std::get<1>(containers_))};
  }

  zip_iterator<C1, C2> end() {
    return {std::end(std::get<0>(containers_)),
            std::end(std::get<1>(containers_))};
  }
};
{% endhighlight %}
{% include code_caption.html caption="두 컨테이너만 받는 <code>zip_impl</code> 구현" %}

`zip_impl`은 입력 컨테이너들을 튜플로 보관하고 `std::begin/end`를 써서 `zip_iterator`를 만든다. 지금은 이 코드가 간단해 보이지만 숨어있는 디테일이 어마어마했다.

{% highlight C++ linenos=table%}
template<typename C1, typename C2>
class zip_iterator {
  using IterType1 = decltype(std::begin(std::declval<C1&>()));
  using IterType2 = decltype(std::begin(std::declval<C2&>()));
  using ElemType1 = decltype(*std::begin(std::declval<C1&>()));
  using ElemType2 = decltype(*std::begin(std::declval<C2&>()));
  
  using TupleType = std::tuple<ElemType1, ElemType2>;

  std::tuple<IterType1, IterType2> its_;

public:
  zip_iterator(IterType1 it1, IterType2 it2) : its_{it1, it2} {}

  zip_iterator& operator++() {
    ++std::get<0>(its_);
    ++std::get<1>(its_);
    return *this;
  }

  bool operator!=(const zip_iterator& rhs) {
    return std::get<0>(its_) != std::get<0>(rhs.its_) &&
           std::get<1>(its_) != std::get<1>(rhs.its_);
  }

  TupleType operator*() {
    // 될 것 같은데 컴파일 오류. std::pair는 됨.
    return {*std::get<0>(its_), *std::get<1>(its_)};
  }  
};
{% endhighlight %}
{% include code_caption.html caption="두 컨테이너만 받는 <code>zip_iterator</code> 구현: 컴파일 오류 있음" %}

`zip_iterator`는 `enumerate_iterator`와 [아주 비슷하다](https://github.com/minjang/minjang.github.io/blob/master/assets/2016/python_like_enumerate/python_like.cpp#L63). 다만 `IterType`과 `ElemType`이 컨테이너 두 개에 대해 각각 필요하고, `PairType` 대신 `TupleType`을 만들었다. 반복자도 두 개를 저장해야 하므로 튜플을 썼다. `!=` 연산자는 둘 중의 더 짧은 컨테이너 길이에서 종료가 되도록 하면 된다. 여기까진 순조롭게 진행된다.

그런데 이렇게 쉽게 완성될리가 없다.

라인 26의 `operator*`는 원소 참조자/값들을 모아 `std::tuple` 타입으로 만들려는 시도이다. `enumerate` 구현에서 보았듯이 `std::pair`의 생성은 `{...}`로 잘 되었다. 안타깝게도 이 글을 쓰는 시점에서 최신인 clang 3.8 기준으로도 `std::tuple`은 `std::pair`처럼 `{...}`로 생성할 수 없다. 컴파일러 오류를 보면 copy-initialization 상황에서 선택된 생성자가 `explicit`이라고 한다. [여기 예](http://en.cppreference.com/w/cpp/language/explicit)를 보면 이 에러를 이해하는데  도움이 된다.

다행히 이 문제는 C++ 위원회에서도 문제로 제기되어 C++17에서 고쳐질 예정이다. 이 문제를 다룬 [n4387 문서](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4387.html)를 보면 자세한 설명이 있으나 제대로 이해하기는 어렵다. 대략 보니까 `tuple` 구현에서 다소 실수한 점이 있다고 한다. 

{% highlight C++ linenos=table%}
std::tuple<int, int> foo_tuple() {
  return {1, -1};           // 에러: C++17 부터 지원됨
  return make_tuple(1, -1); // 항상 됨
  
  std::tuple<int, int> t{1, -1}; // 이것도 잘 됨
  return t;
}
std::pair<int, int> foo_pair() {
  return {1, -1};  // OK
}
{% endhighlight %}
{% include code_caption.html caption="C++14까지는 <code>tuple</code>을 <code>pair</code>처럼 간편하게 생성할 수 없음" %}

코드 7에 `pair`와 `tuple`의 생성 방법 차이가 잘 나타있다. 튜플을 중괄호(brace)로 초기화 하는 것이 C++14에서 안 되므로 `make_pair`를 부르거나 로컬에서 만든 뒤 반환하는 방법을 써야한다. 지역 변수 쓰는 것도 별로 예쁘지 않으니 `make_tuple`를 써보자.

{% highlight C++ %}
TupleType operator*() {
  // 상수 컨테이너나 프락시 반복자를 쓰는 컨테이너만 컴파일 됨.
  // 일반 컨테이너는 원소 참조자로 캐스팅 안 된다는 컴파일 오류.
  return std::make_tuple(*std::get<0>(its_), *std::get<1>(its_));
}
{% endhighlight %}
{% include code_caption.html caption="<code>make_tuple</code>은 decay된 타입으로 튜플 생성" %}

그런데 이것도 컴파일이 안 된다. 정확하게 말하자면, 상수 컨테이너나 프락시 반복자를 쓰는 컨테이너만 컴파일이 되지만, 그 외는 `make_tuple`의 결과 타입을 `TupleType`으로 캐스팅 할 수 없다는 오류가 난다. 예를 들어, `vector<int>` 두 개를 `zip`에 넣었을 때 `TupleType`은 `tuple<int&, int&>`이다. 하지만 `make_tuple`에서 얻어지는 타입을 조사하면 `tuple<int, int>`로서 참조자가 사라진 형태이다. 왜 그럴까?
`make_tuple`의 [설명](http://en.cppreference.com/w/cpp/utility/tuple/make_tuple)에 따르면 주어진 타입을 그대로 모아 튜플로 만들지 않고, `std::decay`로 한번 가공을 한 뒤 튜플을 만든다..

`std::decay`는[^decay] 주어진 타입에서 참조자 기호가 있다면 먼저 모두 제거한다. 그 뒤 타입의 성격에 따라 배열은 포인터로, 함수는 함수 포인터로, 일반 타입은 cv 한정자를 없앤다. 그래서 참조자가 사라진 것이다.

[^decay]: Decay의 일반적인 영어 뜻은 썩다/붕괴 같은 뜻으로 쓰이는데, 이 문맥에서는 타입을 "약하게 한다"는 뜻으로 받아들이면 좋다. 원래 타입은 더 많은 정보, 특히 배열 같은 건 배열 크기([extent](http://en.cppreference.com/w/cpp/types/extent)), cv 한정자 정보가 있었지만, decay된, 다시 말해, 약해진 타입은 이런 부차적인 타입 정보를 제거해서 다루기 쉽게한다.

> 그렇다면 왜 `make_tuple`은 decay된 타입을 쓸까? [이 SO 문답](http://stackoverflow.com/questions/24557000/why-does-make-optional-decay-its-argument-type/24557628#24557628)에서 잘 이해할 수 있다. 간략히 설명하면, 원래 타입의 정보를 그대로 살려서 튜플로 다루기가 힘들때가 있어서이다. 비슷한 예로 `auto`로 타입 추론할 때도 항상 값으로만, 또 cv 한정자는 생략된 채 추론된다. `make_tuple` 뿐만 아니라 `make_pair`도 decay된 타입을 쓴다.

> 또 하나 사소하지만 주의할 점: `{...}`로 생성한 페어나 튜플은 주어진 타입을 그대로 쓰므로 참조자 정보가 살아있다. 그래서 `enumerate`에서는 `{...}`로 `PairType`을 생성해도 잘 되었다. 간단하게 말하면 지금 C++17 기능만 제대로 구현되어 있어도 이런 삽질은 사라진다는 이야기다.

다행히 주어진 타입을 decay 시키지 않고 그대로 튜플을 만들어주는 함수, `std::forward_as_tuple`가 있다. 이 함수를 쓰면 비로서 완성할 수 있다.
<!-- [실제 라이브러리 구현](https://github.com/llvm-mirror/libcxx/blob/master/include/tuple#L889)을 보면 `make_tuple`과 다르게 입력 타입을 그대로 쓴다. -->

{% highlight C++ %}
TupleType operator*() {
  // C++17에서는 아래 코드가 잘 작동할 예정
  // return {*std::get<0>(its_), *std::get<1>(its_)};
  
  // std::ref로 참조자를 붙이는 것은 프락시 반복자에서 컴파일 오류
  // return std::make_tuple(std::ref(*std::get<0>(its_)),
  //                        std::ref(*std::get<1>(its_)));

  // 아래 코드는 make_tuple처럼 타입을 decay하지 않고 그대로 사용
  TupleType t{*std::get<0>(its_), *std::get<1>(its_)};
  return t;
}

TupleType operator*() {
  // 묵시적으로 TupleType으로 형변환
  return std::forward_as_tuple(*std::get<0>(its_), *std::get<1>(its_));
}

decltype(auto) operator*() {
  // 반드시 TupleType으로 형변환해야 함
  // 빠뜨리면 dangling rvalue reference 발생할 수 있음
  return static_cast<TupleType>(
      std::forward_as_tuple(*std::get<0>(its_),
                            *std::get<1>(its_)));
}
{% endhighlight %}
{% include code_caption.html caption="<code>zip_iterator::operator*</code>의 여러 구현 시도" %}

	
All three forms serve different purposes - auto is always a value, auto&& is always a reference, and decltype(auto) can be either, depending on the initializer.

http://stackoverflow.com/questions/21369113/what-is-the-difference-between-auto-and-decltypeauto-when-returning-from-a-fun
