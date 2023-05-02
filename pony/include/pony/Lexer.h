//===- Lexer.h - Lexer for the Pony language -------------------------------===//

#ifndef PONY_LEXER_H
#define PONY_LEXER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>

namespace pony {

/// Structure definition a location in a file.
struct Location {
  std::shared_ptr<std::string> file; ///< filename.
  int line;                          ///< line number.
  int col;                           ///< column number.
};

// List of Token returned by the lexer.
enum Token : int {
  tok_semicolon = ';',
  tok_parenthese_open = '(',
  tok_parenthese_close = ')',
  tok_bracket_open = '{',
  tok_bracket_close = '}',
  tok_sbracket_open = '[',
  tok_sbracket_close = ']',

  tok_eof = -1,

  tok_return = -2,
  tok_var = -3,
  tok_def = -4,

  tok_identifier = -5,
  tok_number = -6,
};

enum Error{
  // digit at wrong location
  identifier_err_1 = 0,
  // invalid char
  identifier_err_2,
  num_err,
  no_err
};

/// The Lexer is an abstract base class providing all the facilities that the
/// Parser expects. It goes through the stream one token at a time and keeps
/// track of the location in the file for debugging purpose.
/// It relies on a subclass to provide a `readNextLine()` method. The subclass
/// can proceed by reading the next line from the standard input or from a
/// memory mapped file.
class Lexer {
public:
  /// Create a lexer for the given filename. The filename is kept only for
  /// debugging purpose (attaching a location to a Token).
  Lexer(std::string filename)
      : lastLocation(
            {std::make_shared<std::string>(std::move(filename)), 0, 0}) {}
  virtual ~Lexer() = default;

  /// Look at the current token in the stream.
  Token getCurToken() { return curTok; }

  /// Move to the next token in the stream and return it.
  Token getNextToken() { return curTok = getTok(); }

  /// Move to the next token in the stream, asserting on the current token
  /// matching the expectation.
  void consume(Token tok) {
    // printf("to consume %d\t%c\tcurrent %d\t%d\n", tok, static_cast<char>(tok), curTok, curTok);
    assert(tok == curTok && "consume Token mismatch expectation");
    getNextToken();
  }

  /// Return the current identifier (prereq: getCurToken() == tok_identifier)
  llvm::StringRef getId() {
    assert(curTok == tok_identifier);
    return identifierStr;
  }

  double getValue() {

    assert(curTok == tok_number);
    return numVal;
  }

  /// Return the location for the beginning of the current token.
  Location getLastLocation() { return lastLocation; }

  // Return the current line in the file.
  int getLine() { return curLineNum; }

  // Return the current column in the file.
  int getCol() { return curCol; }

  // if there is no error, just output all tokens , otherwise will output the errors
  void traverse(){
    fflush(stdout);
    if(error.size() != 0){
      for(auto& t : error){
        if(t.err == num_err){
          printf("Token error[Number error] (line %d, col %d):invalid digit input \"%s\"\n", t.line, t.col, t.content.c_str());
        }
        else if(t.err == identifier_err_2){
          printf("Token error[Identifier error] (line %d, col %d):identifier should contain only digits , alpha and _ \"%s\"\n", t.line, t.col, t.content.c_str());
        }
        else{
          printf("Token error[Identifier error] (line %d, col %d):An identifier cannot begin with digits \"%s\"\n", t.line, t.col, t.content.c_str());
        }
      }
    }
    else{
      for(auto& t : success){
        printf("%s ", t.content.c_str());
      }
      printf("\n");
    }
  }

private:
  /// Delegate to a derived class fetching the next line. Returns an empty
  /// string to signal end of file (EOF). Lines are expected to always finish
  /// with "\n"
  virtual llvm::StringRef readNextLine() = 0;


  // TODO: Implement function getNextChar().
  
  // Function description: 该函数从curLineBuffer中获取当前行的下一个char，如果已经处理到当前行最后一个char，则通过读取下一行
  //                       来更新curLineBuffer以确保curLineBuffer非空。

  // Hints: 1. 函数实现过程中可能会用到lexer的部分成员变量（如curLineBuffer）；
  //        2. 注意读到文档结尾，读到某一行结尾等特殊情况的处理。一般来说，读到文档结尾应返回EOF，某一行结尾最后一个char为'\n'；
  //        3. 注意行列位置信息的同步更新；
  //        4. 关于llvm::StringRef的部分函数：llvm::StringRef example; example.front(); example.drop_front(); example.empty()。
  int getNextChar() {
    /* 
     *
     *  Write your code here.
     *
     */
    // if the curLineBuffer is empty , no data read in, meet EOF
    if(curLineBuffer.empty()){
      return EOF;
    }

    char ch = curLineBuffer.front();
    curCol++;

    // deal with the '\n' case
    if(ch == '\n'){
      curLineNum++;
      curLineBuffer = readNextLine();
      curCol = 0;
    }
    else 
      curLineBuffer = curLineBuffer.drop_front(1);
    if(ch != EOF)
    fflush(stdout);
    return ch;
  }

  ///  Return the next token from standard input.
  Token getTok() {
    // Skip any whitespace.
    while (isspace(lastChar))
      lastChar = Token(getNextChar());

    // Save the current location before reading the token characters.
    lastLocation.line = curLineNum;
    lastLocation.col = curCol;

    // TODO: 补充成员函数getTok()。
    //       1. 能够识别“return”、“def”和“var”三个关键字；
    //       2. 能够识别标识符（函数名，变量名等）：
    //          • 标识符以字母或下划线开头；
    //          • 标识符由字母、数字和下划线组成；
    //          • 按照使用习惯，要求标识符中有数字时，数字须位于标识符末尾；
    //          例如：有效的标识符可以是 a123, b_4, placeholder 等。
    //       3. 在识别每种Token的同时，将其存放在某种数据结构中，以便最终在终端输出
    //
    // Hints: 1. 在实现第1，2点时，可参考getTok()函数中现有的识别数字的方法。
    //        2. 一些有用的函数:  isalpha(); isalnum();
    /* 
     *
     *  Write your code here.
     *
     */
    
    char beg = lastChar;
    Error err = no_err;
    token t{};
    Token thisChar = Token(lastChar);
    std::string str{};

    // return case
    if(isalpha(lastChar) && lastChar == 'r'){
      int i = 0;
      std::string ret{"return"};
      do{
        i++;
        lastChar = Token(getNextChar());
      } while(i < 6 && isalpha(lastChar) && lastChar == ret[i]);

      // the case is that an identifier begin with return, so we have to keep scanning
      // goto identifier 
      if(lastChar != ' ' && lastChar != ';'){
        str += "return";
        goto identifier;
      }
      t.col = lastLocation.col;
      t.line = lastLocation.line;
      t.content = std::string("return");
      success.push_back(t);
      return tok_return;
    }

    // deal with the var case
    if(isalpha(lastChar) && lastChar == 'v'){
      int i = 0;
      std::string var{"var"};
      do{
        i++;
        // printf("%c\n", lastChar);
        str += lastChar;
        lastChar = Token(getNextChar());
      } while(i < 3 && isalpha(lastChar) && lastChar == var[i]);

      // var
      // like return , an identifier begin with var
      // if(i != 3){
      //   assert(i < 3);
        
      // }
      if((i == 3 && lastChar != ' ') || i != 3){
        // str += "var";
        goto identifier;
      }
      t.col = lastLocation.col;
      t.line = lastLocation.line;
      t.content = std::string("var");
      success.push_back(t);
      return tok_var;
    }

    // deal with def case
    if(isalpha(lastChar) && lastChar == 'd'){
      int i = 0;
      std::string def{"def"};
      do{
        i++;
        lastChar = Token(getNextChar());
      } while(i < 3 && isalpha(lastChar) && lastChar == def[i]);
      if((i == 3 && lastChar!= ' ') || i != 3){
        // printf("get here\n");
        // str += "def";
        goto identifier;
      }
      t.col = lastLocation.col;
      t.line = lastLocation.line;
      t.content = std::string("def");
      success.push_back(t);
      return tok_def;
    }

    // only when beg is _ or alpha , the loop will begin
    // otherwise it will conflict with the number detection
    identifier:
    if(isalpha(beg) || beg == '_'){
      // to record whether digit has appeared in the id
      bool flag = true;
      printf("before\n");
      while(isalpha(lastChar) || isdigit(lastChar) || lastChar == '_'){
        if(isdigit(lastChar)){
          flag = false;
        }
        // if a digit mix in the middle of a id
        if(!flag && isalpha(lastChar)){
          err = identifier_err_2;
        }
        str += lastChar;
        printf("%s\n", str.c_str());
        lastChar = Token(getNextChar());
      }
      printf("after\n");
      t.col = lastLocation.col;
      t.line = lastLocation.line;
      t.content = str;
      identifierStr = str;
      // if some error occurs , goto err to deal with it
      if(err == no_err)
        success.push_back(t);
      else
        goto err;
      return tok_identifier;
    }
    
    //TODO: 3. 改进识别数字的方法，使编译器可以识别并在终端报告非法数字，非法表示包括：9.9.9，9..9，.123等。
    if (isdigit(lastChar) || lastChar == '.') {
      std::string numStr;
      char* endptr = nullptr;
      do {
        numStr += lastChar;
        lastChar = Token(getNextChar());
      } while (isdigit(lastChar) || lastChar == '.');

      // if a num end with a char
      if(isalpha(lastChar) || lastChar == '_'){
        // digit before id
        // printf("\nerror\n");
        err = identifier_err_1;
        while((lastChar = Token(getNextChar())) != ' '){
          numStr += lastChar;
        }
        goto err;
      }

      numVal = strtod(numStr.c_str(), &endptr);

      // if the strtod fail to reach the end, some error occurs
      if(strcmp(endptr, "") != 0 || (numStr.length() > 0 && numStr[0] == '.')){
        // printf("\nerror2\n");
        t.err = num_err;
        t.content = numStr;
        goto err;
      }

      t.col = lastLocation.col;
      t.line = lastLocation.line;
      t.content = numStr;
      success.push_back(t);
      return tok_number;
    }

    if (lastChar == '#') {
      // Comment until end of line.
      do {
        lastChar = Token(getNextChar());
        //printf("%d ", lastChar);
      } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

      if (lastChar != EOF)
        return getTok();
    }

    // Check for end of file.  Don't eat the EOF.
    if (lastChar == EOF){
      return tok_eof;
    }

    // Otherwise, just return the character as its ascii value.
    thisChar = Token(lastChar);
    lastChar = Token(getNextChar());

    // if(isalpha(lastChar))

    t.col = lastLocation.col;
    t.line = lastLocation.line;
    t.content += thisChar;
    success.push_back(t);

    return thisChar;

    // deal with error
    err:
    t.line = lastLocation.line;
    t.col = lastLocation.col;
    // t.err = err;
    error.push_back(t);
  }

  /// The last token read from the input.
  Token curTok = tok_eof;

  /// Location for `curTok`.
  Location lastLocation;

  /// If the current Token is an identifier, this string contains the value.
  std::string identifierStr;

  /// If the current Token is a number, this contains the value.
  double numVal = 0;

  /// The last value returned by getNextChar(). We need to keep it around as we
  /// always need to read ahead one character to decide when to end a token and
  /// we can't put it back in the stream after reading from it.
  Token lastChar = Token(' ');

  /// Keep track of the current line number in the input stream
  int curLineNum = 0;

  /// Keep track of the current column number in the input stream
  int curCol = 0;

  /// Buffer supplied by the derived class on calls to `readNextLine()`
  llvm::StringRef curLineBuffer = "\n";

  struct token{
    unsigned col = 0, line = 0;
    Error err = no_err;
    std::string content;
  };

  std::vector<token> success;
  std::vector<token> error;

};

/// A lexer implementation operating on a buffer in memory.
class LexerBuffer final : public Lexer {
public:
  LexerBuffer(const char *begin, const char *end, std::string filename)
      : Lexer(std::move(filename)), current(begin), end(end) {}

private:
  /// Provide one line at a time to the Lexer, return an empty string when
  /// reaching the end of the buffer.
  llvm::StringRef readNextLine() override {
    auto *begin = current;
    while (current <= end && *current && *current != '\n')
      ++current;
    if (current <= end && *current)
      ++current;
    llvm::StringRef result{begin, static_cast<size_t>(current - begin)};
    return result;
  }
  const char *current, *end;
};
} // namespace pony

#endif // PONY_LEXER_H
