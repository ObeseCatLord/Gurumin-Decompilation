// Writes every non-default function signature of the current (JP) program to a TSV:
//   address<TAB>name<TAB>calling convention<TAB>prototype
// Run read-only on the JP project (APP.dll); ApplyJpNames.java reads the file on the EN project.
// @category Gurumin
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.SourceType;
import java.io.*;
import java.nio.charset.StandardCharsets;

public class ExportJpSignatures extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        File out = new File(args.length > 0 ? args[0] : "jp_signatures.tsv");
        int n = 0;
        try (PrintWriter w = new PrintWriter(new OutputStreamWriter(new FileOutputStream(out), StandardCharsets.UTF_8))) {
            w.println("address\tname\tconvention\tprototype");
            FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
            while (it.hasNext()) {
                Function f = it.next();
                if (f.getSignatureSource() == SourceType.DEFAULT) continue;
                w.println(String.format("%08x\t%s\t%s\t%s", f.getEntryPoint().getOffset(), f.getName(),
                        f.getCallingConventionName(), f.getPrototypeString(false, false)));
                n++;
            }
        }
        println("exported " + n + " signatures to " + out);
    }
}
