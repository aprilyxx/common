#pragma once

#include <string>
#include <vector>

/*
 * split string s by character c
 *
 *   SplitString("abc,,hello world", ',', v) ==> "abc", "hello world"
 */
void SplitString(const std::string& s, char c, std::vector<std::string>& v);

/*
 * remove the heading and trailing whitespace
 *
 *   TrimString(" hello world  ") ==> "hello world"
 */
void TrimString(std::string& s);
