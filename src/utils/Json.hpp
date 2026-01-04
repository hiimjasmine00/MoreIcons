#include <matjson.hpp>

namespace Json {
    template <class T>
    T get(const matjson::Value& value, std::string_view key, T defaultValue = T()) {
        return value[key].as<T>().unwrapOr(std::move(defaultValue));
    }
}
