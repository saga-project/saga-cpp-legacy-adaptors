/*
 * Copyright (C) 2008-2009 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2009 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef DEBUG_HPP
#define DEBUG_HPP

#if 0
#include <iostream>

#define DESTRUCTOR(klass)                  \
      ~ klass () { std::cout << "    % " # klass " destructor" << std::endl; }

#define DESTRUCTOR2(klass, value)                  \
      ~ klass () { std::cout << "    % " # klass "(" << value << ") destructor" << std::endl; }

#define VDESTRUCTOR(klass)                  \
      virtual ~ klass () { std::cout << "    % " # klass " destructor" << std::endl; }
#else
#define DESTRUCTOR(klass)  ~ klass () {}
#define DESTRUCTOR2(klass, name)  ~ klass () {}
#define VDESTRUCTOR(klass) virtual ~ klass () {}
#endif

#endif  // DEBUG_HPP
