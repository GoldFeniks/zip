#pragma once
#include <iterator>
#include <optional>

namespace feniks {

    template<typename It>
    class stashing_iterator {

    public:

        using iterator_category = typename It::iterator_category;
        using value_type = typename It::value_type;
        using difference_type = typename It::difference_type;
        using pointer = value_type*;
        using reference = value_type&;

        stashing_iterator() = default;
        explicit stashing_iterator(It iterator) : _iterator(std::move(iterator)) {}

        ~stashing_iterator() {
            delete _value;
        }

        stashing_iterator(const stashing_iterator& other) : _iterator(other._iterator) {}

        stashing_iterator(stashing_iterator&& other) noexcept {
            std::swap(_iterator, other._iterator);
            std::swap(_value, other._value);
        }

        stashing_iterator& operator=(const stashing_iterator& other) {
            if (this == &other)
                return *this;

            this->_iterator = other._iterator;
            return *this;
        }

        stashing_iterator& operator=(stashing_iterator&& other) noexcept {
            if (this == &other)
                return *this;

            std::swap(_iterator, other._iterator);
            std::swap(_value, other._value);
            return *this;
        }

        bool operator==(const stashing_iterator& other) const {
            return other._iterator.value() == _iterator.value();
        }

        bool operator!=(const stashing_iterator& other) const {
            return other._iterator.value() != _iterator.value();
        }

        auto& operator*() {
            _set_value(*_iterator.value());
            return *_value;
        }

        auto& operator*() const {
            _set_value(*_iterator.value());
            return *_value;
        }

        auto& operator++() {
            ++_iterator.value();
            return *this;
        }

        auto operator++(int) {
            return stashing_iterator(_iterator.value()++);
        }

        auto& operator--() requires std::bidirectional_iterator<It> {
            --_iterator.value();
            return *this;
        }

        auto operator--(int) requires std::bidirectional_iterator<It> {
            return stashing_iterator(_iterator.value()--);
        }

        auto operator+(const difference_type& n) const requires std::random_access_iterator<It> {
            return stashing_iterator(_iterator.value() + n);
        }

        auto& operator+=(const difference_type& n) requires std::random_access_iterator<It> {
            _iterator.value() += n;
            return *this;
        }

        auto operator-(const difference_type& n) const requires std::random_access_iterator<It> {
            return stashing_iterator(_iterator.value() - n);
        }

        auto operator-(const stashing_iterator& n) const requires std::random_access_iterator<It>  {
            return _iterator.value() - n._iterator.value();
        }

        auto& operator-=(const ptrdiff_t& n) requires std::random_access_iterator<It> {
            _iterator.value() -= n;
            return *this;
        }

        auto& operator[](const size_t& n) requires std::random_access_iterator<It>  {
            _set_value(_iterator.value()[n]);
            return *_value;
        }

        auto& operator[](const size_t& n) const requires std::random_access_iterator<It>  {
            return _set_value(_iterator.value()[n]);
            return *_value;
        }

        bool operator<(const stashing_iterator& other) const requires std::random_access_iterator<It>  {
            return _iterator.value() < other._iterator.value();
        }

        bool operator<=(const stashing_iterator& other) const requires std::random_access_iterator<It>  {
            return _iterator.value() <= other._iterator.value();
        }

        bool operator>(const stashing_iterator& other) const requires std::random_access_iterator<It>  {
            return _iterator.value() > other._iterator.value();
        }

        bool operator>=(const stashing_iterator& other) const requires std::random_access_iterator<It>  {
            return _iterator.value() >= other._iterator.value();
        }

        It& base() {
            return _iterator.value();
        }

        const It& base() const {
            return _iterator.value();
        }

    private:

        mutable std::optional<It> _iterator{};
        mutable value_type* _value = nullptr;

        void _set_value(value_type&& value) const {
            if (_value != nullptr) {
                _value->~value_type();

                new (_value) value_type(std::move(value));
                return;
            }

            _value = new value_type(std::move(value));
        }

        friend stashing_iterator operator+(const difference_type& n, const stashing_iterator<It>& iterator) requires std::random_access_iterator<It> {
            return n + iterator._iterator.value();
        }

    };

}// namespace feniks