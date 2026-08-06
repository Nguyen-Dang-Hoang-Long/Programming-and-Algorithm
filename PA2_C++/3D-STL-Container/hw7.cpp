#ifndef __PROGTEST__
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <memory>
#include <optional>
#include <iterator>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <cassert>
#include <numeric>

class CRange
{
  public:
                                       CRange                                  ( size_t                                from,
                                                                                 size_t                                to  )
      : m_From ( from ),
        m_To  ( to )
    {
      if ( from > to )
        throw std::invalid_argument ( "invalid range" );
    }
    size_t                             m_From;
    size_t                             m_To;
};

class CRangeRev
{
  public:
                                       CRangeRev                               ( size_t                                from,
                                                                                 size_t                                to  )
      : m_From ( from ),
        m_To  ( to )
    {
      if ( from < to )
        throw std::invalid_argument ( "invalid range" );
    }
    size_t                             m_From;
    size_t                             m_To;
};
#endif /* __PROGTEST__ */

template <typename T_>
class CTensorView;

template <typename T_, size_t DIM_>
class CTensor
{
  private:
    // Recursive init list struct
    // Recursive case (create a init list of dim - 1 of whatever in the list)
    template <typename T, size_t DIM>
    struct initializer_list_helper {
        using Type = std::initializer_list<typename initializer_list_helper<T, DIM - 1>::Type>;
    };
    // Base case (dim 1)
    template <typename T>
    struct initializer_list_helper<T, 1> {
        using Type = std::initializer_list<T>;
    };
    std::vector<T_> m_Data; // T_ flat vector
    std::vector<size_t> m_Sizes; // Sizes of each dim
    std::vector<size_t> m_Index; // Index math for each dim 

    // Alloc mem and init to v
    void set_data_v(const T_& v) {
      // Alloc
      size_t total_size = 1;
      for (size_t s : m_Sizes) total_size *= s;
      // Init
      m_Data.assign(total_size, v);
    }
    
    // Calc index math of each dim
    void set_index() {
      m_Index.resize(DIM_);
      size_t current = 1;
      for (int i = static_cast<int>(DIM_ - 1); i >= 0; i--) {
          m_Index[i] = current;
          current *= m_Sizes[i];
      }
    }

    // Validate each sublist dim and store it 
    template <typename L>
    void valid_dim(const L& list, std::vector<size_t>& dims) {
        // Current level size
        dims.push_back(list.size());

        // Base case
        if constexpr (std::is_same_v<L, std::initializer_list<T_>>) {
            return; 
        } else {

            // Recursive case: check sub-lists
            if (list.size() == 0) return;

            size_t expectedSubSize = list.begin()->size();
            for (const auto& subList : list) {
                if (subList.size() != expectedSubSize) throw std::invalid_argument("Tensor is not a hyper-prism");
            }
            valid_dim(*(list.begin()), dims);
        }
    }

    // Alloc & init for init list 
template <typename L>
    void set_data_l(const L& list) {
        if (m_Data.empty()) {
            size_t total = 1;
            for (auto& s : m_Sizes) total *= s;
            m_Data.reserve(total);
        }
        if constexpr (std::is_same_v<L, std::initializer_list<T_>>) for (const auto& val : list) m_Data.push_back(val); 
        else for (const auto& subList : list) set_data_l(subList);
    }

    // Recursive print 
    void print(std::ostream& os, size_t dim, size_t flatStart, int indent) const {
        os << "{";
        const size_t count = m_Sizes[dim];
        const size_t stride = m_Index[dim];

        for (size_t i = 0; i < count; ++i) {
            const size_t currentPos = flatStart + i * stride;
            if (dim < DIM_ - 1) {
                if (i == 0) {
                    os << "\n" << std::string(indent + 2, ' ');
                } else {
                    os << ",\n" << std::string(indent + 2, ' ');
                }
                print(os, dim + 1, currentPos, indent + 2);
            } else {
                if (i > 0) os << ", ";
                os << m_Data[currentPos];
            }
        }
        if (dim < DIM_ - 1) {
            os << "\n" << std::string(indent, ' ');
        }
        os << "}";
    }
    template <size_t I = 0, typename View, typename... Args>
    void build_slice(View& view, size_t& current_offset, const Args&... args) const {
        if constexpr (I == sizeof...(Args)) {
            // Fill remaining dimensions with full range
            for (size_t d = I; d < DIM_; ++d) {
                view.m_Rules.push_back({m_Sizes[d], 1, m_Index[d]});
            }
            return;
        } else {
            const auto& arg = std::get<I>(std::tuple<const Args&...>{args...});

            using ArgT = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<ArgT, int> || std::is_same_v<ArgT, size_t>) {
                size_t val = static_cast<size_t>(arg);
                if (val >= m_Sizes[I]) throw std::out_of_range("Slice out of bounds");
                current_offset += val * m_Index[I];
            }
            else if constexpr (std::is_same_v<ArgT, CRange>) {
                if (arg.m_To >= m_Sizes[I]) throw std::out_of_range("Slice out of bounds");
                current_offset += arg.m_From * m_Index[I];
                view.m_Rules.push_back({arg.m_To - arg.m_From + 1, 1, m_Index[I]});
            }
            else if constexpr (std::is_same_v<ArgT, CRangeRev>) {
                if (arg.m_From >= m_Sizes[I]) throw std::out_of_range("Slice out of bounds");
                current_offset += arg.m_From * m_Index[I];
                view.m_Rules.push_back({arg.m_From - arg.m_To + 1, -1, m_Index[I]});
            }

            build_slice<I + 1>(view, current_offset, args...);
        }
    }
  public:
    // constructor -- create + set all elements
    template<typename... args>
    CTensor(const T_& v, args... sizes) {
      // Validate
      static_assert(sizeof...(sizes) == DIM_, "Numbers of sizeX must match DIM_");

      // Set up dims
      m_Sizes.resize(DIM_);
      m_Sizes = { static_cast<size_t>(sizes)... };

      // Set up data
      set_data_v(v);

      // Set up index math
      set_index();
    }
    // constructor -- from initializer_list
    CTensor (const typename initializer_list_helper<T_, DIM_>::Type& list) {
      // Validate hyperprism
      std::vector<size_t> dims;
      valid_dim(list, dims);

      // Set up dims
      m_Sizes = dims;

      // Set up data
      set_data_l(list);

      // Set up index math
      set_index();
    }
    // destructor (opt)
    // slice ()
    template <typename... Args>
    auto slice(Args... ranges) const {
        static_assert(sizeof...(ranges) <= DIM_, "Too many slice arguments");

        CTensorView<T_> view;
        view.m_Data = const_cast<std::vector<T_>*>(&m_Data);
        size_t offset = 0;

        build_slice<0>(view, offset, ranges...);
        view.m_Offset = offset;

        return view;
    }
    // operator ()
    template<typename... args>
    T_& operator () (args... indices) {
      // Validate idx cnt
      static_assert(sizeof...(indices) == DIM_, "Index count must be == dim");

      // Extract
      size_t idx[] = {static_cast<size_t>(indices)...};
      size_t real_index = 0;
      for (size_t i = 0; i < DIM_; i++) {

        // Validate idx
        if (idx[i] >= m_Sizes[i]) throw std::out_of_range("Index must be < dim size");
        real_index += idx[i] * m_Index[i];
      }
      return m_Data[real_index];
    }
    template<typename... args>
    T_& operator () (args... indices) const {
      // Validate idx cnt
      static_assert(sizeof...(indices) == DIM_, "Index count must be == dim");

      // Extract
      size_t idx[] = {static_cast<size_t>(indices)...};
      size_t real_index = 0;
      for (size_t i = 0; i < DIM_; i++) {

        // Validate idx
        if (idx[i] >= m_Sizes[i]) throw std::out_of_range("Index must be < dim size");
        real_index += idx[i] * m_Index[i];
      }
      return m_Data[real_index];
    }
    // operator <<
    friend std::ostream& operator << (std::ostream& os, const CTensor& tensor) {
      tensor.print(os, 0, 0, 0);
      return os;
    }
};

template <typename T_>
class CTensorView
{
  public:
    struct dim_rule {
      size_t viewSize; // window => hi - lo + 1
      int dir; // 1 => normal, -1 => reverse
      size_t stride; // index math
    };
    // constructor 
    CTensorView() = default;
    // operator ()
    template <typename... args>
    T_& operator () (args... indices) {
        // Validate idx cnt
        if (sizeof...(indices) != m_Rules.size()) throw std::out_of_range("Number of indices must match dim");

        size_t idx[] = { static_cast<size_t>(indices)... };
        size_t flatIndex = m_Offset;

        for (size_t i = 0; i < m_Rules.size(); ++i) {
            // Validate idx
            if (idx[i] >= m_Rules[i].viewSize) throw std::out_of_range("View index out of bounds");

            // If 1 => CRange, -1 => CRangeRev
            if (m_Rules[i].dir == 1) flatIndex += idx[i] * m_Rules[i].stride;
            else flatIndex -= idx[i] * m_Rules[i].stride;
        }
        return (*m_Data)[flatIndex];
    }
    // operator <<
    friend std::ostream& operator << (std::ostream& os, const CTensorView& view) {
      view.print(os, 0, 0, 0);
      return os;
    }

  private:
    // todo
    std::vector<dim_rule> m_Rules;
    std::vector<T_>* m_Data = nullptr; // A ptr to CTensor's m_Data
    std::vector<size_t> m_Sizes; // CTensorView's m_Sizes
    size_t m_Offset = 0; // Starting point

    template <typename T__, size_t DIM__> friend class CTensor; // Allow CTensor's slice permission

    void print(std::ostream& os, size_t dimIdx, size_t offset, int indent) const {
        os << "{";
        const auto& rule = m_Rules[dimIdx];
        const size_t count = rule.viewSize;

        for (size_t i = 0; i < count; ++i) {
            const long long step = static_cast<long long>(i) * rule.stride * rule.dir;
            const size_t next_offset = offset + step;

            if (dimIdx < m_Rules.size() - 1) {
                if (i == 0) {
                    os << "\n" << std::string(indent + 2, ' ');
                } else {
                    os << ",\n" << std::string(indent + 2, ' ');
                }
                print(os, dimIdx + 1, next_offset, indent + 2);
            } else {
                if (i > 0) os << ", ";
                os << (*m_Data)[m_Offset + next_offset];  
            }
        }

        if (dimIdx < m_Rules.size() - 1) {
            os << "\n" << std::string(indent, ' ');
        }
        os << "}";
    }
};


#ifndef __PROGTEST__
template <typename T_>
std::string  toString ( const T_ & x )
{
  std::ostringstream oss;
  oss << x;
  return oss . str ();
}

int main ()
{
  CTensor<int,2> m1 ( 0, 3, 5 );
  CTensor<int,2> m2 { { 1, 2, 3 }, { 4, 5, 6 } };
  CTensor<std::string,2> m4
  {
    { "test", "progtest" },
    { "PA1", "PA2" }
  };

  assert ( toString ( m1 ) == R"({
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0}
})" );

  assert ( toString ( m2 ) == R"({
  {1, 2, 3},
  {4, 5, 6}
})" );

  assert ( toString ( m4 ) == R"({
  {test, progtest},
  {PA1, PA2}
})" );

  assert ( toString ( m2 . slice ( CRange ( 0, 1 ), CRange ( 1, 2 ) ) ) == R"({
  {2, 3},
  {5, 6}
})" );
  assert ( toString ( m2 . slice ( 0, CRange ( 1, 2 ) ) ) == "{2, 3}" );
  assert ( toString ( m2 . slice ( 1 ) ) == "{4, 5, 6}" );
  assert ( toString ( m2 . slice ( CRange ( 0, 1 ), 2 ) ) == "{3, 6}" );

  auto v2 = m2 . slice ( CRange ( 0, 1 ), CRange ( 1, 2 ) );
  assert ( v2 ( 0, 0 ) == 2 );
  v2 ( 1, 1 ) = 666;
  assert ( v2 ( 1, 1 ) == 666 );

  assert ( toString ( v2 ) == R"({
  {2, 3},
  {5, 666}
})" );

  assert ( toString ( m2 ) == R"({
  {1, 2, 3},
  {4, 5, 666}
})" );

  assert ( toString ( m2 . slice ( 1, CRange ( 1, 2 ) ) ) == "{5, 666}" );
  assert ( toString ( m2 . slice ( 1, CRangeRev ( 2, 1 ) ) ) == "{666, 5}" );
  assert ( toString ( m2 . slice ( 1, CRange ( 2, 2 ) ) ) == "{666}" );
  assert ( toString ( m2 . slice ( CRange ( 1, 1 ), 2 ) ) == "{666}" );
  assert ( toString ( m2 . slice ( CRange ( 1, 1 ), CRange ( 2, 2 ) ) ) == R"###({
  {666}
})###" );

  assert ( toString ( m4 . slice ( CRangeRev ( 1, 0 ), CRangeRev ( 1, 0 ) ) ) == R"({
  {PA2, PA1},
  {progtest, test}
})" );

  try
  {
    m2 ( 3, 4 ) ++;
    assert ( "missing an exception" == nullptr );
  }
  catch ( const std::exception & e )
  {
  }

  try
  {
    v2 ( 2, 0 ) --;
    assert ( "missing an exception" == nullptr );
  }
  catch ( const std::exception & e )
  {
  }

  try
  {
    CTensor<int, 2> m5
    {
      { 3, 5, 8 },
      { 2, 7 }
    };
    assert ( "missing an exception" == nullptr );
  }
  catch ( const std::exception & e )
  {
  }

  try
  {
    m1 . slice ( CRange ( 0, 4 ), CRange ( 0, 2 ) );
    assert ( "missing an exception" == nullptr );
  }
  catch ( const std::exception & e )
  {
  }

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
