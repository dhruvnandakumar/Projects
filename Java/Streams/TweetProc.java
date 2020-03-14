package edu.ucdavis.ecs160.hw3;


import java.io.*;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.function.ToIntFunction;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class TweetProc {

    private static Function<String, String> getTweeter =
            (line) -> {
                if (line == null || line.trim().equals(""))
                    return ("Empty Tweet");
                String[] items = line.split(",");
                return (items[8]);
            };
    private static Function<String, String> getTaggeeSplit =
            (line) -> {
                if (line == null || line.trim().equals(""))
                    return null;
                String[] items = line.split(",");

                String lineS = items[11];

                String[] splits = lineS.split(" ");

                Stream<String> parts = Arrays.stream(splits);

                String part = parts.filter(text -> text.startsWith("@")).distinct().collect(Collectors.joining(" "));

                return part;
            };
    private static Function<String, String> getTaggees =
            (line) -> {

                String[] splits = line.split(" ");

                Stream<String> parts = Arrays.stream(splits);

                String part = parts.filter(text -> text.startsWith("@")).distinct().collect(Collectors.joining(" "));

                return part;

            };
    private static Function<String, String[]> getURLSplit =
            (line) -> {
                if (line == null || line.trim().equals(""))
                    return new String[]{"N/A", "N/A"};
                String[] items = line.split(",");

                String lineS = items[11];

                String[] splits = lineS.split("\\p{javaWhitespace}+");

                Stream<String> parts = Arrays.stream(splits);

                List<String> list = parts.filter(text -> text.startsWith("http://")).collect(Collectors.toList());

                if (!list.isEmpty()) {
                    if (list.get(0).equals(""))
                        return new String[]{items[8], "Empty"};
                    else
                        return new String[]{items[8], list.get(0)};
                } else
                    return new String[]{items[8], "Empty"};
            };
    private static Predicate<String[]> isURL =
            (group) -> group[1].startsWith("h");
    private static Function<String, String> getAtSubstring =
            (line) -> line.substring(1);
    private static Function<String[], String> getWriter =
            (group) -> group[0];
    private static Function<String[], String> getContent =
            (group) -> group[1];
    private static Function<String, String[]> getTweetTweeterSet =
            (line) -> {
                if (line == null || line.trim().equals(""))
                    return new String[]{"N/A", "N/A"};
                String[] items = line.split(",");


                return new String[]{items[8], items[11]};
            };
    private static ToIntFunction<String[]> getWordCount =
            (group) -> {
                String[] splits = group[1].split("\\p{javaWhitespace}+");
                return splits.length;

            };
    private static Function<Map.Entry<String, String>, String> getKeyVal =
            (obj) -> obj.getKey();
    private static Function<Map.Entry<String, String>, Map<String, Long>> getMap =
            (obj) -> {
                String[] splits = obj.getValue().split("DELIMITER");

                List<String> taggees = Arrays.stream(splits).map(getTaggees).collect(Collectors.toList());

                String taggeesJoint = taggees.stream().collect(Collectors.joining(" "));

                String[] splits2 = taggeesJoint.split("\\p{javaWhitespace}+");

                Map<String, Long> counts = Arrays.stream(splits2)./*map(getAtSubstring).*/collect(Collectors.toConcurrentMap(taggee -> taggee, taggee -> Long.valueOf(1), Long::sum));

                return counts;

            };


    /////////////////////////////////////////////////////////////////////

    public static Map<String, Long> getPerTweeterCount(String FullPathname) {
        File inputF;
        InputStream inputFS;
        BufferedReader br;


        try {
            inputF = new File(FullPathname);

            inputFS = new FileInputStream(inputF);
            br = new BufferedReader(new InputStreamReader(inputFS));
        } catch (Exception e) {
            System.out.println(e);
            System.out.println("An error occured. Returning NULL");
            return null;
        }

        Map<String, Long> counts = br.lines().skip(1).map(getTweeter)
                .collect(Collectors.toConcurrentMap(tweeter -> tweeter, tweeter -> Long.valueOf(1), Long::sum));

        return counts;

    }

    public static Map<String, Long> getPerTaggeeCount(String FullPathname) {
        File inputF;
        InputStream inputFS;
        BufferedReader br;


        try {
            inputF = new File(FullPathname);

            inputFS = new FileInputStream(inputF);
            br = new BufferedReader(new InputStreamReader(inputFS));
        } catch (Exception e) {
            System.out.println(e);
            System.out.println("An error occured. Returning NULL");
            return null;
        }

        List<String> taggees = br.lines().skip(1).map(getTaggeeSplit).collect(Collectors.toList());

        String taggeesJoint = taggees.stream().collect(Collectors.joining(" "));

        String[] splits = taggeesJoint.split("\\p{javaWhitespace}+");

        Map<String, Long> counts = Arrays.stream(splits)/*.map(getAtSubstring)*/.collect(Collectors.toConcurrentMap(taggee -> taggee, taggee -> Long.valueOf(1), Long::sum));

        return counts;

    }

    public static Map<String, Long> getTweeterURLTweetCount(String FullPathname) {
        File inputF;
        InputStream inputFS;
        BufferedReader br;


        try {
            inputF = new File(FullPathname);

            inputFS = new FileInputStream(inputF);
            br = new BufferedReader(new InputStreamReader(inputFS));
        } catch (Exception e) {
            System.out.println(e);
            System.out.println("An error occured. Returning NULL");
            return null;
        }

        Map<String, Long> counts = br.lines().skip(1).map(getURLSplit).filter(isURL)
                .collect(Collectors.toConcurrentMap(getWriter, val -> Long.valueOf(1), Long::sum));

        return counts;
    }

    public static Map<String, Long> getTweeterWordCount(String FullPathname, String word) {
        File inputF;
        InputStream inputFS;
        BufferedReader br;


        try {
            inputF = new File(FullPathname);

            inputFS = new FileInputStream(inputF);
            br = new BufferedReader(new InputStreamReader(inputFS));
        } catch (Exception e) {
            System.out.println(e);
            System.out.println("An error occured. Returning NULL");
            return null;
        }

        String bothWord = " " + word + " ";
        String afterWord = word + " ";
        String beforeWord = " " + word;

        Map<String, Long> counts = br.lines().skip(1).map(getTweetTweeterSet).filter((set) -> set[1].startsWith(afterWord) || set[1].endsWith(beforeWord)
                || set[1].contains(bothWord)).collect(Collectors.toConcurrentMap(getWriter, val -> Long.valueOf(1), Long::sum));

        return counts;
    }

    public static Map<String, Double> getTweeterAverageVerbosity(String FullPathname) {
        File inputF;
        InputStream inputFS;
        BufferedReader br;


        try {
            inputF = new File(FullPathname);

            inputFS = new FileInputStream(inputF);
            br = new BufferedReader(new InputStreamReader(inputFS));
        } catch (Exception e) {
            System.out.println(e);
            System.out.println("An error occurred. Returning NULL");
            return null;
        }

        Map<String, Double> counts = br.lines().skip(1).map(getTweetTweeterSet).collect(Collectors.groupingBy(getWriter, Collectors.averagingInt(getWordCount)));

        return counts;
    }

    public static Map<String, Map<String, Long>> getTweeterTaggeeCount(String FullPathname) {
        File inputF;
        InputStream inputFS;
        BufferedReader br;


        try {
            inputF = new File(FullPathname);

            inputFS = new FileInputStream(inputF);
            br = new BufferedReader(new InputStreamReader(inputFS));
        } catch (Exception e) {
            System.out.println(e);
            System.out.println("An error occurred. Returning NULL");
            return null;
        }

        Map<String, String> perTweeter = br.lines().skip(1).map(getTweetTweeterSet).collect(Collectors.toConcurrentMap(getWriter, getContent, (s1, s2) -> s1 + "DELIMITER" + s2));

        Map<String, Map<String, Long>> counts = perTweeter.entrySet().stream().collect(Collectors.toConcurrentMap(getKeyVal, getMap));

        return counts;
    }
}
