// Applies names (and evidence as plate comments) from ../../symbols.tsv to the program.
// @category Gurumin
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SourceType;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class ApplySymbols extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        Path tsv = Paths.get(args.length > 0 ? args[0] : "symbols.tsv");
        int n = 0;
        for (String line : Files.readAllLines(tsv, StandardCharsets.UTF_8)) {
            String[] c = line.replace("\uFEFF", "").split("\t");
            if (c.length < 2 || c[0].equals("address")) continue;
            Address a = toAddr(Long.parseLong(c[0], 16));
            Function f = getFunctionAt(a);
            if (f == null) f = createFunction(a, null);
            if (f == null) { println("no function at " + c[0]); continue; }
            f.setName(c[1], SourceType.USER_DEFINED);
            if (c.length > 2) f.setComment(c[2]);
            n++;
        }
        println("applied " + n + " names");
    }
}

