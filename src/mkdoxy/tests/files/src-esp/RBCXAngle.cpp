#include "RBCXAngle.h"

namespace rb {

const Angle Angle::Pi = Angle::rad(Angle::_T(M_PI));

Angle operator+(Angle a, Angle b) {
    a += b;
    return a;
}

Angle operator-(Angle a, Angle b) {
    a -= b;
    return a;
}

Angle operator*(Angle a, Angle::_T c) {
    a *= c;
    return a;
}

Angle operator/(Angle a, Angle::_T c) {
    a /= c;
    return a;
}

Angle operator"" _deg(long double d) { return Angle::deg(Angle::_T(d)); }

Angle operator"" _rad(long double r) { return Angle::rad(Angle::_T(r)); }

Angle operator"" _deg(unsigned long long int d) { return Angle::deg(d); }

Angle operator"" _rad(unsigned long long int r) { return Angle::rad(r); }

}; // namespace rb
