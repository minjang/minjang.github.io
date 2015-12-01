---
layout: post
title: 블로그 이전
---
2006년부터 2013년까지 글을 썼던 [이글루스 블로그](http://minjang.egloos.com/)를 떠나 [GitHub Pages](https://pages.github.com/)와 [Jekyll](https://jekyllrb.com/) 기반 블로그로 옮겨보았다. 이제는 블로그보다 다른 소셜 미디어가 더 주류가 되어버린 세상이다. 그래도 한번 쓰면 거의 잊어버리는 글이 아닌 글을 다시 블로그에 써 보려고 한다.

블로그 플랫폼으로 텀블러, 미디엄, 워드프레스를 시도해보았으나 결국 깃헙 페이지 + Jekyll로 결정하였다. 블로그 관리를 마치 프로그램 만들듯이 Git으로 관리할 수 있다는 점, 간결하지만 내가 제어할 수 있는 부분이 많다는 점이 매력적이다.

어렵고 불편한 점도 있다. CSS, Ruby/Rails, [Liquid](https://docs.shopify.com/themes/liquid-documentation/basics) 등에 익숙하지 않다면 시행착오를 꽤 겪는다. 버전 차이도 유의해야 한다. 2015년 11월, Jekyll은 버전 3.0까지 나왔지만 깃헙 페이지는 이보다 낮은 버전 2.4를 지원한다. 따라서 깃헙 페이지에 호스팅을 하려면 [버전을 미리 확인](https://pages.github.com/versions/)해야 한다. [여기서부터](https://github.com/github/pages-gem) 시작하면 좋다.

블로그 테마는 상당히 깔끔하고 단순한 [Lanyon](https://github.com/poole/lanyon)를 이용하였다. 웹 디자인에는 극히 초보적 지식만 있지만, 다른 여러 사이트와 크롬 브라우저의 멋진 "Inspect Element" 기능으로 내가 원하는 데로 대충 바꿀 수 있었다. 부족한 기능도 많다. 글 수정한 날짜 표기가 바로 지원되지 않는다. 태그 같은 것도 미지원. 그렇지만 직접 구현하는 재미도 쏠쏠하다. 프로그램 코드 구문 강조 기능이 있긴 하지만 너무 엉성해서 직접 손을 봐야 하는데 두 해결책이 있다: (1) [하이라이트 테이블 개선](http://flanneljesus.github.io/jekyll/2014-08-30/solving-jekyll-highlight-linenos/), (2) [CSS counter 이용](https://drewsilcock.co.uk/proper-linenumbers/). 일단 나는 첫번째 방법을 이용하였다.

{% highlight C++ linenos=table %}
// C++ 코드 예
#include <iostream>

int main(int argc, char *argv[]) {
  [](){ std::cout << "Hello, World!\n"; }();

  // 가로 스크롤 없음
  //
  // 0    1         2         3         4         5         6         7         8
  // 5678901234567890123456789012345678901234567890123456789012345678901234567890
  return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15 + 16;
}
{% endhighlight %}

{% highlight C++ linenos=table %}
{% endhighlight %}

{% highlight python linenos=table %}
# 파이썬 코드 예
def lcs(xstr, ystr):
    """
    >>> lcs('thisisatest', 'testing123testing')
    'tsitest'
    """
    if not xstr or not ystr:
        return ""
    x, xs, y, ys = xstr[0], xstr[1:], ystr[0], ystr[1:]
    if x == y:
        return x + lcs(xs, ys)
    else:
        return max(lcs(xstr, ys), lcs(xs, ystr), key=len)
{% endhighlight %}

Disqus 댓글 플러그인 설치가 생각보다 까다로웠다. 웹 검색에서 나오는 방법이 최신 Disqus 방법과 맞지 않을 수 있으므로 그냥 Disqus [공식 홈페이지가 제시하는 방법](https://disqus.com/admin/universalcode/)을 참고하는 것이 결국 옳았다. 댓글 개수 표시하는 방법도 [공식 홈페이지](https://help.disqus.com/customer/portal/articles/565624-adding-comment-count-links-to-your-home-page)를 차근차근 따라 하면 된다. 다만 URL 인식하는 부분은 세심한 주의가 필요하다.

필요하다면 이 블로그의 [소스 코드](https://github.com/minjang/minjang.github.io)를 보는 것도 도움이 될 수 있다.
