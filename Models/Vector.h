#pragma once
#include <vector>

//Vector struct for basic x,y position logic
struct Vector2 {
  float x;
  float y;

  //Constructors / Default
  Vector2() : x(0.0f), y(0.0f) {};
  Vector2(float x, float y) : x(x), y(y) {};

  //!= Operator
  bool operator!=(const Vector2& other) const {
      return x != other.x || y != other.y;
  }

  //Checks if vector is in given list
  bool IsInList(const std::vector<Vector2>& other) const {
      for (const Vector2& vec2 : other) {
          if (vec2.x == x && vec2.y == y)
              return true;
      }
      return false;
  }
};

//Renames
using Vec2 = Vector2;
using vec2 = Vector2;