#include <iostream>
#include <vector>
#include <string>
#include <typeinfo>

using namespace std;

#if 0
#include <boost/type_index.hpp>
using boost::typeindex::type_id_with_cvr;
using boost::typeindex::type_id;
#endif

// C++ range 구현
template<typename T>
class range_iterator {
  T cur_;
  const T step_ = 1;

public:
  range_iterator(T init) : cur_{init} {}

  range_iterator& operator++() {
    cur_ += step_;
    return *this;
  }

  bool operator!=(const range_iterator<T>& rhs) const {
    return cur_ != rhs.cur_;
  }

  T operator*() const {
    return cur_;
  }
};

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

template<typename T>
range_impl<T> range(const T start, const T stop) {
  return range_impl<T>{start, stop};
}


template <typename T> T fake_create();

// C++ enumerate 구현
template<typename C /* 컨테이너 타입 */>
class enumerate_iterator {
  using IndexType = size_t;
  using IterType = decltype(std::begin(std::declval<C&>()));
  using ElemType = decltype(*std::begin(std::declval<C&>()));
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

template<typename C /* 컨테이너 타입 */>
class enumerate_impl {
  C container_;
  const size_t start_;

public:
  enumerate_impl(C&& container, size_t start) :
    container_{std::forward<C>(container)}, start_{start} {
  }

  enumerate_iterator<C> begin() {
    return {std::begin(container_), start_};
  }

  enumerate_iterator<C> end() {
    return {std::end(container_), 0 /* 중요하지 않음 */};
  }
};

// 포워딩 레퍼런스 버전: 우측값/좌측값 모두 처리
template<typename C>
enumerate_impl<C> enumerate(C&& container, size_t start = 0) {
  return {std::forward<C>(container), start};
}

// 초기화 리스트 받는 버전
template<typename T>
enumerate_impl<std::vector<T>> enumerate(std::initializer_list<T> list, size_t start = 0) {
  return {std::vector<T>(list), start};
}

void test1() {
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
}

void test2() {
  // 2. 일반 배열 예
  cout << "[TEST 2] array\n";
  string C[] = {"foo", "bar", "baz"};
  // auto&&로 받는 것이 범위 기반 for 문에서 일반적이고 효율적인 방법
  // p 타입: pair<size_t, string&>&&, 원소 타입이 string&로 추론
  for (auto&& p : enumerate(C, 100))
    cout << p.first << ": " << (p.second += p.second) << '\n';
  for (auto&& p : enumerate(C, 100))
    cout << p.first << ": " << p.second << '\n';
}

void test3() {
  // 3. const 예제
  cout << "[TEST 3] const\n";
  const string E[] = {"foo", "bar", "baz"};
  // decltype(p) == pair<size_t, string const&>&&
  // p 자체는 상수가 아니므로 인덱스 값은 변경 가능, 배열 값은 수정 불가.
  for (auto&& p : enumerate(E))
    cout << (p.first += 1) << ": " << p.second << '\n';
}

void test4() {
  // 4. 앞서 구현한 range 사용 예
  cout << "[TEST 4] range\n";
  auto&& D = range(100, 103);
  // decltype(p) == pair<size_t, int>&&
  for (auto&& p : enumerate(D))
    cout << p.first << ": " << p.second << '\n';
}

void test5() {
  // 5. 변수를 거치지 않고 직접 사용
  cout << "[TEST 5] in-place through rvalue reference\n";
  for (auto&& p : enumerate(range(100, 103)))
    cout << p.first << ": " << p.second << '\n';

  // decltype(p) == pair<size_t, string&>&&
  for (auto&& p : enumerate(vector<string>{"foo", "bar", "baz"}))
    cout << p.first << ": " << (p.second += p.second) << '\n';

  // 함수 반환값 바로 사용
  auto create = []()->vector<string> { return {"foo", "bar", "baz"}; };
  // decltype(p) == pair<size_t, string&>&&
  for (auto&& p : enumerate(create()))
    cout << p.first << ": " << p.second << '\n';
}

void test6() {
  // 6. 초기화 리스트
  cout << "[TEST 6] initializer list\n";
  for (auto&& p : enumerate({"foo", "bar", "baz"}))
    cout << p.first << ": " << p.second << '\n';
}

int main() {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
}