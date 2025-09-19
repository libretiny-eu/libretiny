#ifndef EXAMPLE_BIRD_H
#define EXAMPLE_BIRD_H

#include "animal.h"

namespace example {
    /*!
     * @ingroup animals
     */
    class Bird: public Animal {
    public:
        Bird(const std::string& name, Bird* mother = nullptr, Bird* father = nullptr);
        Bird(const Bird& other) = delete;
        Bird(Bird&& Bird) noexcept;
        ~Bird() = default;

        void swap(Bird& other) noexcept;

        void move() override;
        void make_sound() override;
        /*!
         * @brief Deleted copy operator
         */
        Bird& operator = (const Bird& other) = delete;
        /*!
         * @brief Move operator
         */
        Bird& operator = (Bird&& other) noexcept;
    };
}

#endif