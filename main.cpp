#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <vector>
#include <stack>
#include <map>

typedef unsigned int uint;

// The list of existing operators
enum OP_CODES {
    // stack manipulation
    OP_PUSH,
    OP_DUP,
    OP_COPY,
    OP_SWAP,
    OP_DISCARD,
    OP_SLIDE,
    // Arithmetic operations
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    // Heap manipulation
    OP_STORE,
    OP_RETRIEVE,
    // Flow control
    OP_SETLABEL,
    OP_CALL_SUBROUTINE,
    OP_JUMP,
    OP_JZERO,
    OP_JNEG,
    OP_END_SUBROUTINE,
    OP_END_PROGRAM,
    // I/O
    OP_PRINT_C,
    OP_PRINT_I,
    OP_READ_C,
    OP_READ_I
};

// An opcode can have no params, or either an integer or a label
enum {
    PARAM_NONE    = 0,
    PARAM_INT   = 1,
    PARAM_LABEL = 2
};

// The categories for the different operations
enum CAT_OP {
    // stack manipulation
    CAT_STACK,
    CAT_ARITH,
    CAT_HEAP,
    CAT_FLOW,
    CAT_IO
};

std::string asciiToReadable (char c){
    if (c == '\t'){
        return "[Tab]";
    }
    else if (c == '\n'){
        return "[LF]";
    }
    else if (c == ' '){
        return "[Space]";
    }

    return "[Inconnu]";
}

std::string displayableCode (const char* program){
    char* c = (char*)program;
    std::string result = "";
    while (*c){
        result += asciiToReadable(*c);
        ++c;
    }

    return result;
}

class whiteOperator {
public:
    std::string opCode;
    int paramType;
    char opCodeId;
    std::string description;
    int category;
};

whiteOperator validOperators[] = {
    /*
    Stack Manipulation (IMP: [Space])

    Stack manipulation is one of the more common operations, hence the shortness of the IMP [Space]. There are four stack instructions.

    Command Parameters  Meaning
    [Space] Number  Push the number onto the stack
    [LF][Space] -   Duplicate the top item on the stack
    [Tab][Space]    Number  Copy the nth item on the stack (given by the argument) onto the top of the stack
    [LF][Tab]   -   Swap the top two items on the stack
    [LF][LF]    -   Discard the top item on the stack
    [Tab][LF]   Number  Slide n items off the stack, keeping the top item
    The copy and slide instructions are an extension implemented in Whitespace 0.3 and are designed to facilitate the implementation of recursive functions. The idea is that local variables are referred to using [Space][Tab][Space], then on return, you can push the return value onto the top of the stack and use [Space][Tab][LF] to discard the local variables.
    */
    { "  "    , PARAM_INT  , OP_PUSH    , "push"    , CAT_STACK     } , // push the param onto the stack
    { " \n "  , PARAM_NONE , OP_DUP     , "dup"     , CAT_STACK     } , // duplicate top item
    { " \t "  , PARAM_INT  , OP_COPY    , "copy"    , CAT_STACK     } , //copy the nth item on stack onto the stack
    { " \n\t" , PARAM_NONE , OP_SWAP    , "swap"    , CAT_STACK     } , //swap the top two items on the stack
    { " \n\n" , PARAM_NONE , OP_DISCARD , "discard" , CAT_STACK     } , //discard the top item
    { " \t\n" , PARAM_INT  , OP_SLIDE   , "slide"   , CAT_STACK     } , //slide n items off the stack
    /*
    Arithmetic (IMP: [Tab][Space])

    Arithmetic commands operate on the top two items on the stack, and replace them with the result of the operation. The first item pushed is considered to be left of the operator.

    Command Parameters  Meaning
    [Space][Space]  -   Addition
    [Space][Tab]    -   Subtraction
    [Space][LF] -   Multiplication
    [Tab][Space]    -   Integer Division
    [Tab][Tab]  -   Modulo
    */
    { "\t   "  , PARAM_NONE , OP_ADD , "add" , CAT_ARITH        }  , // push the param onto the stack
    { "\t  \t" , PARAM_NONE , OP_SUB , "sub" , CAT_ARITH         } , // duplicate top item
    { "\t "    , PARAM_NONE , OP_MUL , "mul" , CAT_ARITH        }  , //copy the nth item on stack onto the stack
    { "\t "    , PARAM_NONE , OP_DIV , "div" , CAT_ARITH        }  , //swap the top two items on the stack
    { "\t "    , PARAM_NONE , OP_MOD , "mod" , CAT_ARITH     }     , //discard the top item
    /*
    Heap Access (IMP: [Tab][Tab])

    Heap access commands look at the stack to find the address of items to be stored or retrieved. To store an item, push the address then the value and run the store command. To retrieve an item, push the address and run the retrieve command, which will place the value stored in the location at the top of the stack.

    Command Parameters  Meaning
    [Space] -   Store
    [Tab]   -   Retrieve
    */
    { "\t\t "  , PARAM_NONE , OP_STORE    , "store"    , CAT_HEAP    } , 
    { "\t\t\t" , PARAM_NONE , OP_RETRIEVE , "retrieve" , CAT_HEAP    } , 
    /*
    Flow Control (IMP: [LF])

    Flow control operations are also common. Subroutines are marked by labels, as well as the targets of conditional and unconditional jumps, by which loops can be implemented. Programs must be ended by means of [LF][LF][LF] so that the interpreter can exit cleanly.

    Command Parameters  Meaning
    [Space][Space]  Label   Mark a location in the program
    [Space][Tab]    Label   Call a subroutine
    [Space][LF] Label   Jump unconditionally to a label
    [Tab][Space]    Label   Jump to a label if the top of the stack is zero
    [Tab][Tab]  Label   Jump to a label if the top of the stack is negative
    [Tab][LF]   -   End a subroutine and transfer control back to the caller
    [LF][LF]    -   End the program
    */
    { "\n  "   , PARAM_LABEL , OP_SETLABEL        , "setlabel"        , CAT_FLOW } , // Mark a location in the program
    { "\n \t"  , PARAM_LABEL , OP_CALL_SUBROUTINE , "call_subroutine" , CAT_FLOW } , // Call a subroutine
    { "\n \n"  , PARAM_LABEL , OP_JUMP            , "jump"            , CAT_FLOW } , // Jump unconditionally to a label
    { "\n\t "  , PARAM_LABEL , OP_JZERO           , "jzero"           , CAT_FLOW } , // Jump to a label if the top of the stack is zero
    { "\n\t\t" , PARAM_LABEL , OP_JNEG            , "jneg"            , CAT_FLOW } , // Jump to a label if the top of the stack is zero
    { "\n\t\n" , PARAM_NONE  , OP_END_SUBROUTINE  , "endofsubroutine" , CAT_FLOW } , // End a subroutine and transfer control back to the caller
    { "\n\n\n" , PARAM_NONE  , OP_END_PROGRAM     , "endofprogram"    , CAT_FLOW } ,
    /*
    I/O (IMP: [Tab][LF])

    Finally, we need to be able to interact with the user. There are IO instructions for reading and writing numbers and individual characters. With these, string manipulation routines can be written (see examples to see how this may be done).

    The read instructions take the heap address in which to store the result from the top of the stack.

    Command Parameters  Meaning
    [Space][
    . Space]  -   Output the character at the top of the stack
    [Space][Tab]    -   Output the number at the top of the stack
    [Tab][Space]    -   Read a character and place it in the location given by the top of the stack
    [Tab][Tab]  -   Read a number and place it in the location given by the top of the stack
    */
    { "\t\n  "   , PARAM_NONE , OP_PRINT_C , "print_c" , CAT_IO     } , 
    { "\t\n \t"  , PARAM_NONE , OP_PRINT_I , "print_i" , CAT_IO     } , 
    { "\t\n\t "  , PARAM_NONE , OP_READ_C  , "read_c"  , CAT_IO     } , 
    { "\t\n\t\t" , PARAM_NONE , OP_READ_I  , "read_i"  , CAT_IO     } , 
} ;


struct token {
    whiteOperator* op;
    std::string paramValue;
};

class Parser {
private:
    std::vector<token> tokens;
public:
    std::vector<token> parseProgram (char* program){
        std::vector<token> tokens;
        int pos = 0;
        while (program[pos] != '\0'){
            token t;
            parseInstruction(&program[pos], &t);
            if (t.op->paramType != PARAM_NONE)
                pos += t.op->opCode.length() + t.paramValue.length() + 1;
            else
                pos += t.op->opCode.length();

            tokens.push_back (t);
        }
        return tokens;
    }
    void parseInstruction(char* instruction, token* t){
        int id = -1;
        for (int i = 0; i < 24; ++i){
            uint opCodeLength = validOperators[i].opCode.length();
            if(strncmp (instruction, validOperators[i].opCode.c_str(), opCodeLength) == 0){
                id = i;
                break;
            }
        }

        if (id != -1){
            t->op = &validOperators[id];
            if (validOperators[id].paramType == PARAM_INT || validOperators[id].paramType == PARAM_LABEL){
                t->paramValue = parseParam (instruction + validOperators[id].opCode.length());
            }
        }
    }
    /*
    Many commands require numbers or labels as parameters.
    Numbers can be any number of bits wide, and are simply represented as a series of [Space] and [Tab],
    terminated by a [LF].
    [Space] represents the binary digit 0,
    [Tab] represents 1.
    The sign of a number is given by its first character, [Space] for positive and [Tab] for negative.
    Note that this is not twos complement, it just indicates a sign.
    Labels are simply [LF] terminated lists of spaces and tabs.
    There is only one global namespace so all labels must be unique.
    */
    std::string parseParam (char* instruction){
        char *c = (char*)instruction;
        int pos = 0;
        while (c[pos] && c[pos] != '\n'){
            ++pos;
        }
        std::string value(instruction, 0, pos);

        return value;
    }

    int parseInteger (std::string value){
        int sign = value[0] == ' ' ? 1 : -1;
        int intValue = 0;
        for (uint i = 1; i < value.length();++i){
            if (value[i] == ' '){
                intValue *= 2;
            }
            else {
                intValue = 2 * intValue +1;
            }
        }

        return intValue * sign;
    }

    void writeProgram(std::vector<token> tokens){
        for (uint i = 0; i < tokens.size(); ++i){
            if (tokens[i].op->paramType == PARAM_INT) {
                printf ("%s %d (%s)\n", tokens[i].op->description.c_str(), parseInteger(tokens[i].paramValue), displayableCode(tokens[i].paramValue.c_str()).c_str());
            }
            else {
                printf ("%s %s\n", tokens[i].op->description.c_str(), displayableCode(tokens[i].paramValue.c_str()).c_str());
            }
        }
    }
};


class VirtualMachine {
    std::stack<int> stk;    // stack
    std::vector<int> heap;  // heap
    std::map<std::string, int> labels; // routine labels
    std::stack<int> callstack; // List of alreayd executed calls so it is possible to transfer back to control to the caller at the end of a routine
    int ip; // instruction pointer;
    Parser parser;

    void executeInstruction (token* instruction){
        if (instruction->op->category == CAT_STACK){
            executeStackInstruction (instruction);
        }
        else if (instruction->op->category == CAT_ARITH){
            executeArithmeticInstruction (instruction);
        }
        else if (instruction->op->category == CAT_HEAP){
            executeHeapInstruction (instruction);
        }
        else if (instruction->op->category == CAT_FLOW){
            executeFlowInstruction (instruction);
        }
        else if (instruction->op->category == CAT_IO){
            executeIOInstruction (instruction);
        }
        else {
            // Error
        }
    }

    /*
        Stack operations
    */
    void executeStackInstruction (token* instruction){
        if (instruction->op->opCodeId == OP_PUSH){
            stk.push (parser.parseInteger(instruction->paramValue));
            ++ip;   
        }
        else if (instruction->op->opCodeId == OP_DUP){
            // todo: check stack size
            stk.push (stk.top());
            ++ip;
        }
        else if (instruction->op->opCodeId == OP_DISCARD){
            // todo: check for existence
            stk.pop();
            ++ip;
        }
        /*
            Todo: implement copy, slide and swap
        */
        else {
            printf ("Unkown instruction : %s\n", instruction->op->description.c_str());
            ++ip;
        }
    }

    /*
        Arithmetic operations
    */
    void executeArithmeticInstruction (token* instruction){
        /*
            Arithmetic instructions
            Should be refactored into something shorter
        */
        if (instruction->op->opCodeId == OP_ADD){
            // todo: check stack size
            int a = stk.top();
            stk.pop();
            int b = stk.top();
            stk.pop();
            stk.push (b + a);
            ++ip;
        }
        else if (instruction->op->opCodeId == OP_SUB){
            // todo: check stack size
            int a = stk.top();
            stk.pop();
            int b = stk.top();
            stk.pop();
            stk.push (b - a);
            ++ip;
        }
        else if (instruction->op->opCodeId == OP_MUL){
            // todo: check stack size
            int a = stk.top();
            stk.pop();
            int b = stk.top();
            stk.pop();
            stk.push (b * a);
            ++ip;
        }
        else if (instruction->op->opCodeId == OP_DIV){
            // todo: check stack size
            int a = stk.top();
            stk.pop();
            int b = stk.top();
            stk.pop();
            stk.push (b / a);
            ++ip;
        }
        else if (instruction->op->opCodeId == OP_MOD){
            // todo: check stack size
            int a = stk.top();
            stk.pop();
            int b = stk.top();
            stk.pop();
            stk.push (b % a);
            ++ip;
        }
        else {
            printf ("Unkown instruction : %s\n", instruction->op->description.c_str());
            ++ip;
        }
    }

    /*
        Heap manipulation
    */
    void executeHeapInstruction (token* instruction){
        if (instruction->op->opCodeId == OP_STORE){
            // todo : check stack size;
            int value = stk.top();
            stk.pop();
            int address = stk.top();
            stk.pop();
            
            // todo : check heap size;
            heap[address] = value;

            ++ip;
        }
        else if (instruction->op->opCodeId == OP_RETRIEVE){
            // todo : check stack size;
            int address = stk.top();
            stk.pop();

            // todo : check heap size;
            stk.push (heap[address]);

            ++ip;
        }
        else {
            printf ("Unkown instruction : %s\n", instruction->op->description.c_str());
            ++ip;
        }
    }

    /*
        Flow control instructions
    */
    void executeFlowInstruction (token* instruction){
        if (instruction->op->opCodeId == OP_SETLABEL){
            // Label creation has already been done during initialization
            ++ip;
        }
        else if (instruction->op->opCodeId == OP_CALL_SUBROUTINE){
            callstack.push (ip);
            // todo: check for existence
            ip = labels[instruction->paramValue];
        }
        else if (instruction->op->opCodeId == OP_JUMP){
            // todo: check for existence
            ip = labels[instruction->paramValue];
        }
        else if (instruction->op->opCodeId == OP_JZERO){
            // todo: check stack size
            int stackTop = stk.top();
            stk.pop();
            if (stackTop == 0)
                ip = labels[instruction->paramValue];
            else
                ++ip;
        }
        else if (instruction->op->opCodeId == OP_JNEG){
            // todo: check stack size
            int stackTop = stk.top();
            stk.pop();
            if (stackTop < 0)
                ip = labels[instruction->paramValue];
            else
                ++ip;
        }
        else if (instruction->op->opCodeId == OP_END_SUBROUTINE){
            // todo: check for validity
            ip = callstack.top ()+1;
            callstack.pop();
        }
        else if (instruction->op->opCodeId == OP_END_PROGRAM){
            // printf ("\n\tEND\n\n");
            ip = -1;
        }
        else {
            printf ("Unkown instruction : %s\n", instruction->op->description.c_str());
            ++ip;
        }
    }

    /*
        I/O operations
    */
    void executeIOInstruction (token* instruction){
        if (instruction->op->opCodeId == OP_PRINT_I){
            // todo: check stack size
            int stackTop = stk.top();
            stk.pop();
            printf ("%d", stackTop);
            ++ip;
        }
        else if (instruction->op->opCodeId == OP_PRINT_C){
            // todo: check stack size
            int stackTop = stk.top();
            stk.pop();
            printf ("%c", (char)stackTop);
            ++ip;
        }
        // Todo : implement read_i and read_c
        else {
            printf ("Unkown instruction : %s\n", instruction->op->description.c_str());
            ++ip;
        }
    }

    void initializeVM(std::vector<token>& instructions){
        ip = 0;
        labels.clear();
        stk = std::stack<int>();
        callstack = std::stack<int>();
        heap.clear();
        // Defaut size = 64 bytes, resized when necessary
        heap.resize(1024);
        
        for (uint i = 0; i < instructions.size(); ++i){
            if (instructions[i].op->opCodeId == OP_SETLABEL){
                // todo: check for unicity
                labels[instructions[i].paramValue] = i;
            }
        }
        // printf ("\n\tSTART\n");
    }

public:
    void execute (std::vector<token>& instructions){
        initializeVM(instructions);

        while (ip != -1) {
            executeInstruction (&instructions[ip]);
        }
    }
};

class Application {
public:
    int run (int argc, char** argv){
        if (argc < 2){
            printf ("usage :\n%s filename\n", argv[0]);
            return EXIT_SUCCESS;
        }
        else {
            Parser g_Parser;
            char *content = readFileContent(argv[1]);
            
            // printf ("%s", displayableCode(content).c_str());
            
            std::vector<token> instructions = g_Parser.parseProgram((char*)content);
            // g_Parser.writeProgram (instructions);
            
            VirtualMachine vm;
            vm.execute(instructions);

            free(content);
        }
        return EXIT_SUCCESS;
    }

private:
    char* readFileContent(char* filename){
        FILE* pFile = fopen(filename, "r");
        fseek (pFile , 0 , SEEK_END );
        int len = ftell(pFile);
        fseek (pFile , 0 , SEEK_SET );

        char* content = (char*)malloc((len+1)*sizeof(char));
        fread(content, len, sizeof(char), pFile);
        content[len] = '\0';

        fclose(pFile);

        return content;
    }
};

int main(int argc, char** argv){
    // Parser parser;
    // int v;

    // token tok;
    // parser.parseInstruction((char*)" \n\t\t\n", &tok);
    // parser.parseInstruction((char*)"   \t\n", &tok);
    // parser.parseInstruction((char*)"\n\t \t \t\n", &tok);
    // std::vector<token> tokens = parser.parseProgram((char*)"   \t\n\n\t \t\n");
    // parser.writeProgram (tokens);

    Application app;
    app.run (argc, argv);
}