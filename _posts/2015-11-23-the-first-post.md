---
layout: post
title: 블로그 이전
---
2006년부터 2013년까지 글을 썼던 [이글루스 블로그](http://minjang.egloos.com/)를 떠나 [GitHub Pages](https://pages.github.com/)와 지킬([Jekyll](https://jekyllrb.com/)) 기반 블로그로 옮겨보았다. 이제는 블로그보다 다른 소셜 미디어가 더 주류가 되어버린 세상이다. 그래도 한번 쓰면 거의 잊어버리는 글이 아닌 글을 다시 블로그에 써 보려고 한다.

블로그 플랫폼으로 텀블러, 미디엄, 워드프레스를 시도해보았으나 결국 깃헙 페이지 + 지킬로 결정하였다. 블로그 관리를 마치 프로그램 만들듯이 Git으로 관리할 수 있다는 점, 간결하지만 내가 제어할 수 있는 부분이 많다는 점이 매력적이다. 어렵고 불편한 점도 있다. CSS, Ruby/Rails, [Liquid](https://docs.shopify.com/themes/liquid-documentation/basics) 등에 익숙하지 않다면 시행착오를 꽤 겪는다.

블로그 테마는 상당히 깔끔하고 단순한 [Poole](https://github.com/poole/poole) 기반의 [Lanyon](https://github.com/poole/lanyon)를 이용하였다. 웹 디자인에는 극히 초보적 지식만 있지만, 다른 여러 사이트와 크롬 브라우저의 멋진 "Inspect Element" 기능으로 내가 원하는 데로 대충 바꿀 수 있었다. 부족한 기능도 많다. 글 수정한 날짜 표기가 바로 지원되지 않는다. 태그 같은 것도 미지원. 구문 강조 기능도 있지만 만족스럽지 못하다. 그렇지만 직접 구현하는 재미도 쏠쏠하다.

혹시나 깃헙 페이지에 지킬 블로그 설치 하시는 분을 위해 고생한 점 몇 가지를 정리해보았다.

---

### Jekyll은 뭔가?

아주 간략히 지킬이 뭔지 설명하면 웹사이트 생성 도구이다. 마크다운, Liquid, YAML, HTML/CSS를 편집하고 지킬 서버를 작동하면 정적 웹페이지를 만들어준다. 블로그나 프로젝트 웹페이지로 안성맞춤이다. 깃헙 페이지는 웹사이트를 호스팅 해주는 곳인데 직접 이 지킬을 지원한다. 깃헙에 `username.github.io`라는 이름의 저장소를 만들고 거기에 지킬 소스 코드를 올리면 깃헙이 지킬을 실행시킨다.


### 로컬에 Github Pages + Jekyll 설치하기

지킬은 최신 버전을 다운로드 받아 로컬에서 얼마든지 테스트할 수 있다. 하지만 깃헙 페이지에 호스팅된 지킬은 최신 버전이 아니다. 2015년 11월, Jekyll은 버전 3.0까지 나왔지만 깃헙 페이지는 이보다 낮은 버전 2.4를 지원한다. 깃헙 페이지에 호스팅을 하려면 [버전을 미리 확인](https://pages.github.com/versions/)하고 로컬에도 맞춰서 설치해야 한다. 다행히 직접 할 필요 없고 [여기](https://github.com/github/pages-gem)를 참고하면 된다.

버전 뿐만 아니라 지킬 실행 옵션도 유의해야 한다. 특히 지킬의 `safe` 옵션이 [항상 켜져 있어서](https://help.github.com/articles/using-jekyll-with-pages/) 여러 플러그인을 사용할 수 없다. 로컬에서도 이 옵션을 `_config.yml`에 꼭 설정하고 테스트해야 한다.


### Disqus 댓글 플러그인 설치

Disqus 댓글 플러그인 설치가 생각보다 까다로웠다. 지킬 테마 중 바로 Disqus를 지원하는 것도 있으나 Poole/Lanyon은 그렇지 않다. 괜히 웹 검색에서 다른 누군가가 했던 방법을 따라 하지 말고, 그냥 Disqus [공식 홈페이지가 제시하는 방법](https://disqus.com/admin/universalcode/)을 참고하는 것이 결국 옳았다. 댓글 개수 표시하는 방법도 [공식 홈페이지](https://help.disqus.com/customer/portal/articles/565624-adding-comment-count-links-to-your-home-page)를 차근차근 따라 하면 된다.

Disqus는 URL을 키로 해서 특정 포스트마다 댓글을 달아주는데 이 URL 인식 부분에 세심한 주의가 필요하다. 자꾸만 댓글이 안 뜰 때가 있었는데 알고 보니 {%raw%}`this.page.title = {{page.title}}`{%endraw%} 부분에서 제목에 `'`만 있어도 되지 않았다. 삽질할 수 있는 곳이 너무 많았다. 이 블로그의 [소스 코드](https://github.com/minjang/minjang.github.io/blob/master/_includes/comments.html)도 도움이 될 수 있다. 댓글 개수 표시는 포스트 페이지에서는 [이렇게](https://github.com/minjang/minjang.github.io/blob/master/_layouts/post.html#L12), 인덱스 페이지에서는 [이렇게](https://github.com/minjang/minjang.github.io/blob/master/index.html#L16) 했다.


### 구문 강조 기능 손보기

지킬에는 자체 [구문 강조 기능](http://jekyllrb.com/docs/templates/#code-snippet-highlighting)이 있다. 하지만 그대로 쓰기에는 예쁘지 않아 직접 손을 봐야 한다. 최대한 [깃헙 Gist](https://gist.github.com/)처럼 비슷하게 만들어보았다. 단순한 외관 문제는 CSS를 열심히 손보면 된다. 하지만 소스 코드에 라인 번호를 넣는 기능에 문제가 많았다. 라인 번호 생성은 마크다운으로 글 작성 시 `highlight` 템플릿에 `linenos` 옵션을 넣으면 되는데 라인 번호가 소스 코드와 같이 선택되는 치명적인 문제가 있다. 대안으로 `linenos=table`을 쓰면 되는데 간혹가다 줄 바뀜(wordwrap)이 일어나서 라인 번호와 소스 코드가 일치하지 않는 일이 벌어질 수 있다.

총 세 가지 방법을 동원해보았다.

1. 테이블(`linenos=table`)을 고치는 [방법](http://flanneljesus.github.io/jekyll/2014-08-30/solving-jekyll-highlight-linenos/)
2. CSS counter와 `lineanchors` 사용하는 [방법](https://drewsilcock.co.uk/proper-linenumbers/)
3. CSS counter와 `linenos` 사용하는 [방법](http://www.cse.iitb.ac.in/~murukesh/2015/09/06/jekyll.html)

결론부터 말하면 방법 2가 가장 좋으나 깃헙 페이지가 지킬을 안전 모드로 작동해서 `lineanchors` 기능을 쓸 수 없었다. 그래서 나머지 두 방법을 구현해보았다.

첫 번째 방법은 `table.highlighttable` 관련 [CSS를 수정](https://github.com/minjang/minjang.github.io/blob/master/public/css/lanyon.css)하였다. 생각보다 많은 수정이 필요치 않다. 대부분 잘 작동하지만, 간혹 너무 긴 행에 대해 줄 바뀜 현상을 볼 수 있었다. 줄 잘림이 발생하면 라인 번호와 맞지 않게 된다.

- 테이블(`linenos=table`)을 이용한 구문 강조:
{% highlight C++ linenos=table %}
#include <iostream>

int main(int argc, char *argv[]) {
  [](){ std::cout << "Hello, World!\n"; }();

  // 가로 스크롤 없음. 드물게 wordwrap이 일어나서 라인 번호와 맞지 않은 일이 발생.
  //
  // 0   1         2         3         4         5         6         7         8
  // 678901234567890123456789012345678901234567890123456789012345678901234567890
  return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15 + 16;
}
{% endhighlight %}

세 번째 방법은 테이블을 쓰지 않는 <code>linenos</code> 옵션을 개선하는 것이다. 코드와 함께 라인 번호가 일단 생성되도록 한다. 그리고 CSS 카운터를 `:after` 또는 `:before`로 덧그린다. 이 카운터를 선택 안 되도록 막은 뒤, 원래 라인 번호를 감추면 된다. 문제점이 하나 있다면 긁어서 복사할 때 소스 코드 앞에 공백 하나가 덧붙여진다는 점. 약간 사소한 미관 문제가 더 있으나 블로그에 올리는 소스 코드 라인이 1,000 이상 넘을 일이 없으므로 무시할 만하다. 구현은 `pre span.lineo` 관련 [CSS를 수정](https://github.com/minjang/minjang.github.io/blob/master/public/css/syntax.css)하면 된다.

- CSS 카운터와 <code>linenos</code>를 이용한 방법:
{% highlight C++ linenos %}
#include <iostream>

int main(int argc, char *argv[]) {
  [](){ std::cout << "Hello, World!\n"; }();

  // 가로 스크롤 없고 항상 라인 번호와 같은 위치. 라인 번호는 선택 안 되도록 함.
  // 다만 소스 코드 복사 시 앞에 공백 하나가 더해지는 문제가 있음.
  //
  // 0   1         2         3         4         5         6         7         8
  // 678901234567890123456789012345678901234567890123456789012345678901234567890
  return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15 + 16;
}
{% endhighlight %}

두 번째 방법은 사용할 수 없어 결국 포기했는데 가장 좋은 방안이긴 하다. 세 번째 방법과 거의 흡사하나 `lineanchors`라는 방법으로 테이블을 안 쓰면서도 소스 코드와 분리되는 라인 번호를 덧붙일 수 있다.

마지막으로 아무런 옵션 없이 `highlight`만 쓰면 아래처럼 나온다.
{% highlight python  %}
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

물론 깃헙 Gist도 손쉽게 임베딩할 수 있다. 다만 인터넷이 연결 안 되면 아무것도 안 뜨는 단점이 있다.

<script src="https://gist.github.com/minjang/21548f14e70a3649a10d.js"></script>


### 구글 어낼리틱스 설치

방문자 추적 도구로 [구글 Analytics](http://www.google.com/analytics/)를 사용하면 좋다. 설치는 자바스크립트 [코드 하나](https://github.com/minjang/minjang.github.io/blob/master/_includes/google_analytics.html)를 넣으면 된다. 다만 실제 통계가 뜨려면 몇 시간 혹은 하루 정도 기다려야 한다. 로컬에서 테스트할 때는 구글 어낼리틱스 코드를 막는 것도 좋다. 혹은 자신의 컴퓨터에서 방문 되는 접근을 필터링하면 되는데 찾아보면 몇 가지 방법이 나온다.


### 기본적인 설치를 마치며

정작 블로그 글쓰기 전에 블로그 설치에 힘을 뺀 것 같다. 웹 관련해선 사실 아는 것이 너무 없어서 삽질을 많이 했는데 크롬의 도움으로 나름 즐겁게 작업하였다. [그림 캡션 넣는 기능](http://codingtips.kanishkkunal.in/image-caption-jekyll/)도 간단히 만들 수 있다.

{% include image.html img="/assets/2015/chrome-inspect-element.png" caption="크롬 웹브라우저의 에뮬레이션 및 인스펙트 기능" %}

마지막으로 이 블로그의 [소스 코드](https://github.com/minjang/minjang.github.io)를 보는 것도 도움이 될런지 모르겠다. HTML/CSS 초보자가 만든 것임을 꼭 잊지 않기를 바란다.
