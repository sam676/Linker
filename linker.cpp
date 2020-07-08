//Stephanie Michalowicz
//Operating Systems



#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <cstring>
#include <sstream>
#include <locale>
#include <vector>
#include <algorithm>
#include <iomanip>


using namespace std;

int lineNumber = 0;     // active line of parsing for error reporting
int token_offset = 0;   // token_offset for error reporting

int moduleNum = 1;     //keep track of which module is being read
int inst_Num = 0;       //keep track of the total number of instructions  

ifstream the_file;

//create symbol class
class Symbol {
    public:
       // Symbol(string _S, int _V) {
        Symbol(string _S) {
            S = string(_S);
            V = 0;
            duplicate = false;
            warningMessage = false;
            used = false;
            module = 0;;
            uselist = true;

        }

        string S;
        int V;
        bool duplicate;  
        bool used;
        bool warningMessage;
        int module;
        bool uselist;
   
};

//global symbol table
vector <Symbol *> symbolTable; //should be size 512 (should support at least 256 symbols) 


char * token;   //should be up to 16 characters long

//////////////////////////////////////////////////////////////////////////////////////////////

//read in a token.
// any sequence of characters that are not whitespace " \t"

char * getToken( ) {

    // these are static variables so they persist over many invocation
    // effectively they are global variables but are only visible in this function

    static int eol_reached = 1;
    static char * currentP = NULL;
    static char buffer[1024];
    static char * strtok_ptr;
    static char * startToken;
    static int lineLength;


    while (1) {  // we break out of this when we have a token or when we reach the end of the file
        if (eol_reached) {
            if (the_file.eof())
                return NULL; // didn't find a token
            if (!the_file.getline( buffer, 1024 )) {
                // this line is empty, so we have to correct the last error position
                // back to the end of the last line
                token_offset = lineLength + 1;
                return NULL; // apparently we are hitting the eof now
            } else {
                eol_reached = 0; // need to reset the flag that tells us to read a new line
                lineLength = strlen(buffer); // remember the last one
                lineNumber++;
                // set the buffer pointers
                strtok_ptr = buffer;
                currentP = buffer;
            }
        }

        startToken = strtok(strtok_ptr, "\t ");
        strtok_ptr = NULL; // need to set to NULL so the next invocation on the same buffer continues
        // this is how <strtok> works  .. it will provide a '\0' terminate string
        if (startToken == NULL) {
            // we hit the end of the line so try for another line
            eol_reached = 1;
            continue;  // jump back to the while loop
        }
        // set the error position to the start of the token for higher level
        token_offset = startToken - buffer + 1;
        //printf("# <%s> %d %d\n", startToken, lineNumber, token_offset);
        return startToken;
        // we never get here but its convenient so we can use "break" and "continue"
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

//Parse Errors abort processing

void __parseerror(int errcode){
    const char *  errstr[] = {
        "NUM_EXPECTED",                 //0        
        "SYM_EXPECTED",                 //1
        "ADDR_EXPECTED",                //2 
        "SYM_TOO_LONG",                 //3
        "TOO_MANY_DEF_IN_MODULE",       //4
        "TOO_MANY_USE_IN_MODULE",       //5
        "TOO_MANY_INSTR",               //6
    };

    printf ("Parse Error line %d offset %d: %s\n", lineNumber, token_offset, errstr[errcode]);
}


////////////////////////////////////////////////////////////////////////////////////////////////


//read an integer return >= 0 if a number or -1 if it failed or parse error otherwise
int readInt(int fail = 1) {
    char * token = getToken();
    int num;

    if( token != NULL) {

        //convert the token into an int
        stringstream ss(token);

        ss.clear();
        ss >> num;
        if(!ss.fail() && (num >= 0)) 
	      return num;
    }

    // we failed    
    if (fail) {
       //PARSE ERROR: Number expected (rule 1)
       __parseerror(0);
       exit( EXIT_FAILURE );  
    } else {
       return -1;
    }
}


//read a symbol
char * readSymbol() {
    char * token = getToken();

   if( token == NULL || !isalnum( *token ) || isdigit( *token ) ) {
       //PARSE ERROR: Symbol expected (rule 1)
        __parseerror(1);
       exit( EXIT_FAILURE );
    }

    if (strlen( token ) > 16 ){ 
        //PARSE ERROR: Symbol Name is too long (limit a)
        __parseerror(3);
       exit( EXIT_FAILURE );
    }

    return token;
}

//read an instruction
char readIAER() {
   char * token = getToken();

   if( (token == NULL) || ((*token != 'A') && (*token !='E') && (*token !='I') && (*token !='R')) ) {
        //PARSE ERROR: tempAddressing expected which is A/E/I/R (rule 1)
        __parseerror(2);
       exit( EXIT_FAILURE ); 
    }

    return *token;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Pass1(){
    vector <Symbol>  useList;    //should support only 16 definitions
    vector <Symbol*> defList;    //should support only 16 definitions

    while (!the_file.eof()){

        ///////////////read the definition list
        int defCount = readInt(0);

        if(defCount == -1) {
            break; // means no more tokens were found so we ended on a good Module
        }

        if(defCount > 16){
            //PARSE ERROR: too many definitions in module (limit b)
            __parseerror(4);
            exit( EXIT_FAILURE ); 
        }

        //////////////////////read definition list
        for(int i=0; i < defCount; i++){
            Symbol * newSym = NULL;
            char * sym = readSymbol();
            int val = readInt(1);

            // define the symbol in the symbol table but check for duplication
            for(int j = 0; j < symbolTable.size(); j++){
                if(symbolTable[j] -> S == sym) {
                    symbolTable[j] -> duplicate = true;
                    newSym = symbolTable[j]; 
                    break;
                }      
            }
            if(newSym == NULL) {
                newSym = new Symbol(sym);
                newSym -> V =  val  + inst_Num;
                newSym -> module = moduleNum;
                symbolTable.push_back(newSym);
            }

            //add to definition list
            defList.push_back(newSym);
        }

        /////////////////////read the use list
        int useCount = readInt(1);

        if(useCount > 16){
        //PARSE ERROR: too many used symbols in module (limit b)
        __parseerror(5);
        exit( EXIT_FAILURE ); 
        }

        for(int i=0; i < useCount; i++){
            char * useSym = readSymbol();
            
            //create symbol
            Symbol useSymbol = Symbol(string(useSym));

            //add to use list
            useList.push_back(useSymbol);
        }

        /////////////read the instruction list
        int codeCount = readInt(1);

        if((inst_Num + codeCount) >= 512){
           //PARSE ERROR: total number of instructions exceeds memory size (512)  (limit c)
            __parseerror(6);
           exit( EXIT_FAILURE );
        } 


        for(int i = 0; i < codeCount; i++){
            char instruction = readIAER();
            int tempAdd = readInt(1);
        }      
    
        //check to see if any definition is greater than the code count - 1 (rule 5: address exceeds size of the module)
        for(int i=0; i < defList.size(); i++){
             if (defList[i]->V >= (inst_Num + codeCount)) {
                 printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", moduleNum, defList[i]->S.c_str(), defList[i]->V - inst_Num, codeCount-1);
                 defList[i]->V = inst_Num;
             }
        }

        //increase the number of instructions every module by adding the code count
        inst_Num = inst_Num + codeCount;

        //increment every time a module is read
        moduleNum++;

        //clear existing lists
        useList.clear();
        defList.clear();

    }  //end while loop


    //print symbol table
    printf("Symbol Table\n");

    for(int i = 0; i < symbolTable.size(); i++){
        printf("%s=%d", symbolTable[i] -> S.c_str(), symbolTable[i] -> V);

        if(symbolTable[i] -> duplicate == true){
            //ERROR: the same symbol is defined twice
            printf(" Error: This variable is multiple times defined; first value used\n");  
        }
        printf("\n");

    
    }

} //////////////////////////////////////////////////////////end pass 1

void Pass2(){
    vector <Symbol> localUseList;
    int moduleLen = 0;

    while (!the_file.eof()){
        ///////////////read the definition list
        int defCount = readInt(0);

        if(defCount == -1) {
            break; // means no more tokens were found so we ended on a good Module
        }

        for(int i= 0; i < defCount; i++){
            char * sym = readSymbol();
            int val = readInt(1);
    
            //create symbol
            Symbol newSymbol = Symbol(string(sym));
            newSymbol.V = val;
        }

        /////////////////////read pass2 local use list
        int useCount = readInt(1);

        for(int i= 0; i < useCount; i++){
            char * sym = readSymbol();

            //create symbol
            Symbol newSymbol = Symbol(string(sym));
            newSymbol.module = moduleNum;
            newSymbol.uselist = false;

            localUseList.push_back(newSymbol); 
        }

        /////////////read the instruction list
            int codeCount = readInt(1);
            int tempAdd;
            int opcode;
            int operand;
            char buffer[256] = "";
            string errorMessage;         

            for (int i = 0; i < codeCount; i++){
                char instruction = readIAER();
                inst_Num += 1;
                tempAdd = readInt(1);
                opcode = tempAdd / 1000;  //first digit
                operand = tempAdd % 1000;  //last three digits
                buffer[0] = '\0';
                if(tempAdd > 10000){
                    if (opcode >= 10){
                       errorMessage = sprintf(buffer," Error: Illegal opcode; treated as 9999");
                    }else{
                        errorMessage = sprintf(buffer," Error: Illegal immediate value; treated as 9999");
                    }
                    tempAdd = 9999;
                }else if(instruction == 'A'){
                    if (operand >= 512){
                        errorMessage = sprintf(buffer, " Error: Absolute address exceeds machine size; zero used");
                        operand = 0;  
                    }
                    tempAdd = (opcode * 1000) + operand; 
                }else if(instruction == 'R'){
                    if((operand) > (codeCount - 1)){  
                        errorMessage = sprintf(buffer, " Error: Relative address exceeds module size; zero used"); 
                        tempAdd = (opcode * 1000);
                    }

                    if (moduleNum != 0){
                        tempAdd = tempAdd + moduleLen;
                    }

                 }else if(instruction == 'E'){

                     //Error: E address is greater than length of uselist
                    if(operand >= localUseList.size() ){ 
                        errorMessage = sprintf(buffer," Error: External address exceeds length of uselist; treated as immediate");
                    } else {
                    
                        string usedSym = localUseList[operand].S; //use list symbol
                        int symValue;  //symbol table value
                        bool isDefined = false;
                    
                        for(int i = 0; i < symbolTable.size(); i++){
                            if( (usedSym.compare(symbolTable[i] -> S)) == 0){
                                symValue = symbolTable[i] -> V;
                                tempAdd = (opcode * 1000) + symValue;
                                symbolTable[i] -> used = true;
                                isDefined = true;
				break;
                            } 
                        }

                        localUseList[operand].uselist = true;
                        
                        if (!isDefined){
                            //ERROR: the symbol is not defined; zero used
                            errorMessage = sprintf(buffer, " Error: %s is not defined; zero used", usedSym.c_str());
                        }
                    }

                } else {
                    // must be Immediate ... all errors are covered
                }      

                printf("%03d: %04d", inst_Num, tempAdd);
                if (strlen(buffer) > 0) printf("%s", buffer);
                printf("\n");
             }  


            for(int i = 0; i < localUseList.size(); i++){
                if (!localUseList[i].uselist){
                    //Warning: symbol is in uselist but not actually used (printed after each module!)
                    printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", moduleNum, localUseList[i].S.c_str());
                }             
            }   

            //increment every time a module is read
            moduleNum++;

            //increment module length
            moduleLen = moduleLen + codeCount;

            localUseList.clear();

    }  //end while loop


    printf("\n");
    //symbol defined but never used! (printed at the end of pass 2 after all of the modules have been processed)
    for(int i = 0; i < symbolTable.size(); i++){
        //WARNING: Module # : symbol defined but never used
        if(!symbolTable[i] -> used){
            printf("Warning: Module %d: %s was defined but never used\n", symbolTable[i] -> module, symbolTable[i] -> S.c_str());
        }  
    }
    printf("\n");
   

}



/////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char *argv[] ) {

    //open and read the files
    //make sure there are two arguments
    if ( argc != 2 ) {
        //Print the program name
        cout<<"usage: "<< argv[0] <<" <filename>\n";
    } else {

        // Open the file
        the_file.open( argv[1] );

        //Check the file
        if ( !the_file.is_open() ) {
            printf("Could not open file\n");
        } else {

            Pass1();

            //Close the file
            the_file.close();

            inst_Num = -1;

            moduleNum = 1;

            // Open the file
            the_file.open( argv[1] );

            printf("\nMemory Map\n");

            Pass2();

            //Close the file
            the_file.close();
        }
    }

    return 0;
}
