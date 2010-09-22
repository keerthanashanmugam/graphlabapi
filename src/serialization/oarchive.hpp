#ifndef GRAPHLAB_OARCHIVE_HPP
#define GRAPHLAB_OARCHIVE_HPP

#include <iostream>
#include <string>
#include <boost/mpl/assert.hpp>
#include <logger/assertions.hpp>
#include <serialization/has_save.hpp>
namespace graphlab {
/**
The output archive object.
It is just a simple wrapper around a ostream
*/
class oarchive{
 public:
  std::ostream* o;

  oarchive(std::ostream& os)
    : o(&os) {}

  ~oarchive() {
    if (o) o->flush();
  }
};


namespace archive_detail {

/**
Implementation of the serializer for different types.
This is the catch-all and is used to call the .save function
if T is a class. Fails at runtime otherwise.
*/
template <typename ArcType, typename T>
struct serialize_impl {
  static void exec(ArcType &o, const T& t) {
    save_or_fail(o, t);
  }
};

/**
Re-dispatch if for some reasons T already has a const
*/
template <typename ArcType, typename T>
struct serialize_impl<ArcType, const T> {
  static void exec(ArcType &o, const T& t) {
    serialize_impl<ArcType, T>::exec(o, t);
  }
};
}// archive_detail


/**
Allows Use of the "stream" syntax for serialization 
*/
template <typename T>
oarchive& operator<<(oarchive& a, const T i) {
  archive_detail::serialize_impl<oarchive, T>::exec(a, i);
  return a;
}

/**
Serializes an arbitrary pointer + length to an archive 
*/
inline oarchive& serialize(oarchive& a, const void* i,const size_t length) {
  // save the length
  operator<<(a,length);
  a.o->write(reinterpret_cast<const char*>(i), length);
  assert(!a.o->fail());
  return a;
}


}


#include <serialization/basic_types.hpp>
#endif  
