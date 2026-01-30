/**
 * @file assembler.h
 * @brief Z80 Assembler - coordinates parsing and code generation
 */

#ifndef Z80_ASSEMBLER_H
#define Z80_ASSEMBLER_H

#include "parser.h"
#include "symbol_table.h"
#include "common/types.h"
#include <string>
#include <vector>

namespace z80 {

/**
 * @brief Represents assembled output
 */
struct AssembledModule {
    std::string moduleName;
    std::vector<ParsedLine> lines;
    SymbolTable symbolTable;
    bool isRelocatable;
    
    // Segment sizes
    Word csegSize;
    Word dsegSize;
    Word commonSize;
    
    AssembledModule() 
        : isRelocatable(false)
        , csegSize(0)
        , dsegSize(0)
        , commonSize(0)
    {}
};

/**
 * @brief Main assembler class - orchestrates the assembly process
 */
class Assembler {
public:
    Assembler();
    
    /**
     * @brief Assemble a source file
     * @param filename Source file to assemble
     * @return Assembled module, or nullptr if errors occurred
     */
    std::unique_ptr<AssembledModule> assemble(const std::string& filename);
    
    /**
     * @brief Get assembly errors
     */
    const std::vector<AssemblyError>& getErrors() const { return errors_; }
    
    /**
     * @brief Check if assembly had errors
     */
    bool hasErrors() const { return !errors_.empty(); }

private:
    Parser parser_;
    std::vector<AssemblyError> errors_;
    
    /**
     * @brief Calculate segment sizes from parsed lines
     */
    void calculateSegmentSizes(AssembledModule& module);
    
    /**
     * @brief Determine if module is relocatable
     */
    bool isModuleRelocatable(const std::vector<ParsedLine>& lines);
    
    /**
     * @brief Derive module name from filename
     */
    std::string deriveModuleName(const std::string& filename);
};

} // namespace z80

#endif // Z80_ASSEMBLER_H
