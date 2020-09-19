/*
  Custom iterator. Inserts the delimeter after the first and
  subsequent items.
*/
#include <iostream>
#include <string>

template<typename T>
class ostream_custom_iterator
{
public:

  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using iterator_category = std::output_iterator_tag;

  ostream_custom_iterator(std::wostream& stream, const T& delimiter)
      : m_stream(stream), m_delimiter(delimiter), m_first(true)
  {
  }

  ostream_custom_iterator& operator++() { return *this; }
  ostream_custom_iterator& operator++(int) { return *this; }
  ostream_custom_iterator& operator*() { return *this; }

  ostream_custom_iterator& operator= (const T& value)
  {
    if (!m_first)
    {
      m_stream << m_delimiter;
    }
    else
    {
      m_first = false;
    }

    m_stream << value;
    return *this;
  }

private:
  std::wostream& m_stream;
  const T m_delimiter;
  bool m_first;
};
