#pragma once

#include "student.hpp"
#include "serialization/serialization_utils.hpp"

#include <cmath>
#include <iomanip>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <utility>

// these operators work with string and file streams as well as console streams
inline std::ostream& operator<<(std::ostream& output, const Student& student) {
    if (student.id <= 0 || !std::isfinite(student.average_score)) {
        output.setstate(std::ios::failbit);
        return output;
    }

    // build the record separately to preserve the caller's precision and locale
    std::ostringstream record;
    record.imbue(output.getloc());
    record << student.id << ' ' << std::quoted(student.first_name) << ' '
           << std::quoted(student.last_name) << ' ' << student.birth_year << ' '
           << std::setprecision(std::numeric_limits<double>::max_digits10)
           << student.average_score;
    if (!record) {
        output.setstate(std::ios::failbit);
        return output;
    }
    return output << record.str();
}

inline std::istream& operator>>(std::istream& input, Student& student) {
    // parse into a temporary so a malformed record leaves the destination intact
    Student parsed;
    try {
        // each field reader consumes one complete value and checks its boundary
        // the names require quotes, so spaces and tree markers remain part of the name
        parsed.id = serialization::read_value<int>(input);
        parsed.first_name = serialization::read_value<std::string>(input);
        parsed.last_name = serialization::read_value<std::string>(input);
        parsed.birth_year = serialization::read_value<int>(input);
        parsed.average_score = serialization::read_value<double>(input);
    } catch (const std::invalid_argument&) {
        // formatted input reports malformed data through the stream state
        // if failbit exceptions are enabled, setstate propagates ios_base::failure
        input.setstate(std::ios::failbit);
        return input;
    }

    if (parsed.id <= 0) {
        input.setstate(std::ios::failbit);
        return input;
    }
    student = std::move(parsed);
    return input;
}
