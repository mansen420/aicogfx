#ifndef FOO_H
#define FOO_H

//1.[hygeine] define your own namespace.
//2.[convention] consider using #pragma once, it is almost universally
//supported. If a compiler can compile this code, it will support #pragma
//ChatGPT: Use #pragma once unless you're writing code for nuclear reactors from the Cold War era or releasing a header-only library to be used on every compiler since the dawn of time.

//3. [for the future] consider using the /src /include and /test directories.
//user-facing includes go in /include, internal includes and implementation
//go in /src, single file main function test goes in /test. Our CMakeLists scheme
//(in the other branches) already supports this.

//4. [API sanity] why does the user have to manually call getDouble(), getChar()?
//in fact, your API does not need to provide such services to the user. the user 
//should be responsible for providing his data, you are simply inserting the API
//in places where it doesn't belong. The user can call std::cin, or read a file,
//or generate random numbers, or whatever. not your business.
double getDouble();
char getChar();
void runEq(double one, double two, char letter);

#endif
