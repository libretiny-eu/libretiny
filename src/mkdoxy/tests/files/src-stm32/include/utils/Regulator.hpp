#pragma once

#include <stdint.h>

struct Regulator {
    typedef uint32_t coef_type;
    typedef int32_t value_type;

    constexpr Regulator& operator=(Regulator&& other) = default;

    Regulator() = default;
    Regulator(value_type max_output, coef_type p, coef_type i, coef_type d)
        : m_stop(false)
        , m_p(p)
        , m_i(i)
        , m_d(d)
        , m_e(0)
        , m_le(0)
        , m_de(0)
        , m_output(0)
        , m_integrator(0)
        , m_max_output(max_output) {}

    void setP(const coef_type& v) { m_p = v; }
    coef_type P() const { return m_p; }
    void setI(const coef_type& v) { m_i = v; }
    coef_type I() const { return m_i; }
    void setD(const coef_type& v) { m_d = v; }
    coef_type D() const { return m_d; }
    void setMaxOutput(value_type max_output) { m_max_output = max_output; }
    value_type maxOutput() { return m_max_output; }

    void stop(bool s = true) {
        m_stop = s;
        m_e = 0;
        m_le = 0;
        m_de = 0;
        m_integrator = 0;
    }

    value_type process(value_type target, value_type actual) {
        if (m_stop)
            return 0;
        m_e = target - actual;

        value_type integrator = m_integrator;

        if (m_i != 0)
            integrator += m_e;

        m_de = m_e - m_le;

        value_type x = ((value_type(m_p) * m_e) >> 8)
            + ((value_type(m_d) * m_de) >> 8)
            + ((value_type(m_i) * integrator) >> 8);

        if (x > m_max_output) {
            m_output = m_max_output;
            if (integrator < 0)
                m_integrator = integrator;
        } else if (x < -m_max_output) {
            m_output = -m_max_output;
            if (integrator > 0)
                m_integrator = integrator;
        } else {
            m_output = x;
            m_integrator = integrator;
        }
        m_le = m_e;
        return m_output;
    }

    void clear() {
        m_integrator = 0;
        m_le = m_e;
    }

    value_type e() const { return m_e; }
    value_type de() const { return m_de; }
    value_type output() const { return m_output; }
    value_type integrator() const { return m_integrator; }

private:
    bool m_stop;
    coef_type m_p;
    coef_type m_i;
    coef_type m_d;
    value_type m_e;
    value_type m_le;
    value_type m_de;
    value_type m_output;
    value_type m_integrator;
    value_type m_max_output; //crop output, absolute value
};
