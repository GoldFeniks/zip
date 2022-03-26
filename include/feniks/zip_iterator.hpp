#pragma once

#include <tuple>
#include <iterator>

namespace feniks {

    namespace _impl {

        template<size_t N, size_t M = N>
        struct gather {

            gather() = delete;

            template<typename Getter, typename Tuple, typename... Values>
            static auto get(Getter&& getter, Tuple&& tuple, Values&&... values) {
                return gather<N, M - 1>::get(getter, tuple, std::forward<Values>(values)..., getter(std::get<N - M>(tuple)));
            }

        };

        template<size_t N>
        struct gather<N, 0> {

            gather() = delete;

            template<typename Getter, typename Tuple, typename... Values>
            static auto get(Getter&& getter, Tuple&& tuple, Values&&... values) {
                return std::tuple<Values...>(std::forward<Values>(values)...);
            }

        };

        template<typename... V>
        using common_tag_t =
            std::conditional_t<(std::random_access_iterator<V>                    && ...), std::random_access_iterator_tag,
            std::conditional_t<(std::bidirectional_iterator<V>                    && ...), std::bidirectional_iterator_tag,
            std::conditional_t<(std::forward_iterator<V>                          && ...), std::forward_iterator_tag,
            std::conditional_t<(std::output_iterator<V, std::iter_reference_t<V>> && ...), std::output_iterator_tag,
            std::enable_if_t<(std::input_or_output_iterator<V> && ...), std::input_iterator_tag>>>>>;

    }// namespace _impl

    template<typename... It>
    class zip_iterator {

    public:

        using iterator_category = _impl::common_tag_t<It...>;
        using value_type = std::tuple<decltype(*std::declval<const It>())...>;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type;

        zip_iterator() = default;
        explicit zip_iterator(std::tuple<It...>&& iterators) : _iterators(std::move(iterators)) {}

        bool operator==(const zip_iterator& other) const {
            return other._iterators == _iterators;
        }

        bool operator!=(const zip_iterator& other) const {
            return other._iterators != _iterators;
        }

        auto operator*() {
            return _impl::gather<N>::get([](const auto& it) -> decltype(*it) { return *it; }, _iterators);
        }

        auto operator*() const {
            return _impl::gather<N>::get([](const auto& it) -> decltype(*it) { return *it; }, _iterators);
        }

        auto& operator++() {
            _impl::gather<N>::get([](auto& it) { return ++it; }, _iterators);
            return *this;
        }

        auto operator++(int) {
            return zip_iterator(_impl::gather<N>::get([](auto& it) { return it++; }, _iterators));
        }

        auto& operator--() requires (std::bidirectional_iterator<It> && ...) {
            _impl::gather<N>::get([](auto& it) { return --it; }, _iterators);
            return *this;
        }

        auto operator--(int) requires (std::bidirectional_iterator<It> && ...) {
            return zip_iterator(_impl::gather<N>::get([](auto& it) { return it--; }, _iterators));
        }

        auto operator+(const ptrdiff_t& n) const requires (std::random_access_iterator<It> && ...) {
            return zip_iterator(_impl::gather<N>::get([&n](const auto& it) { return it + n; }, _iterators));
        }

        auto& operator+=(const ptrdiff_t& n) requires (std::random_access_iterator<It> && ...) {
            _impl::gather<N>::get([&n](auto& it) -> decltype(it)& { return it += n; }, _iterators);
            return *this;
        }

        auto operator-(const ptrdiff_t& n) const requires (std::random_access_iterator<It> && ...) {
            return zip_iterator(_impl::gather<N>::get([&n](const auto& it) { return it - n; }, _iterators));
        }

        auto operator-(const zip_iterator& n) const requires (std::random_access_iterator<It> && ...) {
            return std::get<0>(_iterators) - std::get<0>(n._iterators);
        }

        auto& operator-=(const ptrdiff_t& n) requires (std::random_access_iterator<It> && ...) {
            _impl::gather<N>::get([&n](auto& it) -> decltype(it)& { return it -= n; }, _iterators);
            return *this;
        }

        auto operator[](const size_t& n) requires (std::random_access_iterator<It> && ...) {
            return _impl::gather<N>::get([&n](auto& it) -> decltype(it[0]) { return it[n]; }, _iterators);
        }

        auto operator[](const size_t& n) const requires (std::random_access_iterator<It> && ...) {
            return _impl::gather<N>::get([&n](const auto& it) -> decltype(it[0]) { return it[n]; }, _iterators);
        }

        bool operator<(const zip_iterator& other) const requires (std::random_access_iterator<It> && ...) {
            return _iterators < other._iterators;
        }

        bool operator<=(const zip_iterator& other) const requires (std::random_access_iterator<It> && ...) {
            return _iterators <= other._iterators;
        }

        bool operator>(const zip_iterator& other) const requires (std::random_access_iterator<It> && ...) {
            return _iterators > other._iterators;
        }

        bool operator>=(const zip_iterator& other) const requires (std::random_access_iterator<It> && ...) {
            return _iterators >= other._iterators;
        }

        template<size_t I>
        const auto& get() const {
            return std::get<I>(_iterators);
        }

        template<size_t I>
        auto&& get() && {
            return std::get<I>(_iterators);
        }

    private:

        static constexpr size_t N = sizeof...(It);

        std::tuple<It...> _iterators{};

        friend zip_iterator operator+(const ptrdiff_t& n, const zip_iterator<It...>& iterator) requires (std::random_access_iterator<It> && ...) {
            return _impl::gather<N>::get([&n](const auto& it) { return n + it; }, iterator._iterators);
        }

    };

}// namespace feniks

namespace std {

    template<size_t I, typename... It>
    const auto& get(const feniks::zip_iterator<It...>& it) {
        return it.template get<I>();
    }

    template<size_t I, typename... It>
    auto&& get(feniks::zip_iterator<It...>&& it) {
        return it.template get<I>();
    }

}// namespace std
