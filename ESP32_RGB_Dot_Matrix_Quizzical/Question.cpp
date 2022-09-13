#include "Question.h"
#include <Arduino.h>

Question::Question(char* question, char* answer, char* incorrect1, char* incorrect2) {
  strcpy(this->question, question);
  strcpy(this->answer, answer);
  strcpy(this->incorrect[0], incorrect1);
  strcpy(this->incorrect[1], incorrect2);

  this->correct = 1;

  this->options[0] = 1;
  this->options[1] = 2;
  this->options[2] = 3;
}
char* Question::getQuestion() {
  return this->question;
}

char* Question::getAnswer() {
  return this->answer;
}

char* Question::getIncorrect1() {
  return this->incorrect[0];
}

char* Question::getIncorrect2() {
  return this->incorrect[1];
}

char* Question::getOption(uint8_t option) {
  switch (this->options[option]) {
    case 1:
      return this->getAnswer();
      break;

    case 2:
      return this->getIncorrect1();
      break;

    case 3:
      return this->getIncorrect2();
      break;

    default:
      return "none";
  }
}

void Question::randomise() {
  bool used[4];
  for (uint8_t i = 0; i < 4; i++) {
    used[i] = false;
  }


  for (uint8_t i = 0; i < 3; i++) {
    uint8_t option = random(1, 4);
    bool option_set = false;
    while (option_set == false) {
      if (used[option] == false) {
        used[option] = true;
        option_set = true;
      } else {
        option = random(1, 4);
      }
    }

    if(option == 1) {
      this->correct = i + 1;
    }

    this->options[i] = option;
  }
}

bool Question::isCorrect(uint8_t choice) {
  return this->correct == choice;
}
