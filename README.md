# Linker
Operating Systems Project - Building a Linker

A summary of instructions for this project:

You are to implement a two-pass linker.
Your program must take one input parameter which will be the name of an input file to be processed. All output should go to standard output. 

In general, a linker takes individually compiled code/object modules and creates a single executable by resolving external symbol references (e.g. variables and functions) and module relative addressing by assigning global addresses after placing the modules’ object code at global addresses.

Rather than dealing with complex x86 tool chains, we assume a target machine with the following properties: (a) word addressable, (b) addressable memory of 512 words, and (c) each word consisting of up to 4 decimal digits.

The input to the linker is a file containing a sequence of tokens (symbols and integers and instruction type characters). Don’t assume tokens that make up a section to be on one line, don’t make assumptions about how much space separates tokens or that lines are non-empty for that matter or that each input conforms syntactically. Symbols always begin with alpha characters followed by optional alphanumerical characters, i.e.[a-Z][a-Z0-9]*. Valid symbols can be up to 16 characters. Integers are decimal based. Instruction type characters are (I, A, R, E). Token delimiters are ‘ ‘, ‘\t’ or ‘\n’.

The input file to the linker is structured as a series of “object module” definitions.
Each “object module” definition contains three parts (in fixed order): definition list, use list, and program text.

 definition list consists of a count defcount followed by defcount pairs (S, R) where S is the symbol being defined and R is the relative word address (offset) to which the symbol refers in the module.

 use list consists of a count usecount followed by usecount symbols that are referred to in this module. These could include symbols defined in the definition list of any module (prior or subsequent or not at all).

 program text consists of a count codecount followed by codecount pairs (type, instr), where instr is an upto 4-digit instruction (integer) and type is a single character indicating Immediate, Absolute, Relative, or External. codecount is thus the length of the module.

An instruction is composed of an integer that is separated into an opcode (op/1000) and an operand (op mod 1000). The opcode always remains unchanged by the linker. (Since the instruction value is supposed to be 4 or less digits, read an integer and ensure opcode < 10, see errorcodes below). The operand is modified/retained based on the instruction type in the program text as follows:

(I) an immediate operand is unchanged;

(A) operand is an absolute address which will never be changed in pass2; however it can’t be “>=” the machine size (512);

(R) operand is a relative address which is relocated by replacing the relative address with the absolute address of that relative address after the modules global address has been determined.

(E) operand is an external address which is represented as an index into the uselist. For example, a reference in the program text with operand K represents the Kth symbol in the use list, using 0-based counting, e.g., if the use list is ‘‘2 f g’’, then an instruction ‘‘E 7000’’ refers to f, and an instruction ‘‘E 5001’’ refers to g. You must identify to which global address the symbol is assigned and then replace the address.

The linker must process the input twice (that is why it is called two-pass). 

Pass One parses the input and verifies the correct syntax and determines the base address for each module and the absolute address for each defined symbol, storing the latter in a symbol table. The first module has base address zero; the base address for module X+1 is equal to the base address of module X plus the length of module X. The absolute address for symbol S defined in module M is the base address of M plus the relative address of S within M. After pass one print the symbol table (including errors related to it (see rule2 later)). Do not store parsed tokens, only store meta data (e.g. deflist, uselist, num-instructions) and of course the symboltable.

Pass Two again parses the input and uses the base addresses and the symbol table entries created in pass one to generate the actual output by relocating relative addresses and resolving external references.
You must clearly mark your two passes in the code through comments and/or proper function naming.

You must check the input for various errors. All errors/warnings should follow the message catalog provided below. 

You should continue processing after encountering an error/warning (other than a syntax error) and you should be able to detect multiple errors in the same run.

      1. You should stop processing if a syntax error is detected in the input, print a syntax error message with the line number and the character offset in the input file where observed. A syntax error is defined as a missing token (e.g. 4 used symbols are defined but only 3 are given) or an unexpected token. Stop processing and exit.

      2. If a symbol is defined multiple times, print an error message and use the value given in the first definition. Error message to appear as part of printing the symbol table (following symbol=value printout on the same line)

      3. If a symbol is used in an E-instruction but not defined, print an error message and use the value zero.
      4. If a symbol is defined but not used, print a warning message and continue.
      5. If an address appearing in a definition exceeds the size of the module, print a warning message and treat the address
      given as 0 (relative to the module).
      6. If an external address is too large to reference an entry in the use list, print an error message and treat the address as
      immediate.
      7. If a symbol appears in a use list but it not actually used in the module (i.e., not referred to in an E-type address),
      print a warning message and continue.
      8. If an absolute address exceeds the size of the machine, print an error message and use the absolute value zero.
      9. If a relative address exceeds the size of the module, print an error message and use the module relative value zero
      (that means you still need to remap “0” that to the correct absolute address).
      10. If an illegal immediate value (I) is encountered (i.e. more than 4 numerical digits, aka >= 10000), print an error and
      convert the value to 9999.
      11. If an illegal opcode is encountered (i.e. more than 4 numerical digits, aka >= 10000), print an error and convert the
      <opcode,operand> to 9999.

The following exact limits are in place.

      a) Accepted symbols should be upto 16 characters long (not including terminations e.g. ‘\0’), any longer symbol names
      are erroneous.
      b) a uselist or deflist should support 16 definitions, but not more and an error should be raised.
      c) number instructions are unlimited (hence the two pass system), but in reality they are limited to the machine size.
      d) Symbol table should support at least 256 symbols
      



