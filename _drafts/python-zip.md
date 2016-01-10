---
layout: post
title: 가변인자 템플릿으로 Python의 zip 만들어보기
---
현대 C++로 파이썬 흉내내기 시리즈:

1. [C++11으로 파이썬의 range 흉내내기](/2015/12/03/python-range-c++11/)
2. [현대 C++로 Python의 enumerate 만들어보기](/2016/01/07/python-enumerate-modern-c++/)

일러두기: C++ enumerate 보다는 쉽습니다.

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

파이썬을 잘 몰라도 직관적으로 이해가는 코드이다. 파이썬의 `zip`은 배열 `x`, `y`, `z`의 원소들로 이루어진 튜플(tuple)을 순환마다 돌려준다. `i` 번째 튜플 원소는 역시 직관적으로 `zip` 함수의 `i` 번째 인수에서 얻어낸 원소이다. 파이썬의 튜플은 배열처럼 `[i]`로 접근할 수 있다. 예에서 배열 `z`의 길이가 다른 배열의 길이보다 길지만 가장 짧은 배열의 길이까지만 순환되었음을 알 수 있다. 쉽게 예측 가능한 행동이다. 

두 번째 코드는 `enumerate`와 같이 쓴 예제인데 튜플을 바로 `t0`, `t1` 변수로 풀었다(unpack). 이런 튜플을 낱개 변수로 푸는 언팩은 C++에서 안 되겠지만, 현대 C++로 `zip`과 거의 흡사한 기능을 역시 만들 수 있다.

앞 두 글에서 C++11/14부터 추가된 범위 기반 for 루프, `std::begin/end`, 우측값 참조와 포워딩 레퍼런스, 무브 시맨틱과 퍼펙트 포워딩을 최대한 활용했다. 이번 글은 C++11에 추가된 *가변인자 템플릿*, *variadic template*을 더 쓴다. 

### 1. C++ zip 요구조건 정하기
C++로 `zip`을 만든다면 아마 이렇게 만들어 쓸 수 있을 것이다. 그 결과는 코드 1의 파이썬 예와 같을 것이다.

{% highlight C++ linenos=table%}
vector<string> x = {"e", "p", "i"};
vector<double> y = {2.718, 3.141};
vector<string> z = {"foo", "bar", "baz"};
// for (auto&& t : zip(x, y, z)) 또는
for (std::tuple<string&, double&> t : zip(x, y, z))
  cout << "(" << get<0>(t) << ", " << get<1>(t) << ", " << get<1>(t) << ")\n";
// for (auto&& p : enumerate(zip(x, y))) 또는
for (pair<size_t, tuple<string&, double&>> &&p : enumerate(zip(x, y)))
  cout << p.first << ": (" << get<0>(p.second) << ", " << get<1>(p.second) << ")\n";
{% endhighlight %}
{% include code_caption.html caption="파이썬 코드 예를 C++로 쓴다면 아마 이렇게?" %}


순환마다 원소 참조자들의 - 정확하게는 입력 컨테이너 반복자의 `*` 연산자가 돌려주는 타입 - 튜플을 돌려주도록 만들 수 있다.
