#include "preprocess/H3Map.hpp"
#include <cassert>
#include <cmath>
#include <vector>
#include <cpptoml.h>


namespace poglar {

  namespace {
    template <class Type>
    struct default_value;

    template <>
    struct default_value<double> {
      static double get() { return 0.0; }
    };

    template <>
    struct default_value<vec3d> {
      static vec3d get() { return {0.0, 0.0, 0.0}; }
    };
  } /* namespace <anon> */

  template <class Type>
  H3Map<Type>::H3Map()
  : resolution_(0)
  , default_(default_value<Type>::get()) {}


  template <class Type>
  int
  H3Map<Type>::resolution() const
    { return resolution_; }


  template <class Type>
  Type
  H3Map<Type>::operator[](const H3Index h) const
  {
    /* move the index to the resolution of the map */
    assert(h3GetResolution(h) >= resolution_);
    const H3Index index = h3ToParent(h, resolution_);

    auto it = values_.find(index);
    if (it != values_.end())
      return it->second;
    else
      return default_;
  }


  namespace {
    template <class Type>
    struct value_reader;

    template <>
    struct value_reader<double> {
      static double get(std::shared_ptr<cpptoml::base> node)
        { return node->as<double>()->get(); }
    };

    template <>
    struct value_reader<vec3d> {
      static vec3d get(std::shared_ptr<cpptoml::base> node) {
        const std::vector<double> values =
          *(node->as_array()->get_array_of<double>());
        assert(values.size() == 3);

        return {values[0], values[1], values[2]};
      }
    };
  } /* namespace <anon> */


  template <class Type>
  void
  H3Map<Type>::read(const std::string &filename)
  {
    std::shared_ptr<cpptoml::table> table = cpptoml::parse_file(filename);
    const std::string type = *(table->get_qualified_as<std::string>("h3.type"));
    // TODO check proper type

    resolution_ = *(table->get_qualified_as<int>("h3.resolution"));
    default_ = value_reader<Type>::get(table->get_qualified("h3.default"));

    const double density = table->get_qualified_as<double>("h3.density").value_or(1.0);

    for(const auto& kv: *(table->get_table_qualified("h3.values"))) {
      const H3Index index = stringToH3(kv.first.c_str());
      const Type value = value_reader<Type>::get(kv.second) * density;

      values_.emplace(std::make_pair(index, value));
    }
  }


  namespace {
    template <class Type>
    struct type_writer;

    template <>
    struct type_writer<double> {
      static const char * get() { return "1f"; }
    };

    template <>
    struct type_writer<vec3d> {
      static const char * get() { return "3f"; }
    };


    template <class Type>
    struct value_writer;

    template <>
    struct value_writer<double> {
      static std::string get(const double &value) {
        std::ostringstream os;
        os << std::scientific << value;

        return os.str();
      }
    };

    template <>
    struct value_writer<vec3d> {
      static std::string get(const vec3d &value) {
        std::ostringstream os;
        os << std::scientific << "[" << value.x << ", " << value.y
           << ", " << value.z << "]";

        return os.str();
      }
    };
  } /* namespace <anon> */


  template <class Type>
  void
  H3Map<Type>::write(const std::string &filename) const
  {
    std::ofstream ofile(filename);
    ofile << "[h3]\r\n"
             "resolution = " << resolution() << "\r\n"
             "type = \"" << type_writer<Type>::get() << "\"\r\n"
             "default = " << value_writer<Type>::get(default_) << "\r\n"
             "\r\n"
             "[h3.values]\r\n";

    for (const auto &kv: values_) {
      ofile << std::hex << kv.first << " = "
            << value_writer<Type>::get(kv.second) << "\r\n";
    }
  }


  template <class Type>
  void
  H3Map<Type>::refine(const int offset)
  {
    assert(offset >= 0);
    if (offset == 0)
      return;

    const int new_resolution = resolution_ + offset;
    std::map<H3Index, Type> new_values;

    std::vector<H3Index> children;
    for (const auto &kv: values_) {
      const H3Index k = kv.first;
      const Type v = kv.second;

      children.resize(maxH3ToChildrenSize(k, new_resolution));
      std::fill(children.begin(), children.end(), 0);
      h3ToChildren(k, new_resolution, children.data());

      for (const auto &newk: children)
        if (newk != 0)
          new_values.emplace(std::make_pair(newk, v));
    }

    resolution_ = new_resolution;
    values_ = new_values;
  }


  template <class Type>
  void
  H3Map<Type>::add(const H3Map<Type> &other)
  {
    const int offset = std::max(resolution(), other.resolution()) - resolution();
    refine(offset);

    /* Copying the values from other to new_values, adding the current default */
    std::map<H3Index, Type> new_values;
    {
      std::vector<H3Index> children;
      for (const auto &kv: other.values_) {
        const H3Index k = kv.first;
        const Type v = kv.second;

        children.resize(maxH3ToChildrenSize(k, resolution()));
        std::fill(children.begin(), children.end(), 0);
        h3ToChildren(k, resolution(), children.data());

        for (const auto &newk: children)
          if (newk != 0)
            new_values.emplace(std::make_pair(newk, v + default_));
      }
    }

    /* Adding the current values, taking care of the intersection between the
     * to sets of indices */
    for (const auto &kv: values_) {
        const H3Index k = kv.first;
        const Type v = kv.second;

        auto it = new_values.find(k);
        if (it != new_values.end())
          it->second += v - default_;
        else
          new_values.emplace(std::make_pair(k, v + other.default_));
    }

    /* overwrite default */
    default_ += other.default_;

    /* cleaning up and copy */
    values_.clear();
    for (const auto &kv: new_values)
      if (kv.second != default_)
        values_.emplace(kv);
  }


  template <class Type>
  void
  H3Map<Type>::scale(const double factor)
  {
    default_ *= factor;

    for (auto &kv: values_)
      kv.second *= factor;
  }


  template <class Type>
  void
  H3Map<Type>::scale(const H3Map<double> &factor)
  {
    const int offset = std::max(resolution(), factor.resolution()) - resolution();
    refine(offset);

    /* Copying the values from other to new_values, adding the current default */
    std::map<H3Index, Type> new_values;
    {
      std::vector<H3Index> children;
      for (const auto &kv: factor.values_) {
        const H3Index k = kv.first;
        const double v = kv.second;

        children.resize(maxH3ToChildrenSize(k, resolution()));
        std::fill(children.begin(), children.end(), 0);
        h3ToChildren(k, resolution(), children.data());

        for (const auto &newk: children)
          if (newk != 0 && values_.find(newk) == values_.end())
            new_values.emplace(std::make_pair(newk, default_ * v));
      }
    }

    /* Adding the current values, taking care of the intersection between the
     * to sets of indices */
    for (const auto &kv: values_) {
        const H3Index k = kv.first;
        const Type v = kv.second;

        new_values.emplace(std::make_pair(k, v * factor[k]));
    }

    /* overwrite default */
    default_ *= factor.default_;

    /* cleaning up and copy */
    values_.clear();
    for (const auto &kv: new_values)
      if (kv.second != default_)
        values_.emplace(kv);
  }


  H3Map<vec3d>
  SphericalTopography(const int resolution)
  {
    H3Map<vec3d> result;

    result.resolution_ = resolution;

    std::vector<H3Index> index0(res0IndexCount()),
                         children;
    GeoCoord center;

    getRes0Indexes(index0.data());
    for (const H3Index i0: index0) {
      children.resize(maxH3ToChildrenSize(i0, resolution));
      std::fill(children.begin(), children.end(), 0);
      h3ToChildren(i0, resolution, children.data());

      for (const H3Index i: children)
        if (i != 0) {
          h3ToGeo(i, &center);

          vec3d normal{
            -std::cos(center.lat) * std::cos(center.lon),
            -std::cos(center.lat) * std::sin(center.lon),
            -std::sin(center.lat)
          };
          result.values_[i] = normal;
        }
    }

    return result;
  }


  template class H3Map<double>;
  template class H3Map<vec3d>;

} /* namespace poglar */
