#pragma once

#include <tuple>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <algorithm>

#include "zip_iterator.hpp"
#include "stashing_iterator.hpp"
#include "subscript_iterator.hpp"

namespace feniks {

    namespace _impl {

#define HAS_METHOD(method, return_value)                                \
        template<typename T>                                            \
        concept has_##method = requires(T value) {                      \
            { value.method() } -> return_value;                         \
            { const_cast<const T&>(value).method() } -> return_value;   \
        };

        HAS_METHOD(size, std::integral)
        HAS_METHOD(begin, std::input_or_output_iterator)
        HAS_METHOD(end, std::input_or_output_iterator)

        template<typename T, typename I>
        concept can_subscript = requires(T value, I index) {
            value[index];
        };

        template<typename... T>
        class zip {

        public:

            explicit zip(T&&... containers) : _containers(std::forward<T>(containers)...) {}

            [[nodiscard]] auto size() const requires (has_size<T> && ...) {
                return std::apply(
                        []<typename... S>(const S&... values) { return std::min({ static_cast<std::common_type_t<S...>>(values)... }); },
                        gather<N>::get([](const auto& value) { return value.size(); }, _containers)
                );
            }

            auto operator[](const size_t& i) requires (can_subscript<T, size_t> && ...) {
                return gather<N>::get([&i](auto& value) -> decltype(value[i]) { return value[i]; }, _containers);
            }

            auto operator[](const size_t& i) const requires (can_subscript<T, size_t> && ...) {
                return gather<N>::get([&i](const auto& value) -> decltype(value[i]) { return value[i]; }, _containers);
            }

            auto begin() requires ((has_begin<T> || can_subscript<T, size_t>) && ... ) {
                return zip_iterator(gather<N>::get(
                        []<typename C>(C& container) {
                            if constexpr (has_begin<C>)
                                return container.begin();
                            else
                                return subscript_iterator(&container, 0);
                        },
                        _containers
                    )
                );
            }

            auto begin() const requires ((has_begin<T> || can_subscript<T, size_t>) && ... ) {
                return zip_iterator(gather<N>::get(
                        []<typename C>(const auto& container) {
                            if constexpr (has_begin<C>)
                                return container.begin();
                            else
                                return subscript_iterator(&container, 0);
                        },
                        _containers
                    )
                );
            }

            auto end() requires (((has_begin<T> || can_subscript<T, size_t>) && has_size<zip> || has_end<T>) && ...) {
                return zip_iterator(gather<N>::get(
                        [this]<typename C>(C& container) {
                            if constexpr (has_size<zip>)
                                if constexpr (has_begin<C>)
                                    return std::next(container.begin(), this->size());
                                else
                                    return subscript_iterator(&container, this->size());
                            else
                                return container.end();
                        },
                        _containers
                    )
                );
            }

            auto end() const requires (((has_begin<T> || can_subscript<T, size_t>) && has_size<zip> || has_end<T>) && ...) {
                return zip_iterator(gather<N>::get(
                        [this]<typename C>(C& container) {
                            if constexpr (has_size<zip>)
                                if constexpr (has_begin<C>)
                                    return std::next(container.begin(), this->size());
                                else
                                    return subscript_iterator(&container, this->size());
                            else
                                return container.end();
                        },
                        _containers
                    )
                );
            }

            auto sbegin() {
                return stashing_iterator(begin());
            }

            auto sbegin() const {
                return stashing_iterator(begin());
            }

            auto send() {
                return stashing_iterator(end());
            }

            auto send() const {
                return stashing_iterator(end());
            }

        private:

            static constexpr auto N = sizeof...(T);

            std::tuple<T...> _containers;

        };

    }

    template<typename... T>
    auto zip(T&&... values) {
        return _impl::zip<T...>(std::forward<T>(values)...);
    }

}// namespace feniks
