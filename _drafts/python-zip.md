---
layout: post
title: 가변인자 템플릿으로 Python의 zip 만들어보기
---
현대 C++로 파이썬 흉내내기 시리즈:

1. [C++11으로 파이썬의 range 흉내내기](/2015/12/03/python-range-c++11/)
2. [현대 C++로 Python의 enumerate 만들어보기](/2016/01/07/python-enumerate-modern-c++/)

일러두기: C++ enumerate 보다는 쉽습니다.

<pre>
<b>zip</b>(<i>*iterables</i>)
</pre>

{% highlight bash %}
$ python3
>>> x = ["e", "p"]
>>> y = [2.718, 3.141]
>>> for e1, e2, e3 in zip(x, y, ["foo", "bar", "baz"]):
...     print("({}, {}, {})".format(e1, e2, e3))
...
(e, 2.718, foo)
(p, 3.141, bar)
>>>
>>> for i, t in enumerate(zip(x, y)):
...     print("{}: ({}, {})".format(i, t[0], t[1]))
...
0: (e, 2.718)
1: (p, 3.141)
{% endhighlight %}
{% include code_caption.html caption="Python zip 예제: 여러 컨테이너를 묶어서 동시에 순환할 수 있다." %}


{% highlight C++ linenos=table%}
vector<string> x = {"e", "p"};
vector<double> y = {2.718, 3.141};
for (std::tuple<string&, double&> t : zip(x, y, {"foo", "bar", "baz"}))
  cout << "(" << get<0>(t) << ", " << get<1>(t) << ", " << get<2>(t) << ")\n";
for (pair<size_t, tuple<string&, double&>> &&p : enumerate(zip(x, y)))
  cout << p.first << ": (" << get<0>(t) << ", " << get<1>(t) << ")\n";
{% endhighlight %}
{% include code_caption.html caption="파이썬 코드 예를 C++로 쓴다면 아마 이렇게?" %}
