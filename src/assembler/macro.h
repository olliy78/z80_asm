/**
 * @file macro.h
 * @brief Macro definition and expansion support for Z80 assembler
 * 
 * Implements MACRO-80 compatible macro facilities:
 * - MACRO/ENDM: User-defined macros with parameters
 * - REPT/ENDM: Repeat blocks
 * - IRP/ENDM: Indefinite repeat over argument list
 * - IRPC/ENDM: Indefinite repeat over characters
 * - LOCAL: Local labels in macros
 * - EXITM: Early exit from macro expansion
 * 
 * @author Z80 Assembler Project
 * @date 2026
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace z80 {

/**
 * @enum MacroType
 * @brief Type of macro definition
 */
enum class MacroType {
    UserDefined,    ///< MACRO/ENDM user-defined macro
    Repeat,         ///< REPT/ENDM repeat block
    IndefiniteRP,   ///< IRP/ENDM indefinite repeat over list
    IndefiniteRPC   ///< IRPC/ENDM indefinite repeat over characters
};

/**
 * @struct MacroDefinition
 * @brief Definition of a macro
 * 
 * Stores complete macro definition including:
 * - Macro type and name
 * - Parameter names
 * - Body lines (unexpanded source)
 * - Local label declarations
 */
struct MacroDefinition {
    MacroType type;                         ///< Type of macro
    std::string name;                       ///< Macro name (empty for REPT/IRP/IRPC)
    std::vector<std::string> parameters;    ///< Parameter names (for user-defined macros)
    std::vector<std::string> body;          ///< Macro body lines (raw source)
    std::vector<std::string> localLabels;   ///< Local labels declared with LOCAL
    int definitionLine;                     ///< Line where macro was defined
    std::string definitionFile;             ///< File where macro was defined
    
    // For REPT
    int repeatCount;                        ///< Number of repetitions (REPT)
    
    // For IRP/IRPC
    std::string iteratorName;               ///< Iterator parameter name (IRP/IRPC)
    std::vector<std::string> iteratorValues; ///< Values to iterate over
};

/**
 * @struct MacroExpansion
 * @brief Active macro expansion state
 * 
 * Tracks state during macro expansion:
 * - Which macro is being expanded
 * - Current parameter values
 * - Current line in expansion
 * - Unique ID for local label generation
 */
struct MacroExpansion {
    const MacroDefinition* definition;      ///< Macro being expanded
    std::vector<std::string> arguments;     ///< Actual arguments (substitutions)
    int currentLine;                        ///< Current line in body being expanded
    int uniqueId;                           ///< Unique ID for local labels
    bool exitRequested;                     ///< True if EXITM encountered
    
    // For IRP/IRPC iteration
    size_t iterationIndex;                  ///< Current iteration (IRP/IRPC)
};

/**
 * @class MacroProcessor
 * @brief Manages macro definitions and performs macro expansion
 * 
 * Features:
 * - Store and retrieve macro definitions
 * - Expand macros with parameter substitution (&param)
 * - Generate unique local label names
 * - Support nested macro expansions
 * - Track expansion depth for error detection
 */
class MacroProcessor {
public:
    /**
     * @brief Construct a new MacroProcessor
     */
    MacroProcessor();
    
    /**
     * @brief Define a new macro
     * @param macro Macro definition
     */
    void defineMacro(const MacroDefinition& macro);
    
    /**
     * @brief Check if a name is a defined macro
     * @param name Macro name
     * @return true if macro exists
     */
    bool isMacroDefined(const std::string& name) const;
    
    /**
     * @brief Get macro definition by name
     * @param name Macro name
     * @return const MacroDefinition* Pointer to definition, or nullptr
     */
    const MacroDefinition* getMacro(const std::string& name) const;
    
    /**
     * @brief Begin macro expansion
     * @param name Macro name
     * @param arguments Actual arguments for expansion
     * @return true if expansion started successfully
     */
    bool beginExpansion(const std::string& name, const std::vector<std::string>& arguments);
    
    /**
     * @brief Begin repeat block expansion
     * @param repeatCount Number of repetitions
     * @param body Body lines to repeat
     * @param localLabels Local label declarations
     * @return true if expansion started successfully
     */
    bool beginRepeat(int repeatCount, const std::vector<std::string>& body,
                     const std::vector<std::string>& localLabels = {});
    
    /**
     * @brief Begin IRP expansion
     * @param iteratorName Iterator parameter name
     * @param values List of values to iterate over
     * @param body Body lines for each iteration
     * @param localLabels Local label declarations
     * @return true if expansion started successfully
     */
    bool beginIRP(const std::string& iteratorName, const std::vector<std::string>& values,
                  const std::vector<std::string>& body,
                  const std::vector<std::string>& localLabels = {});
    
    /**
     * @brief Begin IRPC expansion
     * @param iteratorName Iterator parameter name
     * @param chars String of characters to iterate over
     * @param body Body lines for each character
     * @param localLabels Local label declarations
     * @return true if expansion started successfully
     */
    bool beginIRPC(const std::string& iteratorName, const std::string& chars,
                   const std::vector<std::string>& body,
                   const std::vector<std::string>& localLabels = {});
    
    /**
     * @brief Get next expanded line from current macro
     * @param expandedLine Output: expanded line with substitutions
     * @return true if line retrieved, false if expansion complete
     */
    bool getNextLine(std::string& expandedLine);
    
    /**
     * @brief Check if currently expanding a macro
     * @return true if in macro expansion
     */
    bool isExpanding() const { return !expansionStack_.empty(); }
    
    /**
     * @brief Get current expansion depth (for nesting detection)
     * @return int Nesting depth (0 = not expanding)
     */
    int getExpansionDepth() const { return expansionStack_.size(); }
    
    /**
     * @brief Request early exit from current macro (EXITM)
     */
    void exitMacro();
    
    /**
     * @brief Clear all macro definitions
     */
    void clear();
    
private:
    /**
     * @brief Expand line with parameter/iterator substitution
     * @param line Raw macro body line
     * @param expansion Current expansion state
     * @return Expanded line with substitutions
     */
    std::string expandLine(const std::string& line, const MacroExpansion& expansion);
    
    /**
     * @brief Substitute parameter references (&param) in line
     * @param line Line with &param references
     * @param expansion Expansion containing parameter values
     * @return Line with parameters substituted
     */
    std::string substituteParameters(const std::string& line, const MacroExpansion& expansion);
    
    /**
     * @brief Generate unique local label name
     * @param label Original local label name
     * @param uniqueId Unique expansion ID
     * @return Unique label name (e.g., "??0001" for first expansion)
     */
    std::string generateLocalLabel(const std::string& label, int uniqueId);
    
    /**
     * @brief Check if label is a local label in current macro
     * @param label Label name
     * @param expansion Current expansion
     * @return true if it's a local label
     */
    bool isLocalLabel(const std::string& label, const MacroExpansion& expansion);
    
    /**
     * @brief Get next unique ID for local labels
     * @return int Unique ID
     */
    int getNextUniqueId() { return nextUniqueId_++; }
    
    std::map<std::string, MacroDefinition> macros_;     ///< Macro definitions by name
    std::vector<MacroExpansion> expansionStack_;        ///< Stack of active expansions
    int nextUniqueId_;                                   ///< Next unique ID for locals
    
    static constexpr int MAX_EXPANSION_DEPTH = 100;     ///< Maximum nesting depth
};

} // namespace z80
