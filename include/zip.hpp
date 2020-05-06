#pragma once

#include <tuple>
#include <cstddef>

namespace zip {

    namespace __impl {

        template<size_t N, size_t M = N>
        struct gather_values {

            gather_values() = delete;

            template<typename Tuple, typename... Values>
            static auto get(const size_t& index, Tuple& tuple, Values&... values) {
                return gather_values<N, M - 1>::get(index, tuple, values..., std::get<N - M>(tuple)[index]);
            }

        };

        template<size_t N>
        struct gather_values<N, 0> {

            gather_values() = delete;

            template<typename Tuple, typename... Values>
            static auto get(const size_t& index, const Tuple& tuple, Values&... values) {
                return std::tuple<const Values&...>(values...);
            }

            template<typename Tuple, typename... Values>
            static auto get(const size_t& index, Tuple& tuple, Values&... values) {
                return std::tuple<Values&...>(values...);
            }

        };

        template<typename... T>
        class zip {

        public:

            explicit zip(T&&... values)
                : _values(std::forward<T>(values)...), _size(std::get<0>(_values).size()) {}

            auto begin() const {
                return iterator(*this);
            }

            auto end() const {
                return iterator(*this, size());
            }

            auto size() const {
                return _size;
            }

            auto operator[](const size_t& index) {
                return __impl::gather_values<N>::get(index, _values);
            }

            auto operator[](const size_t& index) const {
                return __impl::gather_values<N>::get(index, _values);
            }

        private:

            std::tuple<T...> _values;
            const size_t _size;

            static constexpr auto N = sizeof...(T);

            class iterator {

            public:

                explicit iterator(const zip& owner, const size_t index = 0) : _owner(owner), _index(index) {}

                bool operator==(const iterator& other) const {
                    return _index == other._index;
                }

                bool operator!=(const iterator& other) const {
                    return !(*this == other);
                }

                auto operator*() {
                    return _owner[_index];
                }

                auto operator*() const {
                    return _owner[_index];
                }

                iterator& operator++() {
                    ++_index;
                    return *this;
                }

                iterator operator++(int) {
                    auto res = iterator(_owner, _index);
                    ++_index;
                    return res;
                }

            private:

                const zip& _owner;
                size_t _index = 0;

            };

        };

    }// namespace __impl

    template<typename... T>
    auto zip(T&&... values) {
        return __impl::zip<T...>(std::forward<T>(values)...);
    }

}// namespace zip
