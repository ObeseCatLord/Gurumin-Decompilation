// Applies JP names (and JP signatures where safe) to the EN program (game_en_steam.exe) from decomp_en/jp_en_map.tsv.
//   args: <jp_en_map.tsv> [jp_signatures.tsv] [minimum level: high|medium|low, default medium]
// - Name: the JP symbols.tsv name (jp_name column) when present, else JP_<address>; only functions still named
//   FUN_xxxxxxxx are renamed, so names given by hand in the EN project are kept.
// - Plate comment: "JP twin <module> <address> (<level> <confidence>, <method>)".
// - Signature: copied only for APP.dll twins at level high, and only when every type in the JP prototype parses
//   with the EN program's data types (built-in types and pointers to them). Struct types are left alone.
// @category Gurumin
import ghidra.app.script.GhidraScript;
import ghidra.app.util.parser.FunctionSignatureParser;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.FunctionDefinitionDataType;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SourceType;
import ghidra.app.cmd.function.ApplyFunctionSignatureCmd;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class ApplyJpNames extends GhidraScript {
    private static int rank(String level) {
        switch (level) { case "high": return 3; case "medium": return 2; case "low": return 1; default: return 0; }
    }

    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        Path map = Paths.get(args.length > 0 ? args[0] : "jp_en_map.tsv");
        Map<String, String[]> sigs = new HashMap<>();
        if (args.length > 1 && Files.exists(Paths.get(args[1]))) {
            for (String line : Files.readAllLines(Paths.get(args[1]), StandardCharsets.UTF_8)) {
                String[] c = line.split("\t");
                if (c.length >= 4 && !c[0].equals("address")) sigs.put(c[0], c);
            }
        }
        int minimum = rank(args.length > 2 ? args[2] : "medium");
        FunctionSignatureParser parser = new FunctionSignatureParser(currentProgram.getDataTypeManager(), null);
        int named = 0, typed = 0, kept = 0;
        for (String line : Files.readAllLines(map, StandardCharsets.UTF_8)) {
            String[] c = line.split("\t", -1);
            if (c.length < 11 || c[0].equals("en_address") || c[2].equals("-")) continue;
            if (rank(c[8]) < minimum) continue;
            Address a = toAddr(Long.parseLong(c[0], 16));
            Function f = getFunctionAt(a);
            if (f == null) continue;
            String name = !c[4].isEmpty() ? c[4] : "JP_" + c[3];
            if (f.getName().startsWith("FUN_")) {
                try {
                    f.setName(name.replace("::", "_"), SourceType.IMPORTED);
                    named++;
                } catch (Exception e) {
                    println("cannot name " + c[0] + " " + name + ": " + e.getMessage());
                }
            } else {
                kept++;
            }
            f.setComment(String.format("JP twin %s %s (%s %s, %s)", c[2], c[3], c[8], c[7], c[9]));
            String[] sig = sigs.get(c[3]);
            if (sig != null && c[2].equals("APP.dll") && c[8].equals("high")) {
                try {
                    String proto = sig[3].replaceFirst("\\b" + java.util.regex.Pattern.quote(sig[1]) + "\\s*\\(",
                            f.getName() + "(");
                    FunctionDefinitionDataType def = parser.parse(f.getSignature(), proto);
                    def.setCallingConvention(sig[2]);
                    new ApplyFunctionSignatureCmd(a, def, SourceType.IMPORTED).applyTo(currentProgram);
                    typed++;
                } catch (Exception e) {
                    // a JP type the EN program does not have: keep the EN signature
                }
            }
        }
        println("named " + named + ", kept existing names " + kept + ", signatures copied " + typed);
    }
}
