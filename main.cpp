#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <vector>

// The list of valid characters
enum {
    TOK_TAB   = '\t',
    TOK_LF    = '\n',
    TOK_SP    = ' '
};

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
    // FLow control
    OP_SETLABEL,
    OP_CALL_SUBROUTINE,
    OP_JUMP,
    OP_JZERO,
    OP_JNEG,
    OP_ENDOFSUBROUTINE,
    OP_ENDOFPROGRAM,
    // I/O
    OP_PRINT_C,
    OP_PRINT_I,
    OP_READ_C,
    OP_READ_I,
};

// An opcode can have no params, or either an integer or a label
enum {
    PARAM_NONE    = 0,
    PARAM_INT   = 1,
    PARAM_LABEL = 2
};

const char* asciiToReadable (char c){
    if (c == TOK_TAB){
        return "[Tab]";
    }
    else if (c == TOK_LF){
        return "[LF]";
    }
    else if (c == TOK_SP){
        return "[Space]";
    }

    return "[Inconnu]";
}
void displayCode (const char* program){
    char* c = (char*)program;
    // printf ("%d\n", strlen(program));
    while (*c){
        printf ("%s", asciiToReadable(*c));
        ++c;
    }
}

class whiteOperator {
public:
    std::string opCode;
    int paramType;
    char opCodeId;
    std::string description;
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
    { "  "       , PARAM_INT  , OP_PUSH,"push"         } , // push the param onto the stack
    { " \n "     , PARAM_NONE , OP_DUP,"dup"          } , // duplicate top item
    { " \t "     , PARAM_INT  , OP_COPY,"copy"         } , //copy the nth item on stack onto the stack
    { " \n\t"    , PARAM_NONE , OP_SWAP,"swap"         } , //swap the top two items on the stack
    { " \n\n"    , PARAM_NONE , OP_DISCARD,"discard"      } , //discard the top item
    { " \t\n"    , PARAM_INT  , OP_SLIDE,"slide"        } , //slide n items off the stack
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
    { "\t   "  , PARAM_NONE , OP_ADD , "add"         }  , // push the param onto the stack
    { "\t  \t" , PARAM_NONE , OP_SUB , "sub"          } , // duplicate top item
    { "\t "    , PARAM_NONE , OP_MUL , "mul"         }  , //copy the nth item on stack onto the stack
    { "\t "    , PARAM_NONE , OP_DIV , "div"         }  , //swap the top two items on the stack
    { "\t "    , PARAM_NONE , OP_MOD , "mod"      }     , //discard the top item
    /*
    Heap Access (IMP: [Tab][Tab])

    Heap access commands look at the stack to find the address of items to be stored or retrieved. To store an item, push the address then the value and run the store command. To retrieve an item, push the address and run the retrieve command, which will place the value stored in the location at the top of the stack.

    Command Parameters  Meaning
    [Space] -   Store
    [Tab]   -   Retrieve
    */
    { "\t\t "  , PARAM_NONE , OP_STORE    , "store"        } , 
    { "\t\t\t" , PARAM_NONE , OP_RETRIEVE , "retrieve"     } , 
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
    { "\n  "   , PARAM_LABEL , OP_SETLABEL        , "setlabel" }        , // Mark a location in the program
    { "\n \t"  , PARAM_LABEL , OP_CALL_SUBROUTINE , "call_subroutine" } , // Call a subroutine
    { "\n \n"  , PARAM_LABEL , OP_JUMP            , "jump" }            , // Jump unconditionally to a label
    { "\n\t "  , PARAM_LABEL , OP_JZERO           , "jzero" }           , // Jump to a label if the top of the stack is zero
    { "\n\t\t" , PARAM_LABEL , OP_JNEG            , "jneg" }            , // Jump to a label if the top of the stack is zero
    { "\n\t\n" , PARAM_NONE  , OP_ENDOFSUBROUTINE , "endofsubroutine" } , // End a subroutine and transfer control back to the caller
    { "\n\n\n" , PARAM_NONE  , OP_ENDOFPROGRAM    , "endofprogram" }    , 
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
    { "\t\n  "   , PARAM_NONE , OP_PRINT_C , "print_c"      } , 
    { "\t\n \t"  , PARAM_NONE , OP_PRINT_I , "print_i"      } , 
    { "\t\n\t "  , PARAM_NONE , OP_READ_C  , "read_c"       } , 
    { "\t\n\t\t" , PARAM_NONE , OP_READ_I  , "read_i"       } , 
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
        printf ("done parsing");
        return tokens;
    }
    void parseInstruction(char* instruction, token* t){
        int id = -1;
        for (int i = 0; i < 24; ++i){
            int opCodeLength = validOperators[i].opCode.length();
            if(strncmp (instruction, validOperators[i].opCode.c_str(), opCodeLength) == 0){
                id = i;
                printf("%s\n", validOperators[i].description.c_str());
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

    void writeProgram(std::vector<token> tokens){
        for (int i = 0; i < tokens.size(); ++i){
            printf ("%s %s\n", tokens[i].op->description.c_str(), tokens[i].paramValue.c_str());
        }
    }
};


class Application {
    int argc;
    char** argv;

public:
    Application (int argc, char** argv) {
        this->argc = argc;
        this->argv = argv;
    }
    int run (){
        if (this->argc < 2){
            printf ("usage :\n%s filename\n", this->argv[0]);
            return EXIT_SUCCESS;
        }
        else {
            Parser g_Parser;
            char *content = readFileContent(this->argv[1]);
            
            displayCode(content);

            std::vector<token> tokens = g_Parser.parseProgram((char*)content);
            g_Parser.writeProgram (tokens);

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

    Application app(argc, argv);
    app.run ();
}