#include <iostream>
#include <string>

// Function to calculate the sum of two integers
int add(int a, int b) { return a + b; }

// Function to calculate the difference between two integers
int subtract(int a, int b) { return a - b; }

// Function to calculate the product of two integers
int multiply(int a, int b) { return a * b; }

// Function to calculate the quotient of two integers
float divide(float a, float b) {
  if (b == 0) {
    std::cout << "Error: Division by zero" << std::endl;
    return 0.0f;
  }
  return a / b;
}

int main() {
  int num1 = 5;
  int num2 = 3;

  // Calculate and display the sum
  int sum = add(num1, num2);
  std::cout << "Sum: " << sum << std::endl;

  // Calculate and display the difference
  int diff<| fim_suffix |> << std::endl;

  return 0;
}
