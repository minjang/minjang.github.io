#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <typeinfo>
#include <tuple>
#include <algorithm>
#include <functional>
#include <map>
#include <initializer_list>

//using namespace std;

#if 1
#include <boost/type_index.hpp>
using boost::typeindex::type_id_with_cvr;
using boost::typeindex::type_id;
#endif

template<class Ch, class Tr, class T1, class T2>
auto operator<<(std::basic_ostream<Ch, Tr>& os, std::pair<T1, T2> const& p)
-> std::basic_ostream<Ch, Tr>&
{
  return os << "<" << p.first << ", " << p.second << ">";
}


// C++ range 구현
template<typename T>
class range_value {
  T val_ = -1;

public:
  range_value(T val) : val_(val) {
    std::cout << "range_value ctor with " << val << std::endl;
  }
  ~range_value() {
    std::cout << "range_value dtor with " << val_ << std::endl;
  }

  range_value(const range_value& rhs) {
    this->val_ = rhs.val_;
    std::cout << "range_value copy ctor with " << val_ << std::endl;
  }

  range_value& operator=(const range_value& rhs) {
    this->val_ = rhs.val_;
    std::cout << "range_value = operator with " << val_ << std::endl;
    return *this;
  }

  range_value(range_value&& rhs) {
    T old = val_;
    std::swap(this->val_, rhs.val_);
    std::cout << "range_value move ctor with " << val_ << " from " << old << std::endl;
  }

  range_value& operator=(range_value&& rhs) {
    T old = val_;
    std::swap(this->val_, rhs.val_);
    std::cout << "range_value = operator&& with " << val_ << " from " << old << std::endl;
    return *this;
  }

  bool operator!= (const range_value& rhs) const {
    return val_ != rhs.val_;
  }

  range_value& operator+ (const range_value& rhs) {
    this->val_ += rhs.val_;
    return *this;
  }

  range_value& operator+ (T rhs) {
    this->val_ += rhs;
    return *this;
  }

  range_value& operator+= (const range_value& rhs) {
    this->val_ += rhs.val_;
    return *this;
  }

  range_value& operator+= (T rhs) {
    this->val_ += rhs;
    return *this;
  }

  T val() const {
    return val_;
  }
};

template<class Ch, class Tr, class T>
auto operator<<(std::basic_ostream<Ch, Tr>& os, range_value<T> const& t)
-> std::basic_ostream<Ch, Tr>&
{
  return os << t.val();
}


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
  using namespace std;
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
  using namespace std;
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
  using namespace std;
  // 3. const 예제
  cout << "[TEST 3] const\n";
  const string E[] = {"foo", "bar", "baz"};
  // decltype(p) == pair<size_t, string const&>&&
  // p 자체는 상수가 아니므로 인덱스 값은 변경 가능, 배열 값은 수정 불가.
  for (auto&& p : enumerate(E))
    cout << (p.first += 1) << ": " << p.second << '\n';
}

void test4() {
  using namespace std;
  // 4. 앞서 구현한 range 사용 예
  cout << "[TEST 4] range\n";
  auto&& D = range(100, 103);
  // decltype(p) == pair<size_t, int>&&
  for (auto&& p : enumerate(D))
    cout << p.first << ": " << p.second << '\n';
}

void test5() {
  using namespace std;
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
  using namespace std;
  // 6. 초기화 리스트
  cout << "[TEST 6] initializer list\n";
  for (auto&& p : enumerate({"foo", "bar", "baz"}))
    cout << p.first << ": " << p.second << '\n';
}

namespace oldzip {
using std::begin;
using std::end;
using std::get;
using std::declval;

template<typename C1, typename C2>
class zip_iterator {
  using IterType1 = decltype(std::begin(std::declval<C1&>()));
  using IterType2 = decltype(std::begin(std::declval<C2&>()));
  using ElemType1 = decltype(*std::begin(std::declval<C1&>()));
  using ElemType2 = decltype(*std::begin(std::declval<C2&>()));
  
  using TupleType = std::tuple<ElemType1, ElemType2>;

  std::tuple<IterType1, IterType2> its_;

public:
  zip_iterator(IterType1 it1, IterType2 it2) :
    its_{it1, it2} {}

  zip_iterator& operator++() {
    ++std::get<0>(its_);
    ++std::get<1>(its_);
    return *this;
  }

  bool operator!=(const zip_iterator& rhs) {
    return std::get<0>(its_) != std::get<0>(rhs.its_) &&
           std::get<1>(its_) != std::get<1>(rhs.its_);
  }

#if 0
  decltype/*TupleType*/ operator*() {
    //TupleType &&r = static_cast<TupleType>(
    //    std::forward_as_tuple(*std::get<0>(its_), *std::get<1>(its_)));
    ////std::cout << type_id_with_cvr<decltype(r)>() << std::endl;
    //return std::move(r);
    return 
       std::forward_as_tuple(*std::get<0>(its_), *std::get<1>(its_));
  }
#else
  TupleType operator*() {
    //std::cout << type_id_with_cvr<decltype(std::make_pair(
    //                 (*std::get<0>(its_)), (*std::get<1>(its_))))>()
    //          << std::endl;
    
    // 됨
    TupleType t{*std::get<0>(its_), *std::get<1>(its_)};
    return t;

    //return std::make_tuple(*std::get<0>(its_), *std::get<1>(its_));
    
    // 에러
    //return {*std::get<0>(its_), *std::get<1>(its_)};
    
    //return ret;
    //return std::forward_as_tuple(*std::get<0>(its_),
    //                             *std::get<1>(its_));
    //return std::make_tuple(std::ref(*std::get<0>(its_)),
    //                        std::ref(*std::get<1>(its_)));

  }
#endif
    ////std::cout << type_id_with_cvr<decltype(r)>() << std::endl;
    ////std::cout << type_id_with_cvr<TupleType>() << std::endl;
    ////std::cout << type_id_with_cvr<decltype(*std::get<0>(its_))>() << std::endl;
    //return static_cast<TupleType>(r);
  
};

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

template<typename C1, typename C2>
zip_impl<C1, C2> zip(C1&& c1, C2&& c2) {
  return {std::forward<C1>(c1), std::forward<C2>(c2)};
}

template <typename T1, typename T2>
zip_impl<std::vector<T1>, std::vector<T2>> zip(std::initializer_list<T1> l1,
                                               std::initializer_list<T2> l2) {
  return {std::move(std::vector<T1>(l1)), std::move(std::vector<T2>(l2))};
}

} // namespace

namespace aux {
  template<class Ch, class Tr, class Tuple, std::size_t... Is>
  void print_tuple(std::basic_ostream<Ch, Tr>& os,
    Tuple const& t, std::index_sequence<Is...>){
    using swallow = int[];
    (void)swallow{0, (void(os << (Is == 0 ? "" : ", ") << std::get<Is>(t)), 0)...};
  }
} // aux::

template<class Ch, class Tr, class... Args>
auto operator<<(std::basic_ostream<Ch, Tr>& os, std::tuple<Args...> const& t)
-> std::basic_ostream<Ch, Tr>&
{
  os << "(";
  aux::print_tuple(os, t, std::index_sequence_for<Args...>());
  return os << ")";
}

template <typename Arg, typename... Args>
void doPrint(std::ostream& out, Arg&& arg, Args&&... args)
{
  out << "(" << std::forward<Arg>(arg);
  using expander = int[];
  (void)expander{0, (void(out << ", " << std::forward<Args>(args)),0)...};
  out << ")";
}



template<typename Func, typename Tup, std::size_t... index>
decltype(auto) invoke_helper(Func&& func, Tup&& tup, std::index_sequence<index...>)
{
  return func(std::get<index>(std::forward<Tup>(tup))...);
}

template<typename Func, typename Tup>
decltype(auto) invoke(Func&& func, Tup&& tup)
{
  constexpr auto Size = std::tuple_size<typename std::decay<Tup>::type>::value;
  return invoke_helper(std::forward<Func>(func),
    std::forward<Tup>(tup),
    std::make_index_sequence<Size>{});
}


namespace detail {
template <class F, class G, class Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl(F&& f, G&& g, Tuple&& t,
                                    std::index_sequence<I...>) {
  //return std::invoke(std::forward<F>(f),
  //                  g(std::get<I>(std::forward<Tuple>(t)))...);
  //doPrint(cout, I...);
  //cout << endl;
  return f(g(std::get<I>(std::forward<Tuple>(t)))...);
  // Note: std::invoke is a C++17 feature
}

template <class F, class G, class T1, class T2, std::size_t... I>
constexpr decltype(auto) apply_impl(F&& f, G&& g, T1&& t1, T2&& t2,
  std::index_sequence<I...>) {
  //return std::invoke(std::forward<F>(f),
  //  g(std::get<I>(std::forward<T1>(t1)), std::get<I>(std::forward<T2>(t2)))...);
  // Note: std::invoke is a C++17 feature
  return f(g(std::get<I>(std::forward<T1>(t1)), std::get<I>(std::forward<T2>(t2)))...);
}

//template <class F, class G, class... Tuples, std::size_t... I>
//constexpr decltype(auto) apply_impl(F&& f, G&& g, Tuples&&... t,
//  std::index_sequence<I...>) {
//  return std::invoke(std::forward<F>(f),
//    g(std::get<I>(std::forward<Tuples...>(t)))...);
//  // Note: std::invoke is a C++17 feature
//}

} // namespace detail


template <class F, class G, class Tuple>
constexpr decltype(auto) apply(F&& f, G&& g, Tuple&& t)
{
  return detail::apply_impl(std::forward<F>(f), std::forward<G>(g),
    std::forward<Tuple>(t),
    std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{});
}

template <class F, class G, class T1, class T2>
constexpr decltype(auto) apply(F&& f, G&& g, T1&& t1, T2&& t2)
{
  constexpr auto size1 = std::tuple_size<std::decay_t<T1>>::value;
  constexpr auto size2 = std::tuple_size<std::decay_t<T2>>::value;
  static_assert(size1 == size2, "Tuple sizes are different");
  return detail::apply_impl(std::forward<F>(f), std::forward<G>(g),
    std::forward<T1>(t1), std::forward<T2>(t2),
    std::make_index_sequence<size1>{});
}

#if 1
namespace newzip {
template<class... Containers>
class zip_iterator {
  template<class C>
  using IterType = decltype(std::begin(std::declval<C&>()));
  template<class C>
  using ElemType = decltype(*std::begin(std::declval<C&>()));


  using TupleType = std::tuple<ElemType<Containers>...>;

  std::tuple<IterType<Containers>...> its_;

public:
  zip_iterator(IterType<Containers>... its) :
    its_{its...} {}

  zip_iterator& operator++() {
    auto f = [](auto&&...) {
    };
    auto g = [](auto&& arg)->decltype(auto) {
      return ++std::forward<decltype(arg)>(arg);
    };
    apply(f, g, its_);
    return *this;
  }

  // http://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
  bool operator!=(const zip_iterator& rhs) {
    auto f = [](auto&&... args) {
#if 0 // C++17
      bool ret = true;
      using expander = int[];
      (void)expander{ 0, ( (void) (ret &= args), 0) ... };
      ret;
#else
      return (... && args);
#endif
    };
    auto g = [](auto&& a, auto&& b) {
      return a != b;
    };
    return apply(f, g, its_, rhs.its_);
  }

  decltype(auto) operator*() {
    auto f = [](auto&&... args)->decltype(auto) {
      return static_cast<TupleType>(
          std::forward_as_tuple(std::forward<decltype(args)>(args)...));
    };
    auto g = [](auto&& arg)->decltype(auto) {
      return *std::forward<decltype(arg)>(arg);
    };
    return apply(f, g, its_);
  }
};

template<class... Containers>
class zip_impl {
  std::tuple<Containers...> containers_;
  
  template<class T>
  class adaptor {
  public:
    adaptor(std::initializer_list<T> list) {

    }
  };

public:
  //zip_impl(Containers&&... containers) : 
  //  containers_{std::forward<Containers>(containers)...} {}
  zip_impl(Containers &&... containers)
      : containers_{std::forward<Containers>(containers)...} {}

  zip_iterator<Containers...> begin() {
    auto f = [](auto&&... args)->decltype(auto) {
      return zip_iterator<Containers...>(std::forward<decltype(args)>(args)...);
    };
    auto g = [](auto&& a)->decltype(auto) {
      return std::begin(std::forward<decltype(a)>(a));
    };
    return apply(f, g, containers_);
  }

  zip_iterator<Containers...> end() {
    auto f = [](auto&&... args)->decltype(auto) {
      return zip_iterator<Containers...>(std::forward<decltype(args)>(args)...);
    };
    auto g = [](auto&& a)->decltype(auto) {
      return std::end(std::forward<decltype(a)>(a));
    };
    return apply(f, g, containers_);
  }
};

template<class... Types>
zip_impl<Types...> zip(Types&&... args) {
  return {std::forward<Types>(args)...};
}

template<class... Types>
zip_impl<std::vector<Types>...> zip(std::initializer_list<Types>... lists) {
  return {std::move(std::vector<Types>(lists))...};
}
};
#endif


template<class F, class... Args>
void apply2(F f, Args... args) {
  using swallow = int[];
  (void)swallow{0, (void(f(std::forward<decltype(args)>(args))),0)...};
}

template<class... T>
void test_func(T&&... args) {
  using namespace std;
  auto f = [](auto&& arg) {
    //cout << type_id_with_cvr<decltype(arg)>() << endl;
  };
  apply2(f, std::forward<decltype(args)>(args)...);
}

void test7() {
#if 0
  using namespace std;
  vector<int> X = {1, 2, 3};
  test_func("1", "2");
  test_func("1", 0, vector<int>{1, 2, 3}, X);
  return;

  vector<string> A = {"foo", "bar"};
  vector<int> B = {17, 42, 91};
  vector<double> C = {3.14, 1.414};
  char D[] = {'a', 'b', 'c', 'd'};
  int E[] ={7, 8, 9};
  const vector<float> F = {7.7f};
  using namespace newzip;
  for (auto&& e : zip(D, E)) {
    cout << get<0>(e) << ", " << get<1>(e) << endl;
  }

  for (auto&& e : zip(A, B)) {
    cout << get<0>(e) << ", " << get<1>(e) << endl;
    get<0>(e) += get<0>(e);
    get<1>(e) += get<1>(e);
  }

  for (auto&& e : zip(A, B)) {
    cout << e << endl;
  }

  for (auto&& p : enumerate(zip(set<string>{"a", "b"}, vector<int>{17, 18}))) {
    cout << p.first << ": " << p.second << endl;
  }

  for (auto&& e : zip(A, B, C, D, F)) {
    cout << e << endl;
  }

  //for (auto&& p : enumerate(zip({"a", "b"}, {17, 18}))) {
  //  cout << p.first << ": " << p.second << endl;
  //}
#endif
}

struct T {
  enum { int_t,float_t } m_type;
  template <typename Integer,
    typename = std::enable_if_t<std::is_class<Integer>::value>
  >
    T(Integer) : m_type(int_t) {}

  template <typename Floating, typename Integer,
    typename = std::enable_if_t<std::is_floating_point<Floating>::value>
  >
    T(Floating, Integer) : m_type(float_t) {} // error: cannot overload
};

std::string foo() {
  return "foo";
}

std::string&& goo() {
  std::string r = "foo";
  return std::move(r);
}

void test_zip() {

  //auto &&r2 = foo();
  //auto &&r3 = goo();
  

  using namespace std;
  using namespace newzip;

  // vector<bool>::iterator는 proxy iterator
  vector<bool> A   = {   false,    false,  true};
  size_t B[]       = { 0, 0, 0, 0}; // 길이가 다름
  vector<string> C = {    "pi",      "e", "ans"};
  const double D[] = {3.141592, 2.718281,    42};
  vector<int> E = {1, 2, 3};

  // A, B, C는 수정 가능, range도 같이 사용
  // std::size는 C++1z/17부터 지원
  cout << std::boolalpha;
  for (auto&& t : zip(B, D/*, C, range<size_t>(0, std::size(A))*/)) {
    // decltype(t) == tuple<proxy, size_t&, string&, size_t>&&
    // 임의의 tuple/pair가 cout으로 출력되도록 만들었다고 가정
    //cout << type_id_with_cvr<decltype(t)>() << endl;
    cout << "Before: " << t << endl;
    get<0>(t) = get<0>(t) ^ true;
    cout << "Check: " << t << endl;
    //get<1>(t) = get<3>(t);
    //get<1>(t) = '\"' + get<1>(t) + '\"';
  }

  //// enumerate와 같이 사용
  for (auto&& p : enumerate(zip(B, D/*, C, D*/))) {
  //  // decltype(p) == pair<size_t,
  //  //                     tuple<proxy, size_t&, string&, const double&>>&&
    //cout << type_id_with_cvr<decltype(p)>() << endl;
    cout << "After: " << p << endl;
  }

  // zip, enumerate, range 모두 같이 사용
  for (auto&& t : zip(enumerate(range(11, 14)), D)) {
    //cout << type_id_with_cvr<decltype(t)>() << endl;
    // decltype(t) == tuple<pair<size_t, int>> &&>
    cout << t << endl;
  }

  // A는 수정 가능
  //for (auto&& t : zip(A))
  //  get<0>(t) += ": ";

  // enumerate와 같이 사용
  // decltype(p) == pair<size_t,
  //   tuple<string&, const double&, pair<const string, double>&>>&&;
  //for (auto&& p : enumerate(zip(A, B, C))) {
  //  //cout << type_id_with_cvr<decltype(p)>() << endl;
  //  cout << "[" << p.first << "] "
  //       << get<0>(p.second) << get<1>(p.second) << "\n";
  //  get<2>(p.second).second = get<1>(p.second);
  //}
 
  //// range와 같이 사용
  //for (auto t : zip(B, D)) {
  //  cout << t << endl;
  //  get<1>(t) += get<1>(t);
  //}
  //for (auto t : zip(B, D)) {
  //  cout << t << endl;
  //}

  ////for (auto t : zip({1,2,3}, {0.1, 0.2, 0.3})) {
  ////  cout << t << endl;
  ////}
 
  //for (auto e : enumerate(K)) {
  //  cout << e << endl;
  //}
  //
  //for (auto t : zip(range(100, 105), range(200, 203))) {
  //  cout << type_id_with_cvr<decltype(t)>() << endl;
  //  cout << t << endl;
  //}
  //for (auto p : zip(enumerate({1,2,3}), D)) {
  //  //cout << p.first << ": " << p.second << endl;
  //  cout << p << endl;
  //}
}

void test_tuple() {
  using namespace std;
  {
    vector<int>    x = {   1};
    vector<bool>   y = {true};

    // bool&는 프락시 반복자때문에 컴파일 오류
    pair<int&, bool> p1 {*begin(x), *begin(y)};
    pair<int&, bool> p2 = {*begin(x), *begin(y)};

    // p3 타입은 pair<int, proxy_iterator>&&: int&가 아님에 주목
    auto&& p3 = make_pair(*begin(x), *begin(y));
    // pair<int&, bool> p3 = make_pair(...); 컴파일 오류
    cout << type_id_with_cvr<decltype(p3)>() << endl;

    // p1.first와 p2.first는 같은 x[0]을 가리킴
    p1.first += p1.first;
    p2.first += p2.first;

    // p1.second와 p2.second는 각각 y[0]의 독립 복사본
    p1.second = !p1.second;
    p2.second =  p2.second;

    // p3.first는 x[0]의 복사본
    p3.first += p3.first;

    cout << boolalpha;
    cout << "p1: " << p1 << endl;
    cout << "p2: " << p2 << endl;
    cout << "p3: " << p3 << endl;
  }

  //cout << type_id_with_cvr<decltype(p1)>() << endl;
  //cout << type_id_with_cvr<decltype(p2)>() << endl;
  //cout << type_id_with_cvr<decltype(p3)>() << endl;

  {
    vector<int>    x = {   1};
    vector<bool>   y = {true};
    //vector<double> z = { 1.5};
    //auto&& z = range(100, 102);
    auto&& z = range<range_value<int>>(100, 102);

    tuple<int&, bool, range_value<int>> t1 {*begin(x), *begin(y), *begin(z)};
    // C++14까지 컴파일 오류: C++17부터는 지원될 예정
    // tuple<int&, bool, double&> t2 = {*begin(x), *begin(y), *begin(z)};

    // t3 타입은 pair<int, proxy, double>: 참조자 아님
    auto&& t3 = make_tuple(*begin(x), *begin(y), *begin(z));
    auto&& t4 = forward_as_tuple(*begin(x), *begin(y), *begin(z));
    auto&& t5 = static_cast<tuple<int&, bool, range_value<int>>>(
      forward_as_tuple(*begin(x), *begin(y), *begin(range<range_value<int>>(100, 102))));
    tuple<int &, bool, range_value<int>> t6 =
      forward_as_tuple(*begin(x), *begin(y), *begin(z));

    cout << type_id_with_cvr<decltype(t3)>() << endl;
    cout << type_id_with_cvr<decltype(t4)>() << endl;
    cout << type_id_with_cvr<decltype(t5)>() << endl;
    cout << type_id_with_cvr<decltype(t6)>() << endl;

    //p1.first += p1.first;
    cout << t4 << endl;
    cout << t5 << endl;
    cout << t6 << endl;
  }

  //tuple<int&, double&, bool> t1 {*begin(x), *begin(y), *begin(z)};
}

int main() {
  //extern void test_wiggleSort();
  //test_wiggleSort();
  //return 0;




  //tuple<string&, double&> t {*begin(x), *begin(y)};
  //tuple<string&, double&> u = forward_as_tuple(*begin(x), *begin(y));

  //p.first += p.first;
  //
  //p.second += p.second;

  //cout << p << endl;

  //get<0>(u) += get<0>(u);
  //get<1>(u) += get<1>(u);
  //cout << u << endl;
  //cout << p << endl;
  //for (std::tuple<string&, double&> t : zip(x, y))
  //  cout << "(" << get<0>(t) << ", " << get<1>(t) << ", " << get<1>(t) << ")\n";
  //for (pair<size_t, tuple<string&, double&>> &&p : enumerate(zip(x, y)))
  //  cout << p.first << ": (" << get<0>(p.second) << ", " << get<1>(p.second) << ")\n";

#if 0
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
#endif
  

  //test_tuple();

  //test7();
  test_zip();
}