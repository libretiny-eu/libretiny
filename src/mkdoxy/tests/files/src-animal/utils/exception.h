#ifndef EXAMPLE_EXCEPTION_H
#define EXAMPLE_EXCEPTION_H

#include <exception>
#include <string>

/*!
 * @dir src/utils
 * @brief This is a directory
 * @brief This is a detailed description
 * @ingroup animals
 * @addtogroup animals
 */

namespace example {
    /*!
     * @ingroup animals
     */
    class CustomException : public std::exception {
    public:
        CustomException(const std::string& msg):std::exception(),msg(msg){

        }

        virtual const char* what() const throw() {
            return msg.c_str();
        }

    private:
        std::string msg;
    };

    /*!
     * @ingroup animals
     */
    class NumericException : public std::exception {
    public:
        NumericException(const std::string& msg):std::exception(),msg(msg){

        }

        virtual const char* what() const throw() {
            return msg.c_str();
        }

    private:
        std::string msg;
    };
}

#endif
