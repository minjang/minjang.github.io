---
layout: post
title: "C++로 함수형 언어 Scala 흉내 내기" 
---

다음 문제를 생각해보자.

> "주어진 문자열을 받아 알파벳 빈도 테이블을 만들어 반환하세요."

`"hello"`가 주어졌다면 `[['h', 1], ['e', 1],  ['o', 1], ['l', 2]]`를 돌려주면 된다. 편의상 문자열은 소문자만 있다고 하고 테이블은 특별히 정렬될 필요가 없다고 하자. 쉬운 수준의 코딩 인터뷰 문제이기도 하다. 이번 포스팅에서는 이 문제를 함수형 언어인 Scala (스칼라)로 먼저 풀어보고 이걸 C++14를 이용해서 옮겨 보자고 한다.

### 명령형 언어로 생각하기

바로 함수형 언어로 생각하면 머리가 아프니 지극히 명령형(imperative) 언어로 생각해보자. 편의상 C++로 써보았다.

{% highlight C++ linenos %}
std::map<char, int> wordOccurrences(std::string s) {
  std::map<char, int> t;
  for (char c : s)
    ++t[c];
  return t;
}
{% endhighlight %}

사실 무척 간단하다. 해시 테이블 하나면 된다.
혹시 라인 4에서 알파벳 `c`가 테이블 `t`에 없으면 어떡하느냐고 물을 수 있다.
C++의 `map`(보통 이진 탐색 트리로 구현)이나 `unordered_map`(일반적인 해시 테이블)에서 주어진 키가 없으면, 값 타입(여기서는 `int`)의 기본 생성자가 불러 (키, 값) 쌍을 넣는다.
정의되지 않은 쓰레기 값으로 초기화 되는 것도 아니다. `int x;`는 그러하지만 이 경우는 `int x = int();`처럼 불리므로 `c`가 처음으로 발견되었다면 안전하게 값은 0으로 초기화된다. 관련 스택오버플로우 [문답도 찾아볼 수 있다](http://stackoverflow.com/questions/2667355/mapint-int-default-values).
어쨌든 아주 직관적인 코드이다.

 
### 함수형 언어로 생각하기

이번에는 함수형 언어인 스칼라, Scala로 작성해보자.

사실 이 문제는 Coursera의 "[Functional Programming Principles in Scala](https://www.coursera.org/course/progfun)" 강좌에서 제공되는 숙제에서 힌트를 얻었다. 
숙제 자체는 이보다 훨씬 복잡한 애너그램([anagram](https://ko.wikipedia.org/wiki/%EC%96%B4%EA%B5%AC%EC%A0%84%EC%B2%A0))과 관련된 문제이고, 지금 푸는 빈도 테이블은 그중 작은 문제에 지나지 않는다.
이 문제를 C++로 최대한 스칼라와 비슷하게 만들어 보았는데, 그 과정을 다 적기에는 너무 벅차니까 간단한 예 하나만 이 글에서 다룬다.
참고로 이 수업은 강력 추천한다. 함수형 언어에 대해 잘 이해할 수 있고, 무엇보다 숙제 문제가 너무 좋다. 재귀적으로 생각하는 힘을 기를 수 있는 유익한 수업이다.

스칼라 문법을 몰라도 겁먹을 필요 없다. 나도 이 수업을 거의 3년 전에 들어서 솔직히 지금 스칼라 문법은 잘 모른다. 스칼라에서도 지극히 명령형으로 이 문제를 기술할 수 있지만, 함수형 언어의 철학에 따라 불변형 자료 구조와 고차원 함수 연산으로 풀어보자.

썰렁하지만 스칼라에서는 한 줄로 된다. REPL에서 실행해봤다.

{% highlight scala %}
scala> "hello".groupBy(e => e).map(p => (p._1, p._2.length))
res0: scala.collection.immutable.Map[Char,Int] = Map(h -> 1, e -> 1, o -> 1, l -> 2)
{% endhighlight %}

이 무슨 흑마법 같은 코드인가 싶은데 하나하나 뜯어 보자. `groupBy`와 `map` 함수만 이해하면 된다.

#### 스칼라의 groupBy 메서드

`groupBy` 메서드 벡터나 리스트의 원소들을 주어진 분류 조건으로 그룹핑 한다.[^python]

예를 들어, 임의의 숫자가 담겨있는 리스트가 있고, 짝수와 홀수로 구분하고 싶다고 하자. `groupBy`는 "주어진 숫자를 짝수 또는 홀수로 구분하는 <i>함수</i>"를 입력 인자로 받는다. 이 함수를 `groupBy`의 구별 함수(discriminator function)라고 칭한다. 이제 모든 리스트 원소에 대해 이 구별 함수를 실행해서 그 결과 값(짝수 또는 홀수)에 따라 원소들을 분류한다.

코드를 써보면 이러하다.

[^python]: 파이썬의 `itertools.groupby`와는 그 의미가 다르니 주의해야 한다.

{% highlight scala %}
scala> List(1, 2, 3, 4, 5).groupBy(e => e % 2 == 0)
//                                 ~~~~~~~~~~~~~~~
//                           주어진 e에 대해 짝/홀수 판단
res0: scala.collection.immutable.Map[Boolean,List[Int]] =
        Map(false -> List(1, 3, 5), true -> List(2, 4))
{% endhighlight %}

1부터 5까지 숫자 리스트가 있다.
짝/홀수로 구분하는 함수는 익명 함수, 즉 람다로 주어지는데, 자세한 스칼라 문법을 몰라도 꽤나 직관적이다.
이 익명 함수는 인자 `e`를 받아 짝수면 `true`를 반환한다. `groupBy`는 이 함수를 모든 원소에 대해 수행한다. 짝/홀수 결과 값은 `true` 또는 `false`인 `Boolean`이 되고, 여기에 속하는 값들은 숫자 리스트, `List[Int]`가 된다.
결과 값, `(false -> List(1, 3, 5), true -> List(2, 4))`이 자연스럽게 이해갈 것이다.
타입은 불변형 맵(immutable.Map)이고 키는 `Boolean`, 값은 `List[Int]` 꼴이다. 
참고로 C++은 지네릭(generic) 타입을 `list<int>`같이 쓰는데, 스칼라는 `List[Int]`로 쓴다. 배열 원소 접근과 같아 다소 혼란스럽기는 하다. 

짝수/홀수를 문자열로 바꾸어서 처리하면 보기 좋다.

{% highlight scala %}
scala> List(1, 2, 3, 4, 5).groupBy(e => if (e % 2 == 0) "Even" else "Odd")
res0: scala.collection.immutable.Map[java.lang.String,List[Int]] =
        Map(Even -> List(2, 4), Odd -> List(1, 3, 5))
{% endhighlight %}

자, 다시 원래 문제로 돌아가면, 주어진 문자열을 알파벳으로 구분하는 `groupBy`의 구별 함수는 항등 함수(identify function)로 주어져있다: `e => e`. 좀 헷갈린다. 그런데 생각해보면, 지금 하고자 하는 일이 "알파벳 별로 구분"하는 것이므로 그냥 주어진 알파벳을 그대로 그룹핑하는 기준 키로 쓰면 된다.

이제 아래 코드가 이해된다. 각 알파벳이 키가 되고 발견될 때 마다 각 리스트에 추가한다. 알파벳 리스트는 스칼라에서 `String`과 같다. `l`은 두 번 발견되었으므로 `ll`이 된다.

{% highlight scala %}
scala> "hello".groupBy(e => e)
res0: scala.collection.immutable.Map[Char,String] = Map(h -> h, e -> e, o -> o, l -> ll)
{% endhighlight %}

이제 알파벳 키 마다 있는 스트링을 그 길이로 바꿔주면, 출현한 알파벳의 빈도를 얻게 된다. `for`로도 할 수 있지만, 함수형 언어에서는 보다 우아한 방법을 써야 한다.

#### 스칼라의 map 메서드

Map, 우리말로 사상은 말 그대로 함수의 개념을 추상화한 것이다. `a`를 받아서 `f(a)`로 바꿔주는 셈이다. 역시 `map`도 익명 함수를 인자로 받는다. 주어진 원소를 받아 새로운 원소로 변환하는 일을 한다. 이 작업을 열거형 자료구조의 모든 원소에 대해 수행한다.

{% highlight scala %}
scala> "hello".groupBy(e => e).map(p => (p._1, p._2.length))
//                                 ~~~~~~~~~~~~~~~~~~~~~~~~
//                                p(쌍)에 대해 새로운 쌍을 만들어 반환  
res0: scala.collection.immutable.Map[Char,Int] = Map(h -> 1, e -> 1, o -> 1, l -> 2)
{% endhighlight %}

`map`에 주어진 익명 함수를 보자. `p`는 앞서 `groupBy`의 결과인 `Map[Char/*키*/,String/*값*/]`의 원소에 해당한다. 각 원소는 (키, 값) 형태의 쌍 혹은 튜플로 주어진다. `p._1`는 키인 `Char`에 해당하고, `p._2`는 값인 `String`에 대응된다. 알파벳 키는 그대로 가져가되 스트링을 이제 `length` 메서드로 길이로 바꾸면 된다.
비로소 얻고자 하는 최종 결과 값, `Map(h -> 1, e -> 1, o -> 1, l -> 2)`을 얻을 수 있다.

### C++로 비슷하게 할 수 있을까?

C++11부터 도입된 람다, 가변 인수 템플릿, `std::shared_ptr`등을 잘 쓰면 상당히 함수형스럽게 코딩할 수 있다. 별로 놀랍지 않다. 이미 예전부터 STL의 구조가 상당히 함수적이기 때문이다. `groupBy`와 `map`에 해당하는 C++ 함수를 만들면 스칼라와 비슷하게 코드를 만들 수 있을 것이다.

> 일러두기: 저는 템플릿 메타 프로그래밍 전문가도 아니고, 최신 C++ 문법도 실제 현업에서는 거의 쓰지 안/못합니다. 순전히 취미 수준으로 약간 하는 것이므로 이 코드의 품질은 전혀 보증하지 아니합니다. 보다 좋은 해법이 있으면 언제든지 환영합니다. 


#### C++로 groupBy 모사하기

사실 스칼라의 `groupBy`나 `map`은 `Map`이나 `List`의 메서드로 있다. 여기서는 그냥 전역 함수로 만들고, 임의의 컨테이너 `C`를 받도록 한다.

`groupBy`는 앞서 설명한 구별 함수인 익명 함수 하나를 받는다. 구별 함수의 인자는 컨테이너의 원소 타입이어야 하고, 반환 타입은 사용자 마음이다. 최종적으로 `std::map`을 만들어서 돌려주면 될 것이다.[^dig]

[^dig]: 바로 이 코드에 도달한 것은 아니고 처음에는 여러 삽질을 하였다. 처음에는 익명 함수를 `K (*f)(A)`와 같이 함수 포인터로 받으려고 했었다. 되기는 되는데 람다를 함수 포인터로 인식 시키는데 [흑마법](http://stackoverflow.com/questions/18889028/a-positive-lambda-what-sorcery-is-this/18889029#18889029)이 필요하고, 사용할 때도 익명 함수의 타입을 정해줘야 하는 큰 불편한 점이 있었다.

{% highlight C++ linenos %}
namespace fun {
// groupBy[K](f: (A) ⇒ K): Map[K, List[A]]
// 컨테이터 C의 모든 원소에 대해 f를 실행 후, 그 결과를 map으로 저장 및 반환
// C++14 이상 필요
template<typename C/*Container*/, typename F/*Discriminator*/>
auto groupBy(C&& c, F f) {
  using element_t       = std::decay_t<decltype(*std::data(c))>;
  using discriminator_t = decltype(f(*std::data(c)));
  std::map<discriminator_t, std::vector<element_t>> r;
  for (auto&& e : c)
    r[f(e)].emplace_back(e);
  return r;
}
}
{% endhighlight %}

핵심은 익명 함수 `f`와 컨테이너 변수 `c`로부터 필요한 두 타입을 추출하는 것이다.

먼저, 컨테이너의 원소 타입, `element_t`를 얻어 낸다. 라인 7에 나와있는데, `decltype`과 `std::data`/`std::begin`을 써서 원소 타입을 얻어내는 것은 [이전 포스팅](/2016/01/07/python-enumerate-modern-c++/)에서 자세히 설명했다.

여기서 새롭게 보는 것은 `std::decay_t`이다.[^decay] 이것은 주어진 타입에서 참조자 기호가 있다면 먼저 모두 제거한다. 그 뒤 타입의 성격에 따라 배열은 포인터로, 함수는 함수 포인터로, 일반 타입은 cv 한정자를 없앤다. 보다 다루기 쉽도록 타입을 약화한다.
왜 `decay`가 필요하냐면, 원소 타입에 참조자가 있을 수 있기 때문이다. 최종적으로 돌려주는 `std::map`의 값 타입은 `vector<element_t>`로 해야 한다. 그런데 `element_t`가 참조자면 컴파일부터 제대로 안 된다.

[^decay]: Decay의 일반적인 영어 뜻은 썩다/붕괴 같은 뜻으로 쓰이는데, 이 문맥에서는 타입을 "약하게 한다"는 뜻으로 받아들이면 좋다. 원래 타입은 더 많은 정보, 특히 배열 같은 건 배열 크기(extent), cv 한정자 정보가 있었지만, decay된, 다시 말해, 약해진 타입은 이런 부차적인 타입 정보를 제거해서 다루기 쉽게한다.

라인 8에서 `groupBy`의 구분 기준이 되는 타입을 `discriminator_t`로 이름 지었다. 이 타입은 익명 구별 함수의 반환형과 같으므로 `decltype`으로 간단하게 얻을 수 있다.

라인 9는 이제 이 두 타입으로 최종 `std::map`을 선언한다. 나머지 코드는 굉장히 간단하다. 범위 기반 for 루프로 컨테이너의 모든 원소 `e`에 대해, 구별 함수 `f`를 호출해 그 결과 값이 테이블의 키 값이 된다. 10줄도 안 되는 코드로 그럴듯하게 스칼라의 `groupBy` 함수를 흉내 냈다.[^check]

[^check]: `static_assert`와 `<type_traits>`를 쓰면 익명 함수의 타입을 체크할 수 있겠지만, 어차피 타입이 맞지 않으면 컴파일 에러가 난다. 다만 에러를 이해하기 어려울 뿐이다.
 
이제 스칼라로 했던 숫자 리스트의 짝/홀수로 구분을 C++로 해보자.

{% highlight C++ linenos %}
// val g = List(1, 2, 3, 4, 5).groupBy(
//   e => if (e % 2 == 0) "Even" else "Odd")
auto&& g = fun::groupBy(std::vector<int>{1, 2, 3, 4, 5},
  [](auto e) { return (e % 2 == 0) ? "Even" : "Odd"; });
  
// vector/pair에 대한 ostream << 출력을 오버라이딩 했다고 가정
for (auto&& e : g)
  std::cout << e << std::endl;
{% endhighlight %}

<pre>
# 출력 결과
<Even, {2, 4}>
<Odd, {1, 3, 5}>
</pre>

보다시피 스칼라 코드와 굉장히 흡사하다. C++14 덕분에 익명 함수 인자 `e`의 타입을 지정하지 않고 `auto`로 해도 된다. 표현은 당연히 스칼라보다 군더더기가 많다. 그래도 거의 1:1 수준으로 스칼라 코드와 대응이 된다.

참고로 벡터나 pair 타입 출력을 편히 하고자 아래처럼 `<<` 연산자를 오버라이딩했다. 코드는 스택오버플로우에서 가져 온 것을 약간 손본 것이다.

{% highlight C++ linenos %}
// 벡터 타입을 cout << 로 출력
template<class Ch, class Tr, class T>
auto operator<<(std::basic_ostream<Ch, Tr>& os, std::vector<T> const& v)
-> std::basic_ostream<Ch, Tr>& {
  os << "{";
  for (size_t i = 0; i < v.size(); ++i) {
    os << v[i];
    if (i != v.size() - 1)
      os << ", ";
  }
  os << "}";
  return os;
}

// pair 타입 출력
template<class Ch, class Tr, class T1, class T2>
auto operator<<(std::basic_ostream<Ch, Tr>& os, std::pair<T1, T2> const& p)
-> std::basic_ostream<Ch, Tr>& {
  return os << "<" << p.first << ", " << p.second << ">";
}
{% endhighlight %}


#### C++로 map 따라하기

스칼라 `map` 함수도 비슷한 원리로 만들면 된다. 연관 컨테이너 원소 타입은 `std::pair`이니까 이걸 받아 새로운 `std::pair`을 돌려주는 익명 함수가 필요하다. 코드는 역시 다음처럼 간단하다. 11줄 밖에 안된다!

{% highlight C++ linenos %}
namespace fun {
// def map[B](f: (A) ⇒ B): Map[B]
template<typename C/*Associative container*/, typename F/*map*/>
auto map(C&& c, F f) {
  using new_key_t   = decltype(f(*std::begin(c)).first);
  using new_value_t = decltype(f(*std::begin(c)).second);
  std::map<new_key_t, new_value_t> r;
  for (auto&& e : c) {
    auto&& mapped = f(e);
    r[mapped.first] = mapped.second;
  }
  return r;
}
}
{% endhighlight %}

이제 마지막으로 `groupBy`와 `map`을 동시에 써서 문제를 풀어보자.

{% highlight C++ linenos %}
// val f = "hello".groupBy(e => e).map(p => (p._1, p._2.length))
auto f = fun::map(fun::groupBy(string("Hello"), [](auto x) { return x; }),
              [](auto e) { return make_pair(e.first, e.second.size()); });
for (auto e : f)
  std::cout << e << std::endl;
{% endhighlight %}

<pre>
# 출력 결과
<e, 1>
<h, 1>
<l, 2>
<o, 1>
</pre>

C++ 특성상 `return`이나 `pair`를 만드는 과정이 덜 간결하지만, 이 정도면 사실상 스칼라 코드와 완전히 일대일 대응된다고 해도 틀린 말이 아니다.
참고로 `fun::map`을 `std::pair`가 아닌 `std::tuple`로도 일반화할 수 있다.


### 결론

나는 함수형 언어 전문가가 아니다. Monad가 뭔지 아직 잘 모른다. 그래도 함수형 언어에 보다 쉽게 접근하려면 익명 함수가 뭔지만 잘 알아도 된다. 보다시피 C++로도 그럴듯하게 스칼라의 함수형 작업을 흉내낼 수 있었다.
C++17 혹은 그 이후에 추가되는 기능으로 분명히 더 함수형 언어같은 코딩을 할 수 있을 것이다.

혹시나 함수형 언어에 대해 겁먹고 있는 분이라면, 아마도 해스켈 보다 배우기 수월한 스칼라를 권장한다. 이미 소개했지만 Coursera의 아래 두 강좌를 강력히 추천한다. "함수형 언어" 그러면 복잡한 람다 대수, [괄호 지옥](https://xkcd.com/297/), 모나드만 떠올렸는데 이 수업으로 훨씬 친근해졌다. 아울러 요즘 유행하는 Rx도 배울 수 있다.

- [Functional Programming Principles in Scala](https://www.coursera.org/course/progfun)
- [Principles of Reactive Programming](https://www.coursera.org/course/reactive)

> "그런데 그냥 맨 처음에 썼던 명령형식의 C++ 코드가 다 쉽고 더 빠르지 않을까요?"<br/> "그러게요. 왼쪽 뺨에 붙어있는 밥풀을 오른손으로 머리 뒤로 감아 떼는 격 같긴 하죠."