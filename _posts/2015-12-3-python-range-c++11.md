---
layout: post
title: C++11으로 파이썬의 range 흉내내기
---

이른바 현대 C++, 다시 말해 C++11부터 시작하여 C++14, C++17를 포함하는 C++은 과거 C++98/03에 비해 새로운 기능이 많이 추가되어 거의 새로운 언어가 되었다. 덕분에 파이썬 같은 언어에서나 가능했던 표현도 C++에서 이제 가능하게 되었다. 앞으로 두세 차례의 포스팅에 걸쳐 파이썬 반복 구조 중 일부를 현대 C++로 만들어 보고자 한다.[^terms]

### 파이썬의 유도 변수 없는 for 루프

파이썬으로 0부터 9까지 숫자를 출력하는 반복문을 쓴다면 아래 같이 파이썬스럽게(Pythonic) 할 수 있다.

{% highlight python %}
for n in range(0, 10):
    print(n)
{% endhighlight %}

반면, C/C++에서는 다음 같은 방법이 일반적이다.

{% highlight C++ %}
for (int i = 0; i < 10; ++i)
  std::cout << i << '\n';
{% endhighlight %}

두 방법의 가장 큰 차이는 `i` 같은 **유도 변수**([induction variable](https://en.wikipedia.org/wiki/Induction_variable))의 유무이다.

C++에서는 `i`, `j` 같은 변수로 루프를 구성하는 것이 너무 당연해 보인다. 하지만 유도 변수를 감출 수 있는 루프가 자유도를 좀 잃어도 더 안전하다. C++에서는 유도 변수의 초기화(`i = 0`), 값 비교(`i < 10`), 값 증감(`++i`)을 모두 기술해야 하지만, 파이썬은 `range`로 이 과정을 모두 가린다. 간결한 코드뿐만 아니라 실수를 줄일 수 있다.

파이썬 코드에 있는 변수 `n`은 유도 변수가 아니다. 이 변수의 유효 범위는 오직 한 번의 순환뿐이다. 아무리 `n`을 현재 순환에서 고치더라도 이 값이 다음 순환으로 전달되지 않는다. 하지만 C++ 코드의 변수 `i`는 루프 전체에 걸쳐 값이 유지되는 유도 변수이다.

유도 변수를 직접 쓰지 않는 암묵적인 루프는 예상과 다르게 최적화에도 쉬울 수 있다. C/C++ 프로그래머들은 어셈블리에 가깝게 코딩하는 것이 최적화에 가깝다고 흔히 생각하지만 틀릴 때가 많다. 사실 위 예처럼 `int i` 같은 기본 자료형 유도 변수는 컴파일러가 분석하기 쉬워서 큰 문제는 아니지만, 그래도 모든 순환 동안 살아 있는 명시적인 변수를 제거하면 컴파일러가 더욱 쉽게 최적화를 할 수 있다.

파이썬 같은 동적 언어에서는 유도 변수를 이용한 방법이 성능으로도 더 손해이므로 가능하다면 꼭 암묵적인 루프를 써야 한다. C++에서 정수형 변수의 증감과 대입은 CPU에서 가장 빠른 연산으로 실행된다. 하지만 동적 언어에서 `i`는 어떠한 타입도 될 수 있으므로 파이썬에서 `i += 1`을 하는 과정이 C++의 그것보다 훨씬 복잡하고 시간이 오래 걸린다. 타입을 먼저 검사해야 하고 타입에 맞는 덧셈과 대입 연산을 수행해야 한다. 특별한 동적 컴파일러 최적화 기술을 쓰지 않으면 이 과정은 인라인 되지 못하고 모두 함수 호출로 되어버려 C++에 비해 수백 배 이상 느려질 수 있다.

굳이 _"유도 변수는 위험하다"_, _"induction variables [considered harmful](https://en.wikipedia.org/wiki/Considered_harmful)"_ 같은 주장을 절대로 하는 것이 아니다. 유도 변수를 감추는 루프가 분명 모든 형태의 루프 증감이나 제어 흐름을 기술하지 못하므로 유도 변수가 필요한 곳도 있다. 가능하다면 유도 변수를 피하는 것이 좋다 정도로 받아들이면 좋겠다.


### C++에서도 비슷하게 할 수 있다

그렇다면 C++에서 파이썬스러운 코드를 작성할 수 있을까? 된다. C++11 부터 추가된 **범위 기반 for 루프** ([range-based for loop](http://en.cppreference.com/w/cpp/language/range-for))로 할 수 있다. 파이썬 처럼 행동하는 `range` C++ 함수와 관련 클래스를 만들 수 있다면 다음과 같은 유도 변수를 드러내지 않는 루프를 만들 수 있다.

{% highlight C++ %}
// Python:
// for n in range(0, 10):
//     print(n)
//
// 아래 같은 코드를 만드는 것이 목표.
for (int i : range(0, 10))
  std::cout << i << '\n';
{% endhighlight %}

서론이 너무 길었다. C++에서도 `range` 같은 녀석을 만들어 보자.


### C++11의 범위 기반 for 루프

C++11 부터 재정의된 `auto`로  예를 들어 벡터 내의 원소 순회를 훨씬 간결하게 표기할 수 있다.

{% highlight C++ linenos %}
std::vector<int> A;
// 과거 C++: C++98, C++03
for (std::vector<int>::iterator i = A.begin(), e = A.end(); i != e; ++i)
  foo(A[i]);
// 현대 C++: C++11 부터
for (auto i = A.begin(), e = A.end(); i != e; ++i)
  foo(A[i]);
{% endhighlight %}

하지만 이렇게 모든 원소를 순회할 때는 앞서 말한 유도 변수/객체인 `i`를 직접 쓰지 않는 범위 기반 for 루프을 쓰면 더 깔끔하고[^1] 파이썬 코드와 거의 같은 코드를 만들 수 있다.

{% highlight python linenos %}
# Python
A = [1, 2, 3, 4, 5];
for i in A:
    foo(i);
{% endhighlight %}

{% highlight C++ linenos %}
// C++11
std::vector<int> A = {1, 2, 3, 4, 5};
for (int i : A)
  foo(i);
{% endhighlight %}

방금 예처럼 유도 변수가 C++의 반복자 객체, `std::iterator` 라면 더더욱 범위 기반 루프가 좋다. `auto`를 이용한 예에서 순환의 끝을 미리 변수 `e`에다 저장한 것을 주목하길 바란다. `i != A.end()`로 써도 괜찮긴 하지만, 혹시나 `end()`가 매번 호출되지 않을까 걱정되어 저런 코드로 쓸 때가 흔하다. 반복자의 증감 연산도 전위냐 `++i`, 후위냐 `i++`에 따라 미묘한 성능 차이가 있을 수 있다. 유도 변수가 이렇게 클래스 객체가 되니 사소한 루프에서도 신경 써야 할 점이 더 늘었다. 반면 범위 기반 루프는 아예 이런 반복자를 프로그래머가 신경 쓰지 않도록 하므로 컴파일러가 최적의 방법으로 코드를 생성할 수 있다. 변수 하나가 줄었으니 버그가 날 소지도 줄어든 것도 이점이다.[^2]

그렇다면 이 범위 기반 for 루프의 비밀은 무엇인가? 혹시 STL 객체하고만 작동할까? 아니다. 일반 배열도 되므로 이건 아닐 것이다. 답은 의외로 간단하다. 사실 그냥 간편 문법(syntactic sugar)에 지나지 않는다.


C++ 표준[^3]에 따르면 범위 기반 for 루프 문법은

<p><div class="highlight"><pre style="padding: .25rem .5rem; word-break: break-all; word-wrap: break-word;">
for ( <span class="k"><i>for-range-declaration</i></span> : <span class="k"><i>for-range-initializer</i></span> ) <span class="k"><i>statement</i></span>
</pre></div></p>

아래처럼 풀어진다.
<p><div class="highlight"><pre style="padding: .25rem .5rem;">
{
  auto && __range = <span class="k"><i>for-range-initializer</i></span>;
  for ( auto __begin = <span class="mi">begin-expr</span>,
             __end = <span class="mi">end-expr</span>;
        __begin != __end;
        ++__begin ) {
    <span class="k"><i>for-range-declaration</i></span> = *__begin;
    <span class="k"><i>statement</i></span>
  }
}
</pre></div></p>

여기서 <span class="highlight"><code style="background-color: white" class="mi">begin-expr</code></span>와 <span class="highlight"><code style="background-color: white" class="mi">end-expr</code></span>은 다음처럼 정의된다:

- <span class="highlight"><code style="background-color: white" class="k"><i>for-range-initializer</i></code></span>가 배열일 때, 각각 `__range`와 `(__range + __bound)`가 되고 `__bound`는 배열의 원소 개수이다. 원소 개수를 알아낼 수 없다면 잘못된 꼴(ill-formed)이 된다.
- <span class="highlight"><code style="background-color: white" class="k"><i>for-range-initializer</i></code></span>가 클래스 타입이고 `begin` 또는 `end` 이름의 멤버가 있다면 각각 `__range.begin()`과 `__range.end()`가 된다.
- 그 외라면 각각 `begin(__range)`와 `end(__range)`가 되는 적절한 함수를 찾는다.

요약하면, 특별한 마법이 있는 것이 아니라 루프의 시작과 끝을 `begin`과 `end` 메서드/함수로 얻을 수 있고, 전위 증가, `!=`, 참조(`*`) 연산자만 있으면 범위 기반 for 루프을 쓸 수 있다는 뜻이다. 이 배경 지식으로 이제 `range`를 설계해보자.

### `range` 함수의 설계

파이썬 내장 함수 `range`는 [두 가지 형태](https://docs.python.org/3/library/functions.html#func-range)로 있다.

<pre>
<b>range</b>(<i>stop</i>)
<b>range</b>(<i>start</i>, <i>stop</i>[, <i>step</i>])
</pre>

이 포스팅에서는 시작과 끝만 받는 `range` 함수를 만들어 본다. 인덱스 형으로 `int`만 고려 하는 것이 아니라 `double`, `int64_t` 등도 될 수 있도록 템플릿으로 설계 해보자.

{% highlight C++ %}
template<typename T>
something range(T start, T stop);
{% endhighlight %}

본격적으로 설계할 것은 `range` 함수가 아니라 `something`에 해당하는 클래스이다. `range` 함수는 단지 `something` 클래스를 생성해서 돌려줄 뿐이다. 이 클래스 이름을 짓기 전에 범위 기반 for 루프이 어떻게 변환되는지 직접 `range(0, 10)`을 대입해보자. 그러면 아래 코드가

<p><div class="highlight"><pre style="padding: .25rem .5rem;">
for ( <span class="k"><i>int i</i></span> : <span class="k"><i>range(0, 10)</i></span> ) <span class="k"><i>foo(i);</i></span>
</pre></div></p>

이렇게 펼쳐질 것이다.

<p><div class="highlight"><pre style="padding: .25rem .5rem;">
{
  auto && __range = <span class="k"><i>range(0, 10)</i></span>;
  for ( auto __begin = <span class="mi">__range.begin()</span>,
             __end = <span class="mi">__range.end()</span>;
        __begin != __end;
        ++__begin ) {
    <span class="k"><i>int i</i></span> = *__begin;
    <span class="k"><i>foo(i);</i></span>
  }
}
</pre></div></p>

이제 무엇을 만들어야 하는지 더 확실해졌다. 구체적으로 두 개의 클래스와 다음과 같은 메서드 또는 연산자를 만들어야 한다.

- `__range`에 해당하는 클래스[^4]: `begin()`과 `end()` 메서드 구현.
- `__begin`과 `__end`에 해당하는 클래스: `!=`, `++`(전위), `*` 연산자 구현.

이 두 클래스를 각각 `range_impl`, `range_iterator`로 이름 짓고 만들어 보자.


### `range_impl` 클래스 구현

`range_impl`의 주요 임무는 `range` 함수로부터 시작과 끝 인덱스를 받아 기억하고 시작과 끝 반복자를 반환하는 것이다.

{% highlight C++ linenos=table %}
template<typename T>
class range_impl {
  const T start_;
  const T stop_;

public:
  range_impl(T start, T stop) :
    start_{start}, stop_{stop} {}

  range_iterator<T> begin() const {
    return range_iterator<T>{start_};
  }

  range_iterator<T> end() const {
    return range_iterator<T>{stop_};
  }
};
{% endhighlight %}

특별한 설명이 필요 없을 정도로 비교적 직관적이다. 혹시나 `start_{start}` 같은 구문이 익숙하지 않다면 C++11의 [uniform initialization](https://en.wikipedia.org/wiki/C%2B%2B11#Uniform_initialization)를 참고하기 바란다.


### `range_iterator` 클래스 구현

어떻게 보면 `range_iterator` 클래스가 핵심이다. 직접 인덱스 변수를 가지면서
증감, 비교, 참조 연산을 한다.

언뜻 `std::iterator`로 부터 상속을 받아야 할 것 같지만 C++ 표준에서 보았듯이 이런 제약은 없다. 그저 최소한으로 필요한 연산자 3개만 구현하면 된다. 물론 STL 수준의 라이브러리로 만들려면 더욱 엄밀한 구현이 필요하지만 여기서는 최소한의 기능만 만들어 본다.

{% highlight C++ linenos=table %}
template<typename T>
class range_iterator {
  T cur_;

public:
  range_iterator(T init) : cur_{init} {}

  range_iterator& operator++() {
    cur_ += step_;
    return *this;
  }

  bool operator!=(const range_iterator<T> &rhs) const {
    return cur_ != rhs.cur_;
  }

  T operator*() const {
    return cur_;
  }
};
{% endhighlight %}

뭔가 현대적 C++ 문법이 많이 나올 것 같았지만, 그냥 기초적인 연산자 오버로딩만 지식만 있으면 되었다. 너무 간단하지 않은가? 생각보다 간단한 구현만으로 파이썬의 `range`를 그럴싸하게 흉내 낼 수 있게 되었다.

아 참, 마지막으로 `range` 함수를 완성하자.

{% highlight C++ linenos=table %}
template<typename T>
range_impl<T> range(const T start, const T stop) {
  return range_impl<T>{start, stop};
}
{% endhighlight %}

실제 사용 예는 이러하다.

{% highlight C++ linenos=table %}
for (int i : range(1, 3))
  cout << i << '\n';
for (double i : range(7.5, 9.5))
  cout << i << '\n';
{% endhighlight %}

{% highlight bash %}
1
2
7.5
8.5
{% endhighlight %}

다시 말하지만 이건 최소한의 구현이다. 개선한다면 구현체를 별도의 `namespace`로 두면 좋을 것이고 `range_iterator`를 `range_iter`의 중첩 클래스로 하는 것도 좋다. `range_iter` 생성자를 `private`로 막고 `friend`
 함수 권한을 `range` 함수에 주는 것도 필요하다.


### 결론

C++11부터 시작된 현대 C++의 새로운 기능으로 파이썬, 루비 같은 언어에서나 가능했던 표기도 점점 가능해지고 있다. 파이썬의 `range` 함수를 예제로 왜 유도 변수를 직접 쓰지 않는 루프가 좋은지 먼저 이야기했다. 그리고 C++11의 범위 기반 for 루프로 이를 구현해보았다. 표준 문서에 근거해 범위 기반 루프가 어떻게 정의되는지를 알고 나니 구현은 뜻밖에 간단하였다.

#### 더 생각해볼 문제

- 과연 성능은 어떠할까? 반복자 클래스의 오버헤드가 얼마나 될까?
- _step_ 기능을 구현한다면? 관심 있는 분은 연습 문제로 풀어보면 재밌을 것이다.
- 파이썬에는 `enumerate`라는 루프 지원 함수가 있다. 이것 역시 현대 C++로 만들 수 있다. 아마도 다음 글에서 이 이야기를 할 것 같다.

[^terms]: Modern C++를 '모던' 대신에 '현대' C++로 번역하였다. Loop는 루프, 반복, 순환 정도로 번갈아 썼다.
[^1]: 어떨 때는 아예 `for` 루프 자체를 감춰버리는 [std::accumulate](http://en.cppreference.com/w/cpp/algorithm/accumulate) 같은 알고리즘을 쓸 수도 있다.
[^2]: 하지만 이 블로그의 제목처럼 악마는 디테일에 있다고 범위 기반 for 루프에서 실수를 저지를 수도 있다. 이 이야기는 나중에 기회가 되면 하고 관심있는 독자는 [이 글](http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2014/n3853.htm)을 읽어 보길 바란다.
[^3]: 2015년 12월 현재 진행중인 최신 C++17 [N4567](https://github.com/cplusplus/draft/blob/master/papers/n4567.pdf) § 6.5.4에서 가져왔고 색상 강조는 직접한 것. C++11의 표준 문서인 [N3376](https://github.com/cplusplus/draft/blob/master/papers/N3376.pdf)과 C++14 [N4140](https://github.com/cplusplus/draft/blob/master/papers/n4140.pdf)과 약간의 서술 차이가 있긴 하다.
[^4]: `auto && __range`에서 `&&`는 우측값 참조가 아닌 _만능_ 참조, [universal reference](https://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers)이다. C++17에서는 이를 _전달_ 참조, [forwarding reference](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4164.pdf)로 부른다. 만능/전달 참조 번역은 필자 맘대로 한 것이므로 주의할 것!
