## WebAssembly and C++: How to write new applications and port existing C++ code bases

### Introduction

C++ is a systems programming language that is used for writing Operating Systems, Kernels, Games, desktop applications and any other place where performance is paramount. A vast majority of Software developed since the 1980s was written in C++; this includes the audio encoding and transformation platform `ffmpeg`, compression library `miniz`, the `Windows NT kernel`, and most of the video games in the pre-unity era.

Whenever we need to perform some compute intensive task that involves low level hardware control and bit manipulation, JavaScript rules itself out as a suitable platform because of the constraints of the underlying runtime. In those scenarios we can leverage the power of C++ using Webassembly.

### Pre-Requisites

To be able to write C++ applications, a developer must
1. be familiar with C++ Syntax, data and control structures
2. Understand application memory/organization i.e. Heap and Stacks
3. familiar with the concept of pointers
4. have experience using non-GC languages

Because C++ is a non garbage collected language where memory management is left to the developer, it is very easy to shoot yourself in the foot. That's why when writing C++ code, it's recommended that you follow the `RAII Pattern`. RAII stands for Resource Acquisition is Initialization.
As C++ is a low-level language, its upto the developer where they choose to store their variables. There aren't hidden gotchas like you'd find in JavaScript like: 
```
all primitive types are allocated on the stack and all object types on the heap
```

In C++ array is an actual datatype that can be allocated on the stack and the same is true for struct and class instances. That's why allocation on the heap is explicit and must be done using the `new` keyword and pointers, otherwise everyting goes on the stack.

As the responsibility of memory management lies with the developer, RAII ensures that all heap memory is cleaned when the owner of such memory goes out of scope. This helps prevent memory leaks and the dreaded `segmentation fault` errors.

Let's look at an example.

```cpp
class Car {
    private:
        char *m_name;
    public:
        //Since the object is created on the stack, the constructor
        //allocates memory on the heap when the object comes into scope
        Car(const char *name) {
            size_t len = strlen(name);
            m_name = new char[len];
            strcpy(m_name,name);
        } 

        //The destructor ensures that when the object is popped from the
        //application stack, heap memort is released
        ~Car() {
            delete[] m_name;
        }

        const char * getCarName() const {
            return m_name;
        }
}

```

### Setting up Emscripten

`Emscripten` is a clang based compiler written on top of clang that compiles C/C++ code into WebAssembly module or `wasm`. Wasm can be directly integrated and interoperate with JS in the browser. To install wasm, follow the instructions on their [getting started page](https://emscripten.org/docs/getting_started/index.html).

Emscripten installs `emcc` and `em++`, which are the C and C++ compilers respectively.

### Writing our C++ Wasm Application

All C++ programs consist of application files (`*.cpp`) and header files (`*.hpp`). In our sample application we will write a program that manipulates a linked list written in C++. Here's how our file structure looks like.

```bash
├── LinkedList.hpp
├── main.cpp
```

The contents of these files are as follows

```cpp
//LinkedList.hpp
#pragma once
#include <string>
#include <sstream>

namespace collection {
    template <typename T>
    struct Node {
        T data;
        Node *next;

        Node(T data) {
            this->data = data;
        }
    };

    template <typename T>
    class LinkedList{
    private:
        Node<T>* m_head;
        int m_length;

    public:   
        LinkedList() {
            m_head = nullptr;
            m_length = 0;
        }

        ~LinkedList() {
            Node<T>* ptr = m_head;
            while(ptr != nullptr) {
                Node<T>* temp = ptr;
                ptr =  ptr->next;
                delete temp;
            }

        }

        void insert(T data) {
            Node<T>* temp = new Node<T>(data);
            Node<T>* ptr = m_head;

            if(ptr == nullptr) {
                m_head = temp;
            } else {
                while(ptr->next != nullptr)
                    ptr = ptr->next;
                ptr->next = temp;
            }
            m_length++;
        }

        void insertR(T data) {
            Node<T>* temp = new Node<T>(data);
            temp->next = m_head;
            m_head = temp;
            m_length++;
        }

        void remove() {
            Node<T>* ptr = m_head;
            if(ptr == nullptr)
                return;
            if(ptr->next == nullptr) {
                m_head = nullptr;
                delete ptr;
                return;
            }
            while(ptr->next->next != nullptr) 
                ptr = ptr->next;
            Node<T>* temp = ptr->next;
            ptr->next = nullptr;
            delete temp;
            m_length--;
        }

        void removeR() {
            if(m_head == nullptr)
                return;
            Node<T>* temp = m_head;
            m_head = m_head->next;

            delete temp;
            m_length--;
        }

        int length() const {
            return m_length;
        }

        std::string display() {
            std::stringstream ss;

            Node<T>* ptr = m_head;


            if(ptr == nullptr) return "EMPTY_LIST";

            ss << ptr->data;

            ptr = ptr-> next;

            while(ptr != nullptr) {
                ss << "->" <<  ptr->data;
                ptr = ptr->next;
            }

            return ss.str();
        }
    };
}
```
and

```cpp
//main.cpp

#include "LinkedList.hpp"
#include <iostream>
#include <string>

int main(int argc, char **argv) {
    collection::LinkedList<std::string> ll;

    ll.insert("name");
    ll.insert("is");
    ll.insert("Slow JavaScript");
    ll.remove();
    ll.insert("Fast Wasm");
    ll.insertR("Their");
    ll.removeR();
    ll.insertR("My");

    std::cout << "Length of the linked list is => " << ll.length() << std::endl;
    std::cout << "The contents of the linked list are => " << ll.display() << std::endl;

    return 0; 
}
```

To build this project we are going to use the make utility. Our make file looks like

```makefile
CC := g++
EMCC := em++
FLAGS := -std=c++17

project: base64
	$(CC) $(FLAGS) -o main main.cpp

project_wasm: base64_wasm
	$(EMCC) $(FLAGS) main.cpp -o main.html

```

If you run `make project_wasm` from your terminal, it will produce three files

```
├── main.html
├── main.js
├── main.wasm
```

If you open main.html and open the developer console you will see this output

```
    Length of the linked list is => 4
    The contents of the linked list are => My->name->is->Fast Wasm
```

Which is exactly the same as what you'd see if you compile with `g++` and run the binary on your terminal.

### Integrating C++ Libraries in your Application

Inetgrating existing libraries in C++ can be a bit tricky, but we can use the power of the make utility to make our lives easier. We'll integrate the `cpp-base64` libarry in our application.

Go ahead and git clone the library in the project root folder from [https://github.com/ReneNyffenegger/cpp-base64](https://github.com/ReneNyffenegger/cpp-base64)

When we inspect the makefile in `cpp-base64` folder we can see that there is a pattern for `base64-17.o` that builds the library into object files. Object files are how we link C++ libraries in our application. So we include that command in our make file and modify it. So that it looks like.

```makefile
base64:
	$(CC) $(FLAGS) -c $(BASE64_DIR)/base64.cpp -o $(BASE64_DIR)/base64-11.o

base64_wasm:
	$(EMCC) $(FLAGS) -c $(BASE64_DIR)/base64.cpp -o $(BASE64_DIR)/base64-11.o

b64_objects := $(wildcard cpp-base64/*.o)

project: base64
	$(CC) $(FLAGS) -o main main.cpp $(b64_objects) 

project_wasm: base64_wasm
	$(EMCC) $(FLAGS) main.cpp -o main.html $(b64_objects)
```

we define the base64 patterns that compile the library into object files and then use the wildcard unix procedure to list out all object files from the cpp-base64 directory. 

Then we modify our `project*` patterns and add the base64 patterns after their declaration, what it does is, whenever you run `make project`, `make base64` is run before that which ensures that the library dependency is compiled before our application is compiled.

### Integrating Libraries that use CMake

Some C++ Projects may not use make as their build and may instead choose to use other alternatives like `CMake`. For building projects emscripten provides utility scripts that run with the build tools and alters their configuration: `emconfigure`, `emcmake` and `emmake`.

As an example we will try to build the compression library miniz and include it in our sample application.

Start by cloning miniz into the project directory from [their repo](https://github.com/richgel999/miniz). 

Then cd into miniz and run the following commands:
```
    emcmake make .
    emmake make
```

After running the commands you will see `*.o` files in the directory. You can check whether the compilation step succedded by inspecting the files with the unix `file` command.

Run
```bash
    file miniz.o
```

If you get the output 
```
miniz.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
```

then the wasm build didn't work and the object files target the x86 platform and not wasm. You should be seeing the following output if the object files were build for wasm targets.

```bash
miniz.o: WebAssembly (wasm) binary module version 0x1 (MVP)
```

Since we ran the cmake commands without any conifuration, the build is written in a directory named `CMakeFiles`. For miniz, we find the output in 
`miniz/CMakeFiles/miniz.dir/` directory.

To add the dependencies to our build step we will once again use the wildcard procedure like this

```makefile
b64_objects := $(wildcard cpp-base64/*.o)
miniz_objects := $(wildcard miniz/*.o)
miniz_objects_wasm := $(wildcard miniz/CMakeFiles/miniz.dir/*.c.o)

project: base64
	$(CC) $(FLAGS) -o main main.cpp $(b64_objects) $(miniz_objects) 

project_wasm: base64_wasm
	$(EMCC) $(FLAGS) main.cpp -o main.html $(b64_objects) $(miniz_objects_wasm)

```

Now we can include miniz in `LinkedList.hpp`

```cpp
//LinkedList.hpp
#pragma once
#include <string>
#include <sstream>
#include "cpp-base64/base64.h"
#include "miniz/miniz.h"
...
```

### Interacting with C++ Functions from JavaScript
If all our C++ program compiled to WASM does is - run whatever is in the main method then it is amusing at best and pointless at its worst. The web works by responding to user inputs. To achieve that we need to export functions from our C++ file that can be called from the WebAssembly Module instance. To achieve that Emscripten provides the `emscripten/bind.h` header file.

Let's start by modifying our program to leverage that

```cpp
//main.cpp

#include "LinkedList.hpp"
#include <iostream>
#include <string>

//Let's us use EMSCRIPTEN_BINDINGS
#include <emscripten/bind.h>


...


collection::LinkedList<std::string> ll;

void insert(std::string data) {
    ll.insert(data);
}

void insertR(std::string data) {
    ll.insertR(data);
}

void removeN() {
    ll.remove();
}

void removeR() {
    ll.removeR();
}

int length() {
    return ll.length();
}

std::string items() {
    return ll.display();
}


//Here we export our function by binding them to the WASM Binary

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("insert", &insert);
    emscripten::function("insertR",&insertR);
    emscripten::function("remove", &removeN);
    emscripten::function("removeR",&removeR);
    emscripten::function("length", &length);
    emscripten::function("items",&items);
}
```

We will remove the `main()` method from our source file completely. Now when we build for wasm, our `main.js` file will instantiate the WASM module from `main.wasm` and adds to the global namespace in JS a variable named `Module` that exposes all the functions we exported with `EMSCRIPTEN_BINDINGS`.

Before building we need to modify our make file to add flag that enables `EMSCRIPTEN_BINDINGS`

```makefile
project_wasm: base64_wasm
	$(EMCC) $(FLAGS) main.cpp -lembind -o main.html $(b64_objects) $(miniz_objects) -s ALLOW_MEMORY_GROWTH -sTOTAL_MEMORY=50MB
```
we have also added flags to allow memory growth in wasm and set the initial heap size to 50 MB.


To integrate the WASM module into our web application, we include the `main.js` file into our html file

```html
...
<script src="main.js" async>
...
```

If you don't want to use the JS file generated by Emscripten and want to build WASM yourself in JS, you can do that. There are articles in MDN that go into detail about how that can be done; but we won't be discussing it here.

Now you can call C++ functions in your JavaScript code like
```js
    /**
     * @param {string} data
     */ 
    function insertToLL(data) {
        Module.insert(data);
    }
```

You can checkout a demo at [C++ Wasm Linked List Demo](), but do note that we shouldn't really be allocating memory on the heap in WASM and the program may crash when the WASM heap runs out of memory.

## Types and Gotchas

WASM is not the same as X86 or ARM. They are completely different architectures. So things that work on your PC don't necessarily work on WASM. Here are the type mistakes you should avoid

1. Experienced C developers might be tempted to pass strings as `char *` or `const char *` but that doesn't translate well in JS to WASM communication as they do not share the same address space. So passing pointers or heap allocated data from JS to C++ should be avoided. Instead you should pass objects by value. A nice way to do that is to read data as base64 string and pass it by value and then decode it in WASM

2. Since em++ is CLANG based, being explicit about the types is better. Avoid using types with ambigous definitions across vendors. For example `size_t` is interpreted as `unisgned long long` in GCC/MSVC but as `unsigned long` in CLANG. So, if you are counting on using a variable as `ull` explicitly specify that instead of using ambigous types.

## Debugging WASM

To debug WASM, you need to run your application on Chrome and use the DWARF C++ Extension. Also, you need to pass additional flags to your compiler that tells it to include a source map for debugging purposes.

We will start by adding a new rule to our build file

```makefile
    project_wasm_debug: base64_wasm
	$(EMCC) $(FLAGS) main.cpp -g -gdwarf-5 -gsplit-dwarf -gpubnames -O0 -lembind -o main.html $(bs_files) -sWASM_BIGINT -sERROR_ON_WASM_CHANGES_AFTER_LINK -s NO_EXIT_RUNTIME=1
```

Here we have added flags that add debugging information that dwarf can use. The next step is to install the [DWARF C/C++ dev tool extension](https://chrome.google.com/webstore/detail/cc%20%20-devtools-support-dwa/pdcpmagijalfljmkmjngeonclgbbannb) on chrome. After installing also need to enable the settings in chrome devtools at `Dev Tools>Settings>Experiments>Web Assembly Debugging: Enable Dwarf Support`.

Once you have done that, when running a debug, if an error occurs you will see a full stack trace in the dev console that points to the lines in the C++ file where the error originated. You can also click on sources and see the file directly in DevTools.
