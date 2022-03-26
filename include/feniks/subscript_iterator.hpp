#pragma once

namespace feniks {

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
            return _container != other._container || _index != other._index;;
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

        T& container() {
            return *_container;
        }

        const T& container() const {
            return *_container;
        }

    private:

        ptrdiff_t _index{};
        T* _container = nullptr;

        friend subscript_iterator operator+(const ptrdiff_t& n, const subscript_iterator& iterator) {
            return subscript_iterator(iterator._container, n + iterator._index);
        }

    };

}// namespace feniks