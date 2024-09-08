/** This is a translation of the Python Software Foundation's
 * shlex: 'A lexical analyzer class for simple shell-like syntaxes.'
 * Available here: https://github.com/python/cpython/blob/3.12/Lib/shlex.py
 *
 * Original authors:
 * Eric S. Raymond,
 * Gustavo Niemeyer,
 * Vinay Sajip.
 *
 * Translator:
 * Leonardo Lovera. */

#ifndef LEXER
#define LEXER

#include <deque>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
class lexer
{
private:
    std::stringstream instream;
    bool posix;
    std::string commenters;
    std::string wordchars;
    std::string whitespace;
    std::string quotes;
    std::string escape;
    std::string escapedquotes;
    std::string state;
    std::deque<char> pushback;
    std::string token;

    std::string get_token(void)
    {
        if (!pushback.empty()) {
            std::string tok = "";
            tok += pushback.front();
            pushback.pop_front();
            return tok;
        }

        bool quoted = false;
        std::string escapedstate = " ";
        char nextchar;
        bool nextchar_eof;
        std::string comment;

        while (true) {
            nextchar_eof = instream.get(nextchar) ? false : true;

            if (state == "") {
                token = "";
                break;
            } else if (state == " ") {
                if (nextchar_eof) {
                    state = "";
                    break;
                } else if (whitespace.find(nextchar) != std::string::npos) {
                    if (token != "" || (posix && quoted)) {
                        break;
                    } else {
                        continue;
                    }
                } else if (commenters.find(nextchar) != std::string::npos) {
                    std::getline(instream, comment);
                } else if (posix && escape.find(nextchar) != std::string::npos) {
                    escapedstate = "a";
                    state = nextchar;
                } else if (wordchars.find(nextchar) != std::string::npos) {
                    token = nextchar;
                    state = "a";
                } else if (quotes.find(nextchar) != std::string::npos) {
                    if (!posix)
                        token = nextchar;
                    state = nextchar;
                } else {
                    token = nextchar;
                    state = "a";
                }
            } else if (quotes.find(state) != std::string::npos) {
                quoted = true;
                if (nextchar_eof) {
                    throw std::logic_error("no closing quotation");
                }

                if (state.length() == 1 && nextchar == state.front()) {
                    if (!posix) {
                        token += nextchar;
                        state = " ";
                        break;
                    } else {
                        state = "a";
                    }
                } else if (posix && escape.find(nextchar) != std::string::npos &&
                           escapedquotes.find(state) != std::string::npos) {
                    escapedstate = state;
                    state = nextchar;
                } else {
                    token += nextchar;
                }
            } else if (escape.find(state) != std::string::npos) {
                if (nextchar_eof) {
                    throw std::logic_error("no escaped character");
                }

                if (quotes.find(escapedstate) != std::string::npos &&
                    (state.length() != 1 || nextchar != state.front()) &&
                    (escapedstate.length() != 1 || nextchar != escapedstate.front())) {
                    token += state;
                }

                token += nextchar;
                state = escapedstate;
            } else if (state == "a") {
                if (nextchar_eof) {
                    state = "";
                    break;
                } else if (whitespace.find(nextchar) != std::string::npos) {
                    state = " ";
                    if (token != "" || (posix && quoted)) {
                        break;
                    } else {
                        continue;
                    }
                } else if (commenters.find(nextchar) != std::string::npos) {
                    std::getline(instream, comment);
                    if (posix) {
                        state = " ";
                        if (token != "" || (posix && quoted)) {
                            break;
                        } else {
                            continue;
                        }
                    }
                } else if (posix && quotes.find(nextchar) != std::string::npos) {
                    state = nextchar;
                } else if (posix && escape.find(nextchar) != std::string::npos) {
                    escapedstate = "a";
                    state = nextchar;
                } else {
                    token += nextchar;
                }
            }
        }

        std::string result = token;
        token = "";
        return result;
    }

    void set_commenters(std::string cmts)
    {
        commenters = cmts;
    }

public:
    lexer(std::string in, bool px = true): posix(px)
    {
        instream.str(in);
        commenters = "#";
        wordchars = "abcdfeghijklmnopqrstuvwxyz";
        wordchars += "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

        if (px) {
            wordchars += "ßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ";
            wordchars += "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞ";
        }

        whitespace = " \t\r\n";
        quotes = "\'\"";
        escape = "\\";
        escapedquotes = "\"";
        state = " ";
        token = "";
    }

    std::vector<std::string> tokenize(bool comments = false)
    {
        std::vector<std::string> tokens;
        std::string tok;

        if (!comments) {
            set_commenters("");
        }

        while (true) {
            tok = get_token();
            if (tok == "")
                break;
            tokens.push_back(tok);
        }

        if (!comments) {
            set_commenters("#");
        }

        return tokens;
    }
};

#endif
