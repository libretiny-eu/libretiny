#ifndef EXAMPLE_SPECIAL_BIRD_H
#define EXAMPLE_SPECIAL_BIRD_H

#include "bird.h"

namespace example {
    /*!
     * @ingroup animals
     */
    class SpecialBird: public Bird {
    public:
        SpecialBird(const std::string& name, SpecialBird* mother = nullptr, SpecialBird* father = nullptr);
        SpecialBird(const SpecialBird& other) = delete;
        SpecialBird(SpecialBird&& SpecialBird) noexcept;
        ~SpecialBird() = default;

        void swap(SpecialBird& other) noexcept;
        void do_something_special();

        SpecialBird& operator = (const SpecialBird& other) = delete;
        SpecialBird& operator = (SpecialBird&& other) noexcept;
    };
}

#endif