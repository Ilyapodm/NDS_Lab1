#pragma once

#include <string>
#include <utility>

class Student {
public:
    int id = 0;
    std::string first_name;
    std::string last_name;
    int birth_year = 2000;
    double average_score = 0.0;

    Student() = default;

    Student(int id, std::string first_name, std::string last_name,
            int birth_year, double average_score)
        : id(id), first_name(std::move(first_name)), last_name(std::move(last_name)),
          birth_year(birth_year), average_score(average_score) {}

    bool operator<(const Student& other) const noexcept {
        return id < other.id;
    }

    bool operator==(const Student& other) const {
        return id == other.id && first_name == other.first_name
            && last_name == other.last_name && birth_year == other.birth_year
            && average_score == other.average_score;
    }
};
