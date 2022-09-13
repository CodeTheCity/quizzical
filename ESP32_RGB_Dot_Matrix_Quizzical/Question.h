#include <stdint.h>

#ifndef _QUESTION_H    // Put these two lines at the top of your file.
#define _QUESTION_H    // (Use a suitable name, usually based on the file name.)

class Question {
  private:
    char question[64];
    char answer[64];
    char incorrect[2][64];
    uint8_t options[3];
    uint8_t correct;

  public:
    Question(char* question, char* answer, char* incorrect1, char* incorrect2);
    char* getQuestion();
    char* getAnswer();
    char* getIncorrect1();
    char* getIncorrect2();
    char* getOption(uint8_t option);
    void randomise();
    bool isCorrect(uint8_t choice);
};

#endif // _QUESTION_H    // Put this line at the end of your file.
