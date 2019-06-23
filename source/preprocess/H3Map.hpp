#ifndef GIAGUI_PREPROCESS_H3MAP_HPP_
#define GIAGUI_PREPROCESS_H3MAP_HPP_ 1

#include <h3/h3api.h>
#include <map>
#include <string>


namespace poglar {

  struct vec3d {
    double x, y, z;

    inline bool operator==(const vec3d &v) const;
    inline bool operator!=(const vec3d &v) const;
    
    inline vec3d operator+(const vec3d &v) const;
    inline vec3d operator-(const vec3d &v) const;

    inline vec3d operator*(const double a) const;

    inline vec3d & operator+=(const vec3d &v);
    inline vec3d & operator*=(const double a);
  };


  template <class Type>
  class H3Map {
  public:
    H3Map();

    int resolution() const;
    Type operator[](const H3Index h) const;

    void read(const std::string &filename);
    void write(const std::string &filename) const;

    void refine(const int offset);

    void add(const H3Map<Type> &other);

    void scale(const double factor);
    void scale(const H3Map<double> &factor);

  private:
    int resolution_;
    Type default_;

    std::map<H3Index, Type> values_;

    template <class> friend class H3Map;
  };


  bool vec3d::operator==(const vec3d &v) const
    { return x == v.x && y == v.y && z == v.z; }
  bool vec3d::operator!=(const vec3d &v) const
    { return !(operator==(v)); }


  vec3d vec3d::operator+(const vec3d &v) const
    { return {x + v.x, y + v.y, z + v.z}; }
  vec3d vec3d::operator-(const vec3d &v) const
    { return {x - v.x, y - v.y, z - v.z}; }


  vec3d vec3d::operator*(const double a) const
    { return {a * x, a * y, a * z}; }


  vec3d & vec3d::operator+=(const vec3d &v)
  {
    x += v.x;
    y += v.y;
    z += v.z;

    return *this;
  }


  vec3d & vec3d::operator*=(const double a)
  {
    x *= a;
    y *= a;
    z *= a;
    
    return *this;
  }

} /* namespace poglar */

#endif /* GIAGUI_PREPROCESS_H3MAP_HPP_ */
