#ifndef EXAMPLE_ANIMAL_INTERFACE_H
#define EXAMPLE_ANIMAL_INTERFACE_H

#include <string>

namespace example {
    /*!
     * @ingroup animals
     * @interface AnimalInterface
     */
    class AnimalInterface {
    public:
        /*!
         * @brief Returns the number of limbs
         * @see get_num_of_eyes
         */
        virtual int get_num_of_limbs() const = 0;

        /*!
         * @brief Returns the number of eyes
         * @see get_num_of_limbs
         */
        virtual int get_num_of_eyes() const = 0;

        /*!
         * @brief Returns true if the animal has a tail
         * @see get_num_of_limbs
         * @retval true Does have a tail
         * @retval false Does not have a tail
         */
        virtual bool has_tail() const = 0;
    };
}

#endif