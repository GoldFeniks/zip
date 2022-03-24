#pragma once

#include <tuple>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <algorithm>

namespace feniks {

    namespace _impl {

#define HAS_CONCEPT(name, concept, ...)                                                                        \
        template<typename T, __VA_ARGS__>                                                                      \
        class name {                                                                                           \
                                                                                                               \
        private:                                                                                               \
                                                                                                               \
            template<typename C>                                                                               \
            static char test(std::add_pointer_t<std::remove_reference_t<decltype( concept )>>);                \
                                                                                                               \
            template<typename C>                                                                               \
            static int  test(...);                                                                             \
                                                                                                               \
        public:                                                                                                \
                                                                                                               \
            static constexpr bool value = std::is_same_v<decltype(test<std::remove_reference_t<T>>(0)), char>; \
                                                                                                               \
        };                                                                                                     \
                                                                                                               \
        template<typename... T>                                                                                \
        constexpr bool name##_v = name<T...>::value;

#define HAS_METHOD(method) HAS_CONCEPT(has_##method, std::declval<C>().method (std::declval<Args>()...), typename... Args)

        HAS_METHOD(size)
        HAS_METHOD(begin)
        HAS_METHOD(end)

        HAS_CONCEPT(can_add, std::declval<C>() + std::declval<V>(), typename V)
        HAS_CONCEPT(can_subtract, std::declval<C>() - std::declval<V>(), typename V)
        HAS_CONCEPT(can_inplace_add, std::declval<C>() += std::declval<V>(), typename V)
        HAS_CONCEPT(can_inplace_subtract, std::declval<C>() -= std::declval<V>(), typename V)
        HAS_CONCEPT(can_compare_less, std::declval<C>() < std::declval<V>(), typename V)
        HAS_CONCEPT(can_compare_greater, std::declval<C>() > std::declval<V>(), typename V)
        HAS_CONCEPT(can_compare_less_equal, std::declval<C>() <= std::declval<V>(), typename V)
        HAS_CONCEPT(can_compare_greater_equal, std::declval<C>() >= std::declval<V>(), typename V)

        HAS_CONCEPT(is_subscriptable, std::declval<C>()[std::declval<V>()], typename V)
        HAS_CONCEPT(is_pre_decrementable, --std::declval<C>(), typename = void)
        HAS_CONCEPT(is_post_decrementable, std::declval<C>()--, typename = void)

        template<size_t N, size_t M = N>
        struct gather {

            gather() = delete;

            template<typename Getter, typename Tuple, typename... Values>
            static auto get(Getter&& getter, Tuple&& tuple, Values&&... values) {
                return gather<N, M - 1>::get(getter, tuple, std::forward<Values>(values)..., getter(std::get<N - M>(tuple)));
            }

            template<typename Getter, typename Tuple, typename T, typename... Values>
            static void get(Getter&& getter, Tuple&& tuple, T* location, Values&&... values) {
                gather<N, M - 1>::get(getter, tuple, location, std::forward<Values>(values)..., getter(std::get<N - M>(tuple)));
            }

        };

        template<size_t N>
        struct gather<N, 0> {

            gather() = delete;

            template<typename Getter, typename Tuple, typename... Values>
            static auto get(Getter&& getter, Tuple&& tuple, Values&&... values) {
                return std::tuple<Values...>(values...);
            }

            template<typename Getter, typename Tuple, typename T, typename... Values>
            static void get(Getter&& getter, Tuple&& tuple, T* location, Values&&... values) {
                new (location) std::tuple<Values...>(values...);
            }

        };

        template<typename T>
        class subscript_iterator {

        public:
        
            using iterator_category = std::random_access_iterator_tag;
            using value_type = std::decay_t<decltype(std::declval<T>()[0])>;
            using difference_type = ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            subscript_iterator() = default;
            subscript_iterator(T* container, const ptrdiff_t& index) : _index(index), _container(container) {}

            bool operator==(const subscript_iterator& other) const {
                return _container == other._container && _index == other._index;
            }

            bool operator!=(const subscript_iterator& other) const {
                return !(*this == other);
            }

            auto& operator*() {
                return *_container[_index];
            }

            const auto& operator*() const {
                return *_container[_index];
            }

            auto& operator->() {
                return **this;
            }

            const auto& operator->() const {
                return **this;
            }

            auto& operator++() {
                ++_index;
                return *this;
            }

            auto operator++(int) {
                return subscript_iterator(_container, _index++);
            }

            auto& operator--() {
                --_index;
                return *this;
            }

            auto operator--(int) {
                return subscript_iterator(_container, _index--);
            }

            auto operator+(const ptrdiff_t& n) const {
                return subscript_iterator(_container, _index + n);
            }

            auto& operator+=(const ptrdiff_t& n) {
                _index += n;
                return *this;
            }

            auto operator-(const ptrdiff_t& n) {
                return subscript_iterator(_container, _index - n);
            }

            auto operator-(const subscript_iterator& other) {
                return _index - other._index;
            }

            auto& operator-=(const ptrdiff_t& n) {
                _index -= n;
                return *this;
            }

            auto& operator[](const size_t& n) {
                return *_container[_index + n];
            }

            const auto& operator[](const size_t& n) const {
                return *_container[_index + n];
            }

            bool operator<(const subscript_iterator& other) const {
                return _index < other._index;
            }

            bool operator<=(const subscript_iterator& other) const {
                return _index <= other._index;
            }

            bool operator>(const subscript_iterator& other) const {
                return _index > other._index;
            }

            bool operator>=(const subscript_iterator& other) const {
                return _index >= other._index;
            }

        private:

            ptrdiff_t _index{};
            T* _container = nullptr;

            friend subscript_iterator operator+(const ptrdiff_t& n, const subscript_iterator& iterator) {
                return subscript_iterator(iterator._container, n + iterator._index);
            }

        };

        template<typename... V>
        requires (std::is_reference_v<decltype(*std::declval<V>())> && ...)
        class zip_iterator {

        public:

            using iterator_category = std::forward_iterator_tag;
            using value_type = std::tuple<decltype(*std::declval<const V>())...>;
            using difference_type = ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            zip_iterator() = default;
            explicit zip_iterator(std::tuple<V...>&& iterators) : _iterators(std::move(iterators)) {}

            bool operator==(const zip_iterator& other) const {
                return other._iterators == _iterators;
            }

            bool operator!=(const zip_iterator& other) const {
                return !(*this == other);
            }

            auto& operator*() {
                gather<N>::get([](const auto& it) -> decltype(*it) { return *it; }, _iterators, _value);
                return *reinterpret_cast<value_type*>(_value);
            }

            auto& operator*() const {
                gather<N>::get([](const auto& it) -> decltype(*it) { return *it; }, _iterators, _value);
                return *reinterpret_cast<value_type*>(_value);
            }

            auto operator->() {
                return **this;
            }

            auto operator->() const {
                return **this;
            }

            auto& operator++() {
                gather<N>::get([](auto& it) { return ++it; }, _iterators);
                return *this;
            }

            auto operator++(int) {
                return zip_iterator(gather<N>::get([](auto& it) { return it++; }, _iterators));                
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(is_pre_decrementable_v<V> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            auto& operator--() {
                gather<N>::get([](auto& it) { return --it; }, _iterators);
                return *this;
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(is_post_decrementable_v<V> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            auto operator--(int) {
                return zip_iterator(gather<N>::get([](auto& it) { return it--; }, _iterators));
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_add_v<V, ptrdiff_t> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            auto operator+(const ptrdiff_t& n) const {
                return zip_iterator(gather<N>::get([&n](const auto& it) { return it + n; }, _iterators));
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_inplace_add_v<V, ptrdiff_t> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            auto& operator+=(const ptrdiff_t& n) {
                gather<N>::get([&n](auto& it) -> decltype(it)& { return it += n; }, _iterators);
                return *this;
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_subtract_v<V, ptrdiff_t> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            auto operator-(const ptrdiff_t& n) const {
                return zip_iterator(gather<N>::get([&n](const auto& it) { return it - n; }, _iterators));
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_subtract_v<V, ptrdiff_t> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            auto operator-(const zip_iterator& n) const {
                return std::get<0>(_iterators) - std::get<0>(n._iterators);
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_inplace_subtract_v<V, ptrdiff_t> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            auto& operator-=(const ptrdiff_t& n) {
                gather<N>::get([&n](auto& it) -> decltype(it)& { return it -= n; }, _iterators);
                return *this;
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(is_subscriptable_v<V, size_t> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            auto& operator[](const size_t& n) {
                gather<N>::get([&n](auto& it) -> decltype(it[0])& { return it[n]; }, _iterators, _value);
                return *reinterpret_cast<value_type*>(_value);
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(is_subscriptable_v<V, size_t> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            auto& operator[](const size_t& n) const {
                gather<N>::get([&n](const auto& it) -> const decltype(it[0])& { return it[n]; }, _iterators, _value);
                return *reinterpret_cast<value_type*>(_value);
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_compare_less_v<V, V> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            bool operator<(const zip_iterator& other) const {
                return _iterators < other._iterators;
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_compare_less_equal_v<V, V> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            bool operator<=(const zip_iterator& other) const {
                return _iterators <= other._iterators;
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_compare_greater_v<V, V> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            bool operator>(const zip_iterator& other) const {
                return _iterators > other._iterators;
            }

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_compare_greater_equal_v<V, V> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            bool operator>=(const zip_iterator& other) const {
                return _iterators >= other._iterators;
            }

        private:

            static constexpr size_t N = sizeof...(V);

            std::tuple<V...> _iterators{};
            char _value[sizeof(value_type)];

            template<typename C = std::tuple<V...>, typename = std::enable_if_t<(can_add_v<ptrdiff_t, V> && ...) && std::is_same_v<C, std::tuple<V...>>>>
            friend zip_iterator operator+(const ptrdiff_t& n, const zip_iterator<V...>& iterator) {
                return gather<N>::get([&n](const auto& it) { return n + it; }, iterator._iterators);
            }

        };

        template<bool, typename>
        class sizable;

        template<typename T>
        class sizable<false, T> {

        public:

            sizable(const T& values) : _values(values) {}

        private:

            const T& _values;

        };

        template<typename T>
        class sizable<true, T> {

        public:

            sizable(const T& values) : _values(values) {}

            size_t size() const {
                return std::apply(
                    [](const auto&... values) { return std::min({ values... }); }, 
                    gather<N>::get([](const auto& value) { return value.size(); }, _values)
                );
            }

        private:

            static constexpr auto N = std::tuple_size_v<T>;

            const T& _values;

        };

        template<typename... T>
        using sizable_t = sizable<(has_size_v<T> && ...), std::tuple<T...>>;

        template<bool, typename>
        class subscriptable;

        template<typename T>
        class subscriptable<false, T> {

        public:

            subscriptable(const T& values) : _values(values) {}

        private:

            const T& _values;

        };

        template<typename T>
        class subscriptable<true, T> {

        public:

            subscriptable(const T& values) : _values(values) {}

            auto operator[](const size_t& i) {
                return gather<N>::get([&i](auto& value) -> decltype(value[i]) { return value[i]; }, _values);
            }

            auto operator[](const size_t& i) const {
                return gather<N>::get([&i](const auto& value) -> decltype(value[i]) { return value[i]; }, _values);
            }

        private:

            static constexpr auto N = std::tuple_size_v<T>;

            const T& _values;

        };

        template<typename... T>
        using subscriptable_t = subscriptable<(is_subscriptable_v<T, size_t> && ...), std::tuple<T...>>;

        template<typename... T>
        class zip : public sizable_t<T...>, public subscriptable_t<T...> {

        private:

            using sizable_base_t = sizable_t<T...>;
            using subscriptable_base_t = subscriptable_t<T...>;

        public:

            explicit zip(T&&... containers)
                : _containers(std::forward<T>(containers)...), sizable_base_t(_containers), subscriptable_base_t(_containers) {}

            zip(const zip& other) : _containers(other._containers), sizable_base_t(_containers), subscriptable_base_t(_containers) {}
            zip(zip&& other) : _containers(std::move(other._containers)), sizable_base_t(_containers), subscriptable_base_t(_containers) {}

            zip& operator=(const zip& other) {
                _containers = other._containers;
            }

            zip& operator=(zip&& other) {
                _containers = std::move(other._containers);
            }

            auto begin() {
                return zip_iterator(gather<N>::get(
                        [](auto& container) {
                            using type = decltype(container);

                            static_assert(has_begin_v<type> || is_subscriptable_v<type, size_t>, "Container must either support begin() or be subscriptable");

                            if constexpr (has_begin_v<type>)
                                return container.begin();
                            else if constexpr (is_subscriptable_v<type, size_t>)
                                return subscript_iterator(&container, 0);
                        },
                        _containers
                    )
                );
            }

            auto begin() const {
                return zip_iterator(gather<N>::get(
                        [](const auto& container) {
                            using type = decltype(container);

                            static_assert(has_begin_v<type> || is_subscriptable_v<type, size_t>, "Container must either support begin() or be subscriptable");

                            if constexpr (has_begin_v<type>)
                                return container.begin();
                            else if constexpr (is_subscriptable_v<type, size_t>)
                                return subscript_iterator(&container, 0);
                        },
                        _containers
                    )
                );
            }

            auto end() {
                return zip_iterator(gather<N>::get(
                        [this](auto& container) {
                            using type = decltype(container);

                            static_assert((has_begin_v<type> || is_subscriptable_v<type, size_t>) && has_size_v<type> || has_begin_v<type> && has_end_v<type>, 
                                "Container must support size() or both begin() and end()");

                            static_assert(is_subscriptable_v<type, size_t> && has_size_v<type>, "Foo");

                            if constexpr (has_begin_v<type>) {
                                if constexpr (has_size_v<type>)
                                    return std::next(container.begin(), this->size());
                                else if constexpr (has_end_v<type>)
                                    return container.end();
                            }
                            else if constexpr (is_subscriptable_v<type, size_t> && has_size_v<type>)
                                return std::next(subscript_iterator(&container, 0), this->size());
                        },
                        _containers
                    )
                );
            }

            auto end() const {
                return zip_iterator(gather<N>::get(
                        [this](const auto& container) {
                            using type = decltype(container);

                            static_assert((has_begin_v<type> || is_subscriptable_v<type, size_t>) && has_size_v<type> || has_begin_v<type> && has_end_v<type>, 
                                "Container must support size() or both begin() and end()");

                            if constexpr (has_begin_v<type>) {
                                if constexpr (has_size_v<type>)
                                    return std::next(container.begin(), this->size());
                                else if constexpr (has_end_v<type>)
                                    return container.end();
                            }
                            else if constexpr (is_subscriptable_v<type, size_t> && has_size_v<type>)
                                return std::next(subscript_iterator(&container, 0), this->size());
                        },
                        _containers
                    )
                );
            }

        private:

            static constexpr auto N = sizeof...(T);
            static constexpr bool has_end = (has_end_v<T> && ...);
            static constexpr bool has_size = has_size_v<zip>;
            static constexpr bool has_begin = (has_begin_v<T> && ...);
            static constexpr bool is_subscriptable = is_subscriptable_v<zip, size_t>;

            std::tuple<T...> _containers;

        };

    }

    template<typename... T>
    auto zip(T&&... values) {
        return _impl::zip<T...>(std::forward<T>(values)...);
    }

    template<typename T>
    auto copy(T value) {
        return value;
    }

}// namespace feniks
