---
layout: post
title: 현대 C++로 파이썬의 enumerate 만들어보기
---


- [C++11으로 파이썬의 range 흉내내기](/2015/12/03/python-range-c++11/)를 먼저 읽어 볼 것을 추천합니다.

<!-- <span style="font-size: 1.5rem; vertical-align: -0.25rem">☞</span>  -->

파이썬 내장 함수 중 `enumerate`가 있다. 유도 변수 없는 루프에서도 매 순환마다 하나씩 증가하는 인덱스를 쓸 수 있게 해준다. [함수 원형](https://docs.python.org/3/library/functions.html#enumerate)과 사용 예는 다음과 같다.

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

유도 변수(induction variable)를 피할 수 있다면 피하는 것이 좋다는 건 [이전 글](/2015/12/03/python-range-c++11/)에서 잘 설명하였다. 그런데 위 예 같이 매 순환마다 인덱스를 얻고 싶다면 결국 유도 변수를 사용해야 할 것 같다. 다행히 `enumerate` 같은 방법으로 현재 순환 내에서만 유효한 인덱스 변수, `i`를 얻어낼 수 있다. 파이썬을 부러워만 할 필요 없다. C++11으로 `enumerate`를 얼마든지 만들 수 있다.

### C++ enumerate 요구조건 정하기

 개념적으로 C++ `enumerate`는 주어진 컨테이너 `A`의 반복자로부터 얻은 원소와 인덱스 변수 함께 매 순환마다 돌려준다. 그래서 범위 기반 for 루프에서 `A`를 순회하면서 인덱스도 같이 얻을 수 있도록 한다. 언뜻 보면 간단해 보인다. 그런데 이제 자세히 알게되겠지만, `enumerate`는 고려할 사항이 상당히 많다. 먼저, `enumerate`가 실제 C++ 코드에서 어떻게 쓰일지를 생각하자.

아마도 가장 기본적인 예는 아래처럼 될 것이고 출력도 파이썬의 결과와 같을 것이다.

{% highlight C++ linenos=table%}
vector<string> A = {"foo", "bar"};
// Python의 "for i, e in enumerate(A, 1)"과 비슷하게
for (std::pair<size_t, string&> p : enumerate(A, 1))
  cout << p.first << ": " << p.second << '\n';
{% endhighlight %}

이 외에도 다양한 형태로 `enumerate`를 사용할 수 있을 것이다. 자세한 논의를 하려면 다음 두 가지를 생각해야 한다:

- 매 순환마다 얻는 `std::pair<>`에 해당하는 인덱스-원소 쌍 타입.
- 함수 인수(argument) `A` 자리에 허용되는 컨테이너 타입.

#### `enumerate`의 인덱스-원소 쌍의 타입

C++ `enumerate`는 범위 기반 루프에서 컨테이너 `A`의 원소와 인덱스 값을 묶은 쌍을 매 순환마다 준다. 파이썬은 동적 언어와 복수 반환 값[^mutiret] 덕분에 우아하게 `for i, e in enumerate(A)` 처럼 표현되지만, C++은 어쩔 수 없이 `std::pair`나 `std::tuple` 같은 것을 써야 한다. 직접 클래스를 만들 수도 있다. 여기서는 간단한 `std::pair`를 선택하자.

[^mutiret]: 복수 반환 값은 C++에서는 원래 안 되지만, C++11의 `std::tuple`과 `std::tie`로 [대략 흉내](http://en.cppreference.com/w/cpp/utility/tuple/tie)를 낼 수 있다. 안타깝게도 `tie` 묘수를 범위 기반 for 루프에는 쓸 수가 없다.

<!-- 단점은 약간 모호한 `first`와 `second`라는 이름으로 인덱스와 원소에 각각 접근해야 한다는 점이다. 그런데 이미 `std::map`등 에서도 (key, value) 쌍이 `pair`의 `first`, `second`로 접근되니 큰 문제는 없을 것이다. -->

인덱스 값은 플랫폼에서 최대 객체 크기까지 표현할 수 있는 <a href="http://en.cppreference.com/w/cpp/types/size_t"><code>size_t</code></a>으로 골랐다. STL 컨테이너는 별도로 `container::size_type`이라는 타입을 두는데 `size_t`와 [사실 상  같다](http://stackoverflow.com/questions/918567/size-t-vs-containersize-type). `size_t`는 또한 배열 인덱스로도 안전하게 쓰이므로 이 값을 쓰는 것이 타당해 보인다. 그런데 파이썬의 `enumerate`는 시작 오프셋으로 음수 값도 줄 수 있다. 음수까지 지원하려면 부호 있는 `size_t`인  POSIX의 <a href="http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_types.h.html"><code>ssize_t</code></a>로 선언하면 될 것이다.

예에서 벡터 `A`의 원소를 `string&` 처럼 참조로 받았다. 벡터 `A`가 상수형이 아니므로 참조자로 받으면 루프를 돌면서 바로 원소 값을 변경할 수 있다. 읽기만 한다면 `const string&`으로 받으면 좋다. 아니면 그냥 `string`으로 복사 받아 원 벡터에 영향 없이 변경할 수도 있다. 원소는 이렇게 값, 상수 참조, - 컨테이너가 수정을 허용한다는 조건 하에 - 일반 참조가 모두 가능해야 할 것이다. 이것은 결국 컨테이너 원소를 참조자 또는 상수 참조자로만 받아도 된다는 이야기이다. 값 형태는 얼마든지 참조자로부터 복사해서 얻을 수 있기 때문이다.
파이썬과 다르게 이런 부분에선 C++이 확실히 자유도가 높다. 물론 실수와 어려움의 대가가 따르지만.[^proxy]

[^proxy]: `vector<bool>` 같이 프락시(proxy) 반복자를 이용하는 컨테이너에서는 `bool&`로 받을 수 없다. 이 프락시 반복자는 널리 알려진 [악명 높은 문제](http://www.gotw.ca/publications/mill09.htm)로 지금 프락지 반복자를 [표준에 넣는 노력](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0022r1.html)도 진행 중이다.

요약하면 아래와 같다:

- 반복자의 인덱스 타입은 `size_t`. 원소 타입은 상수 참조자, 경우에 따라 참조자를 허용하며 이 둘은 `std::pair<IndexType, ElementType>`으로 묶는다. `ElementType`의 상수 여부는 결국 입력 벡터/컨테이너의 상수 성질에 달려있다.

여기서 `for (pair<...> e : enumerate(A))` 대신에 간편한 `for (auto e : enumerate(A))`로 쓰면 안 되냐라고 물을 수 있다. 당연히 된다. 다만 이럴 때는 `ElementType`에 해당하는 부분을 맘대로 조절할 수 없게 된다. 뒤에서 볼 테스트 예에서 여러 `auto`의 경우에 대해 다룬다.

#### 어떤 컨테이너가 가능한가? 배열은 되는지?

간단하게 `std::vector<string>`를 예에서 생각해보았다. 그런데 `A`에 올 수 있는 타입은 어떤 것일까? 가능한 타입의 상한선은 명확하다: _범위 기반 for 루프에 쓸 수 있는 모든 것이다._
앞서 살펴 본 범위 기반 루프의 [정의](/2015/12/03/python-range-c++11#range-for-definition)를 상기하며 하나씩 생각해보자. 최종 목표는 `enumerate`가 이 모든 것을 지원하는 것이다.

1. STL 자료구조 중 컨테이너 어댑터인 `stack`, `queue`, `priority_queue`를 제외한 [모든 컨테이너](http://en.cppreference.com/w/cpp/container), 예를 들어, `list`, `unordered_map`을 범위 기반 루프에서 쓸 수 있다. 이 두 그룹의 표면적 차이는 컨테이너의 `begin()`과 `end()` 메서드를 지원 유무이다. 일례로 `stack`은 `begin()` 메서드가 없을 뿐더라 전역 함수 `std::begin` 역시 `stack` 타입을 받을 수 없다. 보다 엄밀히 말하자면 [forward iterator](http://en.cppreference.com/w/cpp/concept/ForwardIterator)가 되는 컨테이너는 범위 기반 for 루프에서 쓸 수 있다.

2. C/C++ 일반 배열도 범위 기반 for 루프에서 쓸 수 있다. 정의에서도 명확하게 배열을 명시하고 있다. 또한, 전역 컨테이너 접근 함수 <code><a href="http://en.cppreference.com/w/cpp/iterator/begin">std::begin</a></code>과 <code><a href="http://en.cppreference.com/w/cpp/iterator/end">std::end</a></code>도 배열을 받을 수 있다. 힌트를 주자면 `std::begin`과 `std::end`는 이번 구현에서 중추적인 역할을 한다. 두 함수의 원형과 사용법을 꼭 보고 자주 쓸 것을 권장한다.[^begin]

3. 컨테이너 `A`는 `const` 타입도 될 수 있다. 이럴 때는 순회할 때 `A`의 원소 변경을 막아야 한다. 보다 일반적으로 말하자면 [cv qualifiers](http://en.cppreference.com/w/cpp/language/cv)가 허용된다. 하지만 C++ 컨테이너에서 `volatile`을 쓰는 일은 거의 없긴 할 것이다.

4. 앞서 구현한 `range`처럼 사용자 정의 타입도 조건만 맞다면 범위 기반 for 루프에 쓸 수 있다. 반드시 STL 컨테이너나 `std::iterator` 인터페이스를 따를 필요가 없다.

5. `A` 같은 변수를 거치지 않고 바로 컨테이너를 `enumerate` 내에서 정의할 수 있다. 함수 리턴값으로도 바로 받을 수 있다. 이 부분에서 이해하기 참 까다로운 C++11의 우측값 참조(rvalue reference), 포워딩/유니버셜 레퍼런스(뒤에서 설명), 퍼펙트 포워딩 같은 것들이 나온다. 이 글에서도 대략 이야기할 것이다.

6. C++11부터 지원되는 초기화 리스트([initializer list](http://en.cppreference.com/w/cpp/utility/initializer_list))도 바로 범위 기반 루프에 쓸 수 있다.

이상적으로 `enumerate`도 위의 여섯 가지를 모두 지원하면 좋을 것이다. 한번에 모든 것을 구현하기 어려우니 차근차근 구현해보자.


### 테스트 케이스 작성

설계한 요구 조건을 검사할 수 있는 테스트 케이스를 만들자. 보기 쉽도록 세 덩이로 나누었다. 각각의 예상 결과도 함께 나열하였다.

#### 테스트 세트 1, 2

<script src="https://gist.github.com/minjang/b69380c0773dafe9cbe5.js"></script>

<pre>
[TEST] vector&lt;string&gt;
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
[TEST] array
100: foofoo
101: barbar
102: bazbaz
100: foofoo
101: barbar
102: bazbaz
</pre>

명시적인 `pair<IndexType, ElementType>` 대신에 `auto`를 쓴 예도 포함했다. 실제 어떤 타입으로 추론되었는지 알려면 C++ 자체의 `typeid`를 쓸 수도 있는데 참조자와 cv 한정자가 생략되므로 boost의 `type_id_with_cvr`를 써야 정확한 타입을 알 수 있다. 자세한 내용은 [Effective Modern C++](http://shop.oreilly.com/product/0636920033707.do)의 Item 4를 참조하면 좋다.

#### 테스트 세트 3, 4
<script src="https://gist.github.com/minjang/f6ed075a7c44760d5677.js"></script>

<pre>
[TEST] const
1: foo
2: bar
3: baz
[TEST] range
0: 100
1: 101
2: 102
</pre>

#### 테스트 세트 5, 6
<script src="https://gist.github.com/minjang/a5ce4b47fd1c65162850.js"></script>

<pre>
[TEST] in-place through rvalue reference
0: 100
1: 101
2: 102
0: foofoo
1: barbar
2: bazbaz
0: foo
1: bar
2: baz
[TEST] initialization list
0: foo
1: bar
2: baz
</pre>

참조자, 상수 컨테이너, 배열, 우측값 등 여러 사항을 고려하니 테스트 케이스가 짧지 않았다. 이제 코딩할 시간이다.

### `enumerate` 함수 인터페이스 구현

임의의 컨테이너 또는 배열을 받아야 하므로 당연히 템플릿으로 설계해야 한다. 보통 템플릿으로 바로 시작하면 컴파일 에러가 복잡해 어려움을 겪기 쉽다. 그럴 땐 특정 타입에 대해 먼저 구현하고 템플릿화 할 수도 있다. `enumerate`은 컨테이너 타입 하나면 되니까 바로 템플릿으로 시작하자.

{% highlight C++ linenos=table%}
template<typename T /* 컨테이너 타입 */>
enumerate_impl enumerate(T &container /* 좌측값 참조자 */, size_t start = 0) {
  return enumerate_impl{/* TODO: arguments... */}
}
{% endhighlight %}

`range`와 비슷하게 `enumerate` 함수 얼개를 이렇게 만들 수 있을 것이다. 거의 팩토리 함수이므로 입력 인자를 받아 구현 객체인 `enumerate_impl`을 돌려준다.

라인 2에서 컨테이너를 일반적인 참조자 형태, `T &container`로 받았다. 그래야 입력 컨테이너의 불필요한 복사를 막고, `T`가 비 상수형일 때 원소 변경도 할 수 있다. 만약, `T`가 상수형이라면 어떻게 될까? 다행히 템플릿 인수 연역 법칙(template argument deduction)에 따르면[^deduct], `T`가 상수형이면 `T &container` 역시 상수형 참조자로 넘어간다. 굳이 `enumerate(const T &container)` 버전을 오버로드할 필요가 없다. 물론 오버로드할 수는 있고, 만약 `T`가 상수형이면 `const` 함수가 먼저 선택된다.
테스트 케이스 1~4까지는 이걸로 해결이 될 것이다.

[^deduct]: 템플릿 인수 타입 연역의 대부분은 직관적이지만 몇몇 규칙은 반드시 알아야 템플릿 프로그래밍을 제대로 할 수 있다. 대표적으로 만약 <code>enumerate(T container)</code>처럼 값으로 쓰인다면, <code>T</code>에 붙어있는 <code>const</code>와 <code>volatile</code> 속성, 다시 말해, cv 한정자(<a href="http://en.cppreference.com/w/cpp/language/cv">cv qualifiers</a>), 그리고 참조자 기호(`&`, `&&`)는 무시되는 법칙이 있다.  [레퍼런스](http://en.cppreference.com/w/cpp/language/template_argument_deduction)는 읽기가 어려우므로 다음 슬라이드를 추천한다: [새 탭에서 보기](http://www.aristeia.com/TalkNotes/C++TypeDeductionandWhyYouCareCppCon2014.pdf){:target="_blank"}.

<!-- 지금 구현은 `enumerate(T &)` 같이 참조자 형태로 템플릿 인자 `T`를 사용했다. 참조자 형태라면 `T`의 참조자 기호는 무시되지만[^refcol] cv 한정자는 보존된다. 따라서 이 원형 하나로 비 상수형과 상수형 컨테이너 모두 해결할 수 있다. 테스트 케이스 1~4까지는 이걸로 해결이 될 것이다. -->

<!-- `template<typename T> void foo(T t);`에서 `t`는 복사로 만들어진 새로운 객체이다. 그런데 굳이 `T`의 `const` 같은 성질까진 가져올 필요 없다는 이유에서 이런 법칙이 나왔다. 반면, 참조자나 포인터 형일 때는 cv 한정자를 유지한다. 법칙만 보면 왜 갸우뚱한데 곰곰히 생각하면 그럴듯한 이유가 있긴 하다.  -->

[^ignorecv]: `template<typename T> void foo(T t);`에서 `t`는 복사로 만들어진 새로운 객체이다. 그런데 굳이 `T`의 `const` 같은 성질까진 가져올 필요 없다는 이유에서 이런 법칙이 나왔다. 반면, 참조자나 포인터 형일 때는 cv 한정자를 유지한다. 법칙만 보면 왜 갸우뚱한데 곰곰히 생각하면 그럴듯한 이유가 있긴 하다.

[^refcol]: 이 부분을 제대로 이해하려면 [reference collapsing rule](http://thbecker.net/articles/rvalue_references/section_08.html)을 알아야 한다. 별로 어렵지는 않다. `&`가 `&&`를 이긴다.


> 이미 현대 C++을 잘 아시는 분, 또는 Effective Modern C++ 책을 읽으신 분은 이것만으로는 테스트 케이스 5가 안 됨을 알 것이다. `enumerate`가 지금 좌측값 참조(lvalue reference)만 받을 수 있어서 우측값(rvalue)인 함수 반환 값이나 임시 객체를 함수 인자로 받을 수 없다. 결국 우측값 참조(rvalue reference), 더 나가 유니버셜/포워딩 레퍼런스(universal/forwarding reference)를 써야 하는데 이 문제는 마지막에 생각할 것이다.

사실 이 템플릿 인자 부분을 대충 넘겨도 괜찮다. 하지만 나중에 우측값까지 고려하려면 결국엔 이 부분을 확실히 공부하는게 좋긴 하다. 일단 복잡한 템플릿 이야기는 잠시 접어 두고 `enumerate` 구현에 집중해보자.

기억을 되살리고자 `enumerate`가 범위 기반 루프에서 어떻게 확장되는지 다시 살펴보자. 아래 같은 코드는

<p><div class="highlight"><pre style="padding: .25rem .5rem;">
for ( <span class="k"><i>std::pair<size_t, string&> p</i></span> : <span class="k"><i>enumerate(A, 1)</i></span> ) <span class="k"><i>foo(p);</i></span>
</pre></div></p>

이렇게 확장된다.

<p><div class="highlight"><pre style="padding: .25rem .5rem;">
{
  auto && __range = <span class="k"><i>enumerate(A, 1)</i></span>;
  for ( auto __begin = <span class="mi">__range.begin()</span>,
             __end = <span class="mi">__range.end()</span>;
        __begin != __end;
        ++__begin ) {
    <span class="k"><i>std::pair<size_t, string&> p</i></span> = *__begin;
    <span class="k"><i>foo(p);</i></span>
  }
}
</pre></div></p>

구현해야 할 것은 아래와 같다:

- `enumerate_impl` 클래스: `begin()`과 `end()` 메서드 구현.
- `enumerate_iterator` 클래스: `!=`, `++`, `*` 연산자 구현.


### `enumerate_impl` 클래스 구현

`enumerate_impl`는 주어진 컨테이너로부터 시작과 끝 반복자를 반환해야 한다. 이 클래스를 어떻게 설계해야 할까?

<p><center><i>잠시 생각할 시간을 가져보세요...</i></center></p>

아마도 가장 좋은 방법은 `enumerate_impl`이 컨테이너 참조자를 가지면서(복사 방지) 직접 <code><a href="http://en.cppreference.com/w/cpp/iterator/begin">std::begin</a></code>과 <code><a href="http://en.cppreference.com/w/cpp/iterator/end">std::end</a></code>를 이 컨테이너에 대해 호출하는 방법일 것이다. 이 방법의 장점은:

- `enumerate_impl` 클래스의 멤버 변수로 컨테이너 참조자와 시작 오프셋만 있으면 되고,
- `std::begin`과 `std::end`의 도움으로 상수형 컨테이너 뿐만 아니라 일반 배열도 한번에 처리된다.

다음과 같이 구현을 해보자.

{% highlight C++ linenos=table%}
template<typename T /* 컨테이너 타입 */>
class enumerate_impl {
  T &container_;
  const size_t start_;

public:
  enumerate_impl(T &container, size_t start) :
    container_{container}, start_{start} {}

  enumerate_iterator<T /* 또는 다른 타입? */> begin() {
    return {std::begin(container_), start_};
  }

  enumerate_iterator<T> end() {
    return {std::end(container_), 0};
  }
};

enumerate_impl<T> enumerate(T &container, size_t start = 0) {
  return {container, start};
}
{% endhighlight %}

역시 구현은 `range_impl`과 거의 같은 구조이다. `enumerate_imple::begin/end` 메서드는 곧 구현할 `enumerate_iterator`를 생성해서 돌려준다. `enumerate_iterator`는 결국 컨테이터 반복자에다 인덱스 변수 하나를 덧붙이는 것에 지나지 않는다. 따라서 그냥 `std::begin/end` 함수로 시작/끝 반복자를 얻고 인덱스 변수 값과 함께 넘겨 주면 될 것이다.

이렇게 비교적 구현이 간단한 이유는 전역 `std::begin/end`가 있어서이다. 상수 컨테이너를 넣으면 [상수 반복자를 반환](http://en.cppreference.com/w/cpp/iterator/begin)하므로 `std::cbegin/cend`로 구분해 호출할 필요도 없다. 무엇보다 일반 배열까지 바로 처리해지므로 한결 구현이 간결해진다. 배열은 메서드가 없으므로 만약 이 구현을 `T::begin/end`에 의존했다면 구현이 더 복잡해진다. 이렇게 전역 보조 함수는 아주 요긴하다. C++17부터는 `std::size`, `std::empty`, `std::data` 같은 다양한 [컨테이너 접근 함수](http://en.cppreference.com/w/cpp/iterator/size)가 지원된다. 어떻게 보면 파이썬의 `iter`, `size` 같은 [내장 함수](https://docs.python.org/3/library/functions.html)를 보는 것 같다.


### `enumerate_iterator` 클래스 구현

역시 핵심은 `enumerate_iterator` 클래스이다. 기본적인 구현은 `range_impl`과 거의 비슷하다. 원 컨테이너 `T`의 반복자와 인덱스 변수를 묶어서 돌려주면 된다. 그런데 두 타입 설정이 좀 까다롭다. 바로 원 컨테이너의 반복자에 해당하는 `IterType`과 컨테이너 원소 참조자 타입인 `ElemType`이다. 자세한 타입 설정은 뒤에서 하고 기본적인 구현부터 해본다. 참고로 타입 정의는 `typedef` 대신 C++11부터 가능해진 `using`를 이용하는 것이 이제 더 명확하다.

{% highlight C++ linenos=table%}
template<typename T /* e.g., T = vector<string> */>
class enumerate_iterator {
  using IndexType = size_t;
  using IterType = ??; // T::iterator? T::const_iterator?
  using ElemType = ??; // 원소에 대한 (상수) 참조자: e.g., string&
  using PairType = std::pair<IterType, ElemType>;

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

  bool operator!=(const enumerate_iterator &rhs) {
    return it_ != rhs.it_;
  }

  PairType operator*() {
    return {index_, *it_};
  }
};
{% endhighlight %}

생성자는 `enumerate_impl`로부터 받은 반복자와 인덱스 값을 받아 저장한다.
`operator++`, `operator!=`의 구현은 간단하다.

 `operator*`가 비로서 실제 프로그래머가 만나게 될 `pair<size_t, ElemType>`를 만든다. 보다시피 원래 컨테이너 반복자의 `*` 연산자로 얻어 돌려준다.

`IterType` 타입을 이제 제대로 따져보자. 쉽게 생각하고자 `T`를 `vector<string>`으로 놓아보자. `IterType`은 `enumerate_impl::begin/end`로 부터 `std::begin/end`의 리턴 타입과 같고, 결과적으로 `vector<string>::begin/end`의 리턴 타입이 된다. 바로 `vector<T>::iterator` 타입이다. 그리고 상수형 벡터일 때는 `vector<T>::const_iterator`가 된다.

<!--
여기서 잠깐 `std::vector`의 실제 표준 정의를 보는 것도 도움이 될 것이다. 구현은 라이브러리 제작자에 달려있지만 기본적인 개요(snopysis)는 C++ 표준이 명확히 정의한다. `vector`는 C++11/14/17 표준 § 23.3.6.1에 있고 [libc++ 예](https://github.com/llvm-mirror/libcxx/blob/master/include/vector){:target="_blank"}를 보는 것도 좋다.

{% highlight C++ linenos=table%}
template <class T, class Allocator = allocator<T> >
class vector
{
public:
    // types:
    typedef value_type&              reference;
    typedef const value_type&        const_reference;
    typedef implementation-defined   iterator;
    typedef implementation-defined   const_iterator;
    ...
    typedef                          T value_type;

    ...
    // iterators:
    iterator           begin() noexcept;
    const_iterator     begin() const noexcept;
    iterator           end() noexcept;
    const_iterator     end() const noexcept;
    ...
};
{% endhighlight %}

보다시피 `begin/end`는 `iterator` 또는 `const_iterator` 타입을 돌려주는 두 함수로 오버로드 되어있다. 컨테이너의 상수 여부에 따라 둘 중 하나가 선택된다.
-->

그렇다면 `IterType`을 어떻게 정의해야 할까? 그냥 `iterator`로 늘 설정해버리면 상수 벡터가 들어오면 컴파일 오류가 난다. `const_iterator`로 정의하면 일반 벡터의 내부 원소 수정을 못하게 된다. 결국 아래처럼 조건부로 타입을 골라야 할 것이다.

{% highlight C++ %}
using IterType = /* T가 상수형이면 */ T::const_iterator
                 /* 아니면 */ T::iterator;
{% endhighlight %}

다행히 C++의 <a href="http://en.cppreference.com/w/cpp/header/type_traits"><code>type_traits</code></a> 도움으로 되긴 된다. 그런데 좀 복잡해진다.

{% highlight C++ %}
// C++11:
using IterType = typename conditional<is_const<T>::value,
                   typename T::const_iterator,
                   typename T::iterator>::type;
// C++14/17:
using IterType = conditional_t<is_const_v<T>,
                   typename T::const_iterator,
                   typename T::iterator>;
{% endhighlight %}

<!-- `std::is_const`는 주어진 타입의 `const` 여부에 따라 참/거짓을 `value`로 가진다. `std::conditional`은 첫번째 템플릿 인자의 참/거짓 유무로 뒤 두 타입 중 하나를 고른다. -->
자세한 설명은 생략한다. 좀 복잡해도 여하간 상수/비상수 반복자를 `T`에 따라 선택적으로 정의할 수 있다.


그런데, 잊고 있던 것이 있었다. 바로 C/C++ 일반 배열! 일반 배열은 `iterator` 같은 내부 타입이 있을리가 없다. 또, 앞서 만든 `range`에서도 `iterator`라는 내부 타입을 정의하지 않았다. `range_iterator`를 `std::iterator`처럼 만들면 되긴 되지만 범위 기반 for 루프는 그것까지 요구하지 않았다. `IterType`을 이 두 가지까지 담으려면 더 복잡해진다. 뭔가 다른 우아한 해법을 찾아야할 것이다. 실마리는 바로 `T`의 반복자를 돌려주는 `std::begin/end`의 리턴 타입으로부터 유추하는 것이다.

C++11의 새로운 기능에 어느 정도 익숙한 분이라면 이제 대략 답을 떠올릴 수 있다. 바로 `auto`의 짝인 `decltype`이다. `is_const`, `conditional` 같은 건 죄다 삽질이었다.

### `decltype`을 이용한 `enumerate_iterator`

<a href="http://en.cppreference.com/w/cpp/language/decltype"><code>decltype</code></a>은 주어진 이름이나 식의 타입을 구하는 C++11부터 추가된 키워드이다. `decltype`도 자세하게 파고 들면 머리가 아픈데 그냥 타입을 알려주는 매직으로 생각하자.



 `std::vector` 같은 것은 STL [iterator traits](http://en.cppreference.com/w/cpp/iterator/iterator_traits)를 지원한다.

  `T::value_type`, `T::reference`를 쓸 수 있다. 비 상수형 벡터라면 원소도 직접 수정을 해야하므로 `T::reference`가 정답이 될 것이다. 참조자에서 상수 참조자나 값으로 캐스팅하는 것이 허용되기 때문이다. 참고로 `T::const_reference` 같은 타입은 필요 없는데 상수형은 `T`가 결정한다. 만약 `T`가 상수형이라면 `T::reference` 역시 상수 참조가 된다.





### `enumerate`의 포워딩 레퍼런스화 하기

 C++11부터 도입된 우측값 참조와 만능/전달 참조로[^uref] 보다 일반적으로 설계할 수 있지만, 일단 C++98/03처럼 좌측값 참조자로 받도록 만들자. `range`에서와 같이 `enumerate_impl`과 `enumerate_iterator`를 만들어야 한다.







[^uref]: 유니버셜/만능 참조: [universal reference](https://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers). C++17에서는 이를 포워딩/전달 참조, [forwarding reference](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4164.pdf)로 부른다. 만능/전달 참조 번역은 필자 맘대로 한 것이므로 주의할 것!

[^cv]: `volatile`도 포함되는데 사실 컨테이너 같은 C++ 자료구조에 `volatile` 쓸 일은 거의 없을 것이다. C++에서 `const`와 `volatile`은 흔히 [cv type qualifiers/한정자](http://en.cppreference.com/w/cpp/language/cv)로 분류되고 템플릿 및 `auto` [타입 연역/추론](http://www.aristeia.com/TalkNotes/C++TypeDeductionandWhyYouCareCppCon2014.pdf){:target="_blank"}에서 특별한 역할을 하기도 한다.

[^begin]: 나는 `A.begin()` 같은 꼴보다 `begin(A)`를 더 선호한다. 그 이유는 단순한데 한 글자 '.'를 적게 쓸 수 있기 때문. 그런데, `std::begin`으로 써야 한다면...


http://stackoverflow.com/questions/15927033/what-is-the-correct-way-of-using-c11s-range-based-for
