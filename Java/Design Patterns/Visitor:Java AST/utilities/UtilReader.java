package ecs160.visitor.utilities;

import java.io.*;
import java.nio.charset.StandardCharsets;

/**
 * A simple utility file to read in the Java source files.
 *
 * @author caseycas
 */
public class UtilReader {
    public static String read(File file) throws IOException {
        try (BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(file), StandardCharsets.UTF_8))) {
            StringBuilder content = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) {
                if (content.length() > 0) content.append('\n');
                content.append(line);
            }
            return content.toString();
        }
    }
}
