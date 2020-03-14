package edu.ucdavis.ecs160;

import edu.ucdavis.ecs160.hw3.TweetProc;


public class Main {

    public static void main(String[] args) {

        System.out.println("*******************************");
        System.out.println(TweetProc.getPerTweeterCount("./cl-tweets-short-clean.csv"));
        System.out.println("*******************************");
        System.out.println(TweetProc.getPerTaggeeCount("./cl-tweets-short-clean.csv"));
        System.out.println("*******************************");
        System.out.println(TweetProc.getTweeterURLTweetCount("./cl-tweets-short-clean.csv"));
        System.out.println("*******************************");
        System.out.println(TweetProc.getTweeterWordCount("./cl-tweets-short-clean.csv", "flight"));
        System.out.println("*******************************");
        System.out.println(TweetProc.getTweeterAverageVerbosity("./cl-tweets-short-clean.csv"));
        System.out.println("*******************************");
        System.out.println(TweetProc.getTweeterTaggeeCount("./cl-tweets-short-clean.csv"));
    }
}
