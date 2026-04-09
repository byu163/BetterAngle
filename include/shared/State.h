#include <iostream>
#include <string>

class Person {
public:
  std::string name;
  int age;

  // Constructor with default values for parameters
  Person(std::string n = "Unknown", int a = 0) : name(n), age(a) {}

  void displayInfo() const {
    std::cout << "Name: " << name << ", Age: " << age << std::endl;
  }
};

int main() {
  // Create an instance of Person with default values
  Person person1;

  // Set the name and age using member variables
  person1.name = "John";
  person1.age = 25;

  // Display information about the person
  person1.displayInfo();

  return 0;
}
